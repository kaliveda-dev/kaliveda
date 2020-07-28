//Created by KVClassFactory on Thu Jun 13 09:49:14 2019
//Author: John Frankland,,,

#ifndef __KVCALIBRATEDSIGNAL_H
#define __KVCALIBRATEDSIGNAL_H

#include "KVDetectorSignal.h"
#include "KVCalibrator.h"

/**
 \class KVCalibratedSignal
 \ingroup Calibration
 \brief Calibrated output from detector
 */

class KVCalibratedSignal : public KVDetectorSignal {

   KVDetectorSignal* fInputSignal;// signal which is used as input to generate calibrated signal
   KVCalibrator*     fCalibrator;// calibrator used to transform input signal
   mutable Bool_t fInversionFail;// set to true when failing to compute an inverted calibrator

protected:
   KVCalibratedSignal(KVDetectorSignal* input, const KVString& output)
      : KVDetectorSignal(output, input->GetDetector()), fInputSignal(input), fCalibrator(nullptr), fInversionFail(false)
   {
      // Constructor used by KVZDependentCalibratedSignal
      SetTitle(Form("Signal %s calculated from signal %s of detector %s", GetName(), input->GetName(), GetDetector()->GetName()));
   }
public:
   KVCalibratedSignal()
      : KVDetectorSignal(), fInputSignal(nullptr), fCalibrator(nullptr), fInversionFail(false)
   {}
   KVCalibratedSignal(KVDetectorSignal* input, KVCalibrator* calib)
      : KVDetectorSignal(calib->GetOutputSignalType(), input->GetDetector()), fInputSignal(input), fCalibrator(calib), fInversionFail(false)
   {
      SetTitle(Form("Signal %s calculated from signal %s of detector %s", GetName(), input->GetName(), GetDetector()->GetName()));
   }
   virtual ~KVCalibratedSignal()
   {}

   Double_t GetValue(const KVNameValueList& params = "") const;
   Double_t GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params = "") const;

   KVCalibrator* GetCalibrator() const
   {
      return fCalibrator;
   }
   Bool_t IsRaw() const
   {
      // Return kFALSE: this is not raw data, it is produced by a calibration procedure
      return kFALSE;
   }
   Bool_t InversionFailure() const
   {
      return fInversionFail;
   }
   virtual Bool_t IsCalibratedFor(const KVNameValueList&) const
   {
      // Can be used in derived classes to determine if a calibration is effectively
      // available based on the extra parameters in the KVNameValueList.
      //
      // Default behaviour is to return kTRUE in all cases.

      return kTRUE;
   }

   ClassDef(KVCalibratedSignal, 1) //Detector signal produced by a calibrator
};

#endif
