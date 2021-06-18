#ifndef __KV_CALIBRATOR__
#define __KV_CALIBRATOR__

#include "KVBase.h"
#include "TRef.h"
#include "KVDetector.h"
#include "TF1.h"

/**
  \class KVCalibrator
  \ingroup Calibration
  \brief Base class for all detector calibrations

  You can define a basic calibrator object with a mathematical formula string (TFormula) and a type name:

  ~~~~~~~~~~~~~{.cpp}
  KVCalibrator* cal = new KVCalibrator("[0]*pow((x-[1]),[2])+[3]", "SomeCalibration");
  ~~~~~~~~~~~~~

  The formula will be used by a TF1, so you can retrieve e.g. the number of parameters with:

  ~~~~~~~~~~~~~{.cpp}
  cout << cal->GetNumberParams() << endl;
  4
  ~~~~~~~~~~~~~

  Setting and retrieving parameter values are handled by:

  ~~~~~~~~~~~~~{.cpp}
  cal->SetParameter(0, 26.7);

  cout << cal->GetParameter(0) << endl;
  26.7
  ~~~~~~~~~~~~~

  Before associating it with a detector to calibrate, you need to define the input 'signal' (see KVDetectorSignal class) which
  must be provided by the detector, and the name of the output 'signal' resulting from this calibration which will be added
  to the detector's list:

  ~~~~~~~~~~~~~{.cpp}
  cal->SetInputSignalType("RawDataParameter");
  cal->SetOutputSignalType("Energy");
  ~~~~~~~~~~~~~

  The input 'signal' is of course the quantity that will be used in place of `x` in the formula given to the constructor.

    > N.B. if the specified input signal is not defined for the detector the calibrator is associated with,
    > no new output signal will be generated for the detector and the calibrator will be unused.

  Associating the calibrator with a detector:

  ~~~~~~~~~~~~~{.cpp}
  // KVDetector det; // some detector or other
  det.AddCalibrator(cal);

  det.GetDetectorSignalValue("Energy"); // will use current value of "RawDataParameter" and the calibration parameters
  ~~~~~~~~~~~~~

    > N.B. the calibrator object is 'adopted' by the detector and will be deleted by the detector.
    > Do not try to use the same calibrator object with more than one detector!

  Adding to the detector will set the name of the calibrator object:

  ~~~~~~~~~~~~~{.cpp}
  cout << det.GetName() << endl;
  Det_A14

  cout << cal->GetName() << endl;
  Det_A14_SomeCalibration
  ~~~~~~~~~~~~~

In order to add calibrations to a dataset,
it needs to have a file `[array_name].CalibrationFiles.dat` which contains the names of files containing
different calibrations for different detectors and different runs.

Each of these files should have the same basic structure:
~~~~~~~~~~
# Any useful comments on a line starting with '#'
# Remember that comments are always useful
# You will remember nothing about this calibration in 3 months' time...
SignalIn:                                PG
SignalOut:                               Volts
CalibType:                               ChannelVolt
CalibOptions:                            func=pol3,min=0,max=1,inverse=true
SI_0801:                                 71.9228,3979.44,32.1727,-27.9312
SI_0802:                                 70.9138,3978.88,31.5155,-27.9611
[etc. etc.]
~~~~~~~~~~

This file defines how to transform the `PG` signal of the listed detectors into a new `Volts` signal.

See the <a href="http://indra.in2p3.fr/kaliveda/UsersGuide/calibrations.html#detector-calibration">chapter</a> in the User's Guide for more details.

\sa KVCalibratedSignal
 */

class KVCalibrator: public KVBase {

