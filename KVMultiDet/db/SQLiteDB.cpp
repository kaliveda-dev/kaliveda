//Created by KVClassFactory on Mon Jul 25 15:33:56 2016
//Author: John Frankland,,,

#include "SQLiteDB.h"
#include <TList.h>
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TSQLTableInfo.h"
#include "TSQLColumnInfo.h"
#include <KVString.h>
#include <iostream>
#include "KVError.h"
#include <iomanip>

ClassImp(KVSQLite::database)
ClassImp(KVSQLite::table)
ClassImp(KVSQLite::column)


namespace KVSQLite {

   // static maps instantiation
   std::map<TString, KVSQLite::column_type::types> table::type_map;
   std::map<KVSQLite::column_type::types, TString> column::inv_type_map;

   unique_ptr<TSQLResult> database::SelectRowsFromTable(const TString& table, const TString& columns, const TString& condition) const
   {
      // \param[in] table name of table
      // \param[in] columns comma-separated list of columns
      // \param[in] condition selection to be applied, if any
      // \returns result of query `SELECT [columns] FROM [table] WHERE [condition]`

      TString query;
      query.Form("SELECT %s FROM '%s'", columns.Data(), table.Data());
      if (condition != "") query += Form(" WHERE %s", condition.Data());
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
      // \param[in] dbfile full path to file
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

   void database::PrintResults(TSQLResult* tabent, int column_width) const
   {
      int nfields = tabent->GetFieldCount();
      unique_ptr<TSQLRow> row(nullptr);
      for (int r = -1; true; ++r) {
         if (r > -1) {
            row.reset(tabent->Next());
            if (row.get() == nullptr) break;
            std::cout << std::setw(6) << r;
         }
         for (int f = 0; f < nfields; ++f) {
            if (r < 0) {
               if (f == 0) std::cout << std::setw(6) << "#";
               std::cout << "|" << std::setw(column_width) << tabent->GetFieldName(f) ;
            }
            else {
               std::cout << "|" << std::setw(column_width) << row->GetField(f) ;
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
      std::cout << std::endl;
   }
   void database::print_selection(const TString& table, const TString& columns, const TString& condition, int column_width) const
   {
      // Print on stdout contents of database

      unique_ptr<TSQLResult> tabent = SelectRowsFromTable(table, columns, condition);
      PrintResults(tabent.get(), column_width);
      std::cout << std::endl;
   }

   void database::add_table(table& t)
   {
      // add table to database (if it does not exist already)
      //
      // _WARNING:_ after calling this method, do not use the table given as argument
      //
      //     it does not correspond to the table in the database
      //
      //     instead use `db["table name"]` to access the table
      //
      //~~~~{.cpp}
      //   e.g. KVSQLite::table tt("some table");
      //        tt.add_column(...);
      //        db.add_table(tt);
      //        db["some table"]["column"].set_data(...);
      //~~~~

      TString command("CREATE ");
      if (t.is_temporary()) command += "TEMPORARY ";
      command += "TABLE IF NOT EXISTS \"";
      command += t.name();
      command += "\"";
      command += " (";
      for (int i = 0; i < t.number_of_columns(); ++i) {
         if (i) command += ", ";
         command += t[i].get_declaration();
      }
      command += ")";
      //std::cout << command << std::endl;
      if (fDBserv->Exec(command))
         fTables.insert(std::pair<std::string, KVSQLite::table>(t.name(), t));
   }

   bool database::prepare_data_insertion(const TString& table)
   {
      // Call this method before insert_dat_row() in order to perform bulk data
      // insertion operation. i.e. something like:
      //
      //~~~~{.cpp}
      //   db.prepare_data_insertion("my_table");
      //   while(...){  // loop over data to insert
      //       // set up data in table
      //       db.insert_data_row();
      //   }
      //   db.end_data_insertion();  // terminate data insertion
      //~~~~
      // Until method end_data_insertion() is called, you cannot call prepare_data_insertion()
      // with a different table name.

      if (fInserting) {
         if (fBulkTable) {
            Error("database::prepare_data_insertion",
                  "bulk insertion in progress for table %s; call database::commit() to terminate transaction",
                  fBulkTable->name());
            return false;
         }
         else {
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
      fBulkTable = &fTables[table.Data()];
      TString com(fBulkTable->get_insert_command());
      int ncol = fBulkTable->number_of_columns();
      int idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (idx) com += ",";
         if (!(*fBulkTable)[i].primary_key()) {
            com += Form("\"%s\"", (*fBulkTable)[i].name());;
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
      //std::cout << com << std::endl;
      fSQLstmt.reset(fDBserv->Statement(com));
      return true;
   }

   const char* table::get_insert_command() const
   {
      switch (fInsert) {
         case KVSQLite::insert_mode::FAIL:
            return Form("INSERT OR FAIL INTO \"%s\"(", name());
            break;
         case KVSQLite::insert_mode::IGNORE:
            return Form("INSERT OR IGNORE INTO \"%s\"(", name());
            break;
         case KVSQLite::insert_mode::REPLACE:
            return Form("INSERT OR REPLACE INTO \"%s\"(", name());
            break;
         default:
         case KVSQLite::insert_mode::DEFAULT:
            break;
      }
      return Form("INSERT INTO \"%s\"(", name());
   }

   const char* column::get_declaration() const
   {
      // return declaration for column, including type & constraint

      static TString decl;
      decl.Form("\"%s\" %s", name(), type_name());
      if (fForeignKey) {
         decl += " REFERENCES ";
         decl += Form("\"%s\"(\"%s\")", fFKtable.Data(), fFKcolumn.Data());
      }
      else {
         decl += " ";
         decl += fConstraint;
      }
      return decl.Data();
   }

   void database::insert_data_row()
   {
      // Call (repeatedly) after a call to prepare_data_insertion()
      // in order to insert current contents of table columns as a new row in the database.
      //
      // Value of each column should first be set like this:
      //
      //~~~~{.cpp}
      //    db["table"]["id"].set_data(6);
      //    db["table"]["name"].set_data("triumph");
      //~~~~
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
         if (!(*fBulkTable)[i].primary_key()) {
            (*fBulkTable)[i].set_data_in_statement(fSQLstmt.get(), idx);
            ++idx;
         }
      }
   }

   void database::end_data_insertion()
   {
      // Call after prepare_data_insertion() & insert_data_row() have been
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

   bool database::select_data(const TString& table, const TString& columns, const TString& selection, bool distinct, const TString& anything_else)
   {
      // Select data in database from given table according to
      //~~~~
      //    SELECT [columns] FROM [table] WHERE [selection] [anything_else]
      //~~~~
      // In order to retrieve results, call get_next_result() until it returns false.
      //
      // \param columns ="*" by default, i.e. data from all columns is retrieved.
      //            If specific column data is to be selected, give a comma-separated list of
      //            column names. These will be quoted correctly in case they contain spaces.
      // \param distinct can be used in conjunction with a selection of specific columns in order
      //             to retrieve only rows of data with different values for the column(s).
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
      fBulkTable = &fTables[table.Data()];

      KVString column_selection(""), _columns(columns);
      if (columns == "*") {
         column_selection = "*";
         distinct = false; // don't allow 'SELECT DISTINCT * FROM ....' (?)
         fSelectedColumns = "*";
      }
      else {
         if (distinct) column_selection = "DISTINCT ";
         fSelectedColumns = "";
         // put quoted column names in column_selection, add plain column names to fSelectedColumns
         _columns.Begin(",");
         int i(0);
         while (!_columns.End()) {
            KVString colnam = _columns.Next();
            if (i) {
               column_selection += ", ";
               fSelectedColumns += ", ";
            }
            column_selection += Form("\"%s\"", colnam.Data());
            fSelectedColumns += colnam.Data();
            ++i;
         }
      }

      TString cond = Form("SELECT %s FROM \"%s\"", column_selection.Data(), table.Data());
      if (selection != "") cond += Form(" WHERE %s", selection.Data());
      if (anything_else != "") cond += Form(" %s", anything_else.Data());
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
      }
      else if (!fSQLstmt->IsError()) {
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
      // \returns kFALSE when no more data is retrieved

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
         if (fSelectedColumns == "*") {
            for (int i = 0; i < fBulkTable->number_of_columns(); ++i) {
               (*fBulkTable)[i].set_data_from_statement(fSQLstmt.get());
            }
         }
         else {
            // only read data for selected columns
            int idx = 0;
            for (int i = 0; i < fBulkTable->number_of_columns(); ++i) {
               if (fSelectedColumns.Contains((*fBulkTable)[i].name())) {
                  (*fBulkTable)[i].set_data_from_statement(fSQLstmt.get(), idx);
                  ++idx;
               }
            }
         }
         return kTRUE;
      }
      fBulkTable = nullptr;
      fSelecting = false;
      return kFALSE;
   }

   KVNumberList database::get_integer_list(const TString& table, const TString& column, const TString& selection, const TString& anything_else)
   {
      // \note Only for INTEGER columns!
      // Fill KVNumberList with all `DISTINCT` values of "column" (only 1 column name at a time) for given selection

      KVNumberList result;
      if (select_data(table, column, selection, true, anything_else)) {
         while (get_next_result()) {
            result.Add((*this)[table][column].get_data<int>());
         }
      }
      return result;
   }

   TString database::get_string_list(const TString& table, const TString& column, const TString& selection, const TString& anything_else)
   {
      // \note Only for TEXT columns!
      // Fill TString with comma-separated list of values of "column" (only 1 column name at a time) for given selection
      // \note Any NULL entries will be ignored

      TString result;
      if (select_data(table, column, selection, false, anything_else)) {
         while (get_next_result()) {
            if ((*this)[table][column].is_null()) continue;
            if (result != "") result += ",";
            result += (*this)[table][column].get_data<TString>();
         }
      }
      return result;
   }

   KVNameValueList database::get_name_value_list(const TString& tablename, const TString& name_column, const TString& value_column, const TString& selection, const TString& anything_else)
   {
      // Fill KVNameValueList with selected rows from table, adding for each row a parameter with the
      // name contained in "name_column" (must be of type `TEXT`) and the value contained in "value_column"
      // (can be `INTEGER`, `REAL`, or `TEXT`)

      KVNameValueList result;
      if (select_data(tablename, Form("%s,%s", name_column.Data(), value_column.Data()), selection, false, anything_else)) {
         table& tb = (*this)[tablename];
         column& nom = tb[name_column];
         column& val = tb[value_column];
         while (get_next_result()) {
            result.SetValue(nom.get_data<TString>(), val.data());
         }
      }
      return result;
   }

   TGraph* database::create_graph(const TString& tablename, const TString& Xcolumn, const TString& Ycolumn, const TString& selection)
   {
      // Create and fill a TGraph from values Xcolumn and Ycolumn in table,
      // using the selection if required

      if (select_data(tablename, Form("%s,%s", Xcolumn.Data(), Ycolumn.Data()), selection)) {
         TGraph* g = new TGraph;
         int i = 0;
         table& tb = (*this)[tablename];
         column& Xcol = tb[Xcolumn];
         column& Ycol = tb[Ycolumn];
         while (get_next_result()) {
            g->SetPoint(i++, Xcol.get_data<double>(), Ycol.get_data<double>());
         }
         return g;
      }
      return nullptr;
   }

   void database::clear_table(const TString& name)
   {
      // Delete all data from table
      delete_data(name);
   }

   int database::count(const TString& table, const TString& column, const TString& selection, bool distinct)
   {
      // Returns number of rows in table for which selection holds true:
      //
      //  + if column="*" all rows are included
      //  + if a column name is given, only rows with a non-NULL value for column are counted
      //  + if distinct=false, count all rows including those with the same value of column
      //  + if distinct=true, count the number of different values of column

      TString qry = "SELECT count(";
      if (distinct) qry += "DISTINCT ";
      qry += Form("\"%s\"", column.Data());
      qry += ") FROM '";
      qry += table;
      qry += "'";
      if (selection != "") {
         qry += " WHERE ";
         qry += selection;
      }

      unique_ptr<TSQLResult> result(fDBserv->Query(qry));
      unique_ptr<TSQLRow> row(result->Next());
      TString number = row->GetField(0);
      return number.Atoi();
   }

   bool database::update(const TString& table, const TString& columns, const TString& selection)
   {
      // update the given columns of an entry in the table corresponding to selection (if given)
      // the current values of the data members of the columns will be used
      //
      // This is equivalent to
      //~~~~
      //    UPDATE [table] SET col1=newval,col2=newval,... [WHERE [selection]]
      //~~~~

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

      fBulkTable = &fTables[table.Data()];
      TString query = Form("UPDATE \"%s\" SET ", table.Data());
      int ncol = fBulkTable->number_of_columns();
      int idx = 0;
      for (int i = 0; i < ncol; ++i) {
         if (columns.Contains((*fBulkTable)[i].name())) {
            if (idx) query += ",";
            query += Form("\"%s\"", (*fBulkTable)[i].name());
            query += "=?";
            ++idx;
         }
      }
      if (selection != "") query += Form(" WHERE %s", selection.Data());
      //std::cout << query << std::endl;
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

   void database::delete_data(const TString& table, const TString& selection)
   {
      // delete rows from the table corresponding to selection
      //
      // This is equivalent to
      //~~~~
      //    DELETE FROM [table] WHERE [selection]
      //~~~~
      // With no selection, deletes all rows of table (clear_table())

      TString query = Form("DELETE FROM \"%s\"", table.Data());
      if (selection != "") query += Form(" WHERE %s", selection.Data());
      fDBserv->Exec(query);
   }

   column& database::add_column(const TString& table, const TString& name, const TString& type)
   {
      // add column to existing table
      // \returns reference to new column
      TString query = Form("ALTER TABLE \"%s\" ADD COLUMN \"%s\" %s", table.Data(), name.Data(), type.Data());
      fDBserv->Exec(query);
      return (*this)[table].add_column(name, type);
   }

   void database::add_missing_columns(const TString& _table_, const KVNameValueList& l)
   {
      // add to table any columns which are defined in the list but don't exist
      // \note cannot be called during data insertion or retrieval!!!

      if (fInserting) {
         Error("database::add_missing_columns",
               "data insertion in progress; call end_data_insertion() before doing anything else");
         return;
      }
      if (fSelecting) {
         Error("database::add_missing_columns",
               "data retrieval in progress; call get_next_result() until it returns false before doing anything else");
         return;
      }
      int ipar = l.GetNpar();
      table& tab = (*this)[_table_];
      for (int i = 0; i < ipar; ++i) {
         KVNamedParameter* par = l.GetParameter(i);
         if (!tab.has_column(par->GetName())) add_column(_table_, par->GetName(), par->GetSQLType());
      }
   }

   void database::copy_table_data(const TString& source, const TString& destination, const TString& columns, const TString& selection)
   {
      // Copy all selected data in 'source' table to 'destination'
      //
      // If the columns of the two tables are not identical, specify the columns to copy in 'columns'
      // (comma-separated list)
      // \note SQLite will not allow copy if the number of selected columns from 'source' is not
      // exactly equal to the number of columns in 'destination'

      TString COLUMNS = columns;
      if (COLUMNS != "*") {
         // quote all column names
         COLUMNS = "";
         KVString _columns(columns);
         _columns.Begin(",");
         while (!_columns.End()) {
            if (COLUMNS != "") COLUMNS += ", ";
            COLUMNS += Form("\"%s\"", _columns.Next(kTRUE).Data());
         }
      }
      TString query = Form("INSERT INTO \"%s\" SELECT %s FROM %s", destination.Data(), COLUMNS.Data(), source.Data());
      if (selection != "") query += Form(" WHERE %s", selection.Data());
      fDBserv->Exec(query);
   }

   void column::init_type_map()
   {
      inv_type_map[KVSQLite::column_type::REAL] = "REAL";
      inv_type_map[KVSQLite::column_type::INTEGER] = "INTEGER";
      inv_type_map[KVSQLite::column_type::TEXT] = "TEXT";
      inv_type_map[KVSQLite::column_type::BLOB] = "BLOB";
   }

   const char* column::_type()
   {
      return inv_type_map[fNameType.second];
   }

   template<> void column::set_data(const KVNamedParameter& x)
   {
      fData.Set(x.GetName(), x);
      fIsNull = false;
   }

   void column::set_data_in_statement(TSQLStatement* s, int idx) const
   {
      // set value of parameter in SQLite statement corresponding to this column
      // \param idx if given, use it as the statement parameter index instead of
      //        the column's index in the table (case where not all columns are treated
      //        in the statement)

      if (idx < 0) idx = index();
      if (fIsNull) {
         // null parameter
         s->SetNull(idx);
         return;
      }
      switch (type()) {
         case KVSQLite::column_type::REAL:
            s->SetDouble(idx, fData.GetDouble());
            break;
         case KVSQLite::column_type::INTEGER:
            s->SetInt(idx, fData.GetInt());
            break;
         case KVSQLite::column_type::TEXT:
            s->SetString(idx, fData.GetString(), -1);
            break;
         case KVSQLite::column_type::BLOB:
            s->SetBinary(idx, fBlob, fBlobSize);
            break;
         default:
            break;
      }
   }
   void column::set_data_from_statement(TSQLStatement* s, int idx)
   {
      // set value of column according to value of parameter in statement
      //
      // any column which has a NULL value will be given value 0, 0.0 or ""
      // (for `INTEGER`, `REAL` or `TEXT` type, respectively): use column::is_null()
      // to check if this corresponds to a null column value.
      //
      // \param idx if given, use it as the statement parameter index instead of
      // the column's index in the table (case where not all columns are treated
      // in the statement)

      if (idx < 0) idx = index();
      fIsNull = s->IsNull(idx);
      switch (type()) {
         case KVSQLite::column_type::REAL:
            fData.Set(fIsNull ? 0.0 : s->GetDouble(idx));
            break;
         case KVSQLite::column_type::INTEGER:
            fData.Set(fIsNull ? 0 : s->GetInt(idx));
            break;
         case KVSQLite::column_type::TEXT:
            fData.Set(fIsNull ? "" : s->GetString(idx));
            break;
         case KVSQLite::column_type::BLOB:
            if (fIsNull) {
               fBlobSize = 0;
            }
            else {
               if (!fBlob) fBlob = (void*) new unsigned char[256];
               s->GetBinary(idx, fBlob, fBlobSize);
            }
            break;
         default:
            break;
      }
   }

   void column::set_foreign_key(const TString& _table, const TString& _column)
   {
      // declare this column to be a foreign key i.e. linked to the given
      // _column name in another _table
      fForeignKey = true;
      fFKtable = _table;
      fFKcolumn = _column;
   }

   void column::set_foreign_key(const table& _table, const column& _column)
   {
      // declare this column to be a foreign key i.e. linked to the given
      // _column name in another _table
      fForeignKey = true;
      fFKtable = _table.name();
      fFKcolumn = _column.name();
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
      // add column to table
      // \return reference to added column.
      // \note cannot be used for existing table in database: see database::add_column()
      fColumns.push_back(c);
      fColMap[c.name()] = c.index();
      return fColumns.back();
   }

   column& table::add_column(const TString& name, const TString& type)
   {
      // add column to table
      // \return reference to added column
      // \note cannot be used for existing table in database: see database::add_column()
      return add_column(name, type_map[type]);
   }

   const column& table::add_primary_key(const TString& name)
   {
      // add a PRIMARY KEY column to the table
      // \returns reference to primary key (cannot be modified)
      //
      // \note by default this is an `INTEGER` type column
      //
      // \note as it is auto-incremented with each inserted row, it should not
      // be included in TSQLStatement used to write data to db

      column& c = add_column(name, KVSQLite::column_type::INTEGER);
      c.set_constraint("PRIMARY KEY");
      return c;
   }

   const column& table::add_foreign_key(const TString& name, const TString& other_table, const TString& other_column)
   {
      // add a foreign key to the table, which is an INTEGER reference to
      // another column in another table.
      // \returns reference to key (cannot be modified)

      column& c = add_column(name, KVSQLite::column_type::INTEGER);
      c.set_foreign_key(other_table, other_column);
      return c;
   }

   const column& table::add_foreign_key(const TString& name, const table& other_table, const column& other_column)
   {
      // add a foreign key to the table, which is an INTEGER reference to
      // another column in another table.
      // \returns reference to key (cannot be modified)

      column& c = add_column(name, KVSQLite::column_type::INTEGER);
      c.set_foreign_key(other_table, other_column);
      return c;
   }

   int table::check_columns(const KVNameValueList& l)
   {
      // make sure that all parameters in the list have corresponding columns in the table
      // \returns the number of columns to be added

      int ncols = 0;
      int ipar = l.GetNpar();
      for (int i = 0; i < ipar; ++i) {
         KVNamedParameter* par = l.GetParameter(i);
         if (!has_column(par->GetName())) ncols++;
      }
      return ncols;
   }

   void table::prepare_data(const KVNameValueList& l, const KVNamedParameter* null_value)
   {
      // fill all columns in table with data contained in KVNameValueList parameters having the same name.
      //
      // any columns which do not appear in the KVNameValueList will be set to 'null'
      //
      // if required, any parameters with the same type&value as "null_value" will be set to 'null' too

      for (int i = 0; i < number_of_columns(); ++i) {
         KVNamedParameter* p = l.FindParameter((*this)[i].name());
         if (p && !(null_value && p->HasSameValueAs(*null_value)))
            (*this)[i].set_data(*p);
         else
            (*this)[i].set_null();
      }
   }

   void table::set_all_columns_null()
   {
      // set the value of all columns in the table to NULL
      for (int i = 0; i < number_of_columns(); ++i) {
         (*this)[i].set_null();
      }
   }

   //____________________________________________________________________________//

}

/**
  \example db_sqlite_examples.C
# Examples of use of KVSQLite interface to sqlite databases

To execute this function, do

    $ kaliveda
    kaliveda[0] .L db_sqlite_examples.C+
    kaliveda[1] sqlite_example()

*/
