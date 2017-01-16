//Created by KVClassFactory on Mon Jan 16 11:36:31 2017
//Author: Quentin Fable,,,

#include "KVVAMOSDataCorrection_e503.h"
#include "KVIDTelescope.h"
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"

ClassImp(KVVAMOSDataCorrection_e503)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSDataCorrection_e503</h2>
<h4>A KVVAMOSDataCorrection derived class specific to e503 experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVVAMOSDataCorrection_e503::KVVAMOSDataCorrection_e503()
{
   // Default constructor
   flist_aoq_cut_sicsi = new KVList();
   flist_aoq_cut_icsi = new KVList();

   ftof_corr_sicsi = ftof_corr_icsi = -666.;

   FindToFCorrectionAoQDuplicationSiCsI();
   FindToFCorrectionAoQDuplicationICSi();
   CreateAoQDuplicationCutsSiCsI();
   CreateAoQDuplicationCutsICSi();
}

//____________________________________________________________________________//

KVVAMOSDataCorrection_e503::~KVVAMOSDataCorrection_e503()
{
   // Destructor
   if (flist_aoq_cut_sicsi) {
      delete flist_aoq_cut_sicsi;
      flist_aoq_cut_sicsi = NULL;
   }

   if (flist_aoq_cut_icsi) {
      delete flist_aoq_cut_icsi;
      flist_aoq_cut_icsi = NULL;
   }
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ApplyCorrections(KVVAMOSReconNuc* nuc)
{
   assert(nuc);
   Bool_t kst_aoq_corr = kFALSE;

   Int_t IDCode = (int) nuc->GetCodes().GetFPCodeIndex();

   //mass:charge duplication corrections
   if (IDCode == 3) kst_aoq_corr = ApplyAoverQDuplicationCorrections(nuc, flist_aoq_cut_sicsi);
   if (IDCode == 4) kst_aoq_corr = ApplyAoverQDuplicationCorrections(nuc, flist_aoq_cut_icsi);

   //other corrections


   //return kst_aoq_corr;
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::FindToFCorrectionAoQDuplicationSiCsI()
{
   //Find the ToF correction (in ns) to apply to the VAMOS nuclei
   //inside the 'flist_aoq_cut_sicsi' list
   //( see also CreateAoQDuplicationCutsSiCsI() method ).

   KVDBParameterSet* tof_corr(static_cast<KVDBParameterSet*>(fRecords->FindObject("tof_aoq_sicsi")));

   if (!tof_corr) {
      return;
   }

   ftof_corr_sicsi = tof_corr->GetParameter(0);
}

void KVVAMOSDataCorrection_e503::FindToFCorrectionAoQDuplicationICSi()
{
   //Find the ToF correction (in ns) to apply to the VAMOS nuclei
   //inside the 'flist_aoq_cut_icsi' list
   //( see also CreateAoQDuplicationCutsICSi() method ).

   KVDBParameterSet* tof_corr(static_cast<KVDBParameterSet*>(fRecords->FindObject("tof_aoq_icsi")));

   if (!tof_corr) {
      return;
   }

   ftof_corr_icsi = tof_corr->GetParameter(0);
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::CreateAoQDuplicationCutsSiCsI()
{
   //Create the list of TCutG* to use for the current run for the
   //mass:charge ratio duplication correction for nucleus identified with
   //KVIDHarpeeSiCsI_e503 telescopes.

   //Loop over fRecords, the set of correction parameters records,
   //in order to find the sicsi duplication corrections.
   //Create TCutG* objects from them, and finally add the cuts in a list.
   TIter next_par_set(fRecords);
   KVDBParameterSet* par_set = NULL;
   Int_t par_set_num = 0;
   while ((par_set = dynamic_cast<KVDBParameterSet*>(next_par_set()))) {
      TString ss = par_set->GetName();
      if (ss.BeginsWith("duplication_aoq_sicsi")) {
         //create the corresponding cut
         TCutG* new_cut = new TCutG(Form("cut_aoq_sicsi%d", par_set_num), par_set->GetParamNumber());
         Int_t ii = 0;
         while ((ii != par_set->GetParamNumber() / 2.)) {
            new_cut->SetPoint(ii, par_set->GetParameter(ii * 2), par_set->GetParameter(ii * 2 + 1));
            ii++;
         }

         flist_aoq_cut_sicsi->Add(new_cut);
         par_set_num++;
      }
   }
}

//___________________________________________________________________________//
void KVVAMOSDataCorrection_e503::CreateAoQDuplicationCutsICSi()
{
   //Create the list of TCutG* to use for the current run for the
   //mass:charge ratio duplication correction for nucleus identified with
   //KVIDHarpeeICSi_e503 telescopes.

   //Loop over fRecords, the set of correction parameters records,
   //in order to find the one for sicsi duplication corrections.
   //Create TCutG* objects from them, add the cuts in a list.
   TIter next_par_set(fRecords);
   KVDBParameterSet* par_set = NULL;
   Int_t par_set_num = 0;
   while ((par_set = dynamic_cast<KVDBParameterSet*>(next_par_set()))) {
      TString ss = par_set->GetName();
      if (ss.BeginsWith("duplication_aoq_icsi")) {
         //create the corresponding cut
         TCutG* new_cut = new TCutG(Form("cut_aoq_icsi%d", par_set_num), par_set->GetParamNumber());
         Int_t ii = 0;
         while ((ii != par_set->GetParamNumber() / 2.)) {
            new_cut->SetPoint(ii, par_set->GetParameter(ii * 2), par_set->GetParameter(ii * 2 + 1));
            ii++;
         }

         flist_aoq_cut_icsi->Add(new_cut);
         par_set_num++;
      }
   }
}

//___________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::ApplyAoverQDuplicationCorrections(KVVAMOSReconNuc* nuc, KVList* aoq_cut)
{
   //Check if the provided nucleus, identified by either KVIDHarpeeSiCsI_e503 or
   //KVIDHarpeeICSi_e503 telescope, needs to be corrected for mass:charge
   //by comparing its (RealZ, RealAoQ) values to the cuts in 'flist_aoq_cut'.
   //Return kTRUE if correction applied.
   //Return kFALSE if no correction needed.

   assert(nuc);
   assert(aoq_cut);

   Float_t Z_nuc   = nuc->GetRealZ();
   Float_t AoQ_nuc = nuc->GetRealAoverQ();

   TIter next_cut(aoq_cut);
   TCutG* cut = NULL;
   while ((cut = dynamic_cast<TCutG*>(next_cut.Next()))) {
      if (cut->IsInside(Z_nuc, AoQ_nuc)) {//if nuc needs to be corrected
         //Find new value of mass:charge ratio
         Double_t new_AoQ = -666.;
         Double_t brho    = nuc->GetBrho();
         Double_t path    = -666.;
         Double_t tof     = -666.;
         nuc->GetCorrFlightDistanceAndTime(path, tof, "TSI_HF");
         Double_t time  = tof + ftof_corr_sicsi;
         Double_t beta  = path / time / KVParticle::C();
         Double_t gamma = 1.0 / TMath::Sqrt(1. - beta * beta);
         Double_t tmp   = beta * gamma;
         new_AoQ = brho * KVParticle::C() * 10. / tmp / KVNucleus::u();
         //Set new value of mass:charge
         nuc->SetRealAoverQ(new_AoQ);

         return kTRUE;
      }
   }

   return kFALSE; //no correction needed
}

//____________________________________________________________________________//

