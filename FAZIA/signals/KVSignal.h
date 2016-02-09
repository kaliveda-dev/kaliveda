//Created by KVClassFactory on Mon Dec 22 15:46:46 2014
//Author: ,,,

#ifndef __KVSIGNAL_H
#define __KVSIGNAL_H

#include "TGraph.h"
#include "TArrayF.h"
#include "TH1F.h"

class KVPSAResult;
class KVDBParameterList;

class KVSignal : public TGraph {
public:
   enum SignalType {
      kQH1,
      kI1,
      kQL1,
      kQ2,
      kI2,
      kQ3,
      kADC,
      kUNKDT
   };


protected:
   Int_t fBlock;
   Int_t fQuartet;
   TString fQuartetName;
   Int_t fTelescope;
   Int_t fChannel;                 //! signal type (see KVSignal::SignalType enum)
   TString fTelName;
   TString fDetName;
   TString fType;
   TString fDet;

   TArrayF fAdc;                    //! needed to use the psa methods copied from FClasses of Firenze
   //results of signal treatement
   Double_t fAmplitude;             // amplitude of the signal
   Double_t fRiseTime;              // rise time of the signal
   Double_t fIMax;                  // position of the maximum in channel
   Double_t fTMax;                  // position of the maximum in ns
   Double_t fYmin, fYmax;           // raw min/max of the signal
   Double_t fBaseLine;              // base line mean value
   Double_t fSigmaBase;             // base line rms

   //parameters for signal treatment
   //which can be changed in the LoadPSAParameters() method
   //if they are defined in the .kvrootrc file
   Double_t fChannelWidth;          // channel width in ns
   Double_t fInterpolatedChannelWidth; // channel width used to produced the interpolated signal
   Int_t fFirstBL, fLastBL;         // first and last channel number to compute the base line
   Double_t fTauRC;                 // tau_rc of the electronics. Used for pole zero cancellation.
   Double_t fTrapRiseTime;          // rise time of the trapezoidal shaper
   Double_t fTrapFlatTop;           // flat top of the trapezoidal shaper
   Double_t fSemiGaussSigma;            // sigma of the semi-gaussian shaper
   Bool_t   fWithPoleZeroCorrection;// use or nor pole zero correction
   Bool_t   fWithInterpolation;     // use of interpolation or not
   Double_t fMinimumValueForAmplitude; //Minimum value to say if detector has been hitted

   //internal parameters
   //
   Bool_t   fPSAIsDone;             // indicate if PSA has been done
   Double_t fChannelWidthInt;       // internal parameter channel width of interpolated signal in ns
   void ResetIndexes();
   virtual void BuildCubicSignal(); //Interpolazione mediante cubic
   virtual void BuildCubicSplineSignal(); //Interpolazione mediante cubic
   void init();

public:
   KVSignal();
   KVSignal(const char* name, const char* title);
   KVSignal(const TString& name, const TString& title);
   virtual ~KVSignal();
   void Copy(TObject& obj) const;

   //
   //routines to link signal to its detector in kaliveda framework
   //
   void SetDetectorName(const Char_t* detname);
   const Char_t* GetDetectorName()  const
   {
      return fDetName.Data();
   }
   void SetDetector(const Char_t* det);
   const Char_t* GetDetector()      const
   {
      return fDet.Data();
   }
   void SetType(const Char_t* type)
   {
      fType = type;
   }
   const Char_t* GetType()          const
   {
      return fType.Data();
   }
   //
   void DeduceFromName();
   Int_t GetBlockNumber()           const
   {
      return fBlock;
   }
   Int_t GetQuartetNumber()         const
   {
      return fQuartet;
   }
   const Char_t* GetQuartetName()   const
   {
      return fQuartetName.Data();
   }
   Int_t GetTelescopeNumber()       const
   {
      return fTelescope;
   }
   const Char_t* GetTelescopeName() const
   {
      return fTelName.Data();
   }
   virtual Bool_t IsCharge() const
   {
      return kFALSE;
   }
   virtual Bool_t IsCurrent() const
   {
      return kFALSE;
   }

   void Print(Option_t* chopt = "") const;

   //operation on data arrays
   void SetData(Int_t nn, Double_t* xx, Double_t* yy);
   virtual void Set(Int_t n);
   void SetADCData();
   TArrayF* GetArray()
   {
      return &fAdc;
   }

   //
   //routines to read/change PSA parameters from configuration file or database
   //
   Double_t GetPSAParameter(const Char_t* parname);
   virtual void LoadPSAParameters();
   virtual void SetDefaultValues();
   virtual void UpdatePSAParameter(KVDBParameterList* par);

   //
   //routines to launch and control PSA
   //
   virtual void TreateSignal();
   virtual KVPSAResult* GetPSAResult() const;
   Bool_t PSAHasBeenComputed() const
   {
      return fPSAIsDone;
   }

   //
   //Indicate the value in (ns or µs) of TGraph X axis width
   //
   void SetChannelWidth(double width)
   {
      fChannelWidth = width;
      fChannelWidthInt = width;
   }
   Double_t GetChannelWidth() const
   {
      return fChannelWidth;
   }
   Bool_t TestWidth() const;
   void ChangeChannelWidth(Double_t newwidth);
   //
   void SetMaxT(double t)
   {
      fAdc.Set((int)(t / fChannelWidth));
   }
   //
   void SetNSamples(int nn)
   {
      fAdc.Set(nn);
   }
   Int_t GetNSamples() const
   {
      return fAdc.GetSize();
   }

