#ifdef __CINT__
#include "RVersion.h"
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;

#pragma link C++ global gIDGridManager;

#ifdef WITH_FITLTG
#pragma link C++ namespace KVTGIDFunctions;
#pragma link C++ class KVTGIDFitter+;
#pragma link C++ class KVTGID-;
#pragma link C++ class KVTGIDZ+;
#pragma link C++ class KVTGIDZA+;
#pragma link C++ class KVTGIDManager+;
#pragma link C++ class KVTGIDGrid+;
#endif

#pragma link C++ class KVIdentificationResult+;
#pragma link C++ class KVVirtualIDFitter+;
#pragma link C++ class KVDroite+;
#pragma link C++ class KVSpiderLine+;
#pragma link C++ class KVSpiderLineSiCsI+;
#pragma link C++ class KVDP2toIDGridConvertor+;
#pragma link C++ class KVDP2toCsIGridConvertor+;
#pragma link C++ class KVIDentifier+;
#pragma link C++ class KVIDGraph-;
#pragma link C++ class KVIDLine-;
#pragma link C++ class KVIDContour+;
#pragma link C++ class KVIDCutContour+;
#pragma link C++ class KVIDCutLine+;
#pragma link C++ class KVIDZAContour+;
#pragma link C++ class KVIDZoneContour+;
#pragma link C++ class KVIDZALine-;
#pragma link C++ class KVIDZLine-;
#pragma link C++ class KVIDMap+;
#pragma link C++ class KVIDGrid+;
#pragma link C++ class KVIDGridManager;
#pragma link C++ class KVIDZAGrid-;
#pragma link C++ class KVIDZAFromZGrid+;
#pragma link C++ class interval+;
#pragma link C++ class interval_set+;
#pragma link C++ class KVIDZGrid-;
#pragma link C++ class KVIDCsIRLLine+;
#pragma link C++ class KVIDGCsI+;
#endif
