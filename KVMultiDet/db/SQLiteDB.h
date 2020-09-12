//Created by KVClassFactory on Mon Jul 25 15:33:56 2016
//Author: John Frankland,,,

#ifndef __SQLITEDB_H
#define __SQLITEDB_H

#include "TSQLiteServer.h"
#include "TSQLStatement.h"

#include "KVConfig.h"
#include <utility>
#include <iostream>
#ifdef WITH_CPP11
#include <unordered_map>
#endif
#include <map>
#include <KVNameValueList.h>
#include <KVNumberList.h>
#include "TGraph.h"

namespace KVSQLite {

   /**
     \class database
     \brief Interface to ROOT SQLite database backend
     \ingroup DB,Core

   Opening a database file
   =======================
   ~~~~{.cpp}
   KVSQLite::database db("my_database.sqlite");
   ~~~~
   *WARNING*: opens in read/write mode. there is no protection against modifying
                an existing database.

   Looking at data
   ===============
   ~~~~{.cpp}
   db.show_tables();   => print names of tables in database
   ~~~~
   Example output:
   ~~~~
           Tables in database:
             SCdaq_reader
             SCdaq_writer
             SCdetectors
             SCelectronics
   ~~~~

   ~~~~{.cpp}
   db["SCdetectors"].show_columns();  => print names of columns in table "SCdetectors"
   ~~~~
   Example output:
   ~~~~
           Columns in table:
           id [INTEGER]
           block [INTEGER]
           quartet [INTEGER]
           telescope [INTEGER]
           detector [TEXT]
           frontEnd [INTEGER]
           module [TEXT]
           parameter [TEXT]
           alias [TEXT]
           channel [TEXT]
           value [TEXT]
           units [TEXT]
           time [TEXT]
   ~~~~

   ~~~~{.cpp}
         db.print_selection("SCelectronics","time,value,units","module='FETemp' AND block=2 AND card='FE3'");
   ~~~~
   Example output:
   ~~~~
           #|                time|               value|               units
           0| 2017-03-11 13:45:31|                  37|             Celsius
           1| 2017-03-11 13:45:31|                  41|             Celsius
           2| 2017-03-11 13:45:31|                  36|             Celsius
           3| 2017-03-11 13:45:31|                  34|             Celsius
           4| 2017-03-11 13:45:31|                  40|             Celsius
           5| 2017-03-11 13:45:31|                  39|             Celsius
           6| 2017-03-11 13:46:09|                  37|             Celsius
           7| 2017-03-11 13:46:09|                  41|             Celsius
           8| 2017-03-11 13:46:09|                  36|             Celsius
   ~~~~

   For more details on use (also for inserting data), see example db_sqlite_examples.C
   */

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

   class table;

   class column {

      friend class table;

      std::pair<TString, KVSQLite_column_type> fNameType; //name & type of column
      TString fConstraint;//column constraint
      int fIndex;//index of column
      static std::map<KVSQLite::column_type::types, TString> inv_type_map;
      KVNamedParameter fData; // data item in column
      void* fBlob;//! binary data
      Long_t fBlobSize;// size of blob
      bool fPrimaryKey;
      bool fForeignKey;
      TString fFKtable;//table for foreign key
      TString fFKcolumn;//column for foreign key
      mutable bool fIsNull;//for inserting NULL values

      void init_type_map();
      const char* _type();

      column(int idx, const TString& name, KVSQLite_column_type type)
         : fNameType(name, type), fConstraint(""), fIndex(idx), fData(name),
           fBlob(nullptr), fBlobSize(0),
           fPrimaryKey(false), fForeignKey(false),
           fFKtable(""), fFKcolumn(""), fIsNull(false)
      {
         if (!inv_type_map.size()) init_type_map();
      }
   public:
      virtual ~column()
      {
         // clean up binary data
         if (fBlob) delete[](unsigned char*)fBlob;
      }

