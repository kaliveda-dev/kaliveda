# problems with modifs in RooFit from 6.18 onwards
if(${ROOT_VERSION} VERSION_GREATER 6.17.00)
    set(lib_exc NewRooAddPdf NewRooFitResult NewRooGlobalFunc NewRooMinimizer NewRooMinimizerFcn NewRooMinuit GenericModel_Binned)
endif()

BUILD_KALIVEDA_MODULE(base
	PARENT ${KVSUBPROJECT}
        LIB_EXCLUDE ${lib_exc}
	EXTRA_LIBS ${KVMultiDet_LIB_LIST} ${ROOT_RooFitCore_LIBRARY} ${ROOT_RooFit_LIBRARY} ${ROOT_Minuit_LIBRARY}
)
