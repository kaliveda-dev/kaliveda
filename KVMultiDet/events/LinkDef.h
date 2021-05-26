#ifdef __CINT__
#include "RVersion.h"
#include "KVConfig.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class KVEvent-;
#pragma link C++ class KVTemplateEvent<KVNucleus>+;
#pragma link C++ class KVNucleusEvent+;
#pragma link C++ class KVTemplateEvent<KVNucleus>::Iterator+;
#pragma link C++ class KVNucleusEvent::Iterator+;
#pragma link C++ class KVTemplateEvent<KVNucleus>::EventIterator+;
#pragma link C++ class KVTemplateEvent<KVNucleus>::EventGroupIterator+;
#pragma link C++ class KVTemplateEvent<KVNucleus>::EventOKIterator+;
#pragma link C++ class EventIterator+;
#pragma link C++ class EventGroupIterator+;
#pragma link C++ class EventOKIterator+;
#ifdef WITH_OPENGL
#pragma link C++ class KVEventViewer+;
#pragma link C++ enum KVEventViewer::EHighlightMode;
#endif
#endif
