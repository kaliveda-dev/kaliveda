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
#include "Binary_t.h"
#include "KVGeoDetectorNode.h"
#include "KVUniqueNameList.h"
#include "KVDetectorSignal.h"

class KVGeoStrucElement;
class KVGroup;
class KVCalibrator;
class TGeoVolume;
class TTree;
class TGraph;
class KVEvent;

/**
 \class KVDetector
 \ingroup Geometry
 \ingroup Stopping
 \brief Base class for detector geometry description, interface to energy-loss calculations

A detector is composed of one or more absorber layers (KVMaterial objects) in which the energy loss of charged particles can be calculated. One of these layers
is set as "active" (by default it is the last added layer) which means that only the energy loss in this layer can actually be "read", e.g. an ionisation chamber is composed of an
"active" gas layer sandwiched between two "inactive" windows :

~~~~~~~~~~~{.cpp}
      KVDetector chio("Myl", 2.5*KVUnits::um);                       //first layer - 2.5 micron mylar window
      KVMaterial *gas = new KVMaterial("C3F8", 5.*KVUnits::cm, 50.0*KVUnits::mbar);
      chio.AddAbsorber(gas);                                         //second layer - 5cm of C3F8 gas at 50mbar pressure and 19 deg. C
      chio.SetActiveLayer(gas);                                      //make gas layer "active"
      KVMaterial *win = new KVMaterial("Myl",2.5*KVUnits::um);       //exit window
      chio.AddAbsorber(win);
~~~~~~~~~~~

A detector is created either with the constructor taking the material type as argument:

~~~~~~~~~~~{.cpp}
      KVDetector det("Si");
~~~~~~~~~~~

or using SetMaterial:

~~~~~~~~~~~{.cpp}
      KVDetector det;
      det.SetMaterial("Si");
~~~~~~~~~~~

or it is created when a ROOT geometry is imported into a KVMultiDetArray object using
KVGeoImport.

###Calculate the energy loss of a charged particle in a detector

Two methods are available: one simply calculates the energy lost by the particle
in the detector, but does not modify either the particle or the detector (GetELostByParticle);
the other simulates the passage of the particle through the detector, the particle's energy
is reduced by the amount lost in the detector's absorbers and the total energy lost in the
detector is increased, e.g.:

~~~~~~~~~~~{.cpp}
      KVNucleus alpha(2,4);           //an alpha-particle
      alpha.SetEnergy(100);           //with 100MeV kinetic energy
      det.DetectParticle(&alpha);     //simulate passage of particle in the detector/target
      det.GetEnergy();                        //energy lost by particle in detector/target
      alpha.GetEnergy();                      //residual energy of particle
      det.Clear();                            //reset detector ready for a new detection
~~~~~~~~~~~

###Important note on detector positions, angles, solid angle, distances etc.

For detector geometries based on the ROOT geometry package, the following methods
refer to the surface of the first volume constituting the detector crossed from the
target (referred to as the "entrance window"):

~~~~~~~~~~~{.cpp}
  TVector3 GetRandomDirection();
  void GetRandomAngles();
  TVector3 GetDirection();
  TVector3 GetCentreOfEntranceWindow() const;
  Double_t GetTheta() const;
  Double_t GetSinTheta() const;
  Double_t GetCosTheta() const;
  Double_t GetPhi();
  Double_t GetDistance() const;
  Double_t GetSolidAngle(void);
~~~~~~~~~~~

*/

class KVDetector: public KVMaterial, public KVPosition {

private:
   KVPosition fEWPosition;//position of entrance window i.e. first volume in detector geometry
   KVUniqueNameList fParentStrucList;//list of geometry structures which directly contain this detector
   KVGeoDetectorNode fNode;//positioning information relative to other detectors
   static Int_t fDetCounter;
   KVMaterial* fActiveLayer;//! The active absorber in the detector
   KVList* fIDTelescopes;       //->list of ID telescopes to which detector belongs
   KVList* fIDTelAlign;         //->list of ID telescopes made of this detector and all aligned detectors placed in front of it
   TList* fIDTele4Ident;  //!list of ID telescopes used for particle ID
   TString fNameOfArray;//! name of multidetector array this detector is part of

