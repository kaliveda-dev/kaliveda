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
   delete fHitGroups;
   fHitGroups = 0;
}

void KVDetectorEvent::Clear(Option_t* opt)
{
   // Reset the list of hit groups, ready for analysis of a new event.
   // Each 'hit' group is cleared (energy losses in detectors set to zero, etc.).
   // unless option "NGR" (No Group Reset) is given
   // Any other option is passed to KVGroup::Reset

   if (strncmp(opt, "NGR", 3)) {
      fHitGroups->R__FOR_EACH(KVGroup, Reset)(opt);
   }
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
