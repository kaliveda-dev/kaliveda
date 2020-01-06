#---Need Gemini++ interface for SimpleGeminiModel
if(NOT WITH_GEMINI OR ${ROOT_VERSION} VERSION_GREATER 6.17.00)
	set(exclude_class ${exclude_class} SimpleGeminiModel_Binned)
endif()
# problems with modifs in RooFit from 6.18 onwards
if(${ROOT_VERSION} VERSION_GREATER 6.17.00)
    set(exclude_class ${exclude_class} Simple2DModel_Binned)
endif()

BUILD_KALIVEDA_MODULE(models
	KVMOD_DEPENDS base
	PARENT ${KVSUBPROJECT}
	EXTRA_LIBS ${KVMultiDet_LIB_LIST}
   LIB_EXCLUDE ${exclude_class}
)
