#ifdef __CINT__
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#ifdef WITH_BUILTIN_GRU
#pragma link C++ class KVINDRADSTReader;
//#pragma link C++ class KVINDRADstToRootTransfert;
#endif
#pragma link C++ class KVINDRAReconDataAnalyser;
#pragma link C++ class KVINDRAGeneDataSelector;
#pragma link C++ class KVINDRAGeneDataAnalyser;
#pragma link C++ class KVOldINDRASelector+;
#pragma link C++ class KVINDRAEventSelector+;
#pragma link C++ class KVDataSelector;
#pragma link C++ class KVDataPatch_INDRA_SanityChecks+;
#pragma link C++ class KVDataPatch_INDRA_camp5_PHDcorrection+;
#pragma link C++ class KVDataPatch_CorrectEtalonModuleIDCode+;
#pragma link C++ class KVDataPatch_INDRA_CorrectEtalonTrajectories+;
#pragma link C++ class KVDataPatch_Camp2MassID+;
#pragma link C++ class KVDataPatch_Camp1MassID+;
#ifdef USING_ROOT6
#pragma link C++ class KVINDRATriggerConditions+;
#endif
#endif
