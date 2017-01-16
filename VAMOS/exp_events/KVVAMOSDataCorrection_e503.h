//Created by KVClassFactory on Mon Jan 16 11:36:31 2017
//Author: Quentin Fable,,,

#ifndef __KVVAMOSDATACORRECTION_E503_H
#define __KVVAMOSDATACORRECTION_E503_H

#include "KVVAMOSDataCorrection.h"

class KVVAMOSDataCorrection_e503 : public KVVAMOSDataCorrection {

protected:
   Double_t ftof_corr_sicsi;          //correction of ToF (in ns) for AoQ duplication correction for SiCsI telescopes
   Double_t ftof_corr_icsi;           //correction of ToF (in ns) for AoQ duplication correction for ICSi telescopes
   KVList* flist_aoq_cut_sicsi;       //list of TCutG* for AoQ duplication correction for SiCsI telescopes
   KVList* flist_aoq_cut_icsi;        //list of TCutG* for AoQ duplication correction for ICSi telescopes

   void FindToFCorrectionAoQDuplicationSiCsI();
   void FindToFCorrectionAoQDuplicationICSi();
   void CreateAoQDuplicationCutsSiCsI();
   void CreateAoQDuplicationCutsICSi();

   Bool_t ApplyAoverQDuplicationCorrections(KVVAMOSReconNuc*, KVList*);

public:
   KVVAMOSDataCorrection_e503();
   virtual ~KVVAMOSDataCorrection_e503();

   virtual void ApplyCorrections(KVVAMOSReconNuc*);

   ClassDef(KVVAMOSDataCorrection_e503, 1) //A KVVAMOSDataCorrection derived class specific to e503 experiment
};

#endif
