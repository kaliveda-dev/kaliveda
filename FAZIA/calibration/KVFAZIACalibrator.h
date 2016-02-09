//Created by KVClassFactory on Thu Dec  3 15:59:09 2015
//Author: bonnet,,,

#ifndef __KVFAZIACALIBRATOR_H
#define __KVFAZIACALIBRATOR_H

#include "KVCalibrator.h"

class KVFAZIACalibrator : public KVCalibrator {

public:
   KVFAZIACalibrator();
   KVFAZIACalibrator(const Char_t* name, const Char_t* type, UShort_t pnum);
   virtual ~KVFAZIACalibrator();

   virtual Double_t Compute(Double_t x) const;
   virtual Double_t operator()(Double_t x);
   virtual Double_t Invert(Double_t x);

   ClassDef(KVFAZIACalibrator, 1) //Basic class for FAZIA calibrators
};

#endif
