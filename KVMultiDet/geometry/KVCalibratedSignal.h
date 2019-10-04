//Created by KVClassFactory on Thu Jun 13 09:49:14 2019
//Author: John Frankland,,,

#ifndef __KVCALIBRATEDSIGNAL_H
#define __KVCALIBRATEDSIGNAL_H

#include "KVDetectorSignal.h"
#include "KVCalibrator.h"

class KVCalibratedSignal : public KVDetectorSignal {
   KVDetectorSignal* fInputSignal;// signal which is used as input to generate calibrated signal
   KVCalibrator*     fCalibrator;// calibrator used to transform input signal

public:
   KVCalibratedSignal()
      : KVDetectorSignal(), fInputSignal(nullptr), fCalibrator(nullptr)
   {}
   KVCalibratedSignal(KVDetectorSignal* input, KVCalibrator* calib)
      : KVDetectorSignal(calib->GetOutputSignalType(), input->GetDetector()), fInputSignal(input), fCalibrator(calib)
   {
      SetTitle(Form("Signal %s calculated from signal %s of detector %s", GetName(), input->GetName(), GetDetector()->GetName()));
   }
   virtual ~KVCalibratedSignal()
   {}

   Double_t GetValue() const;
   Double_t GetInverseValue(Double_t out_val, const TString& in_sig) const;

   KVCalibrator* GetCalibrator() const
   {
      return fCalibrator;
   }
   Bool_t IsRaw() const
   {
      // Return kFALSE: this is not raw data, it is produced by a calibration procedure
      return kFALSE;
   }

   ClassDef(KVCalibratedSignal, 1) //Detector signal produced by a calibrator
};

#endif
