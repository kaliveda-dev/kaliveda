if(NOT WITH_BUILTIN_GRU)
	set(excluded_classes KVIVRawDataReconstructor KVIVRawDataAnalyser)
endif()

BUILD_KALIVEDA_MODULE(analysis
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS calibration exp_events geometry identification
	LIB_EXCLUDE ${excluded_classes}
	EXTRA_LIBS ${KVIndra_LIB_LIST} ${KVMultiDet_LIB_LIST}
)
