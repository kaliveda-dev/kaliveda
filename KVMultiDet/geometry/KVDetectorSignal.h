//Created by KVClassFactory on Wed Jun  5 17:00:08 2019
//Author: John Frankland,,,


#ifndef __KVDETECTORSIGNAL_H
#define __KVDETECTORSIGNAL_H

#include "KVBase.h"
#include "KVNameValueList.h"

class KVDetector;
class KVNumberList;

/**
  \class KVDetectorSignal
  \ingroup Calibration
  \brief Base class for output signal data produced by a detector

 Detectors (see KVDetector) are composed of one or more absorber layers. Among these, a single \e active layer
allows to measure the energy loss of charged particles, which can be reconstructed by calibration of one
or more signals associated to the detector read out by some electronics/DAQ system. Both the raw data
and any calibrated quantities derived from them are handled by KVDetectorSignal and its child classes.

Each KVDetector has a list of associated signals:
~~~~{.cpp}
KVDetector det;

auto sig = det.AddDetectorSignal("raw");

det.GetListOfDetectorSignals().ls();
OBJ: KVUniqueNameList   KVSeqCollection_156  Optimised list in which objects with the same name can only be placed once : 0
 KVDetectorSignal        raw      Signal raw of detector Det_1        [0.000000]

sig->SetValue(50);
det.GetDetectorSignal("raw")->GetValue()
(double) 50.000000
~~~~

\sa KVDetectorSignalExpression
\sa KVCalibratedSignal
*/

class KVDetectorSignal : public KVBase {

   const KVDetector* fDetector;//! associated detector
   Double_t    fValue;// signal value
   Bool_t fFired;// set for raw parameters when read from raw data

public:
   KVDetectorSignal()
      : KVBase(), fDetector(nullptr), fValue(0), fFired(false)
   {}
   KVDetectorSignal(const Char_t* type, const KVDetector* det = nullptr);

   virtual ~KVDetectorSignal()
   {}

   virtual Double_t GetValue(const KVNameValueList& params = "") const
   {
      // \param[in] params [optional] list of extra parameters possibly required to calculate value of signal (see child classes)
      // \returns value of the signal

      IGNORE_UNUSED(params);
      return fValue;
   }
   virtual void SetValue(Double_t x)
   {
      // Set the value of the signal.
      // \param[in] x the value to be set
      // \note this has no effect on calibrated signals or signal expressions
      if (IsRaw() && !IsExpression()) fValue = x;
   }
   virtual void Reset()
   {
      // "Reset" the value of the signal, i.e. usually means set to zero.
      // Only affects signals whose value can be 'Set' (see SetValue).
      //
      // For raw parameters, also resets 'Fired' flag.
      SetValue(0);
      SetFired(false);
   }
   void Clear(Option_t* = "")
   {
      // Override base method in KVBase which resets the name, type and label of the object.
      //
      // Calls Reset(). The Option_t string is not used.
      Reset();
   }
   virtual Double_t GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params = "") const
   {
      //\param[in] out_val output value of signal to invert
      //\param[in] name/type of input signal to calculate by inversion
      //\param[in] [optional] list of extra parameters possibly required to calculate value of signal (see child classes)
      // \returns the value of the input signal for a given value of the output, using the inverse calibration function

      IGNORE_UNUSED(params);
      if (in_sig == GetName()) return out_val;
      return 0.;
   }

   void SetDetector(const KVDetector* d)
   {
      //\param[in] d pointer to detector to associate with this signal
      fDetector = d;
   }

   const KVDetector* GetDetector() const
   {
      //\returns pointer to detector associated with this signal
      return fDetector;
   }
   void SetFired(Bool_t yes = true)
   {
      //\param[in] yes set "Fired" status of signal
      fFired = yes;
   }
   Bool_t IsFired() const
   {
      //\returns "Fired" status of signal
      return fFired;
   }

   virtual Bool_t IsValid() const
   {
      //\returns kTRUE if this signal is valid (see child classes)
      return kTRUE;
   }

   virtual Bool_t IsRaw() const
   {
      // \returns kTRUE if signal is available without any calibration being defined i.e. corresponds to raw data
      return kTRUE;
   }

   virtual Bool_t IsExpression() const
   {
      // \returns kTRUE if this signal is a mathematical combination of other signals (see child classes)
      return kFALSE;
   }

   virtual Bool_t GetValueNeedsExtraParameters() const
   {
      // \returns kTRUE if GetValue() must be called with extra parameters in order to calculate the correct value
      return kFALSE;
   }
   void ls(Option_t* = "") const;

   virtual Int_t GetStatus(const TString&) const;

   TString GetFullName() const;

   ClassDef(KVDetectorSignal, 1) //Data produced by a detector
};

#endif
