/*
$Id: KVIVRawDataReconstructor.cpp,v 1.5 2009/01/14 15:59:11 franklan Exp $
$Revision: 1.5 $
$Date: 2009/01/14 15:59:11 $
*/

//Created by KVClassFactory on Fri Jun  1 14:46:26 2007
//Author: John Frankland

#include "KVIVRawDataReconstructor.h"
#include "KVINDRA.h"
#include "KVVAMOS.h"
using namespace std;

ClassImp(KVIVRawDataReconstructor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIVRawDataReconstructor</h2>
<h4>Reconstructs raw data from INDRA-VAMOS experiments</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIVRawDataReconstructor::KVIVRawDataReconstructor()
{
   //Default constructor
   fIVevent    = NULL;
   fINDRADetEv = NULL;
   fVAMOSDetEv = NULL;
   fNbVAMOSrecon = 0;
}
//________________________________________________________________

KVIVRawDataReconstructor::~KVIVRawDataReconstructor()
{
   //Destructor
   fIVevent = NULL;
}
//________________________________________________________________

void KVIVRawDataReconstructor::InitRun()
{
   // Creates new ROOT file with TTree for reconstructed/calibrated events.
   // By default this file will be written in the same data repository as the raw data file we are reading.
   // This can be changed by setting the environment variable(s):
   //
   //     Reconstruction.DataAnalysisTask.OutputRepository:     [name of repository]
   //     [name of dataset].Reconstruction.DataAnalysisTask.OutputRepository:         [name of repository]
   //
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used. If neither is defined, the new file will be written in the same repository as
   // the raw file (if possible, i.e. if repository is not remote).

   // Create new KVIVReconEvent used to reconstruct & store events
   // The condition used to seed new reconstructed particles (see KVReconstructedEvent::AnalyseTelescopes)
   // is set by reading the value of the environment variables:
   //     Reconstruction.DataAnalysisTask.ParticleSeedCond:        [all/any]
   //     [name of dataset].Reconstruction.DataAnalysisTask.ParticleSeedCond:     [all/any]
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used.

   if (!recev) recev = fIVevent = new KVIVReconEvent;
   KVINDRARawDataReconstructor::InitRun();

   //Remove the first branch 'INDRAReconEvent' of tree created by KVINDRARawDataReconstructor::InitRun()
   //and replace it by new branch 'IVReconEvent' with a KVIVReconEvent object
   TObjArray* branches = tree->GetListOfBranches();
   delete branches->RemoveAt(0);
   branches->Compress();
   KVEvent::MakeEventBranch(tree, "IVReconEvent", "KVIVReconEvent", &fIVevent);

   //Detector events for INDRA and VAMOS
   fINDRADetEv = new KVDetectorEvent;
   fVAMOSDetEv = new KVDetectorEvent;

   //initialise number of reconstructed VAMOS events
   fNbVAMOSrecon = 0;
}
//________________________________________________________________

void KVIVRawDataReconstructor::preAnalysis()
{
   // Before the reconstruction of a new event:
   //  - Fill the detector events for INDRA and VAMOS.
   //  - Initialize VAMOS spectrometer

   KVINDRARawDataReconstructor::preAnalysis();

   KVSeqCollection* fired = fRunFile->GetFiredDataParameters();
   gIndra->GetDetectorEvent(fINDRADetEv, fired);
   gVamos->GetDetectorEvent(fVAMOSDetEv, fired);

   // INDRA event reconstruction
   // Exchange fDetEv with fINDRADetEv in order that GetDetectorEvent()
   // returns the INDRA event for KVINDRARawDataReconstructor::Analysis()
   KVDetectorEvent* tmp = fDetEv;
   fDetEv = fINDRADetEv;
   fINDRADetEv = tmp;


   gVamos->Initialize();
}
//________________________________________________________________

Bool_t KVIVRawDataReconstructor::Analysis()
{
   // Analysis of event measured with the INDRA multidetector array and the VAMOS
   // spectrometer.
   //
   // First, the reconstruction of the VAMOS event is done and then the INDRA event is
   // reconstructed by calling the Analysis method of the mother class
   // KVINDRARawDataReconstructor;

   // VAMOS event reconstruction
   if (fVAMOSDetEv->GetMult() > 0) {
      fIVevent->ReconstructVAMOSEvent(gVamos, fVAMOSDetEv);
      fNbVAMOSrecon++;
   }

   // INDRA event reconstruction
   return  KVINDRARawDataReconstructor::Analysis();
}
//________________________________________________________________

void  KVIVRawDataReconstructor::postAnalysis()
{
   // Clear the detector events of INDRA and VAMOS. It is very
   // important to do that in postAnalysis and not in the preAnalysis
   // method because calling KVDetectorEvent::Clear() will also clear
   // the ACQ parameters of the detectors of this event.
   fINDRADetEv->Clear();
   fVAMOSDetEv->Clear();
}


//________________________________________________________________

void KVIVRawDataReconstructor::EndRun()
{
   fIVevent    = NULL;
   SafeDelete(fINDRADetEv);
   SafeDelete(fVAMOSDetEv);

   cout << endl << " *** Number of reconstructed VAMOS events : "
        << fNbVAMOSrecon << " ***" << endl;

   KVINDRARawDataReconstructor::EndRun();
}
