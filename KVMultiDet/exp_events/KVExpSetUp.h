//Created by KVClassFactory on Thu Feb 25 14:59:59 2016
//Author: bonnet,,,

#ifndef __KVEXPSETUP_H
#define __KVEXPSETUP_H

#include "KVMultiDetArray.h"

class KVExpSetUp : public KVMultiDetArray {

protected:
   KVList fMDAList;  //list of multidetarrays
   KVString lmultidetarrayclasses;

   void init();

public:

   KVExpSetUp();
   virtual ~KVExpSetUp();
   virtual void Build(Int_t run = -1);

   void SetParameters(UShort_t n);
   void FillDetectorList(KVReconstructedNucleus* rnuc, KVHashList* DetList, const KVString& DetNames);

   ClassDef(KVExpSetUp, 1) //Describe an experimental set-up made of several KVMultiDetArray objects
};

#endif
