//Created by KVClassFactory on Tue Jul  6 17:30:28 2010
//Author: Eric Bonnet

#ifndef __KVSIMREADER_MMM_ASYM_H
#define __KVSIMREADER_MMM_ASYM_H

#include "KVSimReader.h"

/**
\class KVSimReader_MMM_asym
\brief Read ascii file for asymptotic events of the MMM code after deexcitation
\ingroup Simulation
*/

class KVSimReader_MMM_asym : public KVSimReader {
   void init()
   {
      tree_name = "MMM_asym";
      Info("init", "%s", branch_name.Data());
      idx = 0;
      fApplyBoost = kFALSE;
   }

protected:
   Int_t idx;
   Bool_t fApplyBoost;
   TVector3 fBoostQP;
   TVector3 fBoostQC;

public:
   KVSimReader_MMM_asym();
   KVSimReader_MMM_asym(KVString filename);
   void SetBoost(TVector3& vQP, TVector3& vQC);

   virtual ~KVSimReader_MMM_asym();

   virtual void ReadFile();
   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();

   ClassDef(KVSimReader_MMM_asym, 1) //Read ascii file for asymptotic events of the MMM code after deexcitation
};

#endif
