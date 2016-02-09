//Created by KVClassFactory on Fri Jan 23 18:44:27 2015
//Author: ,,,

#ifndef __KVFAZIADETECTOR_H
#define __KVFAZIADETECTOR_H

#include "KVDetector.h"
class KVFAZIACalibrator;
class KVSignal;

class KVFAZIADetector : public KVDetector {
protected:
   KVList* fSignals;         //list of electronics signal (current, charge, etc... )

   Int_t fBlock;
   Int_t fQuartet;
   Int_t fTelescope;
   Int_t fIdentifier;   //to difference SI1 SI2 CSI detectors

   Double_t fCharge;


   //values defined for SI1 detectors
   Double_t fAmplitudeQL1;
   Double_t fRawAmplitudeQL1;
   Double_t fBaseLineQL1;
   Double_t fSigmaBaseLineQL1;
   Double_t fRiseTimeQL1;

   Double_t fAmplitudeQH1;
   Double_t fRawAmplitudeQH1;
   Double_t fBaseLineQH1;
   Double_t fSigmaBaseLineQH1;
   Double_t fRiseTimeQH1;

   Double_t fAmplitudeI1;
   Double_t fRawAmplitudeI1;
   Double_t fBaseLineI1;
   Double_t fSigmaBaseLineI1;

   //values defined for SI2 detectors
   Double_t fAmplitudeQ2;
   Double_t fRawAmplitudeQ2;
   Double_t fBaseLineQ2;
   Double_t fSigmaBaseLineQ2;
   Double_t fRiseTimeQ2;

   Double_t fAmplitudeI2;
   Double_t fRawAmplitudeI2;
   Double_t fBaseLineI2;
   Double_t fSigmaBaseLineI2;

   //values defined for CSI detectors
   Double_t fAmplitudeQ3;
   Double_t fRawAmplitudeQ3;
   Double_t fFastAmplitudeQ3;
   Double_t fBaseLineQ3;
   Double_t fSigmaBaseLineQ3;
   Double_t fRiseTimeQ3;

   KVFAZIACalibrator* fChargeToEnergy;//!To obtain energy

   void init();   //initialisatino method called by the constructors
   Bool_t SetProperties();


public:
   enum {                       //determine identification of the detector
      kSI1,
      kSI2,
      kCSI
   };
   KVFAZIADetector();
   KVFAZIADetector(const Char_t* type, const Float_t thick = 0.0);
   virtual ~KVFAZIADetector();
   void Copy(TObject& obj) const;
   virtual void   Clear(Option_t* opt = "");
   virtual void   SetName(const char* name);
   KVList* PrepareIDTelescopeList();
   void SortIDTelescopes();

   virtual Bool_t Fired(Option_t* opt = "any");
   Double_t GetCalibratedEnergy();
   Double_t GetEnergy();

   void SetSignal(KVSignal* signal, const Char_t* type);
   Bool_t HasSignal() const;
   KVSignal* GetSignal(const Char_t* name) const;
   KVSignal* GetSignalByType(const Char_t* type) const;
   KVSignal* GetSignal(Int_t idx) const;
   Int_t GetNumberOfSignals() const;
   KVList* GetListOfSignals() const;
   void SetCalibrators();
   Int_t GetIdentifier() const
   {
      return fIdentifier;
   }
   Int_t GetBlockNumber() const
   {
      return fBlock;
   }
   Int_t GetQuartetNumber() const
   {
      return fQuartet;
   }
   Int_t GetTelescopeNumber() const
   {
      return fTelescope;
   }

   void SetQL1Amplitude(Double_t value)
   {
      fAmplitudeQL1 = value;
   }
   Double_t GetQL1Amplitude() const
   {
      return fAmplitudeQL1;
   }
   void SetQL1RawAmplitude(Double_t value)
   {
      fRawAmplitudeQL1 = value;
   }
   Double_t GetQL1RawAmplitude() const
   {
      return fRawAmplitudeQL1;
   }
   void SetQL1BaseLine(Double_t value)
   {
      fBaseLineQL1 = value;
   }
   Double_t GetQL1BaseLine() const
   {
      return fBaseLineQL1;
   }
   void SetQL1SigmaBaseLine(Double_t value)
   {
      fSigmaBaseLineQL1 = value;
   }
   Double_t GetQL1SigmaBaseLine() const
   {
      return fSigmaBaseLineQL1;
   }
   void SetQL1RiseTime(Double_t value)
   {
      fRiseTimeQL1 = value;
   }
   Double_t GetQL1RiseTime() const
   {
      return fRiseTimeQL1;
   }

