set(excluded_classes KVFocalPlanVamos)

# KVVAMOSReconNuc.h needed in KVVAMOSReconGeoNavigator
KALIVEDA_SET_INCLUDE_DIRS(VAMOS MODULES exp_events)

BUILD_KALIVEDA_MODULE(geometry
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS calibration
	LIB_EXCLUDE ${excluded_classes}
	EXTRA_LIBS ${KVIndra_LIB_LIST} ${KVMultiDet_LIB_LIST}
)
