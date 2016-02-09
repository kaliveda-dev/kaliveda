/***************************************************************************
$Id: VAMOSLinkDef.h,v 1.4 2009/01/14 15:59:49 franklan Exp $
 ***************************************************************************/

#ifdef __CINT__

#include "RVersion.h"
#include "KVConfig.h"

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#ifdef WITH_BUILTIN_GRU
#pragma link C++ class KVIVRawDataAnalyser+;
#pragma link C++ class KVIVRawDataReconstructor+;
#endif

#pragma link C++ class KVIVReconDataAnalyser+;
#pragma link C++ class KVIVReconIdent+;
#pragma link C++ class KVIVSelector+;
#pragma link C++ class KVVAMOSExperimentalFilter+;

#endif