      const char* name() const
      {
         return fNameType.first;
      }
      KVSQLite_column_type type() const
      {
         return fNameType.second;
      }
      const char* type_name() const
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
      void set_data(const T& x)
      {
         fData.Set(x);
         fIsNull = false;
      }
      template <typename T>
      const column& operator=(const T& x)
      {
         // set data in column
         set_data(x);
         return *this;
      }

      void set_null()
      {
         fIsNull = true;
      }
      bool is_null() const
      {
         return fIsNull;
      }

      template<typename T>
      void set_binary_data(T& x)
      {
         fBlob = (void*)&x;
         fBlobSize = sizeof(x);
         fIsNull = false;
      }
      template<typename T>
      void set_binary_data(T* x)
      {
         fBlob = (void*)x;
         fBlobSize = sizeof(*x);
         fIsNull = false;
      }

      void set_data_in_statement(TSQLStatement*, int idx = -1) const;
      void set_data_from_statement(TSQLStatement* s, int idx = -1);
      void set_constraint(const TString& c)
      {
         // set constraint for column, one of:
         //~~~
         //   PRIMARY KEY
         //   UNIQUE
         //   CHECK
         //   NOT NULL
         //   DEFAULT
         //~~~
         fConstraint = c;
         if (c == "PRIMARY KEY") fPrimaryKey = true;
      }
      void set_foreign_key(const TString& _table, const TString& _column);
      void set_foreign_key(const table& _table, const column& _column);
      bool primary_key() const
      {
         return fPrimaryKey;
      }
      bool foreign_key() const
      {
         return fForeignKey;
      }
      const KVNamedParameter& data() const
      {
         return fData;
      }
      template <typename T>
      T get_data() const
      {
         return fData.Get<T>();
      }
      template<typename T>
      T* binary_data() const
      {
         return static_cast<T*>(fBlob);
      }

      ClassDef(column, 0) //Column in an SQLite database table
   };
   template<> void column::set_data(const KVNamedParameter&);

   class table {
      TString fName;//name of table
      KVSQLite_insert_mode fInsert;//insert mode
      std::vector<KVSQLite::column> fColumns;//list of columns
#ifdef WITH_CPP11
      std::unordered_map<std::string, int> fColMap; //map name of column to index
#else
      std::map<std::string, int> fColMap; //map name of column to index
#endif
      static std::map<TString, KVSQLite::column_type::types> type_map;
      bool fTemp;//temporary table?

      void init_type_map();

   public:
      table(const TString& Name = "")
         : fName(Name), fInsert(KVSQLite::insert_mode::DEFAULT), fColumns(), fColMap(), fTemp(false)
      {
         if (!type_map.size()) init_type_map();
      }
      table(const TString& Name, const std::vector<KVSQLite::column>& cols)
         : fName(Name), fInsert(KVSQLite::insert_mode::DEFAULT), fColumns(cols), fColMap(), fTemp(false)
      {
         if (!type_map.size()) init_type_map();
      }
      virtual ~table() {}

      const char* name() const
      {
         return fName;
      }
      void set_name(const TString& name)
      {
         fName = name;
      }

      void show_columns() const;
      void set_insert_mode(KVSQLite_insert_mode i)
      {
         // The insert mode determines how to deal with errors caused by constraints
         // The possible values of i are (with corresponding SQLite meaning):
         //~~~
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
         //~~~
         fInsert = i;
      }
      const char* get_insert_command() const;
      void set_temporary(bool temp = true)
      {
         // Create a temporary table
         fTemp = temp;
      }
      bool is_temporary() const
      {
         return fTemp;
      }

      column& add_column(const KVSQLite::column& c);
      column& add_column(const TString& name, KVSQLite_column_type type)
      {
         // add column to table. return reference to added column.
         return add_column(KVSQLite::column(fColumns.size(), name, type));
      }
      column& add_column(const TString& name, const TString& type);
      const column& add_primary_key(const TString& name);
      const column& add_foreign_key(const TString& name, const TString& other_table, const TString& other_column);
      const column& add_foreign_key(const TString& name, const table& other_table, const column& other_column);
      KVSQLite::column& operator[](int i)
      {
         return fColumns[i];
      }
      bool has_column(const TString& name)
      {
         // return true if column with given name exists
         return fColMap.count(name.Data());
      }

