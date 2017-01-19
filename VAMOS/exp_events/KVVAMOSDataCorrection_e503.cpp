//Created by KVClassFactory on Mon Jan 16 11:36:31 2017
//Author: Quentin Fable,,,

#include "KVVAMOSDataCorrection_e503.h"
#include "KVIDTelescope.h"
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"
#include "KVParticle.h"
#include "KVNucleus.h"

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
   fkverbose      = kFALSE;
   fkInitialised  = kFALSE;
   fkListSiCsI_OK = kFALSE;
   fkListICSi_OK  = kFALSE;
   fkToFSiCsI_OK  = kFALSE;
   fkToFICSi_OK   = kFALSE;
}

//____________________________________________________________________________//
KVVAMOSDataCorrection_e503::~KVVAMOSDataCorrection_e503()
{
#if __cplusplus < 201103L
   if (flist_aoq_cut_sicsi) {
      delete flist_aoq_cut_sicsi;
      flist_aoq_cut_sicsi = NULL;
   }

   if (flist_aoq_cut_icsi) {
      delete flist_aoq_cut_icsi;
      flist_aoq_cut_icsi = NULL;
   }
#endif
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::Copy(TObject& obj) const
{
   // Call parent class copy method (takes care of the inherited stuff)
   KVVAMOSDataCorrection::Copy(obj);

   // Then copy any desired member variables (not copied by the parent method)
   //KVVAMOSDataCorrection_e503 *casted_obj = static_cast<KVVAMOSDataCorrection_e503*>(&obj);
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::Init()
{
   //Initialisation of the correction class:
   //-Creates the cuts (from fRecords) from which the data should be modified
   //-Finds ToF corrections (from fRecords) to apply to mass:charge ratios
   // in the above defined cuts
   //
   // WARNING : fRecords must be defined before calling this method
   //           (see SetIDCorrectionParameters() method ) or nothing
   //           will be done.

   //if already init
   if (fkInitialised) return kTRUE;

   //if records not set/found
   if (!fRecords) {
      Info("Init", "... fRecords is empty, no corrections to apply ...");
      return kFALSE;
   }

   //if ok
   Info("Init", "... initiliasing correction class ...");

   ftof_corr_sicsi = ftof_corr_icsi = -666.;

   fkToFSiCsI_OK  = FindToFCorrectionAoQDuplicationSiCsI();
   fkToFICSi_OK   = FindToFCorrectionAoQDuplicationICSi();
   fkListSiCsI_OK = CreateAoQDuplicationCutsSiCsI();
   fkListICSi_OK  = CreateAoQDuplicationCutsICSi();

   //debug
   if (fkverbose) {
      Info("Init", "... printing ToF corrections then cuts lists to be applied ...");
      printf("[tof_sicsi=%e ; tof_icsi=%e]\n", ftof_corr_sicsi, ftof_corr_icsi);
      flist_aoq_cut_sicsi->ls();
      flist_aoq_cut_sicsi->Print();
      flist_aoq_cut_icsi->ls();
      flist_aoq_cut_icsi->Print();
   }

   fkInitialised = kTRUE;

   Info("Init", "... initiliasing of correction class done ...");

   return kTRUE;
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ApplyCorrections(KVVAMOSReconNuc* nuc)
{
   assert(nuc);
   Int_t IDCode = nuc->GetIDCode();

   if (fkverbose) Info("ApplyCorrections", "... trying to apply e503 corrections to nucleus (IDCode=%d) ...", IDCode);

   //mass:charge duplication corrections
   if (IDCode == 3 && fkListSiCsI_OK && fkToFSiCsI_OK) {
      KVVAMOSDataCorrection_e503::ApplyAoverQDuplicationCorrections(nuc, flist_aoq_cut_sicsi);
   }
   if (IDCode == 4 && fkListICSi_OK && fkToFICSi_OK) {
      KVVAMOSDataCorrection_e503::ApplyAoverQDuplicationCorrections(nuc, flist_aoq_cut_icsi);
   }

   return;
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::FindToFCorrectionAoQDuplicationSiCsI()
{
   //Find the ToF correction (in ns) to apply to the VAMOS nuclei
   //inside the 'flist_aoq_cut_sicsi' list
   //( see also CreateAoQDuplicationCutsSiCsI() method ).

   KVDBParameterSet* tof_corr(static_cast<KVDBParameterSet*>(fRecords->FindObject("tof_aoq_corr_sicsi")));

   if (!tof_corr) {
      if (fkverbose) Error("FindToFCorrectionAoQDuplicationSiCsI", "... No ToF parameter found, was fRecords set ? ...");
      return kFALSE;
   }

   ftof_corr_sicsi = tof_corr->GetParameter(0);
   return kTRUE;
}

Bool_t KVVAMOSDataCorrection_e503::FindToFCorrectionAoQDuplicationICSi()
{
   //Find the ToF correction (in ns) to apply to the VAMOS nuclei
   //inside the 'flist_aoq_cut_icsi' list
   //( see also CreateAoQDuplicationCutsICSi() method ).

   KVDBParameterSet* tof_corr(static_cast<KVDBParameterSet*>(fRecords->FindObject("tof_aoq_corr_icsi")));

   if (!tof_corr) {
      if (fkverbose) Error("FindToFCorrectionAoQDuplicationICSi", "... No ToF parameter found, was fRecords set ? ...");
      return kFALSE;
   }

   ftof_corr_icsi = tof_corr->GetParameter(0);
   return kTRUE;
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::CreateAoQDuplicationCutsSiCsI()
{
   //Create the list of TCutG* to use for the current run for the
   //mass:charge ratio duplication correction for nucleus identified with
   //KVIDHarpeeSiCsI_e503 telescopes.

   //Loop over fRecords, the set of correction parameters records,
   //in order to find the sicsi duplication corrections.
   //Create TCutG* objects from them, and finally add the cuts in a list.

   Bool_t flag_ok = kFALSE; //flag set to kTRUE if record associated to Si-CsI duplication exists in fRecords

#if __cplusplus < 201103L
   flist_aoq_cut_sicsi = new KVList(kFALSE);
#else
   flist_aoq_cut_sicsi.reset(new KVList(kFALSE));
#endif

   TIter next_par_set(fRecords);
   KVDBParameterSet* par_set = NULL;
   Int_t par_set_num = 0;
   while ((par_set = dynamic_cast<KVDBParameterSet*>(next_par_set()))) {
      TString ss = par_set->GetName();

      if (ss.BeginsWith("duplication_aoq_sicsi")) {
         //modify the flag
         flag_ok = kTRUE;
         //create the corresponding cut
         TCutG* new_cut = new TCutG(Form("cut_aoq_sicsi%d", par_set_num), par_set->GetParamNumber() / 2.);
         Int_t ii = 0;
         while ((ii != par_set->GetParamNumber() / 2.)) {
            new_cut->SetPoint(ii, par_set->GetParameter(ii * 2), par_set->GetParameter(ii * 2 + 1));

            //debug
//            if(fkverbose){
//               std::cout << "ii=" << ii << "/" << (par_set->GetParamNumber()/2.) << std::endl;
//               new_cut->Print();
//            }

            ii++;
         }

         flist_aoq_cut_sicsi->Add(new_cut);
         par_set_num++;
      }
   }

   return flag_ok;
}

//___________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::CreateAoQDuplicationCutsICSi()
{
   //Create the list of TCutG* to use for the current run for the
   //mass:charge ratio duplication correction for nucleus identified with
   //KVIDHarpeeICSi_e503 telescopes.

   //Loop over fRecords, the set of correction parameters records,
   //in order to find the one for sicsi duplication corrections.
   //Create TCutG* objects from them, add the cuts in a list.

   Bool_t flag_ok = kFALSE; //flag set to kTRUE if record associated to IC-Si duplication exists in fRecords

#if __cplusplus < 201103L
   flist_aoq_cut_icsi = new KVList(kFALSE);
#else
   flist_aoq_cut_icsi.reset(new KVList(kFALSE));
#endif

   TIter next_par_set(fRecords);
   KVDBParameterSet* par_set = NULL;
   Int_t par_set_num = 0;
   while ((par_set = dynamic_cast<KVDBParameterSet*>(next_par_set()))) {
      TString ss = par_set->GetName();
      if (ss.BeginsWith("duplication_aoq_icsi")) {
         //modify the flag
         flag_ok = kTRUE;
         //create the corresponding cut
         TCutG* new_cut = new TCutG(Form("cut_aoq_icsi%d", par_set_num), par_set->GetParamNumber() / 2.);
         Int_t ii = 0;
         while ((ii != par_set->GetParamNumber() / 2.)) {
            new_cut->SetPoint(ii, par_set->GetParameter(ii * 2), par_set->GetParameter(ii * 2 + 1));

            //debug
//            if(fkverbose){
//                std::cout << "ii=" << ii << "/" << (par_set->GetParamNumber()/2.) << std::endl;
//                new_cut->Print();
//            }

            ii++;
         }

         flist_aoq_cut_icsi->Add(new_cut);
         par_set_num++;
      }
   }

   return flag_ok;
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

   if (fkverbose) Info("ApplyAoverQDuplicationCorrections", "... starting AoQ corrections for the nucleus ...");

   Float_t Z_nuc   = nuc->GetRealZ();
   Float_t AoQ_nuc = nuc->GetRealAoverQ();

   TIter next_cut(aoq_cut);
   TCutG* cut = NULL;
   while ((cut = dynamic_cast<TCutG*>(next_cut.Next()))) {
      if (cut->IsInside(AoQ_nuc, Z_nuc)) {//if nuc needs to be corrected
         //Find new value of mass:charge ratio
         Double_t new_AoQ = -666.;
         Double_t brho    = nuc->GetBrho();
         Double_t path    = -666.;
         Double_t tof     = -666.;
         const Char_t* tof_name = "TSI_HF";
         nuc->GetCorrFlightDistanceAndTime(path, tof, tof_name);
         Double_t time  = tof + ftof_corr_sicsi;
         Double_t beta  = path / time / KVParticle::C();
         Double_t gamma = 1.0 / TMath::Sqrt(1. - beta * beta);
         Double_t tmp   = beta * gamma;
         new_AoQ = brho * KVParticle::C() * 10. / tmp / KVNucleus::u();

         //Set new value of mass:charge
         //nuc->SetRealAoverQ(new_AoQ);

         if (fkverbose) Info("ApplyAoverQDuplicationCorrections", "... finishing AoQ corrections [AoQ_old=%e, AoQ_new=%e] ...", AoQ_nuc, new_AoQ);
         return kTRUE;
      }
   }

   if (fkverbose) Info("ApplyAoverQDuplicationCorrections", "... finishing AoQ corrections, did nothing ...");
   return kFALSE; //no correction needed
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::SetVerbose(Bool_t status)
{
   fkverbose = status;
}
