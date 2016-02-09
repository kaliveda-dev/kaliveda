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

using namespace std;

ClassImp(KVReconstructedEvent);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Event reconstructed from energy losses in a multidetector using identification and
//calibration procedures.
//
//      GetParticle(Int_t i)       -  returns ith reconstructed particle of event (i=1,...,GetMult())
//      GetParticleWithName(const Char_t*)    -  returns the first particle with given name
//      GetParticle(const Char_t*)            -  returns the first particle belonging to a given group
//      UseMeanAngles()      - particle's theta & phi are taken from mean theta & phi of detectors
//      UseRandomAngles()      - particle's theta & phi are randomized within theta & phi limits of detectors
//      HasMeanAngles()/HasRandomAngles()   -  indicate in which of the two previous cases we find ourselves
//
//      GetNextParticle(Option_t* opt)  -
//              Use this method to iterate over the list of particles in the event
//              To make sure you begin at the start of the list, call ResetGetNextParticle()
//              before beginning the iteration.
//              After the last particle GetNextParticle() returns a null pointer and
//              resets itself ready for a new iteration over the particle list:
//
//              Example: (supposing "KVReconstructedEvent* event" points to a valid event)
//              KVReconstructedNucleus* rnuc; event->ResetGetNextParticle();
//              while( (rnuc = event->GetNextParticle()) ){
//                      ...
//              }
//
//              If opt="ok" only particles with KVReconstructedNucleus::IsOK() = kTRUE
//              are included in the iteration.

void KVReconstructedEvent::init()
{
   //default initialisations
   UseRandomAngles();
   fPartSeedCond = "all";
}

KVReconstructedEvent::KVReconstructedEvent(Int_t mult, const char
      *classname): KVEvent(mult,
                              classname)
{
   init();
   CustomStreamer();            //because KVReconstructedNucleus has a customised streamer
}

//_______________________________________________________________________________________//

void KVReconstructedEvent::Streamer(TBuffer& R__b)
{
   //Stream an object of class KVReconstructedEvent.
   //We set the particles' angles depending on whether mean or random angles
   //are wanted (fMeanAngles = kTRUE or kFALSE)

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVReconstructedEvent::Class(), this);
      // if the multidetector object exists, update some informations
      // concerning the detectors etc. hit by this particle
      if (gMultiDetArray) {
         //set angles
         KVReconstructedNucleus* par;
         while ((par = GetNextParticle())) {
            if (HasMeanAngles())
               par->GetAnglesFromStoppingDetector("mean");
            else
               par->GetAnglesFromStoppingDetector("random");
            //reconstruct fAnalStatus information for KVReconstructedNucleus
            if (par->GetStatus() == 99)        //AnalStatus has not been set for particles in group
               if (par->GetGroup())
                  KVReconstructedNucleus::AnalyseParticlesInGroup(par->GetGroup());
         }
      }
   } else {
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
   while ((frag =
              ((KVReconstructedEvent*) this)->GetNextParticle(option))) {
      cout << "RECONSTRUCTED PARTICLE #" << ++i << endl;
      frag->Print();
      cout << endl;
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
         } else if (d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
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

////////////////////////////////////////////////////////////////////////////////////////

