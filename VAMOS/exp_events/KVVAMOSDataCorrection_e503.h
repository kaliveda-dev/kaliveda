//Created by KVClassFactory on Mon Feb 13 18:45:07 2017
//Author: Quentin Fable,,,

#ifndef __KVVAMOSDATACORRECTION_E503_H
#define __KVVAMOSDATACORRECTION_E503_H

#include "KVVAMOSDataCorrection.h"

class KVVAMOSDataCorrection_e503 : public KVVAMOSDataCorrection {

protected:
   Bool_t   fkverbose;
   Bool_t   fkIsInit;

   //ToF corrections
   //-----Si-CsI-----
   KVHashList* flist_HFcuts_sicsi;     //list of TCutG* for HF frequency corrections
   std::vector<Int_t> fvec_nHF_sicsi;  //vector containing the number of times the beam pulse period must be corrected for each cut

   KVHashList* flist_aoq_cut_sicsi;    //list of RealZ:BasicAoQ TCutG* for AoQ duplication corrections
   Float_t ftof_corr_sicsi;            //correction of ToF (in ns) for AoQ duplication corrections

   Bool_t fkfunc_ztof_sicsi_init; //=kTRUE if the pol2 function was correctly init
   TF1*   ffunc_ztof_sicsi; //pol2 function for AoQ=2 misalignments corrections

   //-----IC-Si-----
   KVHashList* flist_HFcuts_icsi;
   std::vector<Int_t> fvec_nHF_icsi;

   KVHashList* flist_aoq_cut_icsi;
   Float_t ftof_corr_icsi;

   Bool_t fkfunc_ztof_icsi_init;
   TF1*    ffunc_ztof_icsi;

   //Readers in DataSet
   void ReadHFCutFilesListInDataSet();
   void ReadHFCutFileList(std::ifstream& file, Int_t type);
   void ReadDuplicationCutFilesListInDataSet();
   void ReadDuplicationCutFileList(std::ifstream& file, Int_t type);
   void ReadDuplicationToFOffsetsInDataSet();
   void ReadToFOffsetZFunctionFilesListInDataSet();
   void ReadToFOffsetZFunctionFileList(std::ifstream& file, Int_t type);

   //Corrections
   Bool_t ApplyHFCorrections(KVVAMOSReconNuc*, KVHashList*, std::vector<Int_t>);
   Bool_t ApplyToFDuplicationCorrections(KVVAMOSReconNuc*, KVHashList*, Float_t);
   Bool_t ApplyToFOffsetZFunctionCorrections(KVVAMOSReconNuc*, TF1*);

public:
   KVVAMOSDataCorrection_e503(Int_t run_number);
   virtual ~KVVAMOSDataCorrection_e503();
   virtual void Copy(TObject&) const;

   virtual void Init();
   virtual Bool_t ApplyCorrections(KVVAMOSReconNuc*);
   virtual void IdentifyCorrectedNucleus(KVVAMOSReconNuc*);
   virtual Double_t CalibrateFromDetList(KVVAMOSReconNuc*);

   void PrintInitInfos();
   virtual void SetVerbose(Bool_t status = kTRUE)
   {
      fkverbose = status;
   }

   Int_t GetNCutHFSiCsI()
   {
      return static_cast<int>(fvec_nHF_sicsi.size());
   }
   Int_t GetNCutHFICSi()
   {
      return static_cast<int>(fvec_nHF_icsi.size());
   }


   ClassDef(KVVAMOSDataCorrection_e503, 1) //Base class to use for VAMOS data corrections for e503 experiment
};

#endif
