#ifdef __CINT__
#include "RVersion.h"
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class BackTrack::Simple1DModel+;
#pragma link C++ class BackTrack::Simple2DModel+;
#if ROOT_VERSION_CODE < ROOT_VERSION(6,18,0)
#pragma link C++ class BackTrack::Simple2DModel_Binned+;
#ifdef WITH_GEMINI
#pragma link C++ class BackTrack::SimpleGeminiModel_Binned+;
#endif
#endif
#endif