   //
   //routines dedicated to calculate the baseline
   //
   void SetBaseLineLength(Int_t length, Int_t first = 0)
   {
      fFirstBL = first;
      fLastBL = length - first;
   }
   Double_t GetBLFirst() const
   {
      return fFirstBL;
   }
   Double_t GetBLLength() const
   {
      return fLastBL - fFirstBL;
   }
   virtual Double_t ComputeBaseLine();
   Double_t GetBaseLine() const
   {
      return fBaseLine;
   }
   Double_t GetSigmaBaseLine()const
   {
      return fSigmaBase;
   }

   //
   //routines dedicated to calculate the rise time (use of filter)
   //
   Double_t ComputeRiseTime();
   Double_t GetRiseTime() const
   {
      return fRiseTime;
   }

   //
   //routines dedicated to calculate the amplitude (use of filter)
   //
   Double_t ComputeAmplitude();
   Double_t GetAmplitude() const
   {
      return fAmplitude;
   }

   //
   //parameters for trapezoidal filter
   //
   void SetTrapShaperParameters(Double_t rise, Double_t flat)
   {
      fTrapRiseTime = rise;
      fTrapFlatTop = flat;
   }
   void SetShaperRiseTime(Double_t rise)
   {
      fTrapRiseTime = rise;
   }
   void SetShaperFlatTop(Double_t flat)
   {
      fTrapFlatTop = flat;
   }
   Double_t GetShaperRiseTime()const
   {
      return fTrapRiseTime;
   }
   Double_t GetShaperFlatTop() const
   {
      return fTrapFlatTop;
   }

   //
   //parameters for semi gaussian filter
   //
   void SetSemiGaussSigma(Double_t sig)
   {
      fSemiGaussSigma = sig;
   }
   Double_t GetSemiGaussSigma() const
   {
      return fSemiGaussSigma;
   }

   //
   //parameters for Pole-Zero correction
   //
   void SetPoleZeroCorrection(Bool_t with = kTRUE)
   {
      fWithPoleZeroCorrection = with;
   }
   void SetTauRC(Int_t taurc)
   {
      fTauRC = taurc;
   }
   Double_t GetTauRC() const
   {
      return fTauRC;
   }

   //
   //parameters for interpolation
   //
   void SetInterpolation(Bool_t with = kTRUE)
   {
      fWithInterpolation = with;
   }
   void SetInterpolatedChannelWidth(double width)
   {
      fInterpolatedChannelWidth = width;
   }
   Double_t GetInterpolatedChannelWidth() const
   {
      return fInterpolatedChannelWidth;
   }

   //
   //routines to calculate the raw amplitude without filter
   //
   virtual void ComputeRawAmplitude(void);
   Double_t GetRawAmplitude() const
   {
      return fYmax - fYmin;
   }
   Double_t GetYmin() const
   {
      return fYmin;
   }
   Double_t GetYmax() const
   {
      return fYmax;
   }

   //routines to manage threshold for minimum charge in the detector
   Double_t GetAmplitudeTriggerValue() const
   {
      return fMinimumValueForAmplitude;
   }
   void SetAmplitudeTriggerValue(Double_t val)
   {
      fMinimumValueForAmplitude = val;
   }

   // compute mean value and rms of a subset of samples
   double FindMedia(double tsta, double tsto);
   double FindMedia(int tsta, int tsto);
   double FindSigma2(double tsta, double tsto);
   double FindSigma2(int tsta, int tsto);

   // multiply the signal  (modify only fAdc)
   void Multiply(Double_t fact);
   void Add(Double_t fact);

   // Interpolations
   Double_t FindTzeroCFDCubic(double level, int Nrecurr);
   double FindTzeroCFDCubic_rev(double level, double tend, int Nrecurr);
   Double_t CubicInterpolation(float* data, int x2, double fmax, int Nrecurr);

   virtual void BuildCubicSignal(double taufinal); //Interpolazione mediante cubic
   virtual void BuildCubicSplineSignal(double taufinal);

   virtual double GetDataInter(double t);
   virtual double GetDataInterCubic(double t);
   virtual double GetDataCubicSpline(double t);

   // different shapers (modify only fAdc)
   void FIR_ApplyTrapezoidal(double trise, double tflat); // trise=sqrt(12)*tausha di CR-RC^4 se tflat=trise/2
   void FIR_ApplySemigaus(double tau_usec);
   void FIR_ApplyRCLowPass(double time_usec, int reverse = 0);
   void FIR_ApplyRCHighPass(double time_usec, int reverse = 0);
   void FIR_ApplyRecursiveFilter(double a0, int N, double* a, double* b, int reverse);
   void FIR_ApplyMovingAverage(int npoints);
   void PoleZeroSuppression(Double_t tauRC);

   // fast fourier transform and windowing of the signal (modify only fAdc)
   void ApplyWindowing(int window_type = 3); // 0: barlett, 1:hanning, 2:hamming, 3: blackman
   static int FFT(unsigned int p_nSamples, bool p_bInverseTransform, double* p_lpRealIn, double* p_lpImagIn, double* p_lpRealOut, double* p_lpImagOut); // nsamples: power of 2
   int FFT(bool p_bInverseTransform, double* p_lpRealOut, double* p_lpImagOut);
   TH1* FFT2Histo(int output, TH1* hh = 0); // 0 modulo, 1 modulo db (normalized), 2, re, 3 im

   // apply modifications of fAdc to the original signal
   void ApplyModifications(TGraph* newSignal = 0, Int_t nsa = -1);

   ClassDef(KVSignal, 3) //Base class for FAZIA signal processing
};

#endif
