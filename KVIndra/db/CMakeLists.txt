set(db_exclude KVINDRADB_e416a KVINDRADB_e475s KVINDRADB_e503 KVINDRADB_e613)
set(upd_exclude KVINDRAUpDater_e416a KVINDRAUpDater_e475s KVINDRAUpDater_e503 KVINDRAUpDater_e613)

BUILD_KALIVEDA_MODULE(db
	PARENT ${KVSUBPROJECT}
#        KVMOD_DEPENDS exp_events daq_cec geometry identification calibration
        KVMOD_DEPENDS exp_events daq_cec geometry identification
        LIB_EXCLUDE ${db_exclude} ${upd_exclude}
)
