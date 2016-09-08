//Created by KVClassFactory on Mon Jul 25 15:33:56 2016
//Author: John Frankland,,,

#include "SQLiteDB.h"
#include <TList.h>
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TSQLTableInfo.h"
#include "TSQLColumnInfo.h"
#include <iostream>
#include "KVError.h"

ClassImp(KVSQLite::database)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSQLite::database, KVSQLite::table, KVSQLite::column</h2>
<h4>Interface to ROOT SQLite database backend</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

ClassImp(KVSQLite::table)



namespace KVSQLite {

   // static maps instantiation
   std::map<std::string, KVSQLite::column_type::types> table::type_map;
   std::map<KVSQLite::column_type::types, std::string> column::inv_type_map;

   unique_ptr<TSQLResult> database::SelectRowsFromTable(const char* table, const char* columns, const char* condition) const
   {
      // return result of query "SELECT [columns] FROM [table] WHERE [condition]"

      TString query;
      query.Form("SELECT %s FROM '%s'", columns, table);
      if (condition) query += Form(" WHERE %s", condition);
      return unique_ptr<TSQLResult>(fDBserv->Query(query));
   }

   void database::read_table_infos()
   {
      // initialise map of database tables from existing database
      unique_ptr<TList> tl(fDBserv->GetTablesList());
      TObject* o;
      TIter it_tab(tl.get());
      while ((o = it_tab())) {

         table t(o->GetName());

         unique_ptr<TSQLTableInfo> ti(fDBserv->GetTableInfo(o->GetName()));

         TIter it_col(ti->GetColumns());
         TSQLColumnInfo* colin;
         while ((colin = (TSQLColumnInfo*)it_col())) t.add_column(colin->GetName(), colin->GetTypeName());

         fTables.insert(std::pair<std::string, KVSQLite::table>(o->GetName(), t));
      }

   }

