#ifndef KVRECONSTRUCTEDNUCLEUS_H
#define KVRECONSTRUCTEDNUCLEUS_H

#include "KVNucleus.h"
#include "KVHashList.h"
#include "KVList.h"
#include "KVDetector.h"
#include "KVIDTelescope.h"
#include "KVIdentificationResult.h"
#include "KVGroup.h"
#include "TClonesArray.h"
#include "KVReconNucTrajectory.h"
class KVTelescope;

class KVReconstructedNucleus: public KVNucleus {

protected:
   const KVReconNucTrajectory* fReconTraj;//! trajectory used to reconstruct particle
   KVString fDetNames; // list of names of detectors through which particle passed
   KVHashList fDetList; //! non-persistent list of pointers to detectors
   KVString fIDTelName;   // name of identification telescope which identified this particle (if any)
   KVIDTelescope* fIDTelescope; //! non-persistent pointer to identification telescope

   enum {
      kIsIdentified = BIT(18),  //flag set when identification of particle is complete
      kIsCalibrated = BIT(19),   //flag set when energy calibration of particle is complete
      kCoherency = BIT(20),   //particle created and/or identified by analysis of energy losses of other particles
      kZMeasured = BIT(21),   //the Z attributed to this nucleus was measured
      kAMeasured = BIT(22)   //the A attributed to this nucleus was measured
   };

   Int_t fNSegDet;              //number of segmented/independent detectors hit by particle
   Int_t fAnalStatus;           //status of particle after analysis of reconstructed event
   Float_t fRealZ;              //Z returned by identification routine
   Float_t fRealA;              //A returned by identification routine
   Double_t fTargetEnergyLoss;   //calculated energy lost in target

   TClonesArray fIDResults;//results of every identification attempt made for this nucleus, in order of the ID telescopes used

   virtual void MakeDetectorList();

public:

   // status codes given to reconstructed particles by KVGroup::AnalyseParticles
   enum {
      kStatusOK,           // = 0 :   identification is, in principle at least, possible straight away
      kStatusOKafterSub,   // = 1 :   identification is, in principle, possible after identification and subtraction
      //         of energy losses of other particles in the same group which have Status=0
      kStatusOKafterShare, // = 2 :   the energy loss in the shared detector of the group must be shared
      //         (arbitrarily) between this and the other particle(s) with Status=2
      kStatusStopFirstStage,//= 3 :   the particle has stopped in the first member of an identification
      //         telescope; a minimum Z could be estimated from the measured energy loss.
      kStatusPileupDE,     // = 4 :   only for filtered simulations: undetectable pile-up in DE detector
      kStatusPileupGhost   // = 5 :   only for filtered simulations: undetectable particle
   };

   KVReconstructedNucleus();
   KVReconstructedNucleus(const KVReconstructedNucleus&);
   void init();
   virtual ~ KVReconstructedNucleus();
   virtual void Print(Option_t* option = "") const;
   virtual void Clear(Option_t* option = "");
   virtual void Reconstruct(KVDetector* kvd);

   virtual void SetIdentification(KVIdentificationResult*);

   const KVSeqCollection* GetDetectorList() const {
      // Return pointer to list of detectors through which particle passed,
      // in reverse order (i.e. first detector in list is the one in which particle stopped).

      if (!fDetList.GetEntries()) {
         const_cast<KVReconstructedNucleus*>(this)->MakeDetectorList();
      }
      return &fDetList;
   };
   KVDetector* GetDetector(int i) const {
      //Returns the detector referenced by fDetectors.At(i)
      //If i=0, this is the detector in which the particle stopped.
      //For i>0 one obtains the names of the detectors through which the particle
      //must have passed before stopping, in inverse order (i.e. i=0 is the last
      //detector, as i increases we get the detectors closer to the target).
      if (i >= GetDetectorList()->GetEntries()) return 0;
      return (KVDetector*) GetDetectorList()->At(i);
   };

   const Char_t* GetDetectorNames() const {
      return fDetNames.Data();
   }
   const Char_t* GetIDtelNames() const {
      return fIDTelName.Data();
   }

