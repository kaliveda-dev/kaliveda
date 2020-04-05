#ifdef __CINT__
#include "RVersion.h"
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class KVEvent-;
#pragma link C++ class KVEvent::Iterator;
#pragma link C++ enum KVEvent::Iterator::Type;
#pragma link C++ class iterator<forward_iterator_tag,KVNucleus,long,KVNucleus*,KVNucleus&>;
#ifdef WITH_OPENGL
#pragma link C++ class KVEventViewer+;
#pragma link C++ enum KVEventViewer::EHighlightMode;
#endif
#endif
