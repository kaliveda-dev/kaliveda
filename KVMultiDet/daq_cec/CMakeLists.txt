if(NOT WITH_BUILTIN_GRU)
    set(excluded_classes GTOneScaler GTScalers GTDataParameters GTGanilData KVGANILDataReader)
else()
    set(libs_to_link ${BUILTIN_GRU_LIB})
endif()
if(NOT WITH_MFM)
    set(excluded_classes ${excluded_classes} KVMFMDataFileReader)
else()
    set(libs_to_link ${libs_to_link} ${mfmlib_LIBRARIES})
endif()
if(NOT WITH_PROTOBUF)
    set(excluded_classes ${excluded_classes} KVProtobufDataReader)
else()
    set(libs_to_link ${libs_to_link} ${PROTOBUF_LIBRARY})
endif()
if(WITH_ZMQ)
    set(libs_to_link ${libs_to_link} ${ZMQ_LIBRARIES})
    set(dict_exclude KVZMQMessage.h)
else(WITH_ZMQ)
    set(excluded_classes ${excluded_classes} KVZMQMessage)
endif(WITH_ZMQ)
if(WITH_MESYTEC)
    set(libs_to_link ${libs_to_link} ${mesytec_data_LIBRARIES})
endif(WITH_MESYTEC)

BUILD_KALIVEDA_MODULE(daq_cec
	PARENT ${KVSUBPROJECT}
        KVMOD_DEPENDS base geometry
	LIB_EXCLUDE ${excluded_classes}
        EXTRA_LIBS ${libs_to_link}
        DICT_EXCLUDE ${dict_exclude}
)
