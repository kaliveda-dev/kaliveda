/***************************************************************************
$Id: VAMOSLinkDef.h,v 1.4 2009/01/14 15:59:49 franklan Exp $
 ***************************************************************************/

#ifdef __CINT__

#include "RVersion.h"

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ enum FocalPosCodes;
#pragma link C++ class KVVAMOSCodes+;
#pragma link C++ class KVChargeStateDistrib+;
#pragma link C++ class KVCutList+;
#pragma link C++ class KVBasicVAMOSFilter+;
#pragma link C++ class KVVAMOSReconGeoNavigator+;
#pragma link C++ class KVVAMOSReconTrajectory+;
#pragma link C++ class KVVAMOSTransferMatrix-; //customized streamer
#pragma link C++ class KVVAMOS+;
#pragma link C++ class KVVAMOSDetector+;
#pragma link C++ class KVSpectroDetector+;

#pragma link C++ class KVCsIVamos+;
#pragma link C++ class KVDriftChamber+;
#pragma link C++ class KVHarpeeCsI-;
#pragma link C++ class KVHarpeeIC+;
#pragma link C++ class KVHarpeeSi-;
#pragma link C++ class KVSeD+;
#pragma link C++ class KVSiliconVamos+;

#endif
