#ifdef __CINT__
#include "RVersion.h"
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class KVRawDataReader+;
#ifdef WITH_BUILTIN_GRU
#pragma link C++ class GTOneScaler+;
#pragma link C++ class GTScalers+;
#pragma link C++ class GTDataParameters+;
#pragma link C++ class GTGanilData+;
#pragma link C++ class KVGANILDataReader+;
#pragma link C++ class KVEBYEDAT_ACQParam+;
#endif
#ifdef WITH_MFM
#pragma link C++ class KVMFMDataFileReader+;
#endif
#ifdef WITH_PROTOBUF
#pragma link C++ class KVProtobufDataReader+;
#endif
#endif