   enum {
      kIsAnalysed = BIT(14),    //for reconstruction of particles
      kActiveSet = BIT(15),     //internal - flag set true when SetActiveLayer called
      kUnidentifiedParticle = BIT(16),  //set if detector is in an unidentified particle's list
      kIdentifiedParticle = BIT(17),    //set if detector is in an identified particle's list
   };

   Int_t fIdentP;               //! temporary counters, determine state of identified/unidentified particle flags
   Int_t fUnidentP;             //! temporary counters, determine state of identified/unidentified particle flags

   // Make KVPosition methods private to avoid misuse
   // N.B. the inherited KVPosition part of KVDetector is used for the ACTIVE layer of the detector
   //      the entrance window is described by member KVPosition fEWPosition
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
   TVector3 GetRandomPointOnSurface() const
   {
      return KVPosition::GetRandomPointOnSurface();
   }
   TVector3 GetSurfaceCentre() const
   {
      return KVPosition::GetSurfaceCentre();
   }
   TVector3 GetVolumeCentre() const
   {
      return KVPosition::GetVolumeCentre();
   }
   TVector3 GetSurfaceNormal() const
   {
      return KVPosition::GetSurfaceNormal();
   }
   Double_t GetSurfaceArea(int npoints = 100000) const
   {
      return KVPosition::GetSurfaceArea(npoints);
   }
   Double_t GetMisalignmentAngle() const
   {
      return KVPosition::GetMisalignmentAngle();
   }

   KVUniqueNameList fDetSignals;//! list of signals associated with detector

   void remove_signal_for_calibrator(KVCalibrator* K);

protected:

   TString fFName;              //!dynamically generated full name of detector
   KVList* fCalibrators;        //list of associated calibrator objects
   KVList* fParticles;         //!list of particles hitting detector in an event
   KVList* fAbsorbers;          //->list of absorbers making up the detector
   UShort_t fSegment;           //used in particle reconstruction
   Double_t fGain;               //gain of amplifier
   Int_t fCalWarning;           //!just a counter so that missing calibrator warning is given only once

   Double_t fTotThickness; //! used to store value calculated by GetTotalThicknessInCM
   Double_t fDepthInTelescope; //! used to store depth of detector in parent telescope

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

   Bool_t fSingleLayer;//! =kTRUE if detector has a single absorber layer

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
   void SetActiveLayer(KVMaterial* actif)
   {
      fActiveLayer = actif;
      SetBit(kActiveSet);
   }
   KVMaterial* GetActiveLayer() const
   {
      //Get pointer to the "active" layer in the detector, i.e. the one in which energy losses are measured
      return fActiveLayer;
   }
   KVMaterial* GetAbsorber(Int_t i) const;
   KVMaterial* GetAbsorber(const Char_t* name) const
   {
      // Return absorber with given name
      return (KVMaterial*)(fAbsorbers ? fAbsorbers->FindObject(name) : 0);
   }
   KVList* GetListOfAbsorbers() const
   {
      return fAbsorbers;
   }
   Int_t GetNumberOfAbsorberLayers() const
   {
      return fAbsorbers->GetEntries();
   }
   virtual const Char_t* GetArrayName();
   virtual Double_t GetDepthInTelescope() const
   {
      return fDepthInTelescope;
   }

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

   virtual Double_t GetCalibratedEnergy() const
   {
      // Returns energy loss in detector calculated using available calibration(s)
      return GetDetectorSignalValue("Energy");
   }
   virtual Double_t GetEnergy() const
   {
      // Returns either the calibrated energy loss measured in the active layer of the detector,
      // or (if IsSimMode()==kTRUE) the simulated energy losses
      Double_t ELoss = GetActiveLayer() ? GetActiveLayer()->GetEnergyLoss() : KVMaterial::GetEnergyLoss();
      if (IsSimMode()) return ELoss; // in simulation mode, return calculated energy loss in active layer
      if (ELoss > 0) return ELoss;
      ELoss = GetCalibratedEnergy();
      if (ELoss < 0) ELoss = 0;
      SetEnergy(ELoss);
      return ELoss;
   }
   virtual void SetEnergy(Double_t e) const
   {
      //
      //Set value of energy lost in active layer
      //
      if (GetActiveLayer()) GetActiveLayer()->SetEnergyLoss(e);
      else KVMaterial::SetEnergyLoss(e);
   }
   virtual Double_t GetEnergyLoss() const
   {
      return GetEnergy();
   }
   virtual void SetEnergyLoss(Double_t e) const
   {
      SetEnergy(e);
   }
   virtual Double_t GetCorrectedEnergy(KVNucleus*, Double_t e =
                                          -1., Bool_t transmission = kTRUE);
   virtual Int_t FindZmin(Double_t ELOSS = -1., Char_t mass_formula = -1);

