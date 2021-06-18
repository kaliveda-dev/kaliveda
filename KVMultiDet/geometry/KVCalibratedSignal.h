//Created by KVClassFactory on Thu Jun 13 09:49:14 2019
//Author: John Frankland,,,

#ifndef __KVCALIBRATEDSIGNAL_H
#define __KVCALIBRATEDSIGNAL_H

#include "KVDetectorSignal.h"
#include "KVCalibrator.h"

/**
 \class KVCalibratedSignal
 \ingroup Calibration
 \brief Output signal from detector obtained by calibration

Calibrated signals are added to detectors when a calibration is available for a given run of a dataset (experiment).
A calibration basically defines how a new calibrated signal is generated from an existing input signal
using a KVCalibrator or derived class:

~~~~~~~~~~~~

   KVDetectorSignal  _______\   KVCalibrator  _______\  KVCalibratedSignal
       'input'              /                        /       'output'

~~~~~~~~~~~~

Supposing a KVCalibrator object has been defined which transforms a signal `raw_ADC` into a signal `Energy`,

~~~~{.cpp}
auto cal = new KVCalibrator("[0]+[1]*x","Energy Calibration");
cal->SetInputSignalType("raw_ADC");
cal->SetOutputSignalType("Energy");
cal->SetParameter(0,0.034)
cal->SetParameter(1,2.465e-03)
~~~~

adding this calibration to a detector which has a signal `raw_ADC` will add a new KVCalibratedSignal `Energy` to the detector:

~~~~{.cpp}
KVDetector det;
det.AddDetectorSignal("raw_ADC");
det.AddCalibrator(cal);

det.GetListOfDetectorSignals().ls();
OBJ: KVUniqueNameList   KVSeqCollection_156  Optimised list in which objects with the same name can only be placed once : 0
 KVDetectorSignal        raw_ADC        Signal raw_ADC of detector Det_1       [0.000000]
 KVCalibratedSignal      Energy      Signal Energy calculated from signal raw_ADC of detector Det_1       [0.034000]
~~~~

and now whenever the `raw_ADC` signal is read/set from data, the `Energy` signal will be calculated from the calibration:

~~~~{.cpp}
det.SetDetectorSignalValue("raw_ADC",45630);

det.GetDetectorSignalValue("Energy")
(double) 112.51195

det.GetEnergy()   // special case: KVDetector::GetEnergy() returns value of "Energy" signal
(double) 112.51195
~~~~

See the <a href="http://indra.in2p3.fr/kaliveda/UsersGuide/calibrations.html#detector-calibration">chapter</a> in the User's Guide for more details.

\sa KVCalibrator
 */

class KVCalibratedSignal : public KVDetectorSignal {

   friend class KVDetector;

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
   KVCalibratedSignal(KVDetectorSignal* input, KVCalibrator* calib)
      : KVDetectorSignal(calib->GetOutputSignalType(), input->GetDetector()), fInputSignal(input), fCalibrator(calib), fInversionFail(false)
   {
      // Constructor used by KVDetector::AddCalibrator()
      SetTitle(Form("Signal %s calculated from signal %s of detector %s", GetName(), input->GetName(), GetDetector()->GetName()));
   }
public:
   KVCalibratedSignal()
      : KVDetectorSignal(), fInputSignal(nullptr), fCalibrator(nullptr), fInversionFail(false)
   {}
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
