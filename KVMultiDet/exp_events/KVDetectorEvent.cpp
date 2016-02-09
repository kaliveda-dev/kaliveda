/***************************************************************************
                          kvdetectorevent.cpp  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVDetectorEvent.cpp,v 1.14 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "Riostream.h"
#include "KVDetectorEvent.h"

using namespace std;

ClassImp(KVDetectorEvent);

//////////////////////////////////////////////
//KVDetectorEvent
//
//Multiparticle event detected/"seen" by a multidetector array.
//The result is basically a list of all the hit groups of the array (see KVGroup and KVMultiDetArray), from which we
//can access the energy losses etc. in the different detectors.
//
//A group is said to be "hit" if at least one detector has fired in the group (see KVDetector::Fired()).
//The list of "hit" groups is filled by KVMultiDetArray::GetDetectorEvent().
//
//GetGroups() returns a pointer to the list of hit groups.
//      To loop over all hit groups in a KVDetectorEvent* kde, do something like this:
//               KVGroup *g;
//               TIter next( kde->GetGroups() );
//               while( (g = (KVGroup*)next()) ){
//                  ...
//               }
//
//ContainsGroup(KVGroup* grp) returns kTRUE if 'grp' points to a KVGroup belonging to the list of hit groups
//i.e. if ContainsGroup() is equal to kTRUE, then the group was hit in the event.

KVDetectorEvent::KVDetectorEvent()
{
   init();
}

void KVDetectorEvent::init()
{
   //Default initialisation
   fHitGroups = new KVUniqueNameList;
}

KVDetectorEvent::~KVDetectorEvent()
{
   Clear();
   delete fHitGroups;
   fHitGroups = 0;
}

void KVDetectorEvent::Clear(Option_t*)
{
   // Reset the list of hit groups, ready for analysis of a new event.
   // Each 'hit' group is cleared (energy losses in detectors set to zero, etc.).

   fHitGroups->R__FOR_EACH(KVGroup, Reset)();
   fHitGroups->Clear();
}

//____________________________________________________________________________
void KVDetectorEvent::Print(Option_t*) const
{
   //Print a listing of hit groups with fired detectors

   cout << "\nKVDetectorEvent" << endl;
   cout << "--------------" << endl;
   cout << "\nGroups hit: " << GetMult() << endl;
   cout << "\n";
   if (GetMult()) {
      KVGroup* g;
      for (UInt_t i = 0; i < GetMult(); i++) {
         g = (KVGroup*) fHitGroups->At(i);
         g->Print("fired");
      }
   }
}