   Bool_t AddCalibrator(KVCalibrator* cal, const KVNameValueList& opts = "");
   Bool_t ReplaceCalibrator(const Char_t* type, KVCalibrator* cal, const KVNameValueList& opts = "");
   KVCalibrator* GetCalibrator(const Char_t* name,
                               const Char_t* type) const;
   KVCalibrator* GetCalibrator(const Char_t* type) const;
   KVList* GetListOfCalibrators() const
   {
      return fCalibrators;
   }
   Bool_t IsCalibrated() const
   {
      // A detector is considered to be calibrated if it has
      // a signal "Energy" available
      return (HasDetectorSignalValue("Energy"));
   }
   Bool_t IsCalibrated(const KVNameValueList& params) const;

   virtual void Clear(Option_t* opt = "");
   virtual void Reset(Option_t* opt = "")
   {
      Clear(opt);
   }
   virtual void Print(Option_t* option = "") const;

   void AddHit(KVNucleus* part)
   {
      // Add to the list of particles hitting this detector in an event

      if (!fParticles) {
         fParticles = new KVList(kFALSE);
         fParticles->SetCleanup();
      }
      fParticles->Add(part);
      SetAnalysed();
   }

   void RemoveHit(KVNucleus* part)
   {
      // Remove from list of particles hitting this detector in an event

      fParticles->Remove(part);
      if (!fParticles->GetEntries()) SetAnalysed(kFALSE);
   }

   // Return the list of particles hitting this detector in an event
   KVList* GetHits() const
   {
      return fParticles;
   }
   void ClearHits()
   {
      // clear the list of particles hitting this detector in an event
      if (fParticles) fParticles->Clear();
   }
   // Return the number of particles hitting this detector in an event
   Int_t GetNHits() const
   {
      return (fParticles ? fParticles->GetEntries() : 0);
   }

   inline UShort_t GetSegment() const;
   inline virtual void SetSegment(UShort_t s);
   Bool_t IsAnalysed()
   {
      return TestBit(kIsAnalysed);
   }
   void SetAnalysed(Bool_t b = kTRUE)
   {
      SetBit(kIsAnalysed, b);
   }
   Bool_t Fired(Option_t* opt = "any") const
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
      //Returns true if ANY of the raw parameters associated with the detector were present in the last handled event
      //opt="all" :
      //Returns true if ALL of the raw parameters associated with the detector were present in the last handled event
      //

      if (!IsDetecting()) return kFALSE; //detector not working, no answer at all
      if (IsSimMode()) return (GetActiveLayer()->GetEnergyLoss() > 0.); // simulation mode: detector fired if energy lost in active layer

      TString OPT(opt);
      OPT.ToLower();
      Bool_t all = (OPT == "all");

