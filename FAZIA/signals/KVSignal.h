//Created by KVClassFactory on Mon Dec 22 15:46:46 2014
//Author: ,,,

#ifndef __KVSIGNAL_H
#define __KVSIGNAL_H

#include "TGraph.h"
#include "TArrayF.h"
#include "TH1F.h"

class KVDetector;
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
   Int_t fIndex;        //!index deduced from block, quartet and telescope numbering
   Int_t fChannel;      //! signal type (see KVSignal::SignalType enum)

   TString fDetName;    //name of the detector, the signal is linked to, needed to find it in the KVMultiDetector
   TString fType;       //string to identify the signal type : "QH1", "I2" etc ...

   Int_t fFPGAOutputNumbers;  //!ASsociated FPGA energy outputs

   TArrayF fAdc;                    //! needed to use the psa methods copied from FClasses of Firenze
   //results of signal treatement
   Double_t fAmplitude;             // amplitude of the signal
   Double_t fRiseTime;              // rise time of the signal
   Double_t fIMax;                  // position of the maximum in channel
   Double_t fTMax;                  // position of the maximum in ns
   Double_t fYmin, fYmax;           // raw min/max of the signal
   Double_t fBaseLine;              // base line mean value
   Double_t fSigmaBase;             // base line rms
   Double_t fEndLine;              // !mean value of the signal line at the end
   Double_t fSigmaEnd;             // !rms value of the signal line at the end

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
   virtual void BuildCubicSplineSignal(); //Interpolazione mediante cubic spline
   virtual void BuildSmoothingSplineSignal(); //Interpolazione mediante cubic spline
   void init();
   void TreateOldSignalName();

public:
   KVSignal();
   KVSignal(const char* name, const char* title);
   KVSignal(const TString& name, const TString& title);
   virtual ~KVSignal();
   void Copy(TObject& obj) const;

   Bool_t IsLongEnough() const;

   //
   //routines to link signal to its detector in kaliveda framework
   //
   const Char_t* GetDetectorName()  const
   {
      return fDetName.Data();
   }
   void SetDetectorName(const Char_t* name)
   {
      fDetName = name;
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
   Int_t GetIndex()           const
   {
      return fIndex;
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

   Int_t GetNFPGAValues() const
   {
      return fFPGAOutputNumbers;
   }

   Bool_t HasFPGA() const
   {
      return (GetNFPGAValues() > 0);
   }

   //
   //routines to read/change PSA parameters from configuration file or database
   //
   Double_t GetPSAParameter(const Char_t* parname);
   virtual void LoadPSAParameters();
   virtual void SetDefaultValues();
   virtual void UpdatePSAParameter(KVDBParameterList* par);
   KVSignal* ConvertTo(const Char_t* type);

   virtual bool IsOK();

   //
   //routines to launch and control PSA
   //
   virtual void TreateSignal();
#define KVSIGNAL_GETPSARESULT_KVDETECTOR 1
   virtual void GetPSAResult(KVDetector*) const
   {
      // store results of PSA in detector signals
   }
   Bool_t PSAHasBeenComputed() const
   {
      return fPSAIsDone;
   }

   //
   //Indicate the value in (ns or us) of TGraph X axis width
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
   virtual Double_t ComputeDuration(Double_t th = 0.2);
   virtual Double_t ComputeEndLine();
   virtual void RemoveBaseLine();

   void BuildReverseTimeSignal();

   Bool_t IsFired();

   Double_t GetBaseLine() const
   {
      return fBaseLine;
   }
   Double_t GetSigmaBaseLine()const
   {
      return fSigmaBase;
   }

   Double_t GetEndLine() const
   {
      return fEndLine;
   }
   Double_t GetSigmaEndLine()const
   {
      return fSigmaEnd;
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
   Double_t ComputeCFDThreshold(Double_t threshold = 0.5);

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
   Bool_t ComputeMeanAndSigma(Int_t start, Int_t stop, Double_t& mean, Double_t& sigma);
   Bool_t ComputeMeanAndSigma(Double_t start, Double_t stop, Double_t& mean, Double_t& sigma);

//    double FindMedia(double tsta, double tsto);
//    double FindMedia(int tsta, int tsto);
//    double FindSigma2(double tsta, double tsto);
//    double FindSigma2(int tsta, int tsto);

   // multiply the signal  (modify only fAdc)
   void Multiply(Double_t fact);
   void Add(Double_t fact);

   // Interpolations
   Double_t ARC_CFD(Double_t threshold = 0.3, Double_t tdelay = 10);
   double FindTzeroLeadingEdgeCubic(double LEVEL, int Nrecurr);
   Double_t FindTzeroCFDCubic(double level, int Nrecurr);
   double FindTzeroCFDCubic_rev(double level, double tend, int Nrecurr);
   Double_t CubicInterpolation(float* data, int x2, double fmax, int Nrecurr);

   virtual void BuildCubicSignal(double taufinal); //Interpolazione mediante cubic
   virtual void BuildCubicSplineSignal(double taufinal);//Interpolazione mediante spline cubic
   virtual void BuildSmoothingSplineSignal(double taufinal, double l = 1, int nbits = -1); //Interpolazione mediante smoothing spline

   virtual double GetDataInter(double t);
   virtual double GetDataInterCubic(double t);
   virtual double GetDataCubicSpline(double t);
   virtual double GetDataSmoothingSplineLTI(double t);//metodo che serve a BuildSmoothingSpline
   virtual double EvalCubicSpline(double X);//metodo che serve a BuildSmoothingSpline

   // different shapers (modify only fAdc)
   void FIR_ApplyTrapezoidal(double trise, double tflat); // trise=sqrt(12)*tausha di CR-RC^4 se tflat=trise/2
   void FIR_ApplySemigaus(double tau_usec);
   void FIR_ApplyRCLowPass(double time_usec, int reverse = 0);
   void FIR_ApplyRCHighPass(double time_usec, int reverse = 0);
   void FIR_ApplyRecursiveFilter(double a0, int N, double* a, double* b, int reverse);
   void FIR_ApplyMovingAverage(int npoints);
   void PoleZeroSuppression(Double_t tauRC);
   int FIR_ApplySmoothingSpline(double l, int nbits = -1);
   double ApplyNewton(double l, double x0);//metodo che serve a FIR_ApplySmoothingSpline

   // fast fourier transform and windowing of the signal (modify only fAdc)
   void ApplyWindowing(int window_type = 3); // 0: barlett, 1:hanning, 2:hamming, 3: blackman
   static int FFT(unsigned int p_nSamples, bool p_bInverseTransform, double* p_lpRealIn, double* p_lpImagIn, double* p_lpRealOut, double* p_lpImagOut); // nsamples: power of 2
   int FFT(bool p_bInverseTransform, double* p_lpRealOut, double* p_lpImagOut);
   TH1* FFT2Histo(int output, TH1* hh = 0); // 0 modulo, 1 modulo db (normalized), 2, re, 3 im

   // apply modifications of fAdc to the original signal
   void ApplyModifications(TGraph* newSignal = 0, Int_t nsa = -1);


   //------------------ OPERATORI ---------------------//
   void ShiftLeft(double);//shift in ns
   void ShiftRight(double);//shift in ns
   void TestDraw();

   static KVSignal* MakeSignal(const char* sig_type);

   ClassDef(KVSignal, 4) //Base class for FAZIA signal processing

};

#endif
