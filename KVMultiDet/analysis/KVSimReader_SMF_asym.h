//Created by KVClassFactory on Mon Jul  5 15:24:25 2010
//Author: bonnet

#ifndef __KVSIMREADER_SMF_ASYM_H
#define __KVSIMREADER_SMF_ASYM_H

#include "KVSimReader.h"

class KVSimReader_SMF_asym : public KVSimReader {

protected:
   Int_t ndes;

public:
   KVSimReader_SMF_asym();
   KVSimReader_SMF_asym(KVString filename);

   void init()
   {
      tree_name = "SMF_asym";
      ndes = 0;
   }

   virtual ~KVSimReader_SMF_asym();

   virtual void ReadFile();
   virtual Bool_t ReadHeader();
   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();


   ClassDef(KVSimReader_SMF_asym, 1) //Read ascii file for asymptotic events of the SMF code after SIMON deexcitation
};

#endif