      TIter raw_it(&GetListOfDetectorSignals());
      KVDetectorSignal* ds;
      int count_raw = 0;
      while ((ds = (KVDetectorSignal*)raw_it())) {
         if (ds->IsRaw()) {
            ++count_raw;
            if (ds->IsFired()) {
               if (!all) return kTRUE;
            }
            else {
               if (all) return kFALSE;
            }
         }
      }
      return all && count_raw;
   }
   virtual void RemoveCalibrators();

   Double_t GetDetectorSignalValue(const TString& type, const KVNameValueList& params = "") const
   {
      // Return value of signal of given type associated with detector
      //
      // Some signals require the necessary calibrators to be present & initialised
      //
      // Any additional parameters which are required can be passed as a string of
      // "param1=value,param2=value,..." parameter/value pairs.
      //
      // If the signal is not available, returns 0.

      KVDetectorSignal* s = GetDetectorSignal(type);
      return (s ? s->GetValue(params) : 0);
   }
   void SetDetectorSignalValue(const TString& type, Double_t val) const
   {
      // Set value of signal of given type associated with detector
      // Only to be used with raw detector signals (i.e. not expressions, not calibrated signals)

      KVDetectorSignal* s = GetDetectorSignal(type);
      if (s) s->SetValue(val);
   }
   Double_t GetInverseDetectorSignalValue(const TString& output, Double_t value, const TString& input, const KVNameValueList& params = "") const
   {
      // Calculate the value of the input signal for a given value of the output signal.
      //
      // This uses the inverse calibrations of all intermediate calibrators.
      //
      // Any additional parameters which are required can be passed as a string of
      // "param1=value,param2=value,..." parameter/value pairs.
      //
      // If the output signal is not defined, or if input & output are not related,
      // this returns 0.

      KVDetectorSignal* s = GetDetectorSignal(output);
      return (s ? s->GetInverseValue(value, input, params) : 0);
   }
   virtual KVDetectorSignal* GetDetectorSignal(const TString& type) const
   {
      // Return the signal with given type, if defined for this detector
      // If signal not defined, returns nullptr.

      return fDetSignals.get_object<KVDetectorSignal>(type);
   }
   Bool_t HasDetectorSignalValue(const TString& type) const
   {
      // Return kTRUE if signal with given type is defind for detector
      return (GetDetectorSignal(type) != nullptr);
   }


   virtual void AddIDTelescope(TObject* idt);
   KVList* GetIDTelescopes()
   {
      //Return list of IDTelescopes to which detector belongs
      return fIDTelescopes;
   }
   KVList* GetAlignedIDTelescopes();
   TList* GetTelescopesForIdentification();

   inline void IncrementUnidentifiedParticles(Int_t n = 1)
   {
      fUnidentP += n;
      fUnidentP = (fUnidentP > 0) * fUnidentP;
      SetBit(kUnidentifiedParticle, (Bool_t)(fUnidentP > 0));
   }
   inline void IncrementIdentifiedParticles(Int_t n = 1)
   {
      fIdentP += n;
      fIdentP = (fIdentP > 0) * fIdentP;
      SetBit(kIdentifiedParticle, (Bool_t)(fIdentP > 0));
   }
   Bool_t BelongsToUnidentifiedParticle() const
   {
      return TestBit(kUnidentifiedParticle);
   }
   Bool_t BelongsToIdentifiedParticle() const
   {
      return TestBit(kIdentifiedParticle);
   }

   static KVDetector* MakeDetector(const Char_t* name, Float_t thick);

   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();
   virtual void GetVerticesInOwnFrame(TVector3* /*corners[8]*/, Double_t /*depth*/, Double_t /*layer_thickness*/);
   virtual Double_t GetEntranceWindowSurfaceArea();
   TVector3 GetActiveLayerSurfaceCentre() const
   {
      // Return centre of entrance surface of active layer
      // [this is NOT necessarily the same as the entrance window]
      return GetSurfaceCentre();
   }
   TVector3 GetActiveLayerVolumeCentre() const
   {
      // Return centre of the active layer volume
      return KVPosition::GetVolumeCentre();
   }
   TGeoBBox* GetActiveLayerShape() const
   {
      // Return geometry of active layer
      return GetShape();
   }
   TGeoHMatrix* GetActiveLayerMatrix() const
   {
      // Return coordinate transformation matrix to active layer
      return GetMatrix();
   }

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
   }
   virtual Double_t GetEResAfterDetector() const
   {
      return fEResforEinc;
   }

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
   }
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

   virtual void DeduceACQParameters(KVEvent*, KVNumberList&) {}

   KVGroup* GetGroup() const;
   UInt_t GetGroupNumber();

   void AddParentStructure(KVGeoStrucElement* elem);
   void RemoveParentStructure(KVGeoStrucElement* elem);
   KVGeoStrucElement* GetParentStructure(const Char_t* type, const Char_t* name = "") const;

   void SetActiveLayerMatrix(const TGeoHMatrix*);
   void SetActiveLayerShape(TGeoBBox*);
   void SetEntranceWindowMatrix(const TGeoHMatrix*);
   void SetEntranceWindowShape(TGeoBBox*);
   const KVPosition& GetEntranceWindow() const
   {
      // Returns KVPosition object corresponding to the entrance window
      // volume i.e. the first volume encountered in the detector
      return fEWPosition;
   }
   const TVector3 GetCentreOfEntranceWindow() const
   {
      // This method does exactly the same as the method in previous versions of KaliVeda:
      // it returns the vector position of the centre of the surface (entrance) of the
      // "entrance window" of the detector i.e. the first volume encountered
      return GetEntranceWindow().GetSurfaceCentre();
   }
   Double_t GetSolidAngle() const
   {
      // Return solid angle [msr] corresponding to the entrance window of the detector
      if (ROOTGeo()) return fEWPosition.GetSolidAngle();
      return KVPosition::GetSolidAngle();
   }
   TVector3 GetRandomDirection(Option_t* t = "isotropic")
   {
      // random direction corresponding to point on entrance window
      if (ROOTGeo()) return fEWPosition.GetRandomDirection(t);
      return KVPosition::GetRandomDirection(t);
   }
   void GetRandomAngles(Double_t& th, Double_t& ph, Option_t* t = "isotropic")
   {
      // random angles [deg.] corresponding to point on entrance window
      if (ROOTGeo()) fEWPosition.GetRandomAngles(th, ph, t);
      else KVPosition::GetRandomAngles(th, ph, t);
   }
   TVector3 GetDirection()
   {
      // direction corresponding to centre of entrance window
      if (ROOTGeo()) return fEWPosition.GetDirection();
      return KVPosition::GetDirection();
   }
   Double_t GetDistance() const
   {
      // distance from target [cm] to entrance window of detector
      if (ROOTGeo()) return fEWPosition.GetDistance();
      return KVPosition::GetDistance();
   }
   Double_t GetTheta() const
   {
      // polar angle [deg.] corresponding to centre of entrance window of detector
      if (ROOTGeo()) return fEWPosition.GetTheta();
      return KVPosition::GetTheta();
   }
   Double_t GetSinTheta() const
   {
      // sinus of polar angle corresponding to centre of entrance window of detector
      if (ROOTGeo()) return fEWPosition.GetSinTheta();
      return KVPosition::GetSinTheta();
   }
   Double_t GetCosTheta() const
   {
      // cosinus of polar angle corresponding to centre of entrance window of detector
      if (ROOTGeo()) return fEWPosition.GetCosTheta();
      return KVPosition::GetCosTheta();
   }
   Double_t GetPhi() const
   {
      // azimuthal angle [deg.] corresponding to centre of entrance window of detector
      if (ROOTGeo()) return fEWPosition.GetPhi();
      return KVPosition::GetPhi();
   }

   void SetThickness(Double_t thick);
   Bool_t IsSingleLayer() const
   {
      // Returns kTRUE for detectors with a single absorber layer
      return fSingleLayer;
   }
   Bool_t HasSameStructureAs(const KVDetector*) const;
   void SetNameOfArray(const TString& n)
   {
      fNameOfArray = n;
   }
   const Char_t* GetNameOfArray() const
   {
      // Return name of multidetector array this detector belongs to
      return fNameOfArray;
   }

   const KVSeqCollection& GetListOfDetectorSignals() const
   {
      return fDetSignals;
   }
   void AddDetectorSignal(KVDetectorSignal* ds)
   {
      // Add KVDetectorSignal object to list of detector's signals.
      // Object ownership is taken over by the detector, i.e. will be deleted by the detector

      fDetSignals.Add(ds);
   }
   Bool_t AddDetectorSignalExpression(const TString& type, const KVString& _expr);

   ClassDef(KVDetector, 10)      //Base class for the description of detectors in multidetector arrays
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
#endif
