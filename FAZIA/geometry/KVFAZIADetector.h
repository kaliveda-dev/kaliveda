//Created by KVClassFactory on Fri Jan 23 18:44:27 2015
//Author: ,,,

#ifndef __KVFAZIADETECTOR_H
#define __KVFAZIADETECTOR_H

#include "KVDetector.h"
class KVSignal;

#define __KVFD_methname(X, Y)  Get ## X ## Y
#define __KVFD_setmethname(X, Y)  Set ## X ## Y
#define __KVFD_dotcat(X, Y)  X.Y
#define __KVFD_str(s) #s
#define __KVFD_xstr(s) __KVFD_str(s)
#define __KVFD_dcs(X, Y) __KVFD_xstr(__KVFD_dotcat(X,Y))

#define __KVFAZIADETECTOR_GETSIGNAL(sig,type) \
   Double_t __KVFD_methname(sig,type)() const \
   { \
      return GetDetectorSignalValue(__KVFD_dcs(sig,type)); \
   } \
   KVDetectorSignal* __KVFD_setmethname(sig,type)(Double_t val) \
   { \
      KVDetectorSignal* ds = GetDetectorSignal(__KVFD_dcs(sig,type)); \
      ds->SetValue(val); \
      ds->SetFired(); \
      return ds; \
   }

/**
\class KVFAZIADetector
\brief Base class for FAZIA detectors
\ingroup FAZIAGeo

### Detector names

 Detector names are assumed to be defined as
 label-xxx
 where xxx is computed as follows (see KVFAZIADetector::GetIndex) :
~~~~
 100*block number+10*quartet number+telescope number
~~~~
 and label can be SI1, SI2 or CSI

For example SI1-123 is the Silicon Si1 of the block 1, the quartet 2 and the telescope 3
*/

class KVFAZIADetector : public KVDetector {
protected:
   KVUniqueNameList fSignals;         //list of electronics signal (current, charge, etc... )

   Int_t fBlock;
   Int_t fQuartet;
   Int_t fTelescope;
   Int_t fIdentifier;   // SI1=0(kSI1) SI2=1(kSI2) CSI=2(kCSI)
   Int_t fIndex;   //!100*block+10*quartet+telescope
   Bool_t fIsRutherford;   //!

   //thresholds defined for this detector
   Double_t fQH1Threshold;
   Double_t fQ2Threshold;
   Double_t fQ3Threshold;

   Bool_t fIsFiredFromSignals;

   void init();   //initialisation method called by the constructors
   Bool_t SetProperties();

public:
   enum {                       //determine identification of the detector
      kSI1,
      kSI2,
      kCSI,
      kOTHER
   };
   KVFAZIADetector();
   KVFAZIADetector(const Char_t* type, const Float_t thick = 0.0);
   virtual ~KVFAZIADetector();
   void Copy(TObject& obj) const;
   virtual void   Clear(Option_t* opt = "");
   virtual void   SetName(const char* name);

   static const Char_t* GetNewName(KVString oldname);

   Bool_t Fired(Option_t* opt = "any") const;

   void SetSignal(TGraph* signal, const Char_t* signal_name);
   Bool_t HasSignal() const;
   KVSignal* GetSignal(const Char_t* name) const;
   KVSignal* GetSignalByType(const Char_t* type) const;
   KVSignal* GetSignal(Int_t idx) const;
   Int_t GetNumberOfSignals() const;
   const KVSeqCollection* GetListOfSignals() const;
   void ComputePSA();

   KVDetectorSignal* SetFPGAEnergy(int sigid, Int_t idx /* Si: alway 0, CsI: 0=max 1=fast */, Double_t energy);

   Int_t GetIdentifier() const
   {
      // Return the identifier of the detector. This is one of
      //   KVFAZIADetector::kSI1
      //   KVFAZIADetector::kSI2
      //   KVFAZIADetector::kCSI
      //   KVFAZIADetector::kOTHER
      return fIdentifier;
   }

   Int_t GetBlockNumber() const
   {
      return fBlock;
   }
   Int_t GetIndex() const
   {
      // Return index of detector, defined as
      //   100*block+10*quartet+telescope
      return fIndex;
   }

   Bool_t IsRutherford() const
   {
      // Returns kTRUE if detector is part of elastic scattering monitor telescope
      return fIsRutherford;
   }

   Int_t GetQuartetNumber() const
   {
      return fQuartet;
   }
   Int_t GetTelescopeNumber() const
   {
      return fTelescope;
   }

   Double_t GetSetupParameter(const Char_t* parname);

   __KVFAZIADETECTOR_GETSIGNAL(I1, Amplitude)
   __KVFAZIADETECTOR_GETSIGNAL(I1, BaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(I1, RawAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(I1, SigmaBaseLine)

   __KVFAZIADETECTOR_GETSIGNAL(I2, Amplitude)
   __KVFAZIADETECTOR_GETSIGNAL(I2, BaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(I2, RawAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(I2, SigmaBaseLine)

   __KVFAZIADETECTOR_GETSIGNAL(Q2, Amplitude)
   __KVFAZIADETECTOR_GETSIGNAL(Q2, BaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(Q2, FPGAEnergy)
   __KVFAZIADETECTOR_GETSIGNAL(Q2, RawAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(Q2, RiseTime)
   __KVFAZIADETECTOR_GETSIGNAL(Q2, SigmaBaseLine)

   __KVFAZIADETECTOR_GETSIGNAL(Q3, Amplitude)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, BaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, FastAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, FastFPGAEnergy)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, FPGAEnergy)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, RawAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, RiseTime)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, SigmaBaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(Q3, SlowAmplitude)

   __KVFAZIADETECTOR_GETSIGNAL(QH1, Amplitude)
   __KVFAZIADETECTOR_GETSIGNAL(QH1, BaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(QH1, FPGAEnergy)
   __KVFAZIADETECTOR_GETSIGNAL(QH1, RawAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(QH1, RiseTime)
   __KVFAZIADETECTOR_GETSIGNAL(QH1, SigmaBaseLine)

   __KVFAZIADETECTOR_GETSIGNAL(QL1, Amplitude)
   __KVFAZIADETECTOR_GETSIGNAL(QL1, BaseLine)
   __KVFAZIADETECTOR_GETSIGNAL(QL1, RawAmplitude)
   __KVFAZIADETECTOR_GETSIGNAL(QL1, RiseTime)
   __KVFAZIADETECTOR_GETSIGNAL(QL1, SigmaBaseLine)

   Int_t GetDetTag() const
   {
      return GetDetectorSignalValue("DetTag");
   }
   Int_t GetGTTag() const
   {
      return GetDetectorSignalValue("GTTag");
   }
   void SetDetTag(Int_t t)
   {
      SetDetectorSignalValue("DetTag", t);
   }
   void SetGTTag(Int_t t)
   {
      SetDetectorSignalValue("GTTag", t);
   }

   ClassDef(KVFAZIADetector, 1) //Base class for FAZIA detector
};

#endif
