//Created by KVClassFactory on Mon Feb 13 18:45:07 2017
//Author: Quentin Fable,,,

#include "KVVAMOSDataCorrection_e503.h"
#include "KVIDTelescope.h"
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"
#include "KVParticle.h"
#include "KVNucleus.h"
#include "TCutG.h"
#include "TKey.h"

ClassImp(KVVAMOSDataCorrection_e503)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSDataCorrection_e503</h2>
<h4>Base class to use for VAMOS data corrections for e503 experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//____________________________________________________________________________//
KVVAMOSDataCorrection_e503::KVVAMOSDataCorrection_e503(Int_t run_number = -1) : KVVAMOSDataCorrection(run_number)
{
   // Default constructor
   fkverbose   = kFALSE;
   fkIsInit    = kFALSE;

   // HF frequency correction
   flist_HFcuts_sicsi = new KVHashList;
   flist_HFcuts_sicsi->SetOwner(kTRUE);

   flist_HFcuts_icsi = new KVHashList;
   flist_HFcuts_icsi->SetOwner(kTRUE);

   //ToF duplications corrections
   flist_aoq_cut_sicsi = new KVHashList;
   flist_aoq_cut_sicsi->SetOwner(kTRUE);
   for (Int_t ii = 1; ii <= 18; ii++) {
      //we add in the main
      //18 lists of cuts, each list
      //for each Si detectors
      flist_aoq_cut_sicsi->Add(new KVHashList);
   }

   flist_aoq_cut_icsi = new KVHashList;
   flist_aoq_cut_icsi->SetOwner(kTRUE);
   for (Int_t ii = 1; ii <= 18; ii++) {
      //we add in the main
      //18 lists of cuts, each list
      //for each Si detectors
      flist_aoq_cut_icsi->Add(new KVHashList);
   }
}