   //void SetDetector(int i, KVDetector *);
   KVDetector* GetStoppingDetector() const {
      // Return pointer to the detector in which this particle stopped
      return GetDetector(0);
   };
   Int_t GetNumDet() const {
      return GetDetectorList()->GetEntries();
   };
   Int_t GetNSegDet() const {
      // return segmentation index of particle used by Identify() and
      // KVGroup::AnalyseParticles
      return fNSegDet;
   };
   void SetNSegDet(Int_t seg) {
      // set segmentation index of particle used by Identify() and
      // KVGroup::AnalyseParticles
      fNSegDet = seg;
   };
   void ResetNSegDet() {
      // recalculate segmentation index of particle used by Identify() and
      // KVGroup::AnalyseParticles
      fNSegDet = 0;
      KVDetector* det;
      TIter nxt(&fDetList);
      while ((det = (KVDetector*)nxt())) fNSegDet += det->GetSegment();
   };
   inline Int_t GetStatus() const {
      // Returns status of reconstructed particle as decided by analysis of the group (KVGroup) in
      // which the particle is reconstructed (see KVGroup::AnalyseParticles).
      // This status is used to decide whether identification of the particle can be attempted
      // straight away or if we need to wait until other particles in the same group have been
      // identified and calibrated (case of >1 particle crossing shared detector in a group).
      //
      //  kStatusOK (0) :   identification is, in principle at least, possible straight away
      //  kStatusOKafterSub (1) :   identification is, in principle, possible after identification and subtraction
      //                                           of energy losses of other particles in the same group which have Status=0
      //  kStatusOKafterShare (2)  :   the energy loss in the shared detector of the group must be shared
      //                                                (arbitrarily) between this and the other particle(s) with Status=2
      //  kStatusStopFirstStage (3) :   the particle has stopped in the first member of an identification
      //                         telescope; a minimum Z could be estimated from the measured energy loss.
      //                         (see KVDetector::FindZmin)
      // kStatusPileupDE,     (4) :   only for filtered simulations: undetectable pile-up in DE detector
      // kStatusPileupGhost   (5) :   only for filtered simulations: undetectable particle
      return fAnalStatus;
   };

   inline void SetStatus(Int_t a) {
      fAnalStatus = a;
   }

   virtual void GetAnglesFromStoppingDetector(Option_t* opt = "random");
   KVGroup* GetGroup() const {
      //Return pointer to group in which the particle is detected
      return (GetStoppingDetector() ?  GetStoppingDetector()->GetGroup() : 0);
   };


   void AddDetector(KVDetector*);

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif

   KVSeqCollection* GetIDTelescopes() const {
      //Gets from detector in which particle stopped the list of all ID telescopes
      //made from the stopping detector and all those aligned in front of it.
      //The first ID telescope in the list is that in which the particle stopped.

      return (GetStoppingDetector() ? GetStoppingDetector()->GetAlignedIDTelescopes() : 0);
   };
   virtual void Identify();
   virtual void Calibrate();

   KVIDTelescope* GetIdentifyingTelescope() const {
      return fIDTelescope;
   };
   void SetIdentifyingTelescope(KVIDTelescope* i) {
      fIDTelescope = i;
      if (i) fIDTelName = i->GetName();
      else fIDTelName = "";
   };

   virtual void SetIDCode(UShort_t s) {
      // Set value of parameter "IDCODE"
      GetParameters()->SetValue("IDCODE", (Int_t)s);
   }
   virtual Int_t GetIDCode() const {
      // Return value of parameter "IDCODE"
      // If no value set, returns -1
      return GetParameters()->GetIntValue("IDCODE");
   }
   virtual void SetECode(UChar_t s) {
      // Set value of parameter "ECODE"
      GetParameters()->SetValue("ECODE", (Int_t)s);
   }
   virtual Int_t GetECode() const {
      // Return value of parameter "ECODE"
      // If no value set, returns -1
      return GetParameters()->GetIntValue("ECODE");
   }

