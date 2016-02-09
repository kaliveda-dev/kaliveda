//Created by KVClassFactory on Tue Jul  6 17:30:28 2010
//Author: Eric Bonnet

#ifndef __KVSIMREADER_MMM_ASYM_H
#define __KVSIMREADER_MMM_ASYM_H

#include "KVSimReader.h"

class KVSimReader_MMM_asym : public KVSimReader {
protected:
   Int_t idx;

public:
   KVSimReader_MMM_asym();
   KVSimReader_MMM_asym(KVString filename);

   virtual ~KVSimReader_MMM_asym();

   void init()
   {
      tree_name = "MMM_asym";
      Info("init", "%s", branch_name.Data());
      idx = 0;
   }

   virtual void ReadFile();
   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();

   ClassDef(KVSimReader_MMM_asym, 1) //Read ascii file for asymptotic events of the MMM code after deexcitation
};

#endif
