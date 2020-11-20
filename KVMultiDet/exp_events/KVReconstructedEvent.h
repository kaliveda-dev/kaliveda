/***************************************************************************
                          kvreconstructedevent.h  -  description
                             -------------------
    begin                : March 11th 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVReconstructedEvent.h,v 1.11 2007/11/12 15:08:32 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVRECONEVENT_H
#define KVRECONEVENT_H

#include "KVTemplateEvent.h"
#include "KVGroup.h"
#include "KVReconstructedNucleus.h"
class KVDetectorEvent;
class TList;

/**
  \class KVReconstructedEvent
  \ingroup Reconstruction,NucEvents
  \brief Physical event reconstructed from data measured with a detector array using implemented identification and calibration procedures.
*/
class KVReconstructedEvent: public KVTemplateEvent<KVReconstructedNucleus> {

   Bool_t fMeanAngles;          //!kTRUE if particle momenta calculated using mean angles of detectors (default: randomised angles)
   TString fPartSeedCond;   //!condition used in AnalyseTelescopes for seeding new reconstructed particle

public:

   void init();
   KVReconstructedEvent(Int_t mult = 50);
   virtual ~ KVReconstructedEvent() {}

   virtual Bool_t AnalyseDetectors(TList* kvtl);
   virtual const Char_t* GetPartSeedCond() const
   {
      return fPartSeedCond;
   }
   virtual void SetPartSeedCond(const Char_t* cond)
   {
      fPartSeedCond = cond;
   }
   virtual void IdentifyEvent();
   virtual void CalibrateEvent();

   virtual void Print(Option_t* t = "") const;
   void ls(Option_t* = "") const;

   inline void UseMeanAngles()
   {
      fMeanAngles = kTRUE;
   }
   inline void UseRandomAngles()
   {
      fMeanAngles = kFALSE;
   }
   inline Bool_t HasMeanAngles()
   {
      return fMeanAngles;
   }
   inline Bool_t HasRandomAngles()
   {
      return !fMeanAngles;
   }

   virtual void SecondaryIdentCalib()
   {
      // Perform identifications and calibrations of particles not included
      // in first round (methods IdentifyEvent() and CalibrateEvent()).
   }
   void MergeEventFragments(TCollection*, Option_t* opt = "");

   ClassDef(KVReconstructedEvent, 3)    //Base class for reconstructed experimental multiparticle events
};

#endif
