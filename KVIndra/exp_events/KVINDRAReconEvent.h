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

/**
  \class KVINDRAReconEvent
  \brief Event reconstructed from energy losses in INDRA multidetector
  \ingroup INDRAReconstruction

By default this contains a TClonesArray of KVINDRAReconNuc objects.

Random vs. mean angles
----------------------
By default reconstructed particles have randomised angles within the
aperture of the detector where they were stopped. To use the mean angles
of the detectors, call UseMeanAngles().

"OK" & not "OK" particles
---------------------------
The "OK" status of particles is defined by setting acceptable identification and calibration
codes using AcceptIDCodes and AcceptECodes. The comparison of each particle's codes
with the "acceptable" codes then determines whether KVParticle::IsOK() is set or not.

Trigger Information
--------------------
Use EventTrigger() to access the KVINDRATriggerInfo object which stores the values
of Selecteur registers STAT_EVE, R_DEC and CONFIG for this event
*/

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

   KVINDRAReconEvent(Int_t mult = 50);
   void init();
   virtual ~ KVINDRAReconEvent();

   virtual void Print(Option_t* option = "") const;

   Bool_t CheckCodes(KVINDRACodeMask& code)
   {
      //returns kTRUE if "code" is compatible with event's code mask
      //if no code mask set for event, returns kTRUE in all cases
      if (!fCodeMask)
         return kTRUE;
      return ((*fCodeMask) & code);
   };

   void AcceptIDCodes(UShort_t code);
   void AcceptECodes(UChar_t code);

   void IdentifyEvent();
   void ChangeFragmentMasses(UChar_t mass_formula);

   virtual void SecondaryIdentCalib();
   void SecondaryAnalyseGroup(KVGroup* grp);

   ClassDef(KVINDRAReconEvent, 6)       //Event reconstructed from energy losses in INDRA array
};

#endif