   void database::show_tables() const
   {
      // print list of tables
      std::cout << "Tables in database:" << std::endl;
#ifdef WITH_CPP11
      for (auto it = fTables.begin();
#else
      for (std::map<std::string, KVSQLite::table>::const_iterator it = fTables.begin();
#endif
            it != fTables.end(); ++it) {
         std::cout << "\t" << it->first << std::endl;
      }
   }

   void database::open(const TString& dbfile)
   {
      // Open/create sqlite db file given path
      TString uri = "sqlite://" + dbfile;
      fDBserv.reset(static_cast<TSQLiteServer*>(TSQLServer::Connect(uri, 0, 0)));
      // check for valid database file
      if (!fDBserv->Exec("pragma schema_version")) {
         fDBserv->Close();
         fIsValid = false;
         return;
      }
      fIsValid = true;
      read_table_infos();
   }

   void database::PrintResults(TSQLResult* tabent) const
   {
      int nfields = tabent->GetFieldCount();
      unique_ptr<TSQLRow> row(nullptr);
      for (int r = -1; true; ++r) {
         if (r > -1) {
            row.reset(tabent->Next());
            if (row.get() == nullptr) break;
            std::cout << "\t" << r;
         }
         for (int f = 0; f < nfields; ++f) {
            std::cout << "\t";
            if (r < 0) {
               if (f == 0) std::cout << "#" << "\t";
               std::cout << tabent->GetFieldName(f) << "\t\t\t";
            } else {
               std::cout << row->GetField(f) << "\t\t";
            }
         }
         std::cout << "\n";
      }
   }

   void database::Dump() const
   {
      // Print on stdout contents of database

      std::cout << "Database : " << fDBserv->GetDB() << " [" << fDBserv->GetDBMS() << "]\n";
      unique_ptr<TList> tl(fDBserv->GetTablesList());
      TObject* o;
      TIter it_tab(tl.get());
      while ((o = it_tab())) {
         std::cout << "\t";
         std::cout << "Table : " << o->GetName() << "\n";
         unique_ptr<TSQLResult> tabent = SelectRowsFromTable(o->GetName());
         PrintResults(tabent.get());
      }
   }

   void database::add_table(table& t)
   {
      // add table to database (if it does not exist already)

      std::string command("CREATE TABLE IF NOT EXISTS ");
      command += "'";
      command += t.name();
      command += "'";
      command += " (";
      for (int i = 0; i < t.number_of_columns(); ++i) {
         if (i) command += ", ";
         command += t[i].get_declaration();
      }
      command += ")";
      if (fDBserv->Exec(command.c_str()))
         fTables.insert(std::pair<std::string, KVSQLite::table>(t.name(), t));
   }

   bool database::prepare_data_insertion(const char* table)
   {
      // Call this method before insert_dat_row() in order to perform bulk data
      // insertion operation. i.e. something like:
      //
      //   db.prepare_data_insertion("my_table");
      //   while(...){  // loop over data to insert
      //       // set up data in table
      //       db.insert_data_row();
      //   }
      //   db.end_data_insertion();  // terminate data insertion
      //
      //  Until method end_data_insertion() is called, you cannot call prepare_data_insertion()
      //  with a different table name.

      if (fInserting) {
         if (fBulkTable) {
            Error("database::prepare_data_insertion",
                  "bulk insertion in progress for table %s; call database::commit() to terminate transaction",
                  fBulkTable->name().c_str());
            return false;
         } else {
            Error("database::prepare_data_insertion",
                  "bulk insertion in progress; call database::commit() to terminate transaction");
            return false;
         }
      }
      if (fSelecting) {
         Error("database::prepare_data_insertion",
               "data retrieval in progress; call get_next_result() until it returns false");
         return false;
      }
      fInserting = true;
      fDBserv->StartTransaction();
      // set up SQL statement for data insertion into table
      fBulkTable = &fTables[table];
      std::string com(fBulkTable->get_insert_command());
      int ncol = fBulkTable->number_of_columns();
      int idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (idx) com += ",";
         if (!(*fBulkTable)[i].primary_key()) {
            com += (*fBulkTable)[i].name();
            ++idx;
         }
      }
      com += ") VALUES (";
      idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (idx) com += ",";
         if (!(*fBulkTable)[i].primary_key()) {
            com += "?";
            ++idx;
         }
      }
      com += ")";
      fSQLstmt.reset(fDBserv->Statement(com.c_str()));
      return true;
   }

   const char* table::get_insert_command() const
   {
      switch (fInsert) {
         case KVSQLite::insert_mode::FAIL:
            return Form("INSERT OR FAIL INTO '%s'(", name().c_str());
            break;
         case KVSQLite::insert_mode::IGNORE:
            return Form("INSERT OR IGNORE INTO '%s'(", name().c_str());
            break;
         case KVSQLite::insert_mode::REPLACE:
            return Form("INSERT OR REPLACE INTO '%s'(", name().c_str());
            break;
         default:
         case KVSQLite::insert_mode::DEFAULT:
            break;
      }
      return Form("INSERT INTO '%s'(", name().c_str());
   }

   const char* column::get_declaration() const
   {
      // return declaration for column, including type & constraint

      static TString decl;
      decl.Form("%s %s", name().c_str(), type_name().c_str());
      if (fConstraint.BeginsWith("FOREIGN KEY")) decl += ",";
      decl += " ";
      decl += fConstraint;
      return decl.Data();
   }

   void database::insert_data_row()
   {
      // Call (repeatedly) after a call to prepare_data_insertion(table_name)
      // in order to insert current contents of table columns as a new row in the database.
      // Value of each column should first be set like this:
      //
      //    db["table"]["id"].set_data(6);
      //    db["table"]["name"].set_data("triumph");
      //
      // Call end_data_insertion() when all data has been inserted

      if (!fInserting || !fBulkTable) {
         Error("database::insert_data_row",
               "no transaction initialized; call prepare_data_insertion(name_table) before this method");
         return;
      }
      if (fSelecting) {
         Error("database::insert_data_row",
               "data retrieval in progress; call get_next_result() until it returns false, then call prepare_data_insertion(name_table) before this method");
         return;
      }
      int ncol = fBulkTable->number_of_columns();
      fSQLstmt->NextIteration();
      int idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (!(*fBulkTable)[i].primary_key())(*fBulkTable)[i].set_data_in_statement(fSQLstmt.get(), idx++);
      }
   }

   void database::end_data_insertion()
   {
      // Call after prepare_data_insertion(const char*) & insert_data_row() have been
      // used to insert data into a table

      if (!fInserting) {
         Error("database::end_data_insertion",
               "no transaction initialized; call prepare_data_insertion(name_table) first");
         return;
      }
      if (fSelecting) {
         Error("database::insert_data_row",
               "data retrieval in progress; call get_next_result() until it returns false, then call prepare_data_insertion(name_table) before this method");
         return;
      }
      fBulkTable = nullptr;
      fSQLstmt->Process();
      fDBserv->Commit();
      fInserting = false;
   }

   bool database::select_data(const char* table, const char* selection, const char* anything_else)
   {
      // Select data in database from given table according to
      //    SELECT * FROM [table]
      // or
      //    SELECT * FROM [table] WHERE [selection]
      // The 'anything else' will just be tacked on verbatim, like so:
      //    SELECT * FROM [table] [anything else]
      // or SELECT * FROM [table] WHERE [selection] [anything else]
      // In order to retrieve results, call get_next_result()

      if (fInserting) {
         Error("database::select_data",
               "data insertion in progress; call end_data_insertion() before retrieving data");
         return false;
      }
      if (fSelecting) {
         Error("database::select_data",
               "data retrieval already in progress; call get_next_result() until it returns false before making new selection");
         return false;
      }
      // set up SQL statement for data retrieval
      fBulkTable = &fTables[table];
      TString cond = Form("SELECT * FROM '%s'", table);
      if (strcmp(selection, "")) cond += Form(" WHERE %s", selection);
      if (strcmp(anything_else, "")) cond += Form(" %s", anything_else);
      fSQLstmt.reset(fDBserv->Statement(cond));
      if (fSQLstmt.get() == nullptr) {
         Error("database::select_data", "problem processing : %s", cond.Data());
         fSelecting = false;
         fBulkTable = nullptr;
         return false;
      }
      fSQLstmt->EnableErrorOutput();
      if (fSQLstmt->Process()) {
         fSQLstmt->StoreResult();
         fSelecting = true;
         fEmptyResultSet = false;
         return true;
      } else if (!fSQLstmt->IsError()) {
         // query ok, no results correspond to selection
         fSQLstmt->StoreResult();
         fSelecting = true;
         fEmptyResultSet = true;
         return true;
      }
      fSelecting = false;
      fBulkTable = nullptr;
      return false;
   }

   bool database::get_next_result()
   {
      // Retrieve next result row resulting from previous call to select_data()
      // Returns kFALSE when no more data is retrieved

      if (fInserting) {
         Error("database::get_next_result",
               "data insertion in progress; call end_data_insertion() then select_data() before this method");
         return false;
      }
      if (!fSelecting) {
         Error("database::get_next_result",
               "no data retrieval in progress; select_data() must be called and return true before calling this method");
         return false;
      }
      if (!fEmptyResultSet && fSQLstmt->NextResultRow()) {
         // set column data
         for (int i = 0; i < fBulkTable->number_of_columns(); ++i) {
            (*fBulkTable)[i].set_data_from_statement(fSQLstmt.get());
         }
         return kTRUE;
      }
      fBulkTable = nullptr;
      fSelecting = false;
      return kFALSE;
   }

   void database::clear_table(const std::string& name)
   {
      // Delete all data from table
      delete_row(name.c_str());
   }

   int database::count(const char* table, const char* column, const char* selection, bool distinct)
   {
      // Returns number of rows in table for which selection holds true
      //  if column="*" all rows are included
      //  if a column name is given, only rows with a non-NULL value for column are counted
      //  if distinct=false, count all rows including those with the same value of column
      //  if distinct=true, count the number of different values of column

      TString qry = "SELECT count(";
      if (distinct) qry += "DISTINCT ";
      qry += column;
      qry += ") FROM '";
      qry += table;
      qry += "'";
      if (strcmp(selection, "")) {
         qry += " WHERE ";
         qry += selection;
      }

      unique_ptr<TSQLResult> result(fDBserv->Query(qry));
      unique_ptr<TSQLRow> row(result->Next());
      TString number = row->GetField(0);
      return number.Atoi();
   }

   bool database::update(const char* table, const char* selection, const TString& columns)
   {
      // update the given columns of an entry in the table corresponding to selection
      // the current values of the data members of the columns will be used
      //
      // This is equivalent to
      //
      //    UPDATE [table] SET [col1=newval,col2=newval,...] WHERE [selection]

      if (fInserting) {
         Error("database::update",
               "data insertion in progress; call end_data_insertion() before doing anything else");
         return false;
      }
      if (fSelecting) {
         Error("database::update",
               "data retrieval in progress; call get_next_result() until it returns false before doing anything else");
         return false;
      }

      fBulkTable = &fTables[table];
      TString query = Form("UPDATE %s SET ", table);
      int ncol = fBulkTable->number_of_columns();
      int idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (columns.Contains((*fBulkTable)[i].name())) {
            if (idx) query += ",";
            query += (*fBulkTable)[i].name();
            query += "=?";
            ++idx;
         }
      }
      query += Form(" WHERE %s", selection);
      fSQLstmt.reset(fDBserv->Statement(query));
      fSQLstmt->NextIteration();
      idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (columns.Contains((*fBulkTable)[i].name())) {
            (*fBulkTable)[i].set_data_in_statement(fSQLstmt.get(), idx);
            ++idx;
         }
      }
      return (fSQLstmt->Process());
   }

   void database::delete_row(const char* table, const char* selection)
   {
      // delete row from the table corresponding to selection
      //
      // This is equivalent to
      //
      //    DELETE FROM [table] WHERE [selection]
      //
      // With no selection, deletes all rows of table (clear_table())

      TString query = Form("DELETE FROM %s", table);
      if (strcmp(selection, "")) query += Form(" WHERE %s", selection);
      fDBserv->Exec(query);
   }

   void column::init_type_map()
   {
      inv_type_map[KVSQLite::column_type::REAL] = "REAL";
      inv_type_map[KVSQLite::column_type::INTEGER] = "INTEGER";
      inv_type_map[KVSQLite::column_type::TEXT] = "TEXT";
      inv_type_map[KVSQLite::column_type::BLOB] = "BLOB";
   }

   std::string column::_type()
   {
      return inv_type_map[fNameType.second];
   }

   void column::set_data_in_statement(TSQLStatement* s, int idx) const
   {
      // set value of parameter in SQLite statement corresponding to this column
      // if idx is given, use it as the statement parameter index instead of
      // the column's index in the table (case where not all columns are treated
      // in the statement)

      if (idx < 0) idx = index();
      switch (type()) {
         case KVSQLite::column_type::REAL:
            s->SetDouble(idx, fData.GetDouble());
            break;
         case KVSQLite::column_type::INTEGER:
            s->SetInt(idx, fData.GetInt());
            break;
         case KVSQLite::column_type::TEXT:
            s->SetString(idx, fData.GetString());
            break;
         case KVSQLite::column_type::BLOB:
         default:
            // nothing to do for BLOB types (yet)
            break;
      }
   }
   void column::set_data_from_statement(TSQLStatement* s)
   {
      // set value of column according to value of parameter in statement
      switch (type()) {
         case KVSQLite::column_type::REAL:
            fData.Set(s->GetDouble(index()));
            break;
         case KVSQLite::column_type::INTEGER:
            fData.Set(s->GetInt(index()));
            break;
         case KVSQLite::column_type::TEXT:
            fData.Set(s->IsNull(index()) ? "" : s->GetString(index()));
            break;
         case KVSQLite::column_type::BLOB:
         default:
            // nothing to do for BLOB types (yet)
            break;
      }
   }

   void table::init_type_map()
   {
      type_map["REAL"] = KVSQLite::column_type::REAL;
      type_map["INTEGER"] = KVSQLite::column_type::INTEGER;
      type_map["TEXT"] = KVSQLite::column_type::TEXT;
      type_map["BLOB"] = KVSQLite::column_type::BLOB;
   }

   void table::show_columns() const
   {
      // print list of columns
      std::cout << "Columns in table:" << std::endl;
#ifdef WITH_CPP11
      for (auto it = fColumns.begin(); it != fColumns.end(); ++it) {
#else
      for (std::vector<KVSQLite::column>::const_iterator it = fColumns.begin(); it != fColumns.end(); ++it) {
#endif
         std::cout << "\t" << it->name() << " [" << it->type_name() << "]" << std::endl;
      }
   }

   column& table::add_column(const column& c)
   {
      // add column to table. return reference to added column.
      fColumns.push_back(c);
      fColMap[c.name()] = c.index();
      return fColumns.back();
   }

   column& table::add_column(const std::string& name, const std::string& type)
   {
      // add column to table. return reference to added column.
      return add_column(name, type_map[type]);
   }

   void table::add_primary_key(const std::string& name)
   {
      // add a PRIMARY KEY column to the table
      // by default this is an INTEGER type column
      // as it is auto-incremented with each inserted row, it should not
      // be included in TSQLStatement used to write data to db
      add_column(name, KVSQLite::column_type::INTEGER).set_constraint("PRIMARY KEY");
   }


   //____________________________________________________________________________//

}
