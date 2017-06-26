//Created by KVClassFactory on Mon Feb 13 18:45:07 2017
//Author: Quentin Fable,,,

#include "KVVAMOSDataCorrection_e503.h"
#include "KVIDTelescope.h"
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"
#include "KVIDQA.h"
#include "KVParticle.h"
#include "KVNucleus.h"
#include "TCutG.h"
#include "TKey.h"
#include "TF1.h"

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

   fkfunc_ztof_sicsi_init = kFALSE;
   ffunc_ztof_sicsi = new TF1("func_ztof_sicsi", "pol2", 0, 25);
   ffunc_ztof_sicsi->SetParameters(0., 0., 0.);

   fkfunc_ztof_icsi_init = kFALSE;
   ffunc_ztof_icsi  = new TF1("func_ztof_icsi", "pol2", 0, 25);
   ffunc_ztof_icsi->SetParameters(0., 0., 0.);
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

   if (ffunc_ztof_sicsi) {
      delete ffunc_ztof_sicsi;
      ffunc_ztof_sicsi = NULL;
   }

   if (ffunc_ztof_icsi) {
      delete ffunc_ztof_icsi;
      ffunc_ztof_icsi = NULL;
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

         //ToF offset in function of Z for AoQ=2 misalignments
         ReadToFOffsetZFunctionFilesListInDataSet();

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
                  Info("ReadHFCutFileList", "... Run %d | runlist of the file is '%s' ...",
                       fRunNumber, runlist);
               }

               //check if 'fRunNumber' is in run range
               KVNumberList nl(runlist);
               if (nl.Contains(fRunNumber)) {
                  Info("ReadHFCutFileList", "... Run %d is in the runlist '%s' of the file '%s' ...",
                       fRunNumber, runlist, file->GetName());

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

               else if (fkverbose) {
                  Info("ReadHFCutFileList", "... Run %d is NOT in the runlist '%s' of the file '%s' ...",
                       fRunNumber, runlist, file->GetName());
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
                  Info("ReadDuplicationCutFileList", "... Run %d | runlist of the file is '%s' ...",
                       fRunNumber, runlist);
               }

               //check if 'fRunNumber' is in run range
               KVNumberList nl(runlist);
               if (nl.Contains(fRunNumber)) {
                  Info("ReadDuplicationCutFileList", "... Run %d is in the runlist '%s' of the file '%s' ...",
                       fRunNumber, runlist, file->GetName());

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
               } else if (fkverbose) {
                  Info("ReadDuplicationCutFileList", "... Run %d is NOT the runlist '%s' of the file '%s' ...",
                       fRunNumber, runlist, file->GetName());
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
   //By default this offset is equal to 1.05 ns (value found from 40Ca+40Ca elastic
   //duplicated pics).

   ftof_corr_sicsi = gDataSet->GetDataSetEnv("INDRA_e503.KVVAMOSDataCorrection_e503.DuplicationToFOffset", 1.05);  // in ns
   ftof_corr_icsi  = gDataSet->GetDataSetEnv("INDRA_e503.KVVAMOSDataCorrection_e503.DuplicationToFOffset", 1.05);  //in ns
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadToFOffsetZFunctionFilesListInDataSet()
{
   // Read the file containing infos to use
   // for e503 A/Q=2 misalignments in function
   // of the charge Z of the VAMOS particle.
   // This file is set in the dataset directory:
   // $KVROOT/KVFiles/INDRA_e503
   //
   // This file, specific to a given dataset, is found and
   // opened using its name given by the following environment
   // variable defined in $KVROOT/KVFiles/.kvrootrc:
   //
   // INDRA_e503.KVVAMOSDataCorrection_e503.ToFOffsetZFunctionSiCsI:  [file_name]
   //
   // This file contains a list of *.root file names, each
   // of them associated to a system/range of runs.
   // Each of these *.root files contains more specificly a
   // TEnv object with the parameters of a function to use to correct the
   // A/Q=2 misalignments, by applying an offset in the ToF of the identified particle.
   // The correction applied will be: ToF+=ToF_offset,
   // where ToF_offset = f(Z) and 'f' is a pol2 function.
   // In the associated TEnv is also saved the run range.
   //
   // The idea to apply an ToF offset depending on the Z of the particle comes
   // from a possible wrong calibration of the ToF general offset applied
   // when in was calibrated (by using 40Ca elastic collisions). In fact it
   // is possible that this calibration is wrong as the raise time of
   // the Si signals (used as a 'start' in ToF measurements) depends
   // strongly on the charge Z of the particle passing through the Si.
   //
   // As an example, we observe for the 40Ca+48Ca system an offset varying between 0 ns
   //(Z=20) and -2.7 ns (Z=5), and the tendency matches a pol2 function.


   //--------Si-CsI--------
   //Find the list file to use
   TString filename0 = gDataSet->GetDataSetEnv("KVVAMOSDataCorrection_e503.ToFOffsetZFunctionSiCsI");
   if (filename0 == "") {
      Warning("ReadToFOffsetZFunctionFilesListInDataSet", "No filename defined for Si-CsI telescopes. It can be defined by %s.KVVAMOSDataCorrection_e503.ToFOffsetZFunctionSiCsI", gDataSet->GetName());
      return;
   }
   std::ifstream ifile0;
   if (gDataSet->OpenDataSetFile(filename0.Data(), ifile0)) {
      Info("ReadToFOffsetZFunctionFilesListInDataSet", "list of Si-CsI files in: '%s' ", filename0.Data());
      ReadToFOffsetZFunctionFileList(ifile0, 0);
      ifile0.close();
   }

   //--------IC-Si--------
   //Find the list file to use
   TString filename1 = gDataSet->GetDataSetEnv("KVVAMOSDataCorrection_e503.ToFOffsetZFunctionICSi");
   if (filename1 == "") {
      Warning("ReadToFOffsetZFunctionFilesListInDataSet", "No filename defined for IC-SI telescopes. It can be defined by %s.KVVAMOSDataCorrection_e503.ToFOffsetZFunctionICSi", gDataSet->GetName());
      return;
   }
   std::ifstream ifile1;
   if (gDataSet->OpenDataSetFile(filename1.Data(), ifile1)) {
      Info("ReadToFOffsetZFunctionFilesListInDataSet", "list of IC-Si cut files in: '%s' ", filename1.Data());
      ReadToFOffsetZFunctionFileList(ifile1, 1);
      ifile1.close();
   }
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::ReadToFOffsetZFunctionFileList(std::ifstream& file, Int_t type)
{
   //Protected method to read and set the ToF correction in function of the
   //charge Z of the particle for the dataset e503.
   //Set 'type' to 0 for SiCsI telescopes and set 'type' to 1 for ICSi telescopes
   //For each file listed in this list file we:
   //- read the TEnv in the file to access to the run range of the file
   //- if 'fRunNumber' is in the run range we create in 'ffunc_ztof_sicsi'
   //  the pol2 function with the parameters set in the TEnv and stop the research.

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
                  Info("ReadToFOffsetZFunctionFileList", "... file %s opened, infos follow ...", file->GetName());
                  file->ls();
               }

               //find the TEnv and access to runlist and tof_corr values
               TEnv* env = (TEnv*) file->Get("TEnv");
               assert(env);

               if (fkverbose) {
                  Info("ReadToFOffsetZFunctionFileList", "... TEnv infos follow ...");
                  env->Print();
               }

               const Char_t* runlist = env->GetValue("runlist", "");
               if (fkverbose) {
                  Info("ReadToFOffsetZFunctionFileList", "... Run %d | runlist of the file is '%s' ...",
                       fRunNumber, runlist);
               }

               //check if 'fRunNumber' is in run range
               KVNumberList nl(runlist);
               if (nl.Contains(fRunNumber)) {
                  Info("ReadToFOffsetZFunctionFileList", "... Run %d is in the runlist '%s' of the file '%s' ...",
                       fRunNumber, runlist, file->GetName());


                  //exctract pol2 coefficients from TEnv
                  Double_t xmin = env->GetValue("xmin", 0.);
                  Double_t xmax = env->GetValue("xmax", 25.);
                  Double_t p0   = env->GetValue("p0", 0.);
                  Double_t p1   = env->GetValue("p1", 0.);
                  Double_t p2   = env->GetValue("p2", 0.);

                  if (type == 0) {
                     ffunc_ztof_sicsi->SetRange(xmin, xmax);
                     ffunc_ztof_sicsi->SetParameters(p0, p1, p2);
                     fkfunc_ztof_sicsi_init = kTRUE;
                  }

                  else if (type == 1) {
                     ffunc_ztof_icsi->SetRange(xmin, xmax);
                     ffunc_ztof_sicsi->SetParameters(p0, p1, p2);
                     fkfunc_ztof_icsi_init = kTRUE;
                  }

                  found = kTRUE;
               }

               else if (fkverbose) {
                  Info("ReadToFOffsetZFunctionFileList", "... Run %d is NOT in the runlist '%s' of the file '%s' ...",
                       fRunNumber, runlist, file->GetName());
               }

               file->Close();
            }//end of file is ok
         }//end of SearchKVFile
      }//end of line ok
   }//all lines read
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
   Float_t aq = nuc->GetBasicRealAoverQ();
   nuc->SetCorrectedRealAoverQ(aq);

   if (fkverbose) {
      printf("\n\n_________________________________________________________________\n");
      Info("ApplyCorrections", "... trying to apply e503 corrections to a new nucleus (IDCode=%d) ...", IDCode);
   }

   //-----Re-calculate energy-----
   //As the recon->ident energy calibration is somehow
   //foggy, apply own energy calibration here.
   if (fkverbose) Info("ApplyCorrections", "... energy re-computation ...");
   //before corrections of energy
   Double_t KEold       = nuc->GetKE();
   Double_t EbfVAMOSold = nuc->GetEnergyBeforeVAMOS();
   Float_t  realAEold   = nuc->GetCorrectedRealAE();
   Bool_t   IsCold      = nuc->IsCorrected();

   //energy correction
   Double_t KEnew;
   Double_t EbfVAMOSnew;
   CalibrateFromDetList(nuc, KEnew, EbfVAMOSnew);
   if (EbfVAMOSnew >= 0. && KEnew >= 0.) {
      Double_t AE  = KEnew / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());
      nuc->SetCorrectedRealAE(AE);
      nuc->SetCorrectedEnergyBeforeVAMOS(EbfVAMOSnew);
      nuc->SetCorrectedEnergy(KEnew);
      nuc->SetCorrected(kTRUE);
   }

   //debug
   if (fkverbose) {
      printf("BEFORE: IsCorrected=%d, EbfVAMOS=%lf MeV, KE=%lf MeV, CorrAE=%lf\n", (int) IsCold, EbfVAMOSold, KEold, realAEold);
      printf("AFTER:  IsCorrected=%d, EbfVAMOS=%lf MeV, KE=%lf MeV, CorrAE=%lf\n", (int) nuc->IsCorrected(), nuc->GetCorrectedEnergyBeforeVAMOS(), nuc->GetCorrectedEnergy(), nuc->GetCorrectedRealAE());
   }

   //-----AoQ - ToF corrections-----
   //Apply ToF duplications/offset corrections using
   //corrected AoQ obervable, as it is only dependant of ToF
   //(considering path and brho are OK, which is more or less sure...)
   //Si-CsI telescopes corrections
   Bool_t kCsI_HFcorr   = kFALSE;
   Bool_t kCsI_DUcorr   = kFALSE;
   Bool_t kCsI_ZToFfunc = kFALSE;
   if (IDCode == 3) {
      kCsI_HFcorr = ApplyHFCorrections(nuc, flist_HFcuts_sicsi, fvec_nHF_sicsi);
      kCsI_DUcorr = ApplyToFDuplicationCorrections(nuc, flist_aoq_cut_sicsi, ftof_corr_sicsi);
      if (fkfunc_ztof_sicsi_init) kCsI_ZToFfunc = ApplyToFOffsetZFunctionCorrections(nuc, ffunc_ztof_sicsi);
   }

   //IC-Si telescopes corrections
   Bool_t kSi_HFcorr   = kFALSE;
   Bool_t kSi_DUcorr   = kFALSE;
   Bool_t kSi_ZToFfunc = kFALSE;
   if (IDCode == 4) {
      kSi_HFcorr = ApplyHFCorrections(nuc, flist_HFcuts_icsi, fvec_nHF_icsi);
      kSi_DUcorr = ApplyToFDuplicationCorrections(nuc, flist_aoq_cut_icsi, ftof_corr_icsi);
      if (fkfunc_ztof_icsi_init) kSi_ZToFfunc = ApplyToFOffsetZFunctionCorrections(nuc, ffunc_ztof_icsi);
   }

   //Save corrections status
   if (kCsI_HFcorr || kCsI_DUcorr || kCsI_ZToFfunc || kSi_HFcorr || kSi_DUcorr || kSi_ZToFfunc) nuc->SetCorrected(kTRUE);

   //-----Identification with grids-----
   //Use KVIDQA grids on previously corrected
   //'AE vs AoQ' observables to find
   //the charge state 'Q' and mass 'A'
   Bool_t kidentOK = kFALSE;
   if (nuc->IsCorrected()) {
      if (fkverbose) Info("ApplyCorrections", "... trying to apply KVIDQA grids identification to nucleus ...");
      kidentOK = IdentifyCorrectedNucleus(nuc);
      if (fkverbose) {
         if (kidentOK)  printf("identification with grids was success\n");
         else printf("identification with grids was failure");
      }
   }

   //-----Re-calculate-----
   //At this step: -all corrections were applied
   //              -energy re-computation was applied
   //              -QA ID grids were used on corrected 'AE vs AoQ' data to
   //               extract the final charge state and mass
   //
   //The final step done here is to re-calculate the energy with the
   //final mass, as the final mass can be different from the mass saved
   //in ID telescope's identification result used in all previous energy
   //calibrations (in the case of an KVIDHarpeeSiCsI_e503 inherited telescope,
   //the mass saved in the IDR is the one from minimizer; in the case of an
   //KVIDHarpeeICSi_e503 inherited telescope it is usually the one
   //from mass formula).
   if (kidentOK) {
      Double_t KElast;
      Double_t EbfVAMOSlast;

      if (fkverbose) {
         Info("ApplyCorrections", "... final identification set for the nucleus, infos follow ...");
         printf("BEFORE: IsCorrQandAID=%d, IsBasicQandAID=%d, RealZ=%lf, Z=%d, RealQ=%lf, Q=%d, RealA=%lf, A=%d, KE=%lf MeV, EBeforeVAMOS=%lf MeV\n",
                (int) nuc->IsCorrectedQandAidentified(), (int) nuc->IsBasicQandAidentified(),
                nuc->GetRealZ(), nuc->GetZ(), nuc->GetRealQ(), nuc->GetQ(), nuc->GetRealA(), nuc->GetA(),
                nuc->GetKE(), nuc->GetCorrectedEnergyBeforeVAMOS());
      }

      CalibrateFromDetList(nuc, KElast, EbfVAMOSlast);
      nuc->SetCorrectedEnergy(KElast);
      nuc->SetCorrectedEnergyBeforeVAMOS(EbfVAMOSlast);

      //use all corrected observables to set the final
      //mass, charge and energies...
      nuc->SetCorrectedQandAIdentification();

      if (fkverbose) {
         Info("ApplyCorrections", "... final identification set for the nucleus, infos follow ...");
         printf("AFTER: IsCorrQandAID=%d, IsBasicQandAID=%d, RealZ=%lf, Z=%d, RealQ=%lf, Q=%d, RealA=%lf, A=%d, KE=%lf MeV, EBeforeVAMOS=%lf MeV\n",
                (int) nuc->IsCorrectedQandAidentified(), (int) nuc->IsBasicQandAidentified(),
                nuc->GetRealZ(), nuc->GetZ(), nuc->GetRealQ(), nuc->GetQ(), nuc->GetRealA(), nuc->GetA(),
                nuc->GetKE(), nuc->GetCorrectedEnergyBeforeVAMOS());
      }
   }

   return kTRUE;
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
   //
   //We then re-calculate the nucleus corrected AE and AoQ taking the new ToF into
   //account. For AE, we also consider the new energy estimation with 'ecorr'

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

   if (idt->InheritsFrom("KVIDHarpeeICSi_e503")) {
      KVIDHarpeeICSi_e503* icsi = static_cast<KVIDHarpeeICSi_e503*>(idt);
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
         Double_t AE  = nuc->GetCorrectedEnergy() / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());

         nuc->SetCorrectedRealAoverQ(AoQ);
         nuc->SetCorrectedRealAE(AE);
         nuc->SetCorrected(kTRUE);

         if (fkverbose) {
            Info("ApplyHFCorrection_e503", "... HF corrections applied with cut '%s' ...\nIDCode=%d \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
                 cut->GetName(), nuc->GetIDCode(), nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
                 nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
         }

         return kTRUE;
      }

      else if (fkverbose) Info("ApplyHFCorrection_e503", "... HF corrections not applied with cut '%s' ...", cut->GetName());
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
   //We then re-calculate the nucleus corrected AE and AoQ taking the new ToF into
   //account.
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
         Double_t AE  = nuc->GetCorrectedEnergy() / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());

         nuc->SetCorrectedRealAoverQ(AoQ);
         nuc->SetCorrectedRealAE(AE);
         nuc->SetCorrected(kTRUE);

         //debug
         if (fkverbose) {
            Info("ApplyToFDuplicationCorrections", "... after ToF duplication corrections with cut '%s' ...\nIDCode=%d, nSi=%d \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
                 cut->GetName(), nuc->GetIDCode(), nSi, nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
                 nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
         }

         return kTRUE;
      }

      else Info("ApplyToFDuplicationCorrections", "... ToF duplication corrections not applied with cut '%s' ...", cut->GetName());
   }

   if (fkverbose) Info("ApplyToFDuplicationCorrections", "... finished ToF duplication corrections, did nothing ...");
   return kFALSE; //no correction needed
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::ApplyToFOffsetZFunctionCorrections(KVVAMOSReconNuc* nuc, TF1* func)
{
   //Apply time of flight offset in function of the charge Z of the particle
   //to correct AoQ=2 misalignments.
   //
   //We then re-calculate the nucleus corrected AE and AoQ taking the new ToF into
   //account. For AE, we also consider the new energy estimation with 'ecorr'
   //
   //Return kTRUE if correction applied.
   //Return kFALSE if no correction needed.
   //
   //NOTE: for the moment the duplication corrections are applied AFTER HF corrections,
   //      AFTER ToF duplication corrections and AFTER the global ToF+Path offset ...

   assert(nuc);

   //debug
   if (fkverbose) {
      Info("ApplyToFOffsetZFunctionCorrections", "... before ToF AoQ=2 misalignment corrections ...\nIDCode=%d, \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
           nuc->GetIDCode(), nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
           nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
   }

   //apply corrections...
   //as the duplication corrections are applied after HF corrections
   //and after ToF duplication corrections, we modify here the former
   //corrected ToF
   Float_t realZ    = nuc->GetRealZ();
   Float_t tof      = nuc->GetCorrectedToF();
   Float_t corr_tof = func->Eval(realZ);
   nuc->SetCorrectedToF(tof + corr_tof);

   Double_t AoQ = nuc->GetBrho() * KVParticle::C() * 10. / nuc->GetCorrectedBeta() / nuc->GetCorrectedGamma() / KVNucleus::u();
   Double_t AE  = nuc->GetCorrectedEnergy() / ((nuc->GetCorrectedGamma() - 1.) * KVNucleus::u());

   nuc->SetCorrectedRealAoverQ(AoQ);
   nuc->SetCorrectedRealAE(AE);
   nuc->SetCorrected(kTRUE);

   //debug
   if (fkverbose) {
      Info("ApplyToFOffsetZFunctionCorrections", "... after ToF AoQ=2 misalignment corrections ...\nIDCode=%d, \nBasicTof=%lf, BasicPath=%lf, BasicAE=%lf, BasicAoQ=%lf\nCorrToF=%lf, CorrPath=%lf, CorrAE=%lf, CorrAoQ=%lf",
           nuc->GetIDCode(), nuc->GetBasicToF(), nuc->GetBasicPath(), nuc->GetBasicRealAE(), nuc->GetBasicRealAoverQ(),
           nuc->GetCorrectedToF(), nuc->GetCorrectedPath(), nuc->GetCorrectedRealAE(), nuc->GetCorrectedRealAoverQ());
   }

   return kTRUE;
}

//____________________________________________________________________________//
void KVVAMOSDataCorrection_e503::CalibrateFromDetList(KVVAMOSReconNuc* nuc, Double_t& ke, Double_t& eBeforeVAMOS)
{
   // Iterate over the list of aligned detectors (the list is in reverse
   // order) and add up the energies of all the absorbers, making our way
   // back towards the target. We must do this explicitly as the VAMOS
   // definition of "calibrated detector" may differ from the standard
   // KaliVeda definition.  All we care about here is that the detector is
   // calibrated in energy.

   Double_t total_energy   = 0.;
   Double_t detector_eloss = 0.;
   Double_t absorber_eloss = 0.;

   KVVAMOSDetector* det(NULL);

   //stopping detector
   KVVAMOSDetector* stop_det = static_cast<KVVAMOSDetector*>(nuc->GetStoppingDetector());
   assert(stop_det);

   //identifying telescope
   KVIDTelescope* idt = nuc->GetIdentifyingTelescope();
   if (!idt) {
      if (fkverbose) Error("CalibrateFromDetList", "... no ID telescope, returning ...");
      ke = -666.;
      eBeforeVAMOS = -666.;
      return;
   }

   KVVAMOSDetector* resi_det = static_cast<KVVAMOSDetector*>(idt->GetDetector(2));
   TList* det_list = resi_det->GetAlignedDetectors(KVGroup::kBackwards);

   //const Bool_t kICSi(idt->InheritsFrom("KVIDHarpeeICSi_e503"));
   const Bool_t kSiCsI(idt->InheritsFrom("KVIDHarpeeSiCsI_e503"));

   //idt identification result
   KVIdentificationResult* idr = nuc->GetIdentificationResult(idt);
   if (!idr) {
      if (fkverbose) Error("CalibrateFromDetList", "... no IDR for the ID telescope, returning ...");
      ke = -666.;
      eBeforeVAMOS = -666.;
      return;
   }

   KVNucleus sim_nucleus; //buffer nucleus for backward propagation
   if (idr->Zident && idr->Aident) {
      sim_nucleus.SetZandA(idr->Z, idr->A);
   } else {
      // A Value is automatically calculated.
      sim_nucleus.SetZ(idr->Z);
   }

   //debug
   if (fkverbose) {
      Info("CalibrateFromDetList", "... initialisation ... \nstop_det is %s \nresi_det is %s \nidt is %s \n... printing idt idr ...",
           stop_det->GetName(), resi_det->GetName(), idt->GetName());
      printf("IDattempted=%d, IDOK=%d, IDquality=%d, IDcode=%d \nZident=%d, Aident=%d\n Z=%d, A=%d \nComment='%s'\n",
             (int) idr->IDattempted, (int) idr->IDOK, idr->IDquality, idr->IDcode,
             (int) idr->Zident, (int) idr->Aident, idr->Z, idr->A, idr->GetComment());
      printf("... printing list of aligned detectors (backward) ...");
      det_list->Print();
   }

   //-----Iteration over detector list in REVERSE order -----
   TIter next_det(det_list);
   while ((det = static_cast<KVVAMOSDetector*>(next_det()))) {

      const Bool_t kDetectorIsDC(det->InheritsFrom("KVDriftChamber"));
      //const Bool_t kDetectorIsIC(det->InheritsFrom("KVHarpeeIC"));
      const Bool_t kDetectorIsSi(det->InheritsFrom("KVHarpeeSi"));
      const Bool_t kDetectorIsCsI(det->InheritsFrom("KVHarpeeCsI"));

      //-----Iteration over all absorbers of the current detector-----
      // For each detector we iterate IN REVERSE over the absorbers (making
      // our way towards the target) and sum either the calculated energy
      // loss in the dead layers or the calibrated energy in the active
      // layer.
      KVList* absorbers(det->GetListOfAbsorbers());

      detector_eloss = 0.;
      KVMaterial* abs = NULL;

      TIter next_absorber(absorbers, kIterBackward);
      while ((abs = static_cast<KVMaterial*>(next_absorber()))) {
         absorber_eloss = 0.;

         //-----Absorber is ACTIVE LAYER-----
         // The absorber is the active layer of the detector and so we
         // can simply call the detector's GetCalibE function, or in the
         // case of the CsI detector (which requires the Z and A values)
         // GetCorrectedEnergy(). Also the drift chamber is not
         // calibrated in energy and must be calculated.
         if (abs == det->GetActiveLayer()) {
            //-----CsI case-----
            if (kDetectorIsCsI) {
               KVHarpeeCsI* csi(static_cast<KVHarpeeCsI*>(det));
               assert(csi);

               // Be careful: This only works because the energy of
               // the nucleus is not used in GetCorrectedEnergy(), should
               // this change then we will need to rethink.
               absorber_eloss = csi->GetCorrectedEnergy(&sim_nucleus, -1., 0);

               if (fkverbose) Info("CalibrateFromDetList", "... det is CsI, eloss=%lf MeV...", absorber_eloss);
            }

            //-----Drift Chambers-----
            //DC are not energy calibrated, we calculate energy loss
            else if (kDetectorIsDC) {
               KVMaterial* drift_active = static_cast<KVMaterial*>(abs);
               assert(drift_active);

               absorber_eloss = drift_active->GetDeltaEFromERes(sim_nucleus.GetZ(), sim_nucleus.GetA(), total_energy);
               if (fkverbose) Info("CalibrateFromDetList", "... det is DC%d, eloss=%lf MeV...", det->GetNumber(), absorber_eloss);
            }

            //-----IC or Si-----
            else {
               absorber_eloss = det->GetCalibE();
               if (fkverbose) Info("CalibrateFromDetList", "... det is %s, eloss=%lf MeV", det->GetName(), absorber_eloss);
            }

            //sum energies
            detector_eloss += absorber_eloss;
            total_energy += absorber_eloss;

            if (fkverbose) Info("CalibrateFromDetList", "... after active layer: det_eloss=%lf MeV, etot=%lf MeV, ...", detector_eloss, total_energy);
            continue;
         }//end of active layer case

         //-----Absorber IS NOT ACTIVE LAYER-----
         // Total energy is the residual energy since we are working towards
         // the target from the stopping detector
         Double_t incident_energy = abs->GetIncidentEnergyFromERes(sim_nucleus.GetZ(), sim_nucleus.GetA(), total_energy);

         // Vector normal to absorber surface, pointing from the origin
         // toward the material - in this case simply the z direction (beam
         // direction)
         TVector3 norm(0, 0, 1);
         sim_nucleus.SetEnergy(incident_energy);
         sim_nucleus.SetTheta(nuc->GetThetaF());
         sim_nucleus.SetPhi(nuc->GetPhiF());

         absorber_eloss = abs->GetELostByParticle(&sim_nucleus, &norm);

         detector_eloss += absorber_eloss;
         total_energy += absorber_eloss;

         if (fkverbose) {
            Info("CalibrateFromDetList", "... after inactive layer: absorber_eloss=%lf MeV, det_eloss=%lf MeV, etot=%lf MeV...",
                 absorber_eloss, detector_eloss, total_energy);
         }
      }//end of absorber loop

      //-----Si->CsI dead layer correction -----
      if (kDetectorIsSi && kSiCsI) {
         // Be careful here! This calculation relies upon the silicon
         // detector having only one absorber, which is calibrated. If the
         // silicon detector has any dead layers then this calculation will
         // not be accurate, as we only make use of the calibrated energy.
         //
         // The alternative would be to use detector_eloss in the incident
         // energy calculation but that is also problematic, as you introduce
         // a cyclic dependency on the calculated total energy! We end up in
         // this situation as the residual energy from the CsI detector is
         // zero and so reliable calculations of the CsI incident energy are
         // not possible.

         // silicon detector incident energy
         assert((detector->GetCalibE() >= 0.) &&
                (detector->GetCalibE() < 3000.));

         Double_t incident_energy = det->GetEResFromDeltaE(sim_nucleus.GetZ(), sim_nucleus.GetA(), det->GetCalibE());

         // Vector normal to dead-layer surface, pointing from the origin
         // toward the material - in this case simply the z direction (beam
         // direction)
         TVector3 norm(0, 0, 1);
         KVMaterial isobutane("C4H10", 13.65 * KVUnits::cm);
         isobutane.SetPressure(40.*KVUnits::mbar);

         sim_nucleus.SetEnergy(incident_energy);
         sim_nucleus.SetTheta(nuc->GetThetaF());
         sim_nucleus.SetPhi(nuc->GetPhiF());

         absorber_eloss = isobutane.GetELostByParticle(&sim_nucleus, &norm);
         total_energy += absorber_eloss;

         // The silicon detector losses have already been taken into account
         // above.
         if (fkverbose) {
            Info("CalibrateFromDetList", "... IC->CsI dead layer correction: absorber_eloss=%lf MeV, total energy=%lf MeV  ...",
                 absorber_eloss, total_energy);
         }
         continue;
      }//end of si-csi dead layer corrections

      //-----SED corrections-----
      if (kDetectorIsDC && (det->GetNumber() == 2)) {
         // We calculate the incident energy to the second drift chamber
         // from its residual energy and use it as the basis for calculating
         // the energy loss in the sed. Care is needed to set the correct
         // effective thickness of the material for the particle.

         // Vector normal to SED surface, pointing from the origin toward the
         // material - in this case 45 degrees (the beam [0,0,1] sees the
         // thickness as 1.273um)
         TVector3 norm(0, -1, 1);
         KVMaterial sed("Myl", 0.9 * KVUnits::um);

         // total_energy at this point includes the second drift chamber
         // energy losses and so this needs to be subtracted again to get the
         // residual energy.
         Double_t drift_2_incident_energy = det->GetIncidentEnergyFromERes(sim_nucleus.GetZ(), sim_nucleus.GetA(), total_energy - detector_eloss);

         // Nucleus is defined just prior to entering the second drift
         // chamber and is used only to evaluate the direction in which it is
         // travelling.
         sim_nucleus.SetEnergy(drift_2_incident_energy);
         sim_nucleus.SetTheta(nuc->GetThetaI());
         sim_nucleus.SetPhi(nuc->GetPhiI());

         // Normalised!
         TVector3 momentum(sim_nucleus.GetMomentum());
         Double_t mag(momentum.Mag());

         TVector3 dir;
         dir.SetX(momentum.X() / mag);
         dir.SetY(momentum.Y() / mag);
         dir.SetZ(momentum.Z() / mag);

         Double_t effective_thickness(sed.GetEffectiveThickness(norm, dir));

         // Set the linear thickness of the SED to the correct effective
         // thickness.
         sed.SetThickness(effective_thickness);

         absorber_eloss = sed.GetDeltaEFromERes(sim_nucleus.GetZ(), sim_nucleus.GetA(), drift_2_incident_energy);
         total_energy += absorber_eloss;

         // The drift chamber energy losses have already been accounted for
         // above.
         if (fkverbose) Info("CalibrateFromDetList", "... SED correction: absorber_eloss=%lf, etot=%lf ...", absorber_eloss, total_energy);
         continue;
      }//end of SED corrections
   }//end of detector loop

   //Set energy prior entering in VAMOS, after target and strip foil
   eBeforeVAMOS = total_energy;

   //-----Final corrections to the energy for the strip foil and the target-----
   //strip foil
   absorber_eloss = gVamos->GetStripFoilEnergyLossCorrection(nuc);
   total_energy += absorber_eloss;
   if (fkverbose) Info("CalibrateFromDetList", "... strip foil correction: absorber_eloss=%lf, etot=%lf ...", absorber_eloss, total_energy);

   //target
   if (gMultiDetArray->GetTarget()) {
      absorber_eloss = gMultiDetArray->GetTargetEnergyLossCorrection(nuc);
      total_energy += absorber_eloss;
      if (fkverbose) Info("CalibrateFromDetList", "... target corrections: absorber_eloss=%lf, etot=%lf ...", absorber_eloss, total_energy);
   } else if (fkverbose) Warning("CalibrateFromDetList", "... target corrections can't be applied, target not found ...");

   //Set kinetic energy before target and strip foil
   ke = total_energy;
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection_e503::IdentifyCorrectedNucleus(KVVAMOSReconNuc* nuc)
{
   //Once the nucleus has been corrected for energy (see CalibrateFromDetList() method)
   //and for ToF (see ApplyHFCorrections(), ApplyToFDuplicationCorrections, and
   //ApplyToFOffsetZFunctionCorrections() methods), we need
   //to identify its final charge state 'Q' and mass 'A'.
   //
   //To do so we use the corrected "CorrAE vs CorrAoQ" maps, where 'CorrAE' is the corrected
   //mass found by energy conservation law and 'CorrAoQ' is the corrected
   //mass:charge ratio found by Lorentz law.
   //The integer value of charge state 'Q_int' is thus found using the associated
   //KVIDTelescope's KVIDQAGrid grid (see KVIDQA class) where the value of Q is linearised.
   //The definitive mass 'A' is then obtained from 'CorrAoQ/Q_int'.

   Bool_t ok = kFALSE;

   if (fkverbose) {
      Info("IdentifyCorrectedNucleus", "... before identification ...");
      printf("RealQ=%lf, Q=%d, RealA=%lf, A=%d\n", nuc->GetRealQ(), nuc->GetQ(), nuc->GetRealA(), nuc->GetA());
   }

   //Find the KVIDQA telescopes
   KVSeqCollection* idt_list = nuc->GetIDTelescopes();
   if (fkverbose) {
      Info("IdentifyCorrectedNucleus", "... list of nucleus' associated KVIDTelescopes follows ...");
      idt_list->Print();
   }

   if (idt_list && idt_list->GetSize() > 0) {
      KVIDTelescope* idt = NULL;
      TIter next(idt_list);

      while ((idt = (KVIDTelescope*) next())) {
         if (idt->InheritsFrom(KVIDQA::Class())) {
            if (fkverbose) {
               printf("\n");
               Info("IdentifyCorrectedNucleus", "... found the following KVIDQA telescopes: '%s' ...", idt->GetName());
            }

            //Find the correct associated KVIDQA telescope,
            //depending of nucleus' IDCode
            TString name = idt->GetName();
            Bool_t idt_ok = kFALSE;

            if ((nuc->GetIDCode() == 3) && (name.Contains("VID_QA_CSI"))) idt_ok = kTRUE;
            else if ((nuc->GetIDCode() == 4) && (name.Contains("VID_QA_CSI"))) idt_ok = kTRUE;

            //Corresponding KVIDQA found
            if (idt_ok) {
               KVIDQA* qa_idt = (KVIDQA*)idt;
               static KVIdentificationResult IDR;

               //loop over the time acquisition parameters as
               //the QA ID telescopes need the ToF as argument
               const Char_t* tof_name = NULL;
               for (Short_t i = 0; !ok && (tof_name = nuc->GetCodes().GetToFName(i)); i++) {
                  IDR.Clear();
                  IDR.IDattempted = kTRUE;

                  qa_idt->Identify(&IDR, tof_name, nuc->GetCorrectedRealAoverQ(), nuc->GetCorrectedRealAE());
                  if (fkverbose) {
                     Info("IdentifyCorrectedNucleus", "... identification results from KVIDQA='%s' for tof_name='%s' follow ...", qa_idt->GetName(), tof_name);
                     printf("IDR::IDOK=%d, IDR::IDCode=%d \nIDR::Zident=%d, IDR::Aident=%d \nIDR::PID=%lf, IDR::Z=%d, IDR::A=%d\n",
                            (int) IDR.IDOK, IDR.IDcode, (int) IDR.Zident, (int) IDR.Aident, IDR.PID, IDR.Z, IDR.A);
                  }

                  //ID went well
                  if (IDR.IDOK) {
                     if (fkverbose) {
                        Info("IdentifyCorrectedNucleus", "... idr from idt='%s' kept, setting final id results to the nucleus ...", qa_idt->GetName());
                        printf("BEFORE: CorrRealQ=%lf, CorrQ=%d, CorrRealA=%lf, CorrA=%d\n",
                               nuc->GetCorrectedRealQ(), nuc->GetCorrectedQ(), nuc->GetCorrectedRealA(), nuc->GetCorrectedA());
                     }

                     //The kept mass is the ratio between the corrected AoQ
                     //and the integer value of the found charge
                     //NOTE: Corrected Q and A values must be changed here, but corrected final Q and
                     //A will be changed after energy re-calculation !!!
                     nuc->SetCorrectedRealQ(IDR.PID);
                     Double_t aa = nuc->GetCorrectedRealAoverQ() * nuc->GetCorrectedQ();
                     nuc->SetCorrectedRealA(aa);
                     ok = kTRUE;
                     if (fkverbose) {
                        printf("AFTER: CorrRealAoQ=%lf \nCorrRealQ=%lf, CorrQ=%d, CorrRealA=%lf, CorrA=%d\n",
                               nuc->GetCorrectedRealAoverQ(),
                               nuc->GetCorrectedRealQ(), nuc->GetCorrectedQ(), nuc->GetCorrectedRealA(), nuc->GetCorrectedA());
                     }
                  }
               }
            }

            //Corresponding KVIDQA not found
            else if (fkverbose) Error("IdentifyCorrectedNucleus", "... KVIDTelescope='%s' not used for ID (IDCode=%d) ...", idt->GetName(), nuc->GetIDCode());
         }
      }
   }

   return ok;
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
   printf("%d cuts set\n", flist_aoq_cut_sicsi->GetEntries());
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
   printf("%d cuts set\n", flist_aoq_cut_icsi->GetEntries());
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

   //-------ToF offset in function of the charge Z of the particle to correct AoQ=2 misalignments-------
   printf("###### AoQ=2 misalignment: ToF correction as a function of Z ######\n");
   printf("-> Si-CsI:\n");
   printf("Initialised = %d\n", (int) fkfunc_ztof_sicsi_init);
   ffunc_ztof_sicsi->Print();
   printf("p0=%lf \np1=%f \np2=%f\n", ffunc_ztof_sicsi->GetParameter(0), ffunc_ztof_sicsi->GetParameter(1), ffunc_ztof_sicsi->GetParameter(2));

   printf("-> IC-Si:\n");

   printf("Initialised = %d\n", (int) fkfunc_ztof_icsi_init);
   ffunc_ztof_icsi->Print();
   printf("p0=%lf \np1=%f \np2=%f\n", ffunc_ztof_icsi->GetParameter(0), ffunc_ztof_icsi->GetParameter(1), ffunc_ztof_icsi->GetParameter(2));
}
