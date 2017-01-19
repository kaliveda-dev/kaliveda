//Created by KVClassFactory on Mon Jan 16 11:36:31 2017
//Author: Quentin Fable,,,

#ifndef __KVVAMOSDATACORRECTION_E503_H
#define __KVVAMOSDATACORRECTION_E503_H

#include "KVVAMOSDataCorrection.h"

class KVVAMOSDataCorrection_e503 : public KVVAMOSDataCorrection {

protected:
   Bool_t   fkverbose;
   Bool_t   fkInitialised;
   Bool_t   fkListSiCsI_OK;
   Bool_t   fkListICSi_OK;
   Bool_t   fkToFSiCsI_OK;
   Bool_t   fkToFICSi_OK;

   Double_t ftof_corr_sicsi;          //correction of ToF (in ns) for AoQ duplication correction for SiCsI telescopes
   Double_t ftof_corr_icsi;           //correction of ToF (in ns) for AoQ duplication correction for ICSi telescopes
   KVList* flist_aoq_cut_sicsi;       //list of TCutG* for AoQ duplication correction for SiCsI telescopes
   KVList* flist_aoq_cut_icsi;        //list of TCutG* for AoQ duplication correction for ICSi telescopes

   virtual Bool_t FindToFCorrectionAoQDuplicationSiCsI();
   virtual Bool_t FindToFCorrectionAoQDuplicationICSi();
   virtual Bool_t CreateAoQDuplicationCutsSiCsI();
   virtual Bool_t CreateAoQDuplicationCutsICSi();

   Bool_t ApplyAoverQDuplicationCorrections(KVVAMOSReconNuc*, KVList*);

public:
   KVVAMOSDataCorrection_e503();
   virtual ~KVVAMOSDataCorrection_e503();

   virtual void Copy(TObject&) const;

   virtual Bool_t Init();
   virtual void ApplyCorrections(KVVAMOSReconNuc*);

   virtual void SetVerbose(Bool_t status = kTRUE);

   ClassDef(KVVAMOSDataCorrection_e503, 1) //A KVVAMOSDataCorrection derived class specific to e503 experiment
};

#endif
