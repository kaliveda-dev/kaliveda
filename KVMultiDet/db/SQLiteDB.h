//Created by KVClassFactory on Mon Jul 25 15:33:56 2016
//Author: John Frankland,,,

#ifndef __SQLITEDB_H
#define __SQLITEDB_H

#include "TSQLiteServer.h"
#include "TSQLStatement.h"

#include "KVConfig.h"
#include <utility>
#include <string>
#include <iostream>
#include <map>
#include <KVNameValueList.h>

namespace KVSQLite {
   namespace column_type {
      enum types {
         REAL,
         INTEGER,
         TEXT,
         BLOB
      };
   }
   namespace insert_mode {
      enum types {
         DEFAULT,
         IGNORE,
         FAIL,
         REPLACE
      };
   }

   typedef KVSQLite::insert_mode::types KVSQLite_insert_mode;
   typedef KVSQLite::column_type::types KVSQLite_column_type;

   class column {
      std::pair<std::string, KVSQLite_column_type> fNameType; //name & type of column
      TString fConstraint;//column constraint
      int fIndex;//index of column
      static std::map<KVSQLite::column_type::types, std::string> inv_type_map;
      KVNamedParameter fData; // data item in column
      bool fPrimaryKey;

      void init_type_map();
      std::string _type();

   public:
      column(int idx, const std::string& name, KVSQLite_column_type type)
         : fNameType(name, type), fConstraint(""), fIndex(idx), fData(name.c_str()), fPrimaryKey(false)
      {
         if (!inv_type_map.size()) init_type_map();
      }
      virtual ~column() {}

      const std::string& name() const
      {
         return fNameType.first;
      }
      KVSQLite_column_type type() const
      {
         return fNameType.second;
      }
      std::string type_name() const
      {
         return const_cast<column*>(this)->_type();
      }
      const char* get_declaration() const;

      int index() const
      {
         return fIndex;
      }

      void print() const
      {
         std::cout << fIndex << "\t" << name() << "\t" << type_name() << "\n";
      }
      template<typename T>
      void set_data(T x)
      {
         fData.Set(x);
      }

      void set_data_in_statement(TSQLStatement*, int idx = -1) const;
      void set_data_from_statement(TSQLStatement* s);
      void set_constraint(const TString& c)
      {
         // set constraint for column, one of:
         //   PRIMARY KEY
         //   UNIQUE
         //   FOREIGN KEY
         //   CHECK
         //   NOT NULL
         //   DEFAULT
         fConstraint = c;
         if (c == "PRIMARY KEY") fPrimaryKey = true;
      }
      bool primary_key() const
      {
         return fPrimaryKey;
      }
      const KVNamedParameter& data() const
      {
         return fData;
      }

      ClassDef(column, 0) //Column in an SQLite database table
   };

   class table {
      std::string fName;//name of table
      KVSQLite_insert_mode fInsert;//insert mode
      std::vector<KVSQLite::column> fColumns;//list of columns
      std::map<std::string, int> fColMap; //map name of column to index
      static std::map<std::string, KVSQLite::column_type::types> type_map;

      void init_type_map();

   public:
      table(const std::string& Name = "")
         : fName(Name), fInsert(KVSQLite::insert_mode::DEFAULT), fColumns(), fColMap()
      {
         if (!type_map.size()) init_type_map();
      }
      table(const std::string& Name, const std::vector<KVSQLite::column>& cols)
         : fName(Name), fInsert(KVSQLite::insert_mode::DEFAULT), fColumns(cols), fColMap()
      {
         if (!type_map.size()) init_type_map();
      }
      virtual ~table() {}

