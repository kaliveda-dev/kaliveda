if(NOT WITH_FITLTG)
	set(do_not_compile KVIDSiCsI5)
endif()

BUILD_KALIVEDA_MODULE(identification_camp5
	PARENT ${KVSUBPROJECT}
        KVMOD_DEPENDS identification
        LIB_EXCLUDE ${do_not_compile}
)