   KVDetector* fDetector;           //! associated detector
   TF1*        fCalibFunc;          //! calibration function
   Bool_t      fReady;              //  = kTRUE if calibration formula & parameters have been set up
   TString     fInputSignal;        //  type of signal used as input
   TString     fOutputSignal;       //  type of output calibrated signal
   Bool_t      fUseInverseFunction; // true if inverse i.e. TF1::GetX should be used for Compute()
   Double_t    fInputMin;           // required minimum input signal for inverse calibration
   Double_t    fInputMax;           // required maximum input signal for inverse calibration
   mutable Bool_t fInversionFail;   // problem inverting calibration function

protected:
   void SetCalibFunction(TF1* f)
   {
      // Set function for calibration. Delete any previous function.
      SafeDelete(fCalibFunc);
      fCalibFunc = f;
   }
   TF1* GetCalibFunction() const
   {
      return fCalibFunc;
   }
   Double_t do_inversion(Double_t x) const
   {
      // Invert calibration function to find input value corresponding to output value x.
      //
      // In case of problems with the inversion (x not included in the range of values of the
      // function) InversionFail() will return kTRUE.

      int status;
      Double_t res = ProtectedGetX(fCalibFunc, x, status); // fCalibFunc->GetX(x);
      fInversionFail = (status != 0);
      return res;
   }
   bool in_range(double X, double Xmin, double Xmax)
   {
      // return true if X is within the given range
      return (X >= Xmin) && (X <= Xmax);
   }
   void adjust_range_of_inverse_calibration();
public:
   KVCalibrator()
      : KVBase("Calibrator", "KVCalibrator"), fDetector(nullptr), fCalibFunc(nullptr), fReady(kFALSE), fUseInverseFunction(kFALSE), fInputMin(99), fInputMax(-1), fInversionFail(kFALSE) {}
   KVCalibrator(const TString& formula, const TString& type)
      : KVBase("Calibrator", type), fDetector(nullptr), fCalibFunc(new TF1("KVCalibrator::fCalibFunc", formula)), fReady(kFALSE), fUseInverseFunction(kFALSE), fInputMin(99), fInputMax(-1), fInversionFail(kFALSE)
   {
      // Set up calibrator using mathematical formula
   }
   virtual ~KVCalibrator()
   {
      SafeDelete(fCalibFunc);
   }

   Int_t GetNumberParams() const
   {
      // return number of parameters in formula
      return (fCalibFunc ? fCalibFunc->GetNpar() : 0);
   }
   void SetParameter(int i, Double_t par_val) const
   {
      if (fCalibFunc) fCalibFunc->SetParameter(i, par_val);
   }
   Double_t GetParameter(int i) const
   {
      return (fCalibFunc ? fCalibFunc->GetParameter(i) : 0);
   }
   void SetStatus(Bool_t ready)
   {
      fReady = ready;
      if (ready)
         if (IsUseInverseFunction())
            adjust_range_of_inverse_calibration();
   }
   Bool_t GetStatus() const
   {
      return fReady;
   }
   virtual void Print(Option_t* opt = "") const;
   virtual void Reset()
   {
      for (Int_t i = 0; i < GetNumberParams(); i++) {
         SetParameter(i, 0.0);
      }
      SetStatus(kFALSE);
   }
   virtual Double_t Compute(Double_t x, const KVNameValueList&) const
   {
      // Compute calibrated value from input x

      return (fUseInverseFunction ? do_inversion(x) : fCalibFunc->Eval(x));
   }
   virtual Double_t Invert(Double_t x, const KVNameValueList&) const
   {
      // Compute value of input for given output value (inverted calibration)

      return (!fUseInverseFunction ? do_inversion(x) : fCalibFunc->Eval(x));
   }
   Double_t operator()(Double_t x, const KVNameValueList& par = "")
   {
      // Same as Compute(x,par)
      return Compute(x, par);
   }
   //virtual TGraph* MakeGraph(Double_t xmin, Double_t xmax, Int_t npoints = 50) const;

   static KVCalibrator* MakeCalibrator(const Char_t* type);

   virtual void SetOptions(const KVNameValueList&);
   void SetInputSignalType(const TString& type)
   {
      fInputSignal = type;
   }
   void SetOutputSignalType(const TString& type)
   {
      fOutputSignal = type;
   }
   TString GetInputSignalType() const
   {
      return fInputSignal;
   }
   TString GetOutputSignalType() const
   {
      return fOutputSignal;
   }
   void SetDetector(KVDetector* d)
   {
      fDetector = d;
   }
   KVDetector* GetDetector() const
   {
      return fDetector;
   }

   void SetUseInverseFunction(Bool_t yes = kTRUE)
   {
      // Set true if the calibration function is actually the inverse
      // i.e. if TF1::GetX(x) should be called in Compute() instead of TF1::Eval(x)
      fUseInverseFunction = yes;
   }
   Bool_t IsUseInverseFunction() const
   {
      // Returns true if the calibration function is actually the inverse
      // i.e. if TF1::GetX(x) should be called in Compute() instead of TF1::Eval(x)
      return fUseInverseFunction;
   }
   Bool_t InversionFailure() const
   {
      // Returns kTRUE after a failed attempt to invert the calibration function
      // Value returned by calibrator should not be used in this case
      return fInversionFail;
   }

   ClassDef(KVCalibrator, 2)//Base class for calibration of detectors
};

#endif
