if(USE_DEPRECATED_VAMOS)
	BUILD_KALIVEDA_MODULE(deprecated
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS analysis calibration geometry identification 
	EXTRA_LIBS ${KVIndra_LIB_LIST} ${KVMultiDet_LIB_LIST}
	)
endif()
