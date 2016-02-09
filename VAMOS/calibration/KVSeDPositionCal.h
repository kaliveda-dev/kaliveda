//Created by KVClassFactory on Wed Feb 13 10:18:35 2013
//Author: Guilain ADEMARD

#ifndef __KVSEDPOSITIONCAL_H
#define __KVSEDPOSITIONCAL_H

#include "KVCalibrator.h"
#include "KVMacros.h" // 'UNUSED' macro

#define SEDPOSCAL_FIT_ORDER 4
#define NPAR_SEDPOSCAL 4*(SEDPOSCAL_FIT_ORDER+1)*(SEDPOSCAL_FIT_ORDER+1)

class KVSeDPositionCal : public KVCalibrator {

public:
   KVSeDPositionCal();
   KVSeDPositionCal(KVDetector* det);
   virtual ~KVSeDPositionCal();

   virtual Double_t Compute(Double_t chan) const;
   virtual Double_t operator()(Double_t chan);
   virtual Double_t Invert(Double_t volts);

   virtual Bool_t Compute(Double_t X, Double_t Y, Double_t& Xcal, Double_t& Ycal);
   virtual Bool_t operator()(Double_t X, Double_t Y, Double_t& Xcal, Double_t& Ycal);

   ClassDef(KVSeDPositionCal, 1) //Position calibration for SeD detectors of VAMOS
};

#endif
