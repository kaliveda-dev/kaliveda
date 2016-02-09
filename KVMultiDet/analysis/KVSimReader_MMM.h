//Created by KVClassFactory on Tue Jul  6 16:48:05 2010
//Author: Eric Bonnet

#ifndef __KVSIMREADER_MMM_H
#define __KVSIMREADER_MMM_H

#include "KVSimReader.h"

class KVSimReader_MMM : public KVSimReader {

protected:
   Int_t idx;

public:
   KVSimReader_MMM();
   KVSimReader_MMM(KVString filename);

   virtual ~KVSimReader_MMM();

   void init()
   {
      tree_name = "MMM";
      Info("init", "%s", branch_name.Data());
      idx = 0;
   }

   virtual void ReadFile();
   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();

   ClassDef(KVSimReader_MMM, 1) //Read ascii file for events of the MMM code at Freeze Out
};

#endif