      KVSQLite::column& operator[](const TString& n)
      {
         if (!has_column(n)) {
            std::cout << "Error in <KVSQLite::table::operator[const TString&]> : "
                      << n << " is not a column of table " << name() << std::endl;
            return fColumns[0];
         }
         return fColumns[fColMap[n.Data()]];
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
      int check_columns(const KVNameValueList&);
      void prepare_data(const KVNameValueList&, const KVNamedParameter* = nullptr);
      void set_all_columns_null();

      ClassDef(table, 0) //Table in an SQLite database
   };

   class database {
      unique_ptr<TSQLiteServer> fDBserv;       //connection to database
#ifdef WITH_CPP11
      std::unordered_map<std::string, KVSQLite::table> fTables; //map of tables in database
#else
      std::map<std::string, KVSQLite::table> fTables; //map of tables in database
#endif
      unique_ptr<TSQLStatement> fSQLstmt;     //used for bulk operations
      KVSQLite::table* fBulkTable;            //pointer to table currently used with fSQLstmt
      bool fInserting;
      bool fSelecting;
      bool fEmptyResultSet;
      bool fIsValid;
      TString fSelectedColumns;

      void PrintResults(TSQLResult* tabent, int column_width = 20) const;
      unique_ptr<TSQLResult> SelectRowsFromTable(
         const TString& table,
         const TString& columns = "*",
         const TString& condition = "") const;

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
         //
         // therefore if we "copy" a database, we create a new, independent interface to the same database
         //
         // this is basically a workaround/kludge for C++11/14/g++6
         if (db.good()) open(db.fDBserv->GetDB());
      }
      database& operator= (const database& db)
      {
         if (&db != this && db.good()) open(db.fDBserv->GetDB());
         return *this;
      }
      void show_tables() const;
      int get_number_of_tables() const
      {
         return fTables.size();
      }
      virtual ~database() {}
      bool is_inserting() const
      {
         // \return true if data insertion is in progress (i.e. after call to
         // database::prepare_data_insertion() and before call to
         // database::end_data_insertion())
         return fInserting;
      }

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
      bool has_table(const TString& table)
      {
         // \returns true if "table" exists in database
         return fTables.count(table.Data());
      }

      KVSQLite::table& operator[](const TString& name)
      {
         if (!fTables.count(name.Data())) {
            std::cout << "Error in <KVSQLite::database::operator[const TString&]> : "
                      << name << " is not a table of database" << std::endl;
            return fTables.begin()->second;
         }
         return fTables[name.Data()];
      }

      bool prepare_data_insertion(const TString&);
      void insert_data_row();
      void end_data_insertion();

      bool select_data(const TString& table, const TString& columns = "*", const TString& selection = "",
                       bool distinct = false, const TString& anything_else = "");
      bool get_next_result();
      KVNumberList get_integer_list(const TString& table, const TString& column,
                                    const TString& selection = "", const TString& anything_else = "");
      TString get_string_list(const TString& table, const TString& column,
                              const TString& selection = "", const TString& anything_else = "");
      KVNameValueList get_name_value_list(const TString& table, const TString& name_column, const TString& value_column,
                                          const TString& selection = "", const TString& anything_else = "");
      TGraph* create_graph(const TString& tablename, const TString& Xcolumn, const TString& Ycolumn, const TString& selection = "");

      void clear_table(const TString& name);

      int count(const TString& table, const TString& column = "*", const TString& selection = "", bool distinct = false);
      bool update(const TString& table, const TString& columns, const TString& selection = "");
      void delete_data(const TString& table, const TString& selection = "");

      column& add_column(const TString& table, const TString& name, const TString& type);
      void add_missing_columns(const TString& table, const KVNameValueList& l);

      void copy_table_data(const TString& source, const TString& destination, const TString& columns = "*", const TString& selection = "");
      void print_selection(const TString& table, const TString& columns, const TString& condition, int column_width = 20) const;

      ClassDef(database, 0) //Interface to ROOT SQLite database backend
   };
}

#endif
