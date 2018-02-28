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
   KVHashList* flist_HFcuts_sicsi;            //list of TCutG* for HF frequency corrections
   std::vector<Float_t> fvec_nHF_DE_ToF_sicsi;  //vector containing the number of times the beam pulse period must be corrected for each DeltaE_ToF cut
   std::vector<Float_t> fvec_nHF_AE_AoQ_sicsi;  //vector containing the number of times the beam pulse period must be corrected for each AE_AoQ cut

   KVHashList* flist_aoq_cut_sicsi;    //list of RealZ:BasicAoQ TCutG* for AoQ duplication corrections
   Float_t ftof_corr_sicsi;            //correction of ToF (in ns) for AoQ duplication corrections
   Float_t ftof_corr_sicsi_global;     //a global ToF correction (in ns) to correct AoQ=2 misalignments

   Bool_t fkfunc_ztof_sicsi_init; //=kTRUE if the pol2 function was correctly init
   TF1*   ffunc_ztof_sicsi; //pol2 function for AoQ=2 misalignments corrections for Si-CsI telescopes

   //-----IC-Si-----
   KVHashList* flist_HFcuts_icsi;
   std::vector<Float_t> fvec_nHF_DE_ToF_icsi;
   std::vector<Float_t> fvec_nHF_AE_AoQ_icsi;

   KVHashList* flist_aoq_cut_icsi;
   Float_t ftof_corr_icsi;
   Float_t ftof_corr_icsi_global;

   Bool_t fkfunc_ztof_icsi_init;
   TF1*   ffunc_ztof_icsi_11; //IDCode 11
   TF1*   ffunc_ztof_icsi_4; //IDCode 4

   //Readers in DataSet
   //global offset on ToF before any corrections
   void ReadGlobalToFOffsetsInDataSet();
   //HF beam frequency corrections
   void ReadHFCutFilesListInDataSet();
   void ReadHFCutFileList(std::ifstream& file, Int_t type);
   //ToF duplication correction
   void ReadDuplicationCutFilesListInDataSet();
   void ReadDuplicationCutFileList(std::ifstream& file, Int_t type);
   void ReadDuplicationToFOffsetsInDataSet();
   //ToF A/Q=2 misalignment correction
   void ReadToFOffsetZFunctionFilesListInDataSet();
   void ReadToFOffsetZFunctionFileList(std::ifstream& file, Int_t type);

   //Corrections
   //global offset on ToF before any corrections
   Bool_t ApplyGlobalToFCorrection(KVVAMOSReconNuc*, Float_t);
   //HF beam frequency corrections
   Bool_t ApplyHFCorrections(KVVAMOSReconNuc*);
   Bool_t ApplyHFCorrectionsDeltaE_ToF(KVVAMOSReconNuc*);
   Bool_t ApplyHFCorrectionsAE_AoverQ(KVVAMOSReconNuc*);
   //ToF duplication correction
   Bool_t ApplyToFDuplicationCorrections(KVVAMOSReconNuc*, KVHashList*, Float_t);
   //ToF A/Q=2 misalignment correction
   Bool_t ApplyToFOffsetZFunctionCorrections(KVVAMOSReconNuc*, TF1*);

   //Identification
   Bool_t IdentifyCorrectedNucleus(KVVAMOSReconNuc*);

public:
   KVVAMOSDataCorrection_e503(Int_t run_number);
   virtual ~KVVAMOSDataCorrection_e503();
   virtual void Copy(TObject&) const;

   virtual void Init();
   virtual Bool_t ApplyCorrections(KVVAMOSReconNuc*);
   virtual void CalibrateFromDetList(KVVAMOSReconNuc*, Double_t& ke, Double_t& eBeforeVAMOS);

   void PrintInitInfos();
   virtual void SetVerbose(Bool_t status = kTRUE)
   {
      fkverbose = status;
   }

   Int_t GetNCutHFSiCsI_DeltaE_ToF()
   {
      return static_cast<int>(fvec_nHF_DE_ToF_sicsi.size());
   }
   Int_t GetNCutHFICSi_DeltaE_ToF()
   {
      return static_cast<int>(fvec_nHF_DE_ToF_icsi.size());
   }

   Int_t GetNCutHFSiCsI_AE_AoverQ()
   {
      return static_cast<int>(fvec_nHF_AE_AoQ_sicsi.size());
   }
   Int_t GetNCutHFICSi_AE_AoverQ()
   {
      return static_cast<int>(fvec_nHF_AE_AoQ_icsi.size());
   }


   ClassDef(KVVAMOSDataCorrection_e503, 1) //Base class to use for VAMOS data corrections for e503 experiment
};

#endif
