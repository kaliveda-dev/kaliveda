#ifdef __CINT__
#include "RVersion.h"
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ global gDataBase;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class KV2Body+;
#pragma link C++ class KVDataBase;
#pragma link C++ class KVExpDB;
#pragma link C++ class KVDBKey;
#pragma link C++ class KVDBRecord;
#pragma link C++ class KVDBRun;
#pragma link C++ class KVDBSystem;
#pragma link C++ class KVDBTable;
#pragma link C++ class KVDBParameterSet-;//customized streamer
#pragma link C++ class KVRunListLine;
#pragma link C++ class KVDBParameterList;
#ifdef WITH_RSQLITE
#pragma link C++ namespace KVSQLite;
#pragma link C++ namespace KVSQLite::column_type;
#pragma link C++ enum KVSQLite::column_type::types;
#pragma link C++ namespace KVSQLite::insert_mode;
#pragma link C++ enum KVSQLite::insert_mode::types;
#pragma link C++ class KVSQLite::column+;
#pragma link C++ class KVSQLite::table+;
#pragma link C++ class KVSQLite::database+;
#endif
#endif
