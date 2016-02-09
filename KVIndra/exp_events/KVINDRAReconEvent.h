/***************************************************************************
                          kvindrareconevent.h  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by Alexis Mignon
    email                : mignon@ganil.fr
$Id: KVINDRAReconEvent.h,v 1.38 2008/01/21 12:44:05 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVINDRARECONEVENT_H
#define KVINDRARECONEVENT_H

#include "KVReconstructedEvent.h"
#include "KVMultiDetArray.h"
#include "KVINDRACodes.h"

class KVDetectorEvent;
class KVGroup;
class KVList;
class KVINDRAReconNuc;


class KVINDRAReconEvent: public KVReconstructedEvent {

private:
   KVINDRACodeMask* fCodeMask;  //!VEDA codes accepted for "good" particles (IsOK)
   inline KVINDRACodeMask* GetCodeMask()
   {
      if (!fCodeMask)
         fCodeMask = new KVINDRACodeMask;
      return fCodeMask;
   };
   KVUniqueNameList* fHitGroups;//! non-persistent pointer to list of hit groups used in SecondaryIdentAndCalib()

public:

   KVINDRAReconEvent(Int_t mult = 50, const char* classname =
                        "KVINDRAReconNuc");
   void init();
   virtual ~ KVINDRAReconEvent();


   KVINDRAReconNuc* AddParticle();
   KVINDRAReconNuc* GetParticle(Int_t npart) const;

   KVINDRAReconNuc* GetNextParticle(Option_t* opt = "");

   virtual void Print(Option_t* option = "") const;

   inline Bool_t CheckCodes(KVINDRACodeMask& code)
   {
      //returns kTRUE if "code" is compatible with event's code mask
      //if no code mask set for event, returns kTRUE in all cases
      if (!fCodeMask)
         return kTRUE;
      return ((*fCodeMask) & code);
   };

   void AcceptIDCodes(UShort_t code);
   void AcceptECodes(UChar_t code);

   virtual Bool_t IsOK();

   void IdentifyEvent();
   void ChangeFragmentMasses(UChar_t mass_formula);

   virtual void SecondaryIdentCalib();
   void SecondaryAnalyseGroup(KVGroup* grp);

   ClassDef(KVINDRAReconEvent, 6)       //Event reconstructed from energy losses in INDRA array
};

#endif
