BUILD_KALIVEDA_MODULE(gui
	PARENT ${KVSUBPROJECT}
#        KVMOD_DEPENDS analysis db exp_events daq_cec geometry calibration identification
        KVMOD_DEPENDS analysis db exp_events daq_cec geometry identification
)
