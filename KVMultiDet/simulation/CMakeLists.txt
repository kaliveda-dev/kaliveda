#---Gemini++ interface
if(WITH_GEMINI)
	set(extra_lib ${extra_lib} ${GEMINI_LIB})
else()
	set(exclude_class ${exclude_class} KVGemini)
endif()

BUILD_KALIVEDA_MODULE(simulation
	PARENT ${KVSUBPROJECT}
   KVMOD_DEPENDS base particles events stopping geometry db exp_events identification analysis globvars data_management
	EXTRA_LIBS ${extra_lib}
	LIB_EXCLUDE ${exclude_class}
)
