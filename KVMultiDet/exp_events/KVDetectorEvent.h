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
#include "KVEvent.h"
#include "KVGroup.h"

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
   };
   virtual void Clear(Option_t* opt = "");
   virtual void Print(Option_t* t = "") const;
   KVUniqueNameList* GetGroups()
   {
      return fHitGroups;
   };
   Bool_t ContainsGroup(KVGroup* grp)
   {
      return (fHitGroups->FindObject(grp->GetName()) != 0);
   };
   virtual UInt_t GetMult() const
   {
      return fHitGroups->GetSize();
   };

   ClassDef(KVDetectorEvent, 2) // List of hit groups in a multidetector array
};
#endif