      const std::string& name() const
      {
         return fName;
      }
      void show_columns() const;
      void set_insert_mode(KVSQLite_insert_mode i)
      {
         // The insert mode determines how to deal with errors caused by constraints
         // The possible values of i are (with corresponding SQLite meaning):
         //   KVSQLite::insert_mode::DEFAULT   ("INSERT INTO [table] ...")
         //   KVSQLite::insert_mode::FAIL      ("INSERT OR FAIL INTO [table] ...")
         //     - if any data in row being inserted fails a constraint on one or more
         //       columns, insertion fails with error
         //   KVSQLite::insert_mode::IGNORE    ("INSERT OR IGNORE INTO [table] ...")
         //     - if any data in row being inserted fails a constraint on one or more
         //       columns, the row is silently ignored (existing row unchanged)
         //   KVSQLite::insert_mode::REPLACE   ("INSERT OR REPLACE INTO [table] ...")
         //     - if any data in row being inserted fails a constraint on one or more
         //       columns, we replace the old row with the new one
         fInsert = i;
      }
      const char* get_insert_command() const;
      column& add_column(const KVSQLite::column& c);
      column& add_column(const std::string& name, KVSQLite_column_type type)
      {
         // add column to table. return reference to added column.
         return add_column(KVSQLite::column(fColumns.size(), name, type));
      }
      column& add_column(const std::string& name, const std::string& type);
      void add_primary_key(const std::string& name);
      KVSQLite::column& operator[](int i)
      {
         return fColumns[i];
      }
      KVSQLite::column& operator[](const std::string& n)
      {
         if (!fColMap.count(n)) {
            std::cout << "Error in <KVSQLite::table::operator[const std::string&]> : "
                      << n << " is not a column of table " << name() << std::endl;
            return fColumns[0];
         }
         return fColumns[fColMap[n]];
      }

      void print() const
      {
         std::cout << name() << "\n";
         for (std::vector<KVSQLite::column>::const_iterator it = fColumns.begin(); it != fColumns.end(); ++it) it->print();
      }
      int number_of_columns() const
      {
         return fColumns.size();
      }

      ClassDef(table, 0) //Table in an SQLite database
   };

   class database {
      unique_ptr<TSQLiteServer> fDBserv;       //connection to database
      std::map<std::string, KVSQLite::table> fTables; //map of tables in database
      unique_ptr<TSQLStatement> fSQLstmt;     //used for bulk operations
      KVSQLite::table* fBulkTable;            //pointer to table currently used with fSQLstmt
      bool fInserting;
      bool fSelecting;
      bool fEmptyResultSet;
      bool fIsValid;

      void PrintResults(TSQLResult* tabent) const;
      unique_ptr<TSQLResult> SelectRowsFromTable(
         const char* table,
         const char* columns = "*",
         const char* condition = nullptr) const;

      void read_table_infos();

   public:
      database() : fDBserv(nullptr), fTables(), fSQLstmt(nullptr), fBulkTable(nullptr), fInserting(false), fSelecting(false), fIsValid(false) {}
      database(const TString& dbfile) : fDBserv(nullptr), fTables(), fSQLstmt(nullptr), fBulkTable(nullptr), fInserting(false), fSelecting(false), fIsValid(false)
      {
         open(dbfile);
      }
      database(const database& db) : fDBserv(nullptr), fTables(), fSQLstmt(nullptr), fBulkTable(nullptr), fInserting(false), fSelecting(false), fIsValid(false)
      {
         // because of the use of std::unique_ptr, we cannot copy the address of the TSQLiteServer from db - it would delete the database connection in db
         // therefore if we "copy" a database, we create a new, independent interface to the same database
         // this is basically a workaround/kludge for C++11/14/g++6
         if (db.good()) open(db.fDBserv->GetDB());
      }
      database& operator= (const database& db)
      {
         if (&db != this && db.good()) open(db.fDBserv->GetDB());
         return *this;
      }
      void show_tables() const;
      virtual ~database() {}

      void open(const TString& dbfile);
      void close()
      {
         fDBserv->Close();
         fTables.clear();
      }
      bool good() const
      {
         return fIsValid;
      }

      bool is_open() const
      {
         return fDBserv->IsConnected();
      }

      void Dump() const;

      void add_table(KVSQLite::table&);

      KVSQLite::table& operator[](const std::string& name)
      {
         if (!fTables.count(name)) {
            std::cout << "Error in <KVSQLite::database::operator[const std::string&]> : "
                      << name << " is not a table of database" << std::endl;
            return fTables.begin()->second;
         }
         return fTables[name];
      }

      bool prepare_data_insertion(const char* table);
      void insert_data_row();
      void end_data_insertion();

      bool select_data(const char* table, const char* selection = "", const char* anything_else = "");
      bool get_next_result();

      void clear_table(const std::string& name);

      int count(const char* table, const char* column = "*", const char* selection = "", bool distinct = false);
      bool update(const char* table, const char* selection, const TString& columns);
      void delete_row(const char* table, const char* selection = "");

      ClassDef(database, 0) //Interface to ROOT SQLite database backend
   };
}

#endif
