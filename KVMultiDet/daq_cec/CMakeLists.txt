if(NOT WITH_BUILTIN_GRU)
	set(excluded_classes GTOneScaler GTScalers GTDataParameters GTGanilData)
else()
   set(not_in_dict GTOneScaler GTScalers GTDataParameters GTGanilData)
endif()

BUILD_KALIVEDA_MODULE(daq_cec
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS base
	DICT_EXCLUDE ${not_in_dict}
	LIB_EXCLUDE ${excluded_classes}
	EXTRA_LIBS ${BUILTIN_GRU_LIB}
)
