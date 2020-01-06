if(NOT WITH_FITLTG)
	set(do_not_compile KVIDSiCorrCsI KVIDSi75SiLi_e494s KVIDSiLiCsI_e494s)
endif()

BUILD_KALIVEDA_MODULE(identification_E494S
	PARENT ${KVSUBPROJECT}
        KVMOD_DEPENDS identification
        LIB_EXCLUDE ${do_not_compile}
)
