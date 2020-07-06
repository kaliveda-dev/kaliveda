BUILD_KALIVEDA_MODULE(gui
	PARENT ${KVSUBPROJECT}
   KVMOD_DEPENDS simulation analysis exp_events geometry data_management db stopping events base identification particles globvars impact_parameter
   EXTRA_LIBS ${ROOT_Spectrum_LIBRARY}
)
