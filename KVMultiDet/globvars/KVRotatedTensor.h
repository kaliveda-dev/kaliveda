//Created by KVClassFactory on Wed Jan 18 14:44:36 2017
//Author: John Frankland,,,

#ifndef __KVROTATEDTENSOR_H
#define __KVROTATEDTENSOR_H

#include "KVEvent.h"
#include "KVTensPCM.h"

class KVRotatedTensor: public KVTensPCM {

   Int_t fNROT;//number of rotations to perform
   TRotation fRot;//the rotation
   void init_KVRotatedTensor();

public:
   KVRotatedTensor(void);
   KVRotatedTensor(const Char_t* nom);
   KVRotatedTensor(const KVRotatedTensor& a);

   virtual ~KVRotatedTensor(void);

   virtual void Copy(TObject& obj) const;

   KVRotatedTensor& operator = (const KVRotatedTensor& a);

   virtual void Init(void);
   void FillN(KVEvent* e);

   ClassDef(KVRotatedTensor, 1) //Tensor analysis using multiple rotations of events around beam axis
};


#endif
