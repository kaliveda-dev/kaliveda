#set(deps calibration signals)
set(deps signals)
if(WITH_PROTOBUF)
    set(deps ${deps} daq_cec protobuf)
endif(WITH_PROTOBUF)

BUILD_KALIVEDA_MODULE(geometry
	PARENT ${KVSUBPROJECT}
   KVMOD_DEPENDS ${deps}
   EXTRA_LIBS KVMultiDetexp_events
)
