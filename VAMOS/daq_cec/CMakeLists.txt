if(NOT WITH_BUILTIN_GRU)
	set(excluded_classes GTGanilDataVAMOS)
else()
	set(not_in_dict GTGanilDataVAMOS)
endif()

BUILD_KALIVEDA_MODULE(daq_cec
	PARENT ${KVSUBPROJECT}
	DICT_EXCLUDE ${not_in_dict}
	LIB_EXCLUDE ${excluded_classes}
	EXTRA_LIBS ${KVIndra_LIB_LIST} ${KVMultiDet_LIB_LIST}
)
