set(excluded_classes KVReconIdent_e475s KVINDRADstToRootTransfert)

if(NOT WITH_BUILTIN_GRU)
        set(excluded_classes ${excluded_classes} KVINDRADstToRootTransfert KVINDRADSTReader)
endif()
if(ROOT_VERSION VERSION_LESS 6.00.00)
    set(excluded_classes ${excluded_classes} KVINDRATriggerConditions)
endif()
BUILD_KALIVEDA_MODULE(analysis
	PARENT ${KVSUBPROJECT}
        KVMOD_DEPENDS exp_events daq_cec geometry identification db
        LIB_EXCLUDE ${excluded_classes}
   DICT_EXCLUDE KVSelector.h
	EXTRA_LIBS KVMultiDetgui
)
