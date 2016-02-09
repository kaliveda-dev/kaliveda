/***************************************************************************
                          kvdetector.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVDetector.h,v 1.71 2009/05/22 14:45:40 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVDETECTOR_H
#define KVDETECTOR_H

#ifndef KVD_RECPRC_CNXN
#define KVD_RECPRC_CNXN 1
#endif
#ifndef KVD_NORECPRC_CNXN
#define KVD_NORECPRC_CNXN 0
#endif

#include "KVMaterial.h"
#include "KVPosition.h"
#include "KVList.h"
#include "KVNucleus.h"
#include "KVACQParam.h"
#include "Binary_t.h"
#include "KVGeoDetectorNode.h"
#include "KVUniqueNameList.h"

class KVGeoStrucElement;
class KVGroup;
class KVCalibrator;
class TGeoVolume;
class TTree;
class TGraph;

class KVDetector: public KVMaterial, public KVPosition {

private:
   KVPosition fEWPosition;//position of entrance window
   KVUniqueNameList fParentStrucList;//list of geometry structures which directly contain this detector
   KVGeoDetectorNode fNode;//positioning information relative to other detectors
   static Int_t fDetCounter;
   Short_t fActiveLayer;        //The active absorber in the detector
   KVList* fIDTelescopes;       //->list of ID telescopes to which detector belongs
   KVList* fIDTelAlign;         //->list of ID telescopes made of this detector and all aligned detectors placed in front of it
   TList* fIDTele4Ident;  //!list of ID telescopes used for particle ID

   enum {
      kIsAnalysed = BIT(14),    //for reconstruction of particles
      kActiveSet = BIT(15),     //internal - flag set true when SetActiveLayer called
      kUnidentifiedParticle = BIT(16),  //set if detector is in an unidentified particle's list
      kIdentifiedParticle = BIT(17),    //set if detector is in an identified particle's list
   };

   Int_t fIdentP;               //! temporary counters, determine state of identified/unidentified particle flags
   Int_t fUnidentP;             //! temporary counters, determine state of identified/unidentified particle flags

   /* make KVPosition methods private to avoid misuse */
   Bool_t ROOTGeo() const
   {
      return KVPosition::ROOTGeo();
   }
   void SetMatrix(const TGeoHMatrix* m)
   {
      KVPosition::SetMatrix(m);
   }
   void SetShape(TGeoBBox* s)
   {
      KVPosition::SetShape(s);
   }
   TGeoHMatrix* GetMatrix() const
   {
      return KVPosition::GetMatrix();
   }
   TGeoBBox* GetShape() const
   {
      return KVPosition::GetShape();
   }
   static TString fKVDetectorFiredACQParameterListFormatString;
protected:

   TString fFName;              //!dynamically generated full name of detector
   KVList* fCalibrators;        //list of associated calibrator objects
   KVList* fACQParams;          //list of raw data parameters read from coders
   KVList* fParticles;         //!list of particles hitting detector in an event
   KVList* fAbsorbers;          //->list of absorbers making up the detector
   UShort_t fSegment;           //used in particle reconstruction
   Double_t fGain;               //gain of amplifier
   Int_t fCalWarning;           //!just a counter so that missing calibrator warning is given only once

   Double_t fTotThickness; //! used to store value calculated by GetTotalThicknessInCM
   Double_t fDepthInTelescope; //! used to store depth of detector in parent telescope

   Binary8_t  fFiredMask;//bitmask used by Fired to determine which parameters to take into account

   Double_t ELossActive(Double_t* x, Double_t* par);
   Double_t EResDet(Double_t* x, Double_t* par);
   Double_t RangeDet(Double_t* x, Double_t* par);

   TF1* fELossF; //! parametric function dE in active layer vs. incident energy
   TF1* fEResF; //! parametric function Eres residual energy after all layers of detector
   TF1* fRangeF; //! parametric function range of particles in detector

   Double_t fEResforEinc;//! used by GetIncidentEnergy & GetCorrectedEnergy
   TList* fAlignedDetectors[2];//! stores lists of aligned detectors in both directions

   Bool_t fSimMode;//! =kTRUE when using to simulate detector response, =kFALSE when analysing data
   Bool_t fPresent;//! =kTRUE if detector is present, =kFALSE if it has been removed
   Bool_t fDetecting;//! =kTRUE if detector is "detecting", =kFALSE if not
