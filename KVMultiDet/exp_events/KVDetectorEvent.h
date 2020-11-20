/***************************************************************************
                          kvdetectorevent.h  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVDetectorEvent.h,v 1.8 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVDETECTOREVENT_H
#define KVDETECTOREVENT_H

#include "KVBase.h"
#include "KVUniqueNameList.h"
#include "KVTemplateEvent.h"
#include "KVGroup.h"

/**
 \class KVDetectorEvent
 \ingroup Reconstruction
 \brief List of hit groups in a multidetector array

Multiparticle event detected/"seen" by a multidetector array.
The result is basically a list of all the hit groups of the array (see KVGroup and KVMultiDetArray), from which we
can access the energy losses etc. in the different detectors.

A group is said to be "hit" if at least one detector has fired in the group (see KVDetector::Fired()).
The list of "hit" groups is filled by KVMultiDetArray::GetDetectorEvent().

GetGroups() returns a pointer to the list of hit groups.

To loop over all hit groups in a KVDetectorEvent* kde, do something like this:
~~~~~~~{.cpp}
   KVGroup *g;
   TIter next( kde->GetGroups() );
   while( (g = (KVGroup*)next()) ){
        ...
   }
~~~~~~~~

ContainsGroup(KVGroup* grp) returns kTRUE if 'grp' points to a KVGroup belonging to the list of hit groups
i.e. if ContainsGroup() returns kTRUE, then the group was hit in the event.

 */
class KVDetectorEvent: public KVBase {

private:

   KVUniqueNameList* fHitGroups;         //->list of groups hit by particles in the event

public:

   KVDetectorEvent();
   virtual ~ KVDetectorEvent();
   virtual void init();
   void AddGroup(KVGroup* grp)
   {
      fHitGroups->Add(grp);
   }
   virtual void Clear(Option_t* opt = "");
   virtual void Print(Option_t* t = "") const;
   KVUniqueNameList* GetGroups()
   {
      return fHitGroups;
   }
   Bool_t ContainsGroup(KVGroup* grp)
   {
      return (fHitGroups->FindObject(grp->GetName()) != 0);
   }
   virtual UInt_t GetMult() const
   {
      return fHitGroups->GetSize();
   }

   ClassDef(KVDetectorEvent, 2) // List of hit groups in a multidetector array
};
#endif
