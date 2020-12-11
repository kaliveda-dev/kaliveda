//Created by KVClassFactory on Wed May 17 12:26:57 2006
//Author: franklan

#include "KVINDRAe503.h"
#include "KVConfig.h"
#ifdef WITH_FITLTG
#include "KVRTGIDManager.h"
#endif

ClassImp(KVINDRAe503)
KVINDRAe503::KVINDRAe503()
{
   //Default constructor
}

KVINDRAe503::~KVINDRAe503()
{
   //Destructor
}

void KVINDRAe503::SetGroupsAndIDTelescopes()
{
   //Find groups of telescopes in angular alignment placed on different layers.
   //List is in fGroups.
   //Also creates all ID telescopes in array and stores them in fIDTelescopes.
   //Any previous groups/idtelescopes are deleted beforehand.
   // As any ID grids stored in gIDGridManager and any ID functions stored in
   // KVRTGIDManger::fIDGlobalList will have been associated to the
   // old ID telescopes (whose addresses now become obsolete), we clear the ID grid manager
   // deleting all ID grids and the KVRTGIDManger::fIDGlobalList deleting all
   // ID functions. You should therefore follow this with a call to
   // SetIdentifications() in order to reinitialize all that.


#ifdef WITH_FITLTG
   KVRTGIDManager::Clear();
#endif
   KVINDRA::SetGroupsAndIDTelescopes();
}
