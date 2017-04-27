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

   KVHashList* flist_aoq_cut_sicsi;    //list of TCutG* for AoQ duplication corrections
   Float_t ftof_corr_sicsi;            //correction of ToF (in ns) for AoQ duplication corrections

   //-----IC-Si-----
   KVHashList* flist_HFcuts_icsi;
   std::vector<Int_t> fvec_nHF_icsi;

   KVHashList* flist_aoq_cut_icsi;
   Float_t ftof_corr_icsi;

   //-----Global offset-----
   Float_t ftof_offset; //glogal offset in ToF to correct AoQ=2 misalignments
   Float_t fpath_offset; //global offset in Path to correct AoQ=2 misalignments

   //Readers in DataSet
   void ReadHFCutFilesListInDataSet();
   void ReadHFCutFileList(std::ifstream& file, Int_t type);
   void ReadDuplicationCutFilesListInDataSet();
   void ReadDuplicationCutFileList(std::ifstream& file, Int_t type);
   void ReadDuplicationToFOffsetsInDataSet();
   void ReadAdditionalToFAndPathOffsetsInDataSet();

   //Corrections
   Bool_t ApplyHFCorrections(KVVAMOSReconNuc*, KVHashList*, std::vector<Int_t>);
   Bool_t ApplyToFDuplicationCorrections(KVVAMOSReconNuc*, KVHashList*, Float_t);
   Bool_t ApplyToFAndPathOffset(KVVAMOSReconNuc*);


public:
   KVVAMOSDataCorrection_e503(Int_t run_number);
   virtual ~KVVAMOSDataCorrection_e503();
   virtual void Copy(TObject&) const;

   virtual void Init();
   virtual Bool_t ApplyCorrections(KVVAMOSReconNuc*);

   void PrintInitInfos();
   virtual void SetVerbose(Bool_t status = kTRUE) {
      fkverbose = status;
   }


   ClassDef(KVVAMOSDataCorrection_e503, 1) //Base class to use for VAMOS data corrections for e503 experiment
};

#endif