   void SetIsIdentified() {
      //When the "identification" state of the particle is set, we add 1 identified particle and
      //subtract 1 unidentified particle from each detector in its list
      SetBit(kIsIdentified);
      const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementIdentifiedParticles)(1);
      const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementUnidentifiedParticles)(-1);
   };
   void SetIsCalibrated() {
      SetBit(kIsCalibrated);
   };
   void SetIsUnidentified() {
      //When the "identification" state of the particle is reset, i.e. it becomes an "unidentified particle",
      //we add 1 unidentified particle and subtract 1 identified particle from each detector in its list
      ResetBit(kIsIdentified);
      const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementIdentifiedParticles)(-1);
      const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementUnidentifiedParticles)(1);
   };
   void SetIsUncalibrated() {
      ResetBit(kIsCalibrated);
   };
   Bool_t IsIdentified() const {
      return TestBit(kIsIdentified);
   };
   Bool_t IsCalibrated() const {
      return TestBit(kIsCalibrated);
   };

   void SetRealZ(Float_t zz) {
      fRealZ = zz;
   }
   void SetRealA(Float_t A) {
      fRealA = A;
   }
   Float_t GetRealZ() const {
      if (fRealZ > 0) {
         //debug
         //std::cout << "KVReconstructedNucleus::GetRealZ() returning fRealZ" << std::endl;
         return fRealZ;
      } else {
         //debug
         //std::cout << "KVReconstructedNucleus::GetRealZ() returning GetZ() because fRealZ=" << fRealZ << std::endl;
         return (Float_t) GetZ();
      }
   }
   Float_t GetRealA() const {
      if (fRealA > 0)
         return fRealA;
      else
         return (Float_t) GetA();
   }
   virtual Float_t GetPID() const {
      //Return particle identification PID for this particle.
      //If particle Z & A have been identified, this is "real Z" + 0.1*("real A"-2*"real Z")
      //If only Z identification has been performed, it is the "real Z"
      if (IsAMeasured())
         return (GetRealZ() + 0.1 * (GetRealA() - 2. * GetRealZ()));
      return GetRealZ();
   };
   virtual void SetTargetEnergyLoss(Double_t e) {
      // Set energy loss in target (in MeV) of reconstructed nucleus
      fTargetEnergyLoss = e;
   };
   virtual Double_t GetTargetEnergyLoss() const {
      // Return calculated energy loss in target of reconstructed nucleus (in MeV)
      return fTargetEnergyLoss;
   };

   virtual void SetZMeasured(Bool_t yes = kTRUE) {
      // Call with yes=kTRUE for reconstructed nuclei whose
      // atomic number, Z, was measured, not calculated
      SetBit(kZMeasured, yes);
   };
   virtual void SetAMeasured(Bool_t yes = kTRUE) {
      // Call with yes=kTRUE for reconstructed nuclei whose
      // mass number, A, was measured, not calculated
      SetBit(kAMeasured, yes);
   };
   virtual Bool_t IsZMeasured() const {
      // Returns kTRUE for reconstructed nuclei whose
      // atomic number, Z, was measured, not calculated
      return TestBit(kZMeasured);
   };
   virtual Bool_t IsAMeasured() const {
      // Returns kTRUE for reconstructed nuclei whose
      // mass number, A, was measured, not calculated
      return TestBit(kAMeasured);
   };
   KVIdentificationResult* GetIdentificationResult(Int_t i) {
      // Returns the result of the i-th (i>0) identification attempted for this nucleus.
      // i=1 : identification telescope in which particle stopped
      // i=2 : identification telescope immediately in front of the first
      // etc. etc.
      //
      // N.B. This method will return a valid KVIdentificationResult object for any
      //      value of i (objects are created as necessary in the TClonesArray fIDresults).
      //      To test whether an identification was attempted, do
      //         if(GetIdentificationResult(i)->IDattempted){...}
      //      rather than
      //         if(GetIdentificationResult(i)){ // always true }
      KVIdentificationResult* id = nullptr;
      if (i) id = (KVIdentificationResult*)fIDResults.ConstructedAt(i - 1);
      id->SetNumber(i);
      return id;
   }
   Int_t GetNumberOfIdentificationResults() const {
      // Returns the number of KVIdentificationResult objects in the TClonesArray fIDresults.
      // Do not assume that all of these correspond to attemted identifications
      // (see comments in GetIdentificationResult(Int_t))
      return fIDResults.GetEntries();
   }

   KVIdentificationResult* GetIdentificationResult(const Char_t* idtype) {
      // Return pointer to result of attempted identification of given type.
      // This type is the type of the KVIdentificationTelescope which was used
      // (i.e. the string returned by KVIdentificationTelescope::GetType()).
      // Returns nullptr if no identification of given type found/attempted

      Int_t n = GetNumberOfIdentificationResults();
      for (int i = 1; i <= n; i++) {
         KVIdentificationResult* id = GetIdentificationResult(i);
         if (!strcmp(id->GetIDType(), idtype)) {
            return id;
         }
      }
      return nullptr;
   }

   KVIdentificationResult* GetIdentificationResult(KVIDTelescope* idt) {
      // Return pointer to result of identification attempted with a
      // KVIdentificationTelescope of the given type.
      // Returns nullptr if no identification of given type found.

      if (!idt) return nullptr;
      return GetIdentificationResult(idt->GetType());
   }
   KVIdentificationResult* GetIdentificationResult(Int_t i) const {
      return const_cast<KVReconstructedNucleus*>(this)->GetIdentificationResult(i);
   }
   KVIdentificationResult* GetIdentificationResult(const Char_t* idtype) const {
      return const_cast<KVReconstructedNucleus*>(this)->GetIdentificationResult(idtype);
   }
   KVIdentificationResult* GetIdentificationResult(KVIDTelescope* idt) const {
      return const_cast<KVReconstructedNucleus*>(this)->GetIdentificationResult(idt);
   }

   virtual void SubtractEnergyFromAllDetectors();
   inline static UInt_t GetNIdentifiedInGroup(KVGroup* grp);
   inline static UInt_t GetNUnidentifiedInGroup(KVGroup* grp);
   static void AnalyseParticlesInGroup(KVGroup* grp);

   const KVReconNucTrajectory* GetReconstructionTrajectory() const {
      return fReconTraj;
   }
   void SetReconstructionTrajectory(const KVReconNucTrajectory* t) {
      fReconTraj = t;
   }

   ClassDef(KVReconstructedNucleus, 17)  //Nucleus detected by multidetector array
};

inline UInt_t KVReconstructedNucleus::GetNIdentifiedInGroup(KVGroup* grp)
{
   //number of identified particles reconstructed in group
   UInt_t n = 0;
   if (grp->GetHits()) {
      TIter next(grp->GetParticles());
      KVReconstructedNucleus* nuc = 0;
      while ((nuc = (KVReconstructedNucleus*) next()))
         n += (UInt_t) nuc->IsIdentified();
   }
   return n;
};
inline UInt_t KVReconstructedNucleus::GetNUnidentifiedInGroup(KVGroup* grp)
{
   //number of unidentified particles reconstructed in group
   return (grp->GetHits() - GetNIdentifiedInGroup(grp));
};

#endif
