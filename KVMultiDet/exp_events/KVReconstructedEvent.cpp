/***************************************************************************
                          kvreconstructedevent.cpp  -  description
                             -------------------
    begin                : March 11th 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVReconstructedEvent.h"
#include "KVDetectorEvent.h"
#include "KVGroup.h"
#include "Riostream.h"
#include "KVReconstructedNucleus.h"
#include "KVTelescope.h"
#include "KVDetector.h"
#include "KVTarget.h"
#include "KVMultiDetArray.h"
#include <iomanip>

using namespace std;

ClassImp(KVReconstructedEvent);



void KVReconstructedEvent::init()
{
   //default initialisations
   UseRandomAngles();
   fPartSeedCond = "all";
}

KVReconstructedEvent::KVReconstructedEvent(Int_t mult): KVTemplateEvent(mult)
{
   init();
   CustomStreamer();            //because KVReconstructedNucleus has a customised streamer
}

//_______________________________________________________________________________________//

void KVReconstructedEvent::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVReconstructedEvent.
   //
   // If the KVMultiDetArray corresponding to the event has been built, we:
   //
   //  - set the values of raw data parameters in the corresponding detectors
   //
   //  - set the particles' angles depending on whether mean or random angles
   //    are wanted (fMeanAngles = kTRUE or kFALSE)

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVReconstructedEvent::Class(), this);
      // if the multidetector object exists, update some informations
      // concerning the detectors etc. hit by this particle
      if (gMultiDetArray) {
         // reset raw data in detectors if found in parameter list
         gMultiDetArray->SetRawDataFromReconEvent(fParameters);
         //set angles
         KVReconstructedNucleus* par;
         for (KVReconstructedEvent::Iterator it = begin(); it != end(); ++it) {
            par = it.get_pointer();
            if (HasMeanAngles())
               par->GetAnglesFromReconstructionTrajectory("mean");
            else
               par->GetAnglesFromReconstructionTrajectory("random");
            //reconstruct fAnalStatus information for unidentified KVReconstructedNucleus
            if (!par->IsIdentified() && par->GetStatus() == 99)        //AnalStatus has not been set for particles in group
               if (par->GetGroup())
                  KVReconstructedNucleus::AnalyseParticlesInGroup(par->GetGroup());
            // apply identification & calibration code selection
            gMultiDetArray->AcceptParticleForAnalysis(par);
         }
      }
   }
   else {
      R__b.WriteClassBuffer(KVReconstructedEvent::Class(), this);
   }
}


//______________________________________________________________________________________//

Bool_t KVReconstructedEvent::AnalyseDetectors(TList* kvtl)
{
   // Loop over detectors in list
   // if any detector has fired, start construction of new detected particle
   // More precisely: If detector has fired,
   // making sure fired detector hasn't already been used to reconstruct
   // a particle, then we create and fill a new detected particle.
   // In order to avoid creating spurious particles when reading data,
   // by default we ask that ALL coder values be non-zero here i.e. data and time-marker.
   // This can be changed by calling SetPartSeedCond("any"): in this case,
   // particles will be reconstructed starting from detectors with at least 1 fired parameter.

   KVDetector* d;
   TIter next(kvtl);
   while ((d = (KVDetector*)next())) {
      /*
          If detector has fired,
          making sure fired detector hasn't already been used to reconstruct
          a particle, then we create and fill a new detected particle.
      */
      if ((d->Fired(fPartSeedCond.Data()) && !d->IsAnalysed())) {

         KVReconstructedNucleus* kvdp = AddParticle();
         //add all active detector layers in front of this one
         //to the detected particle's list
         kvdp->Reconstruct(d);

         //set detector state so it will not be used again
         d->SetAnalysed(kTRUE);
      }
   }

   return kTRUE;
}

//______________________________________________________________________________

void KVReconstructedEvent::Print(Option_t* option) const
{
   //Print out list of particles in the event.
   //If option="ok" only particles with IsOK=kTRUE are included.

   cout << "     ***//***  RECONSTRUCTED EVENT #" << GetNumber() << "  ***//***" << endl;
   cout << GetTitle() << endl;  //system
   cout << GetName() << endl;   //run
   cout << "MULTIPLICITY = " << ((KVReconstructedEvent*) this)->
        GetMult(option) << endl << endl;

   KVReconstructedNucleus* frag = 0;
   int i = 0;
   ((KVReconstructedEvent*) this)->ResetGetNextParticle();
   while ((frag =
              ((KVReconstructedEvent*) this)->GetNextParticle(option))) {
      cout << "RECONSTRUCTED PARTICLE #" << ++i << endl;
      frag->Print();
      cout << endl;
   }

}

