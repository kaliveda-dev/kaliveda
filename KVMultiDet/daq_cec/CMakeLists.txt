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

BUILD_KALIVEDA_MODULE(daq_cec
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS base
	LIB_EXCLUDE ${excluded_classes}
        EXTRA_LIBS ${libs_to_link}
)