public:
   KVDetector();
   KVDetector(const Char_t* type, const Float_t thick = 0.0);
   KVDetector(const KVDetector&);
   void init();
   virtual ~ KVDetector();

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   virtual void SetMaterial(const Char_t* type);
   void AddAbsorber(KVMaterial*);
   void SetActiveLayer(KVMaterial*);
   void SetActiveLayer(Short_t actif)
   {
      fActiveLayer = actif;
      SetBit(kActiveSet);
   };
   KVMaterial* GetActiveLayer() const
   {
      //Get pointer to the "active" layer in the detector, i.e. the one in which energy losses are measured
      return GetAbsorber(fActiveLayer);
   };
   KVMaterial* GetAbsorber(Int_t i) const;
   KVMaterial* GetAbsorber(const Char_t* name) const
   {
      // Return absorber with given name
      return (KVMaterial*)(fAbsorbers ? fAbsorbers->FindObject(name) : 0);
   };
   KVList* GetListOfAbsorbers() const
   {
      return fAbsorbers;
   };
   virtual const Char_t* GetArrayName();
   virtual Double_t GetDepthInTelescope() const
   {
      return fDepthInTelescope;
   };

   Double_t GetTotalThicknessInCM()
   {
      // Calculate and return the total thickness in centimetres of ALL absorbers making up the detector,
      // not just the active layer (value returned by GetThickness()).

      fTotThickness = 0;
      TIter next(fAbsorbers);
      KVMaterial* mat;
      while ((mat = (KVMaterial*)next())) fTotThickness += mat->GetThickness();
      return fTotThickness;
   };
   KVGeoDetectorNode* GetNode()
   {
      return &fNode;
   }

   const Char_t* GetMaterialName() const
   {
      if (GetActiveLayer())
         return GetActiveLayer()->GetName();
      return KVMaterial::GetName();
   };
   virtual void DetectParticle(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetELostByParticle(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetParticleEIncFromERes(KVNucleus*, TVector3* norm = 0);

   void SetGain(Double_t gain);
   Double_t GetGain() const;

   virtual Double_t GetEnergy()
   {
      //
      // Returns energy lost in active layer by particles.
      //
      return (GetActiveLayer() ? GetActiveLayer()->GetEnergyLoss() : KVMaterial::GetEnergyLoss());
   };
   virtual void SetEnergy(Double_t e)
   {
      //
      //Set value of energy lost in active layer
      //
      if (GetActiveLayer()) GetActiveLayer()->SetEnergyLoss(e);
      else KVMaterial::SetEnergyLoss(e);
   };
   virtual Double_t GetEnergyLoss()
   {
      return GetEnergy();
   };
   virtual void SetEnergyLoss(Double_t e)
   {
      SetEnergy(e);
   };
   virtual Double_t GetCorrectedEnergy(KVNucleus*, Double_t e =
                                          -1., Bool_t transmission = kTRUE);
   virtual Int_t FindZmin(Double_t ELOSS = -1., Char_t mass_formula = -1);

   void AddACQParam(KVACQParam*);
   virtual KVACQParam* GetACQParam(const Char_t* /*name*/);
   KVList* GetACQParamList()
   {
      return fACQParams;
   };
   virtual Float_t GetACQData(const Char_t* /*name*/);
   virtual Float_t GetPedestal(const Char_t* /*name*/);
   virtual void SetPedestal(const Char_t* /*name*/, Float_t);

   Bool_t AddCalibrator(KVCalibrator* cal);
   KVCalibrator* GetCalibrator(const Char_t* name,
                               const Char_t* type) const;
   KVCalibrator* GetCalibrator(const Char_t* type) const;
   KVList* GetListOfCalibrators() const
   {
      return fCalibrators;
   };
   virtual Bool_t IsCalibrated() const;

   virtual void Clear(Option_t* opt = "");
   virtual void Reset()
   {
      Clear();
   };
   virtual void Print(Option_t* option = "") const;

   void AddHit(KVNucleus* part)
   {
      // Add to the list of particles hitting this detector in an event

      if (!fParticles) {
         fParticles = new KVList(kFALSE);
         fParticles->SetCleanup();
      }
      fParticles->Add(part);
   };

   // Return the list of particles hitting this detector in an event
   KVList* GetHits() const
   {
      return fParticles;
   };
   void ClearHits()
   {
      // clear the list of particles hitting this detector in an event
      if (fParticles) fParticles->Clear();
   };
   // Return the number of particles hitting this detector in an event
   Int_t GetNHits() const
   {
      return (fParticles ? fParticles->GetEntries() : 0);
   };

   inline UShort_t GetSegment() const;
   inline virtual void SetSegment(UShort_t s);
   Bool_t IsAnalysed()
   {
      return TestBit(kIsAnalysed);
   };
   void SetAnalysed(Bool_t b = kTRUE)
   {
      SetBit(kIsAnalysed, b);
   };
   inline virtual Bool_t Fired(Option_t* opt = "any");
   inline virtual Bool_t FiredP(Option_t* opt = "any");

   virtual void SetACQParams();
   virtual void SetCalibrators();
   virtual void RemoveCalibrators();

   virtual void AddIDTelescope(TObject* idt);
   KVList* GetIDTelescopes()
   {
      //Return list of IDTelescopes to which detector belongs
      return fIDTelescopes;
   };
   KVList* GetAlignedIDTelescopes()
   {
      //return list of ID telescopes made of this detector
      //and all aligned detectors placed in front of it
      return fIDTelAlign;
   };
   TList* GetTelescopesForIdentification();

   inline void IncrementUnidentifiedParticles(Int_t n = 1)
   {
      fUnidentP += n;
      fUnidentP = (fUnidentP > 0) * fUnidentP;
      SetBit(kUnidentifiedParticle, (Bool_t)(fUnidentP > 0));
   };
   inline void IncrementIdentifiedParticles(Int_t n = 1)
   {
      fIdentP += n;
      fIdentP = (fIdentP > 0) * fIdentP;
      SetBit(kIdentifiedParticle, (Bool_t)(fIdentP > 0));
   };
   Bool_t BelongsToUnidentifiedParticle() const
   {
      return TestBit(kUnidentifiedParticle);
   };
   Bool_t BelongsToIdentifiedParticle() const
   {
      return TestBit(kIdentifiedParticle);
   };

   static KVDetector* MakeDetector(const Char_t* name, Float_t thick);
   const TVector3& GetNormal();

   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();
   virtual void GetVerticesInOwnFrame(TVector3* /*corners[8]*/, Double_t /*depth*/, Double_t /*layer_thickness*/);
   virtual Double_t GetEntranceWindowSurfaceArea();

   virtual void SetFiredBitmask(KVString&);
   Binary8_t GetFiredBitmask() const
   {
      return fFiredMask;
   };
   virtual const Char_t* GetFiredACQParameterListFormatString() const
   {
      return fKVDetectorFiredACQParameterListFormatString.Data();
   }
   virtual Short_t GetCalcACQParam(KVACQParam*, Double_t) const
   {
      // Calculates & returns value of given acquisition parameter corresponding to
      // given calculated energy loss in the detector
      // Returns -1 if detector is not calibrated
      //
      // This method should be redefined in child classes i.e. for specific
      // detector implementations: this version just returns -1.
      return -1;
   };
   virtual Double_t GetMaxDeltaE(Int_t Z, Int_t A);
   virtual Double_t GetEIncOfMaxDeltaE(Int_t Z, Int_t A);
   virtual Double_t GetDeltaE(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetTotalDeltaE(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetERes(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetIncidentEnergy(Int_t Z, Int_t A, Double_t delta_e =
                                         -1.0, enum SolType type = kEmax);
   /*virtual Double_t GetEResFromDeltaE(...)  - DON'T IMPLEMENT, CALLS GETINCIDENTENERGY*/
   virtual Double_t GetDeltaEFromERes(Int_t Z, Int_t A, Double_t Eres);
   virtual Double_t GetIncidentEnergyFromERes(Int_t Z, Int_t A, Double_t Eres);
   virtual Double_t GetRange(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetLinearRange(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetPunchThroughEnergy(Int_t Z, Int_t A);
   virtual TGraph* DrawPunchThroughEnergyVsZ(Int_t massform = KVNucleus::kBetaMass);
   virtual TGraph* DrawPunchThroughEsurAVsZ(Int_t massform = KVNucleus::kBetaMass);

   virtual TF1* GetEResFunction(Int_t Z, Int_t A);
   virtual TF1* GetELossFunction(Int_t Z, Int_t A);
   virtual TF1* GetRangeFunction(Int_t Z, Int_t A);

   virtual Double_t GetSmallestEmaxValid(Int_t Z, Int_t A);

   virtual void SetEResAfterDetector(Double_t e)
   {
      fEResforEinc = e;
   };
   virtual Double_t GetEResAfterDetector() const
   {
      return fEResforEinc;
   };

   virtual void ReadDefinitionFromFile(const Char_t*);

   virtual TList* GetAlignedDetectors(UInt_t direction = /*KVGroup::kBackwards*/ 1);
   void ResetAlignedDetectors(UInt_t direction = /*KVGroup::kBackwards*/ 1);

   virtual void SetSimMode(Bool_t on = kTRUE)
   {
      // Set simulation mode of detector
      // If on=kTRUE (default), we are in simulation mode (calculation of energy losses etc.)
      // If on=kFALSE, we are analysing/reconstruction experimental data
      // Changes behaviour of Fired(): in simulation mode, Fired() returns kTRUE
      // whenever the energy loss in the active layer is >0
      fSimMode = on;
   };
   virtual Bool_t IsSimMode() const
   {
      // Returns simulation mode of detector:
      //   IsSimMode()=kTRUE : we are in simulation mode (calculation of energy losses etc.)
      //   IsSimMode()=kFALSE: we are analysing/reconstruction experimental data
      // Changes behaviour of Fired(): in simulation mode, Fired() returns kTRUE
      // whenever the energy loss in the active layer is >0
      return fSimMode;
   }

   virtual Bool_t IsPresent() const
   {
      // return the presence or not of the detector
      return fPresent;
   }
   void SetPresent(Bool_t yes = kTRUE)
   {
      fPresent = yes;
   }
   virtual Bool_t IsDetecting() const
   {
      // return if the detector is ready to detect or not
      return fDetecting;
   }
   void SetDetecting(Bool_t yes = kTRUE)
   {
      fDetecting = yes;
   }

   virtual Bool_t IsOK() const
   {
      //return kTRUE if detector is here and working
      return (fPresent && fDetecting);
   }

   virtual void DeduceACQParameters(Int_t /*zz*/ = -1, Int_t /*aa*/ = -1) {};

   KVGroup* GetGroup() const;
   UInt_t GetGroupNumber();

   void AddParentStructure(KVGeoStrucElement* elem);
   void RemoveParentStructure(KVGeoStrucElement* elem);
   KVGeoStrucElement* GetParentStructure(const Char_t* type, const Char_t* name = "") const;

   //virtual KVGeoDNTrajectory* GetTrajectoryForReconstruction();

   void SetActiveLayerMatrix(const TGeoHMatrix*);
   void SetActiveLayerShape(TGeoBBox*);
   TGeoHMatrix* GetActiveLayerMatrix() const;
   TGeoBBox* GetActiveLayerShape() const;
   void SetEntranceWindowMatrix(const TGeoHMatrix*);
   void SetEntranceWindowShape(TGeoBBox*);
   TGeoHMatrix* GetEntranceWindowMatrix() const;
   TGeoBBox* GetEntranceWindowShape() const;
   TVector3 GetRandomPointOnEntranceWindow() const;
   TVector3 GetCentreOfEntranceWindow() const;
   const KVPosition& GetEntranceWindow() const
   {
      return fEWPosition;
   }

   ClassDef(KVDetector, 9)      //Base class for the description of detectors in multidetector arrays
};

inline KVCalibrator* KVDetector::GetCalibrator(const Char_t* name,
      const Char_t* type) const
{
   if (fCalibrators)
      return (KVCalibrator*) fCalibrators->FindObjectWithNameAndType(name, type);
   return 0;
}

inline KVCalibrator* KVDetector::GetCalibrator(const Char_t* type) const
{
   if (fCalibrators)
      return (KVCalibrator*) fCalibrators->FindObjectByType(type);
   return 0;
}

inline UShort_t KVDetector::GetSegment() const
{
   //used in reconstruction of particles
   return fSegment;
}

inline void KVDetector::SetSegment(UShort_t s)
{
   //set segmentation level - used in reconstruction of particles
   fSegment = s;
}

inline void KVDetector::SetGain(Double_t gain)
{
   fGain = gain;
}

inline Double_t KVDetector::GetGain() const
{
   return fGain;
}

//_________________________________________________________________________________

Bool_t KVDetector::Fired(Option_t* opt)
{
   // Returns kTRUE if detector was hit (fired) in an event
   //
   // The actual meaning of hit/fired depends on the context and the option string opt.
   //
   // If the detector is in "simulation mode", i.e. if SetSimMode(kTRUE) has been called,
   // this method returns kTRUE if the calculated energy loss in the active layer is > 0.
   //
   // In "experimental mode" (i.e. IsSimMode() returns kFALSE), depending on the option:
   //
   //opt="any" (default):
   //Returns true if ANY* of the working acquisition parameters associated with the detector were fired in an event
   //opt="all" :
   //Returns true if ALL* of the working acquisition parameters associated with the detector were fired in an event
   //opt="Pany" :
   //Returns true if ANY* of the working acquisition parameters associated with the detector were fired in an event
   //and have a value greater than their pedestal value
   //opt="Pall" :
   //Returns true if ALL* of the working acquisition parameters associated with the detector were fired in an event
   //and have a value greater than their pedestal value
   //
   // *the actual parameters taken into account can be fine tuned using environment variables such as
   //          KVDetector.Fired.ACQParameterList.[type]: PG,GG,T
   // See KVDetector::SetFiredBitmask() for more details.

   if (!IsDetecting()) return kFALSE; //detector not working, no answer at all
   if (IsSimMode()) return (GetActiveLayer()->GetEnergyLoss() > 0.); // simulation mode: detector fired if energy lost in active layer

   if (opt[0] == 'P') return FiredP(opt + 1);

   Binary8_t event; // bitmask for event
   TIter next(fACQParams);
   KVACQParam* par;
   Int_t id = 0;
   while ((par = (KVACQParam*)next())) {
      if (par->Fired()) event.SetBit(id);
      else event.ResetBit(id);
      id++;
   }
   Binary8_t ok = fFiredMask & event;
   // "all" considered parameters fired if ok == mask
   // "any" considered parameters fired if ok != 0
   if (!strcmp(opt, "all"))  return (ok == fFiredMask);
   return (ok != "0");
}
//_________________________________________________________________________________

Bool_t KVDetector::FiredP(Option_t* opt)
{
   //opt="any" :
   //Returns true if ANY* of the working acquisition parameters associated with the detector were fired in an event
   //and have a value greater than their pedestal value
   //opt="all" :
   //Returns true if ALL* of the working acquisition parameters associated with the detector were fired in an event
   //and have a value greater than their pedestal value
   //
   // *the actual parameters taken into account can be fine tuned using environment variables such as
   //          KVDetector.Fired.ACQParameterList.[type]: PG,GG,T
   // See KVDetector::SetFiredBitmask() for more details.

   if (!IsDetecting()) return kFALSE; //detector not working, no answer at all

   Binary8_t event; // bitmask for event
   TIter next(fACQParams);
   KVACQParam* par;
   Int_t id = 0;
   while ((par = (KVACQParam*)next())) {
      if (par->Fired("P")) event.SetBit(id);
      else event.ResetBit(id);
      id++;
   }
   Binary8_t ok = fFiredMask & event;
   // "all" considered parameters fired if ok == mask
   // "any" considered parameters fired if ok != 0
   if (!strcmp(opt, "all"))  return (ok == fFiredMask);
   return (ok != "0");
}

#endif