void KVReconstructedEvent::ls(Option_t*) const
{
   // compact listing of reconstructed event
   printf(":::%s   #%07d    M=%03d\n", ClassName(), GetNumber(), GetMult());
   GetParameters()->Print();
   int i(0);
   for (KVReconstructedEvent::Iterator it = begin(); it != end(); ++it) {
      KVReconstructedNucleus& nuc = it.get_reference();
      printf(" %3d", i);
      nuc.ls();
      ++i;
   }
}

//____________________________________________________________________________

void KVReconstructedEvent::IdentifyEvent()
{
   //All particles which have not been previously identified (IsIdentified=kFALSE), and which
   //may be identified independently of all other particles in their group according to the 1st
   //order coherency analysis (KVReconstructedNucleus::GetStatus=0), will be identified.
   //Particles stopping in first member of a telescope (KVReconstructedNucleus::GetStatus=3) will
   //have their Z estimated from the energy loss in the detector (if calibrated).

   KVReconstructedNucleus* d;
   while ((d = GetNextParticle())) {
      if (!d->IsIdentified()) {
         if (d->GetStatus() == KVReconstructedNucleus::kStatusOK) {
            // identifiable particles
            d->Identify();
         }
         else if (d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
            // particles stopped in first member of a telescope
            // estimation of Z (minimum) from energy loss (if detector is calibrated)
            UInt_t zmin = d->GetStoppingDetector()->FindZmin(-1., d->GetMassFormula());
            if (zmin) {
               d->SetZ(zmin);
               d->SetIsIdentified();
               // "Identifying" telescope is taken from list of ID telescopes
               // to which stopping detector belongs
               d->SetIdentifyingTelescope((KVIDTelescope*)d->GetStoppingDetector()->GetIDTelescopes()->At(0));
            }
         }
      }
   }
}

//_____________________________________________________________________________

void KVReconstructedEvent::CalibrateEvent()
{
   // Calculate and set energies of all identified particles in event.
   //
   // This will call the KVReconstructedNucleus::Calibrate() method of each
   // uncalibrated particle (those for which KVReconstructedNucleus::IsCalibrated()
   // returns kFALSE).
   //
   // In order to make sure that target energy loss corrections are correctly
   // calculated, we first set the state of the target in the current multidetector

   KVTarget* t = gMultiDetArray->GetTarget();
   if (t) {
      t->SetIncoming(kFALSE);
      t->SetOutgoing(kTRUE);
   }

   KVReconstructedNucleus* d;

   while ((d = GetNextParticle())) {

      if (d->IsIdentified() && !d->IsCalibrated()) {
         d->Calibrate();
      }

   }

}

void KVReconstructedEvent::MergeEventFragments(TCollection* events, Option_t* opt)
{
   // Merge all events in the list into one event (this one)
   // First we clear this event, then we fill it with copies of each particle in each event
   // in the list.
   // If option "opt" is given, it is given as argument to each call to
   // KVEvent::Clear() - this option is then passed on to the Clear()
   // method of each particle in each event.
   // NOTE: the events in the list will be empty and useless after this!
   //
   // This method overrides the one defined in KVEvent. As KVReconstructedNucleus objects
   // are referenced by the detectors used in their reconstruction, we also have to update
   // the references in the detectors otherwise they will still reference the particles
   // in the events in the list, which will no longer be valid after this operation.

   Clear(opt);
   TIter it(events);
   KVReconstructedEvent* e;
   while ((e = (KVReconstructedEvent*)it())) {
      KVReconstructedNucleus* n;
      if (e->GetMult()) {
         e->ResetGetNextParticle();
         while ((n = e->GetNextParticle())) {
            AddParticle()->CopyAndMoveReferences(n);
         }
      }
      GetParameters()->Merge(*(e->GetParameters()));
      e->Clear(opt);
   }
}



