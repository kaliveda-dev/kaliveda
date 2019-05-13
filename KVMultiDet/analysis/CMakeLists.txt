BUILD_KALIVEDA_MODULE(analysis
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS base events particles stopping data_management db globvars
   EXTRA_LIBS ${ROOT_Proof_LIBRARY} ${ROOT_ProofPlayer_LIBRARY} ${ROOT_Spectrum_LIBRARY}
)
