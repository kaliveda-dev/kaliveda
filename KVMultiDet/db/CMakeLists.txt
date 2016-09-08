#---Need ROOT with SQLite support for SQLiteDB
if(WITH_RSQLITE)
   set(extra_lib ${ROOT_RSQLite_LIBRARY})
else()
   set(exclude_class SQLiteDB)
endif()

BUILD_KALIVEDA_MODULE(db
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS base events particles stopping
   EXTRA_LIBS ${extra_lib}
   LIB_EXCLUDE ${exclude_class}
)