//____________________________________________________________________________//
KVVAMOSDataCorrection_e503::~KVVAMOSDataCorrection_e503()
{
#if __cplusplus < 201103L
   if (flist_HFcuts_sicsi) {
      delete flist_HFcuts_sicsi;
      flist_HFcuts_sicsi = NULL;
   }

   if (flist_aoq_cut_sicsi) {
      delete flist_aoq_cut_sicsi;
      flist_aoq_cut_sicsi = NULL;
   }

   if (flist_HFcuts_icsi) {
      delete flist_HFcuts_icsi;
      flist_HFcuts_icsi = NULL;
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
void KVVAMOSDataCorrection_e503::Init()
{
   //Initialisation of the correction class:
   //-Creates the cuts from which the data should be modified
   //-Finds ToF corrections to apply to mass:charge ratios
   // in the above defined cuts

   //if already init
   if (fkIsInit) {
      Info("Init", "... correction class already initialised...");
      return;
   }

   //if not already init
   else {
      if (fRunNumber != -1) {
         Info("Init", "... starting initiliasing correction class for run %d ...", fRunNumber);

         //HF frequency corrections
         fvec_nHF_sicsi.clear();
         fvec_nHF_icsi.clear();
         ReadHFCutFilesListInDataSet();

         //AoQ duplication corrections
         ftof_corr_sicsi = -666.;
         ftof_corr_icsi  = -666.;
         ReadDuplicationCutFilesListInDataSet();
         ReadDuplicationToFOffsetsInDataSet();

         //Global ToF offset for AoQ=2 misalignments
         ftof_offset = -666.;
         ReadAdditionalToFAndPathOffsetsInDataSet();

         Info("Init", "... initiliasing of correction class done ...");
         fkIsInit = kTRUE;

         //debug
         //if (fkverbose) {
         Info("Init", "... printing informations from initialisation ...\n");
         PrintInitInfos();
         //}
      }

      else Error("Init", "... no run number set , will do nothing ...");
   }
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadHFCutFilesListInDataSet()
{
   // Read the file containing TCutG to use
   // for e503 HF frequency corrections from
   // a file set in the dataset directory:
   // $KVROOT/KVFiles/INDRA_e503
   //
   // This file, specific to a given dataset, is found and
   // opened using its name given by the following environment
   // variable defined in $KVROOT/KVFiles/.kvrootrc:
   //
   // INDRA_e503.KVVAMOSDataCorrection_e503.HFCutListSiCsI:  [file_name]
   //
   // This file contains a list of *.root file names, each
   // of them associated to a system/range of runs.
   // Each of these *.root files contains more specificly
   // TCutG objects (in DeltaE, ToF) and a TEnv where is saved the run range
   // and the number of time N the beam pulse period must be added/remove
   // to apply to the VAMOS nucleus with
   // (DeltaE, ToF) inside the former cuts.

   //--------Si-CsI--------
   //Find the list file to use
   TString filename0 = gDataSet->GetDataSetEnv("KVVAMOSDataCorrection_e503.HFCutListSiCsI");
   if (filename0 == "") {
      Warning("ReadHFCutFileListInDataSet", "No filename defined for Si-CsI telescopes. It can be defined by %s.KVVAMOSDataCorrection_e503.HFCutListSiCsI", gDataSet->GetName());
      return;
   }
   std::ifstream ifile0;
   if (gDataSet->OpenDataSetFile(filename0.Data(), ifile0)) {
      Info("ReadHFCutFileListInDataSet", "list of cut files in: '%s' ", filename0.Data());
      ReadHFCutFileList(ifile0, 0);
      ifile0.close();
   }

   //--------IC-Si--------
   //Find the list file to use
   TString filename1 = gDataSet->GetDataSetEnv("KVVAMOSDataCorrection_e503.HFCutListICSi");
   if (filename1 == "") {
      Warning("ReadHFCutFilesListInDataSet", "No filename defined for IC-SI telescopes. It can be defined by %s.KVVAMOSDataCorrection_e503.HFCutListICSi", gDataSet->GetName());
      return;
   }
   std::ifstream ifile1;
   if (gDataSet->OpenDataSetFile(filename1.Data(), ifile1)) {
      Info("ReadHFCutFileListInDataSet", "list of cut files in: '%s' ", filename1.Data());
      ReadHFCutFileList(ifile1, 1);
      ifile1.close();
   }
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadHFCutFileList(std::ifstream& file, Int_t type)
{
   //Protected method to read the HF cuts list file for the dataset e503
   //Set 'type' to 0 for SiCsI telescopes and set 'type' to 1 for ICSi telescopes
   //For each file listed in this list file we:
   //- read the file TEnv and to access to the run range of the file
   //- if 'fRunNumber' is in the run range we save all associated cuts in
   //  'flist_HFcuts_type' list and stop the research.
   //- if 'fRunNumber' is in the run range we also save the values of N=
   //  number of time we add/remove the beam pulse in 'fvec_nHF_type'

   Bool_t found = kFALSE; //flag set to kTRUE as soon as we have found the good file

   std::string newline;
   while ((std::getline(file, newline)) && (!found)) {
      //Ignore comments in the file
      if (newline.compare(0, 1, "#") != 0) {

         TString fullpath;
         TFile* file = NULL;
         if (gDataSet->SearchKVFile(newline.c_str(), fullpath, gDataSet->GetName())) {
            file = TFile::Open(fullpath.Data());
            if (file) {

               if (fkverbose) {
                  Info("ReadHFCutFileList", "... file %s opened, infos follow ...", file->GetName());
                  file->ls();
               }

               //find the TEnv and access to runlist and tof_corr values
               TEnv* env = (TEnv*) file->Get("TEnv");
               assert(env);

               if (fkverbose) {
                  Info("ReadHFCutFileList", "... TEnv infos follow ...");
                  env->Print();
               }

               const Char_t* runlist = env->GetValue("runlist", "");
               if (fkverbose) {
                  Info("ReadHFCutFileList", "... run (=%d) | runlist of the file (=%s) ...",
                       fRunNumber, runlist);
               }

               //check if 'fRunNumber' is in run range
               KVNumberList nl(runlist);
               if (nl.Contains(fRunNumber)) {

                  if (fkverbose) {
                     Info("ReadHFCutFileList", "... run (=%d) is in the runlist of the file (=%s) ...",
                          fRunNumber, runlist);
                  }

                  //exctract and set cuts list
                  TKey* key = NULL;
                  TList* list = file->GetListOfKeys();
                  assert(list);
                  TIter it(list);
                  while ((key = (TKey*) it.Next())) {
                     if ((key->ReadObj())->InheritsFrom("TCutG")) {
                        const TCutG* cutg = (TCutG*) key->ReadObj();
                        TCutG* copy_cut = new TCutG(*cutg);
                        if (type == 0) flist_HFcuts_sicsi->Add(copy_cut);
                        else if (type == 1) flist_HFcuts_icsi->Add(copy_cut);

                        //save associated nHF value
                        //it will work only if the cut saved in the file
                        //has a name 'cut%d' (returned by GetName() method)
                        TString ss        = cutg->GetName();
                        TObjArray* obj_ar = ss.Tokenize("cut");
                        Int_t nn          = (((TObjString*) obj_ar->At(0))->String()).Atoi();

                        if (fkverbose) Info("ReadHFCutFileList", "... cut name = '%s' -> associated nHF found =%d ...", cutg->GetName(), nn);

                        if (type == 0) fvec_nHF_sicsi.push_back(nn);
                        if (type == 1) fvec_nHF_icsi.push_back(nn);
                     }
                  }

                  found = kTRUE;
               }

               file->Close();
            }//end of file is ok
         }//end of SearchKVFile
      }//end of line ok
   }//all lines read
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadDuplicationCutFilesListInDataSet()
{
   // Read the file containing TCutG to use
   // for e503 duplication corrections from
   // a file set in the dataset directory:
   // $KVROOT/KVFiles/INDRA_e503
   //
   // This file, specific to a given dataset, is found and
   // opened using its name given by the following environment
   // variable defined in $KVROOT/KVFiles/.kvrootrc:
   //
   // INDRA_e503.KVVAMOSDataCorrection_e503.DuplicationCutListSiCsI:  [file_name]
   //
   // This file contains a list of *.root file names, each
   // of them associated to a system/range of runs.
   // Each of these *.root files contains more specificly
   // TCutG objects (in A/Q,Z) and a TEnv where is saved the run range
   // and the ToF correction (in ns) to apply to the VAMOS nucleus with
   // (A/Q, Z) inside the former cuts.

   //--------Si-CsI--------
   //Find the list file to use
   TString filename0 = gDataSet->GetDataSetEnv("KVVAMOSDataCorrection_e503.DuplicationCutListSiCsI");
   if (filename0 == "") {
      Warning("ReadDuplicationCutFilesListInDataSet", "No filename defined for Si-CsI telescopes. It can be defined by %s.KVVAMOSDataCorrection_e503.DuplicationCutListSiCsI", gDataSet->GetName());
      return;
   }
   std::ifstream ifile0;
   if (gDataSet->OpenDataSetFile(filename0.Data(), ifile0)) {
      Info("ReadDuplicationCutFilesListInDataSet", "list of Si-CsI cut files in: '%s' ", filename0.Data());
      ReadDuplicationCutFileList(ifile0, 0);
      ifile0.close();
   }

   //--------IC-Si--------
   //Find the list file to use
   TString filename1 = gDataSet->GetDataSetEnv("KVVAMOSDataCorrection_e503.DuplicationCutListICSi");
   if (filename1 == "") {
      Warning("ReadDuplicationCutFilesListInDataSet", "No filename defined for IC-SI telescopes. It can be defined by %s.KVVAMOSDataCorrection_e503.DuplicationCutListICSi", gDataSet->GetName());
      return;
   }
   std::ifstream ifile1;
   if (gDataSet->OpenDataSetFile(filename1.Data(), ifile1)) {
      Info("ReadDuplicationCutFileListInDataSet", "list of IC-Si cut files in: '%s' ", filename1.Data());
      ReadDuplicationCutFileList(ifile1, 1);
      ifile1.close();
   }
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadDuplicationCutFileList(std::ifstream& file, Int_t type)
{
   //Protected method to read the duplication cuts list file for the dataset e503
   //Set 'type' to 0 for SiCsI telescopes and set 'type' to 1 for ICSi telescopes
   //For each file listed in this list file we:
   //- read the file TEnv and to access to the run range of the file
   //- if 'fRunNumber' is in the run range we save all associated cuts in
   //  'flist_aoq_cut_sicsi' list and stop the research.
   //- if 'fRunNumber' is in the run range we also save the value of ToF correction

   Bool_t found = kFALSE; //flag set to kTRUE as soon as we have found the good file

   std::string newline;
   while ((std::getline(file, newline)) && (!found)) {
      //Ignore comments in the file
      if (newline.compare(0, 1, "#") != 0) {
         TString fullpath;
         TFile* file = NULL;
         if (gDataSet->SearchKVFile(newline.c_str(), fullpath, gDataSet->GetName())) {
            //debug
            //std::cout << fullpath.Data() << std::endl;

            file = TFile::Open(fullpath.Data());
            if (file) {

               if (fkverbose) {
                  Info("ReadDuplicationCutFileList", "... file %s opened, infos follow ...", file->GetName());
                  file->ls();
               }

               //find the TEnv and access to runlist and tof_corr values
               TEnv* env = (TEnv*) file->Get("TEnv");
               assert(env);

               if (fkverbose) {
                  Info("ReadDuplicationCutFileList", "... TEnv infos follow ...");
                  env->Print();
               }

               const Char_t* runlist = env->GetValue("runlist", "");
               if (fkverbose) {
                  Info("ReadDuplicationCutFileList", "... run (=%d) | runlist of the file (=%s) ...",
                       fRunNumber, runlist);
               }

               //check if 'fRunNumber' is in run range
               KVNumberList nl(runlist);
               if (nl.Contains(fRunNumber)) {

                  if (fkverbose) {
                     Info("ReadDuplicationCutFileList", "... run (=%d) is in the runlist of the file (=%s) ...",
                          fRunNumber, runlist);

                  }

                  //exctract cuts list
                  TKey* key = NULL;
                  TList* list = file->GetListOfKeys();
                  assert(list);
                  TIter it(list);

                  while ((key = (TKey*) it.Next())) {
                     if ((key->ReadObj())->InheritsFrom("TCutG")) {
                        const TCutG* cutg = (TCutG*) key->ReadObj();
                        TCutG* copy_cut = new TCutG(*cutg);

                        //save cuts associated to a given silicon
                        //into its corresponding list
                        //it will work only if the cut saved in the file
                        //has a name 'Si%d_cut%d' (returned by GetName() method)
                        TString ss        = cutg->GetName();
                        TObjArray* obj_ar = ss.Tokenize("_");
                        TString ss0       = ((TObjString*) obj_ar->At(0))->String();
                        TString ss1       = ((TObjString*) obj_ar->At(1))->String();
                        Int_t nSi         = ((TString)(ss0(2, 2))).Atoi();
                        Int_t nCut        = ((TString)(ss1(3, 1))).Atoi();

                        if (fkverbose) Info("ReadDuplicationCutFileList", "... cut name='%s' -> nSi=%d, nCut=%d ...", cutg->GetName(), nSi, nCut);

                        if (type == 0) {
                           KVHashList* ll0 = (KVHashList*) flist_aoq_cut_sicsi->At(nSi - 1);
                           assert(ll0);
                           ll0->Add(copy_cut);
                        }

                        else if (type == 1) {
                           KVHashList* ll1 = (KVHashList*) flist_aoq_cut_icsi->At(nSi - 1);
                           assert(ll1);
                           ll1->Add(copy_cut);
                        }
                     }
                  }

                  found = kTRUE;
               }

               file->Close();
            }//end of file is ok
         }//end of SearchKVFile
      }//end of line ok
   }//all lines read
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadDuplicationToFOffsetsInDataSet()
{
   //Find in '$KVROOT/KVFiles/VAMOS/etc/.kvrootrc' the values of the ToF offsets
   //to apply to correct ToF duplications due to a possible experimental
   //jitter when measuring the ToF.
   //Data with (RealZ, RealAoQ) inside the defined cuts
   //(see ReadDuplicationCutFilesListInDataSet() method) will be applied the offset.
   //
   //NOTE: these corrections are applied AFTER HF corrections
   //
   //By default this offset is equal to 1.25 ns.

   ftof_corr_sicsi = gDataSet->GetDataSetEnv("INDRA_e503.KVVAMOSDataCorrection_e503.DuplicationToFOffset", 1.25);  // in ns
   ftof_corr_icsi  = gDataSet->GetDataSetEnv("INDRA_e503.KVVAMOSDataCorrection_e503.DuplicationToFOffset", 1.25);  //in ns
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadAdditionalToFAndPathOffsetsInDataSet()
{
   //Find in '$KVROOT/KVFiles/VAMOS/etc/.kvrootrc' the values of general ToF offset
   //and also Path offset to apply to correct AoQ=2 misalignments.
   //All data will be applied these ToF and Path offsets.
   //
   //NOTE: these corrections are applied AFTER HF corrections and AFTER ToF duplication
   //corrections
   //

   ftof_offset  = gDataSet->GetDataSetEnv("INDRA_e503.KVVAMOSDataCorrection_e503.ToFOffset", 0.);  //in ns
   fpath_offset = gDataSet->GetDataSetEnv("INDRA_e503.KVVAMOSDataCorrection_e503.PathOffset", 0.); //in cm
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::ApplyCorrections(KVVAMOSReconNuc* nuc)
{
   assert(nuc);
   Int_t IDCode = nuc->GetIDCode();

   //First set all corrected values equal to the values found with the
   //basic (KVVAMOSReconNuc::IdentifyQandA()) identification routine.
   //We will then only modify the wanted one and all informations
   //for ID will be available in corrected observables
   nuc->SetCorrectedToF(nuc->GetBasicToF());
   nuc->SetCorrectedPath(nuc->GetBasicPath());
   nuc->SetCorrectedRealAE(nuc->GetBasicRealAE());
   nuc->SetCorrectedRealAoverQ(nuc->GetBasicRealAoverQ());

   if (fkverbose) Info("ApplyCorrections", "... trying to apply e503 corrections to nucleus (IDCode=%d) ...", IDCode);

   //Si-CsI telescopes corrections
   Bool_t kCsI_HFcorr = kFALSE;
   Bool_t kCsI_DUcorr = kFALSE;
   if (IDCode == 3) {
      kCsI_HFcorr = ApplyHFCorrections(nuc, flist_HFcuts_sicsi, fvec_nHF_sicsi);
      kCsI_DUcorr = ApplyToFDuplicationCorrections(nuc, flist_aoq_cut_sicsi, ftof_corr_sicsi);
   }

   //IC-Si telescopes corrections
   Bool_t kSi_HFcorr = kFALSE;
   Bool_t kSi_DUcorr = kFALSE;
   if (IDCode == 4) {
      kSi_HFcorr = ApplyHFCorrections(nuc, flist_HFcuts_icsi, fvec_nHF_icsi);
      kSi_DUcorr = ApplyToFDuplicationCorrections(nuc, flist_aoq_cut_icsi, ftof_corr_icsi);
   }

   //AoQ=2 misalignment corrections
   Bool_t koffset = ApplyToFAndPathOffset(nuc);

   if (kCsI_HFcorr || kCsI_DUcorr || kSi_HFcorr || kSi_DUcorr  || koffset) return kTRUE;
   else return kFALSE;
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::ApplyHFCorrections(KVVAMOSReconNuc* nuc, KVHashList* cut_list, std::vector<Int_t> nHF_vec)
{
   //Check if the provided nucleus, identified by either a KVIDHarpeeSiCsI_e503 or
   //a KVIDHarpeeICSi_e503 telescope, needs to be corrected in ToF,
   //by adding/removing N times the beam pulse period.
   //
   //To do so, it will use EMeV:ToF TCutG* objects set in 'flist_hfcuts', where:
   //-EMeV is the energy loss in the DeltaE detector of the identifying telescope
   //-ToF is the not corrected time of Flight of the VAMOS nucleus found during
   // the IdentifyQandA() call in recon->ident
   //
   //If the nucleus EMeV and ToF values are in a defined cut, then its
   //time of flight will be modified by adding/removing N times the beam
   //period, where N is found by looking at the 'fvec_nHF' associated vector.
   //(see ReadHFCutFilesListInDataSet() and ReadHFCutFileList() methods).

   //some checks
   assert(nuc);
   assert(cut_list);
   Int_t list_size = cut_list->GetEntries();
   Int_t vec_size  = static_cast<int>(nHF_vec.size());
   if (list_size != vec_size) {
      Error("ApplyHFCorrections", "... list size(=%d) != nHF_vec size(=%d) ...", list_size, vec_size);
      return kFALSE;
   }

   //find basic infos
   Float_t basic_tof    = nuc->GetBasicToF();
   Float_t basic_deltae = -666.;
   KVIDTelescope* idt   = nuc->GetIdentifyingTelescope();
   assert(idt);
   if (idt->InheritsFrom("KVIDHarpeeSiCsI_e503")) {
      KVIDHarpeeSiCsI_e503* sicsi = static_cast<KVIDHarpeeSiCsI_e503*>(idt);
      basic_deltae = sicsi->GetIDMapY();
      if (fkverbose) Info("ApplyHFCorrection_e503", "Si-CsI: BasicTof=%lf, DeltaE=%lf", basic_tof, basic_deltae);
   }

   if (idt->InheritsFrom("KVIDHarpeeICS")) {
      KVIDHarpeeSiCsI_e503* icsi = static_cast<KVIDHarpeeSiCsI_e503*>(idt);
      basic_deltae = icsi->GetIDMapY();
      if (fkverbose) Info("ApplyHFCorrection_e503", "IC-Si: BasicTof=%lf, DeltaE=%lf", basic_tof, basic_deltae);
   }

   //check if inside a defined cut
   Int_t nn = 0;
   TIter next_cut(cut_list);
   TCutG* cut = NULL;
   while ((cut = dynamic_cast<TCutG*>(next_cut.Next()))) {
      if (cut->IsInside(basic_tof, basic_deltae)) {//inside a cut, apply corrections

         Int_t nHF =  nHF_vec.at(nn);
         nuc->SetCorrectedToF(basic_tof - nHF * gVamos->GetBeamPeriod());

         Double_t AoQ = nuc->GetBrho() * KVParticle::C() * 10. / nuc->GetCorrectedBeta() / nuc->GetCorrectedGamma() / KVNucleus::u();
         Double_t AE  = nuc->GetEnergyBeforeVAMOS() / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());

         nuc->SetCorrectedRealAoverQ(AoQ);
         nuc->SetCorrectedRealAE(AE);
         nuc->SetCorrected(kTRUE);

         if (fkverbose) {
            Info("ApplyHFCorrection_e503", "... HF corrections applied ...\nIDCode=%d \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
                 nuc->GetIDCode(), nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
                 nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
         }

         return kTRUE;
      }

      nn++;
   }

   if (fkverbose) Info("ApplyHFCorrections", "... finished ToF HF corrections, did nothing ...");
   return kFALSE; //no correction needed
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::ApplyToFDuplicationCorrections(KVVAMOSReconNuc* nuc, KVHashList* aoq_cut, Float_t tof_offset)
{
   //Check if the provided nucleus, identified by either a KVIDHarpeeSiCsI_e503 or
   //a KVIDHarpeeICSi_e503 telescope, needs to be corrected for ToF duplication,
   //by comparing its (RealZ, RealAoQ) values to the cuts in 'flist_aoq_cut' (set for a
   //given run and a given IDCode).
   //
   //Return kTRUE if correction applied.
   //Return kFALSE if no correction needed.
   //
   //NOTE: for the moment the duplication corrections are applied AFTER HF corrections...

   assert(nuc);
   assert(aoq_cut);

   //find fired Si detector
   Int_t nSi = -666;
   KVIDTelescope* idt   = nuc->GetIdentifyingTelescope();
   KVList* det_list     = (KVList*) idt->GetDetectors();
   assert(det_list);
   TIter next(det_list);
   KVVAMOSDetector* det = NULL;
   while ((det = (KVVAMOSDetector*) next())) {
      if (det->InheritsFrom("KVHarpeeSi")) {
         KVHarpeeSi* harpeesi = static_cast<KVHarpeeSi*>(det);
         assert(harpeesi);
         nSi = (int) harpeesi->GetNumber();
      }
   }

   if (fkverbose) Info("ApplyToFDuplicationCorrections", "... starting ToF duplication corrections for the nucleus (nSi=%d) ...", nSi);

   //iterate over corresponding list
   Float_t AoQ = nuc->GetCorrectedRealAoverQ();
   Float_t Z   = nuc->GetRealZ();
   KVHashList* list = (KVHashList*) aoq_cut->At(nSi - 1);

   assert(list);
   TIter next_cut(list);
   TCutG* cut = NULL;
   while ((cut = dynamic_cast<TCutG*>(next_cut.Next()))) {
      if (cut->IsInside(AoQ, Z)) {//if nuc needs to be corrected

         //debug
         if (fkverbose) {
            Info("ApplyToFDuplicationCorrections", "... before ToF duplication corrections ...\nIDCode=%d, nSi=%d \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
                 nuc->GetIDCode(), nSi, nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
                 nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
         }

         //apply corrections
         //as the duplication corrections are applied after HF corrections,
         //we modify here the former corrected ToF
         Float_t corr_tof = nuc->GetCorrectedToF();
         nuc->SetCorrectedToF(corr_tof + tof_offset);

         Double_t AoQ = nuc->GetBrho() * KVParticle::C() * 10. / nuc->GetCorrectedBeta() / nuc->GetCorrectedGamma() / KVNucleus::u();
         Double_t AE  = nuc->GetEnergyBeforeVAMOS() / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());

         nuc->SetCorrectedRealAoverQ(AoQ);
         nuc->SetCorrectedRealAE(AE);
         nuc->SetCorrected(kTRUE);

         //debug
         if (fkverbose) {
            Info("ApplyToFDuplicationCorrections", "... after ToF duplication corrections ...\nIDCode=%d, nSi=%d \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
                 nuc->GetIDCode(), nSi, nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
                 nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
         }

         return kTRUE;
      }
   }

   if (fkverbose) Info("ApplyToFDuplicationCorrections", "... finished ToF duplication corrections, did nothing ...");
   return kFALSE; //no correction needed
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::ApplyToFAndPathOffset(KVVAMOSReconNuc* nuc)
{
   //Apply a general ime of flight offset to correct AoQ=2 misalignments
   //
   //Return kTRUE if correction applied.
   //Return kFALSE if no correction needed.
   //
   //NOTE: for the moment the duplication corrections are applied AFTER HF corrections
   //      and AFTER ToF duplication corrections...

   assert(nuc);

   //debug
   if (fkverbose) {
      Info("ApplyToFAndPathOffset", "... before ToF and Path corrections ...\nIDCode=%d, \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
           nuc->GetIDCode(), nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
           nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
   }

   //apply corrections
   //as the duplication corrections are applied after HF corrections
   //and after ToF duplication corrections, we modify here the former
   //corrected ToF
   Float_t corr_tof = nuc->GetCorrectedToF();
   Float_t corr_path = nuc->GetCorrectedPath();
   nuc->SetCorrectedToF(corr_tof + ftof_offset);
   nuc->SetCorrectedPath(corr_path + fpath_offset);

   Double_t AoQ = nuc->GetBrho() * KVParticle::C() * 10. / nuc->GetCorrectedBeta() / nuc->GetCorrectedGamma() / KVNucleus::u();
   Double_t AE  = nuc->GetEnergyBeforeVAMOS() / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());

   nuc->SetCorrectedRealAoverQ(AoQ);
   nuc->SetCorrectedRealAE(AE);
   nuc->SetCorrected(kTRUE);

   //debug
   if (fkverbose) {
      Info("ApplyToFOffset", "... after ToF and Path corrections ...\nIDCode=%d, \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
           nuc->GetIDCode(), nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
           nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
   }

   return kTRUE;
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::PrintInitInfos()
{
   //--------HF frequency corrections--------
   printf("###### HF frequency corrections ######\n");
   printf("->Si-CsI:\n");
   printf("%d cuts set\n", GetNCutHFSiCsI());
   if (fkverbose) {
      printf("list of Si-CsI HF cuts follow:\n");
      Int_t ii = 0;
      TCutG* cut = NULL;
      for (std::vector<Int_t>::iterator it = fvec_nHF_sicsi.begin() ; it != fvec_nHF_sicsi.end(); ++it) {
         Int_t nHF = *it;
         cut       = (TCutG*) flist_HFcuts_sicsi->At(ii);
         assert(cut);
         printf("nHF=%d\n", nHF);
         cut->Print();
         printf("\n");
         ii++;
      }
   }

   printf("\n->IC-Si:\n");
   printf("%d cuts set\n", GetNCutHFICSi());
   if (fkverbose) {
      printf("list of IC-Si HF cuts follow:\n");
      Int_t ii = 0;
      TCutG* cut = NULL;
      for (std::vector<Int_t>::iterator it = fvec_nHF_icsi.begin() ; it != fvec_nHF_icsi.end(); ++it) {
         Int_t nHF = *it;
         cut       = (TCutG*) flist_HFcuts_icsi->At(ii);
         assert(cut);
         printf("nHF=%d\n", nHF);
         cut->Print();
         printf("\n");
         ii++;
      }
   }

   //-------ToF duplication corrections--------
   printf("###### ToF duplication corrections ######\n");
   printf("-> Si-CsI:\n");
   printf("tof_corr=%lf\n", ftof_corr_sicsi);
   if (fkverbose) {
      flist_aoq_cut_sicsi->ls();
      printf("list of cuts follows:\n");

      KVHashList* ll0 = NULL;
      TIter it0(flist_aoq_cut_sicsi);
      while ((ll0 = (KVHashList*) it0.Next())) {
         ll0->ls();
         ll0->Print();
      }
   }

   printf("\n-> IC-Si:\n");
   printf("tof_corr=%lf\n", ftof_corr_icsi);
   if (fkverbose) {
      flist_aoq_cut_icsi->ls();
      printf("list of cuts follows:\n");

      KVHashList* ll1 = NULL;
      TIter it1(flist_aoq_cut_icsi);
      while ((ll1 = (KVHashList*) it1.Next())) {
         ll1->ls();
         ll1->Print();
      }
   }

   //-------Global ToF offset to correct AoQ=2 misalignments-------
   printf("###### AoQ=2 misalignment ToF and Path corrections ######\n");
   printf("ToF Offset = %lf ns\n", ftof_offset);
   printf("Path Offset = %lf cm\n", fpath_offset);
}
