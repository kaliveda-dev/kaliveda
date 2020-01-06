#ifdef __CINT__
#include "RVersion.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ namespace BackTrack;
#if ROOT_VERSION_CODE < ROOT_VERSION(6,18,0)
#pragma link C++ class NewRooMinuit+;
#pragma link C++ class NewRooMinimizer+;
#pragma link C++ class NewRooMinimizerFcn+;
#pragma link C++ class NewRooFitResult+;
#pragma link C++ class NewRooAddPdf+;
#pragma link C++ class BackTrack::GenericModel_Binned+;
#endif
#pragma link C++ class BackTrack::GenericPDFModel+;
#pragma link C++ class BackTrack::GenericModel+;
#endif
