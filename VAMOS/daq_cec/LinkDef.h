/***************************************************************************
$Id: VAMOSLinkDef.h,v 1.4 2009/01/14 15:59:49 franklan Exp $
 ***************************************************************************/

#ifdef __CINT__

#include "RVersion.h"
#include "KVConfig.h"

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class ParameterName+;
#pragma link C++ class Parameters+;

#ifdef WITH_BUILTIN_GRU
#pragma link C++ class GTGanilDataVAMOS+;
#endif

#endif