   void SetQH1Amplitude(Double_t value)
   {
      fAmplitudeQH1 = value;
      if (fIdentifier == kSI1)
         fCharge = value;
   }
   Double_t GetQH1Amplitude() const
   {
      return fAmplitudeQH1;
   }
   void SetQH1RawAmplitude(Double_t value)
   {
      fRawAmplitudeQH1 = value;
   }
   Double_t GetQH1RawAmplitude() const
   {
      return fRawAmplitudeQH1;
   }
   void SetQH1BaseLine(Double_t value)
   {
      fBaseLineQH1 = value;
   }
   Double_t GetQH1BaseLine() const
   {
      return fBaseLineQH1;
   }
   void SetQH1SigmaBaseLine(Double_t value)
   {
      fSigmaBaseLineQH1 = value;
   }
   Double_t GetQH1SigmaBaseLine() const
   {
      return fSigmaBaseLineQH1;
   }
   void SetQH1RiseTime(Double_t value)
   {
      fRiseTimeQH1 = value;
   }
   Double_t GetQH1RiseTime() const
   {
      return fRiseTimeQH1;
   }

   void SetI1Amplitude(Double_t value)
   {
      fAmplitudeI1 = value;
   }
   Double_t GetI1Amplitude() const
   {
      return fAmplitudeI1;
   }
   void SetI1RawAmplitude(Double_t value)
   {
      fRawAmplitudeI1 = value;
   }
   Double_t GetI1RawAmplitude() const
   {
      return fRawAmplitudeI1;
   }
   void SetI1BaseLine(Double_t value)
   {
      fBaseLineI1 = value;
   }
   Double_t GetI1BaseLine() const
   {
      return fBaseLineI1;
   }
   void SetI1SigmaBaseLine(Double_t value)
   {
      fSigmaBaseLineI1 = value;
   }
   Double_t GetI1SigmaBaseLine() const
   {
      return fSigmaBaseLineI1;
   }

   void SetQ2Amplitude(Double_t value)
   {
      fAmplitudeQ2 = value;
      if (fIdentifier == kSI2)
         fCharge = value;
   }
   Double_t GetQ2Amplitude() const
   {
      return fAmplitudeQ2;
   }
   void SetQ2RawAmplitude(Double_t value)
   {
      fRawAmplitudeQ2 = value;
   }
   Double_t GetQ2RawAmplitude() const
   {
      return fRawAmplitudeQ2;
   }
   void SetQ2BaseLine(Double_t value)
   {
      fBaseLineQ2 = value;
   }
   Double_t GetQ2BaseLine() const
   {
      return fBaseLineQ2;
   }
   void SetQ2SigmaBaseLine(Double_t value)
   {
      fSigmaBaseLineQ2 = value;
   }
   Double_t GetQ2SigmaBaseLine() const
   {
      return fSigmaBaseLineQ2;
   }
   void SetQ2RiseTime(Double_t value)
   {
      fRiseTimeQ2 = value;
   }
   Double_t GetQ2RiseTime() const
   {
      return fRiseTimeQ2;
   }

   void SetI2Amplitude(Double_t value)
   {
      fAmplitudeI2 = value;
   }
   Double_t GetI2Amplitude() const
   {
      return fAmplitudeI2;
   }
   void SetI2RawAmplitude(Double_t value)
   {
      fRawAmplitudeI2 = value;
   }
   Double_t GetI2RawAmplitude() const
   {
      return fRawAmplitudeI2;
   }
   void SetI2BaseLine(Double_t value)
   {
      fBaseLineI2 = value;
   }
   Double_t GetI2BaseLine() const
   {
      return fBaseLineI2;
   }
   void SetI2SigmaBaseLine(Double_t value)
   {
      fSigmaBaseLineI2 = value;
   }
   Double_t GetI2SigmaBaseLine() const
   {
      return fSigmaBaseLineI2;
   }

   void SetQ3Amplitude(Double_t value)
   {
      fAmplitudeQ3 = value;
      if (fIdentifier == kCSI)
         fCharge = value;
   }
   Double_t GetQ3Amplitude() const
   {
      return fAmplitudeQ3;
   }
   void SetQ3RawAmplitude(Double_t value)
   {
      fRawAmplitudeQ3 = value;
   }
   Double_t GetQ3RawAmplitude() const
   {
      return fRawAmplitudeQ3;
   }
   void SetQ3FastAmplitude(Double_t value)
   {
      fFastAmplitudeQ3 = value;
   }
   Double_t GetQ3FastAmplitude() const
   {
      return fFastAmplitudeQ3;
   }
   void SetQ3BaseLine(Double_t value)
   {
      fBaseLineQ3 = value;
   }
   Double_t GetQ3BaseLine() const
   {
      return fBaseLineQ3;
   }
   void SetQ3SigmaBaseLine(Double_t value)
   {
      fSigmaBaseLineQ3 = value;
   }
   Double_t GetQ3SigmaBaseLine() const
   {
      return fSigmaBaseLineQ3;
   }
   void SetQ3RiseTime(Double_t value)
   {
      fRiseTimeQ3 = value;
   }
   Double_t GetQ3RiseTime() const
   {
      return fRiseTimeQ3;
   }


   ClassDef(KVFAZIADetector, 1) //Base class for FAZIA detector
};

#endif
