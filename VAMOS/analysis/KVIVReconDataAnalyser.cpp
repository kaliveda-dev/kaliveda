//Created by KVClassFactory on Mon Feb 25 09:01:43 2013
//Author: Guilain ADEMARD

#include "KVIVReconDataAnalyser.h"
#include "KVVAMOS.h"
#include "KVINDRADB.h"

ClassImp(KVIVReconDataAnalyser)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIVReconDataAnalyser</h2>
<h4>For analysing reconstructed INDRA+VAMOS data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIVReconDataAnalyser::KVIVReconDataAnalyser()
{
   // Default constructor
}
//________________________________________________________________

KVIVReconDataAnalyser::~KVIVReconDataAnalyser()
{
   // Destructor
}
//________________________________________________________________

void KVIVReconDataAnalyser::preInitRun()
{
   // Called by currently-processed KVSelector when a new file in the TChain is opened.
   // We call gMultiDetArray->SetParameters for the current run.

   Int_t run = GetRunNumberFromFileName(theChain->GetCurrentFile()->GetName());
   gMultiDetArray->SetParameters(run);
   KVINDRADBRun* CurrentRun = gIndraDB->GetRun(run);
   SetSelectorCurrentRun(CurrentRun);
   cout << endl << " ===================  New Run  =================== " <<
        endl << endl;

   CurrentRun->Print();
   if (CurrentRun->GetSystem()) {
      if (CurrentRun->GetSystem()->GetKinematics())
         CurrentRun->GetSystem()->GetKinematics()->Print();
   }

   cout << endl << " ================================================= " <<
        endl << endl;
   ConnectRawDataTree();
   PrintTreeInfos();
}
//________________________________________________________________

void KVIVReconDataAnalyser::preAnalysis()
{
   // Read and set raw data for the current reconstructed event.
   // Initialize VAMOS spectrometer before starting the analysis
   // of the new event.
   //
   // This method overwrites the same method of the mother class
   // KVINDRAReconDataAnalyser where gIndra is replaced by gMultiDetArray
   // in order to read and set raw data of VAMOS as well as the ones of
   // INDRA. Indeed the ACQ parameter list of gMultiDetArray contains the
   // parameters of both multidetectors. (for more information see the
   // class KVINDRA_VAMOS)

   if (!theRawData) return;
   // all recon events are numbered 1, 2, ... : therefore entry number is N-1
   Long64_t rawEntry = GetRawEntryNumber();

   TIter it_acqpar(gMultiDetArray->GetACQParams());
   TObject* acqpar;
   while ((acqpar = it_acqpar())) acqpar->Clear();

   theRawData->GetEntry(rawEntry);
   for (int i = 0; i < NbParFired; i++) {
      KVEBYEDAT_ACQParam* par = gMultiDetArray->GetACQParam((*parList)[ParNum[i]]->GetName());
      if (par) {
         par->SetData(ParVal[i]);
      }
   }

   if (gVamos) gVamos->Initialize();
}
