#include "KVDetector.h"
#include "Riostream.h"
#include "TROOT.h"
#include "TRandom3.h"
#include "TBuffer.h"
#include "KVUnits.h"
#include "KVTelescope.h"
#include "KVGroup.h"
#include "KVCalibrator.h"
#include "TPluginManager.h"
#include "TObjString.h"
#include "TClass.h"
/*** geometry ***/
#include "TGeoVolume.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoBBox.h"
#include "TGeoArb8.h"
#include "TGeoTube.h"
#include "KVCalibratedSignal.h"
#include "KVDetectorSignalExpression.h"
#include "KVZDependentCalibratedSignal.h"

#include <TGeoPhysicalNode.h>
#include <TGraph.h>

using namespace std;

ClassImp(KVDetector)


Int_t KVDetector::fDetCounter = 0;

void KVDetector::init()
{
   //default initialisations
   fCalibrators = 0;
   fParticles = 0;
   fCalWarning = 0;
   fAbsorbers = new KVList;
   fActiveLayer = nullptr;
   fIDTelescopes = new KVList(kFALSE);
   fIDTelescopes->SetCleanup(kTRUE);
   fIDTelAlign = new KVList(kFALSE);
   fIDTelAlign->SetCleanup(kTRUE);
   fIDTele4Ident = 0;
   fIdentP = fUnidentP = 0;
   fTotThickness = 0.;
   fDepthInTelescope = 0.;
   fELossF = fEResF = fRangeF = 0;
   fEResforEinc = -1.;
   fAlignedDetectors[0] = 0;
   fAlignedDetectors[1] = 0;
   fSimMode = kFALSE;
   fPresent = kTRUE;
   fDetecting = kTRUE;
   fParentStrucList.SetCleanup();
   fSingleLayer = kTRUE;
   fNode.SetDetector(this);
   // detector owns any signals which are defined for it
   fDetSignals.SetOwner();
   // adding a new signal with the same name as an existing one
   // will delete the existing signal and replace it
   fDetSignals.ReplaceObjects();
}

KVDetector::KVDetector()
{
//default ctor
   init();
   fDetCounter++;
   SetName(Form("Det_%d", fDetCounter));
}

//________________________________________________________________________________________
KVDetector::KVDetector(const Char_t* type,
                       const Float_t thick): KVMaterial()
{
   // Create a new detector of a given material and thickness in centimetres (default value = 0.0)

   init();
   SetType("DET");
   fDetCounter++;
   SetName(Form("Det_%d", fDetCounter));
   AddAbsorber(new KVMaterial(type, thick));
}

//_______________________________________________________________
KVDetector::KVDetector(const KVDetector& obj) : KVMaterial(), KVPosition()
{
//copy ctor
   init();
   fDetCounter++;
   SetName(Form("Det_%d", fDetCounter));
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVDetector&) obj).Copy(*this);
#endif
}

//_______________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVDetector::Copy(TObject& obj) const
#else
void KVDetector::Copy(TObject& obj)
#endif
{
   //copy 'this' to 'obj'
   //The structure of the detector is copied, with new cloned objects for
   //each absorber layer. The active layer is set in the new detector.

   TIter next(fAbsorbers);
   KVMaterial* mat;
   while ((mat = (KVMaterial*) next())) {
      ((KVDetector&) obj).AddAbsorber((KVMaterial*) mat->Clone());
   }
   //set active layer
   Int_t in_actif = fAbsorbers->IndexOf(fActiveLayer);
   ((KVDetector&) obj).SetActiveLayer(((KVDetector&)obj).GetAbsorber(in_actif));
}


//_______________________________________________________________
KVDetector::~KVDetector()
{
   fIDTelescopes->Clear();
   SafeDelete(fIDTelescopes);
   SafeDelete(fCalibrators);
   SafeDelete(fParticles);
   delete fAbsorbers;
   SafeDelete(fELossF);
   SafeDelete(fEResF);
   SafeDelete(fRangeF);
   fIDTelAlign->Clear();
   SafeDelete(fIDTelAlign);
   SafeDelete(fIDTele4Ident);
   if (fAlignedDetectors[0]) fAlignedDetectors[0]->Clear("nodelete");
   SafeDelete(fAlignedDetectors[0]);
   if (fAlignedDetectors[1]) fAlignedDetectors[1]->Clear("nodelete");
   SafeDelete(fAlignedDetectors[1]);
}

//________________________________________________________________
void KVDetector::SetMaterial(const Char_t* type)
{
   // Set material of active layer.
   // If no absorbers have been added to the detector, create and add
   // one (active layer by default)

   if (!GetActiveLayer())
      AddAbsorber(new KVMaterial(type));
   else
      GetActiveLayer()->SetMaterial(type);
}

//________________________________________________________________
void KVDetector::DetectParticle(KVNucleus* kvp, TVector3* norm)
{
   //Calculate the energy loss of a charged particle traversing the detector,
   //the particle is slowed down, it is added to the list of all particles hitting the
   //detector. The apparent energy loss of the particle in the active layer of the
   //detector is set.
   //Do nothing if particle has zero (or -ve) energy.
   //
   //If the optional argument 'norm' is given, it is supposed to be a vector
   //normal to the detector, oriented from the origin towards the detector.
   //In this case the effective thicknesses of the detector's absorbers 'seen' by the particle
   //depending on its direction of motion is used for the calculation.

   if (kvp->GetKE() <= 0.)
      return;

   AddHit(kvp);                 //add nucleus to list of particles hitting detector in the event
   //set flag to say that particle has been slowed down
   kvp->SetIsDetected();
   //If this is the first absorber that the particle crosses, we set a "reminder" of its
   //initial energy
   if (!kvp->GetPInitial())
      kvp->SetE0();

   Double_t* thickness = 0;
   if (norm) {
      // modify thicknesses of all layers according to orientation,
      // and store original thicknesses in array
      TVector3 p = kvp->GetMomentum();
      thickness = new Double_t[fAbsorbers->GetEntries()];
      KVMaterial* abs;
      int i = 0;
      TIter next(fAbsorbers);
      while ((abs = (KVMaterial*) next())) {
         thickness[i++] = abs->GetThickness();
         Double_t T = abs->GetEffectiveThickness((*norm), p);
         abs->SetThickness(T);
      }
   }
   Double_t eloss = GetTotalDeltaE(kvp->GetZ(), kvp->GetA(), kvp->GetEnergy());
   Double_t dE = GetDeltaE(kvp->GetZ(), kvp->GetA(), kvp->GetEnergy());
   if (norm) {
      // reset thicknesses of absorbers
      KVMaterial* abs;
      int i = 0;
      TIter next(fAbsorbers);
      while ((abs = (KVMaterial*) next())) {
         abs->SetThickness(thickness[i++]);
      }
      delete [] thickness;
   }
   Double_t epart = kvp->GetEnergy() - eloss;
   if (epart < 1e-3) {
      //printf("%s, pb d arrondi on met l energie de la particule a 0\n",GetName());
      epart = 0.0;
   }
   kvp->SetEnergy(epart);
   Double_t eloss_old = GetEnergyLoss();
   SetEnergyLoss(eloss_old + dE);
}

//_______________________________________________________________________________

Double_t KVDetector::GetELostByParticle(KVNucleus* kvp, TVector3* norm)
{
   //Calculate the total energy loss of a charged particle traversing the detector.
   //This does not affect the "stored" energy loss value of the detector,
   //nor its ACQData, nor the energy of the particle.
   //
   //If the optional argument 'norm' is given, it is supposed to be a vector
   //normal to the detector, oriented from the origin towards the detector.
   //In this case the effective thicknesses of the detector's absorbers 'seen' by the particle
   //depending on its direction of motion is used for the calculation.

   Double_t* thickness = 0;
   if (norm) {
      // modify thicknesses of all layers according to orientation,
      // and store original thicknesses in array
      TVector3 p = kvp->GetMomentum();
      thickness = new Double_t[fAbsorbers->GetEntries()];
      KVMaterial* abs;
      int i = 0;
      TIter next(fAbsorbers);
      while ((abs = (KVMaterial*) next())) {
         thickness[i++] = abs->GetThickness();
         Double_t T = abs->GetEffectiveThickness((*norm), p);
         abs->SetThickness(T);
      }
   }
   Double_t eloss = GetTotalDeltaE(kvp->GetZ(), kvp->GetA(), kvp->GetEnergy());
   if (norm) {
      // reset thicknesses of absorbers
      KVMaterial* abs;
      int i = 0;
      TIter next(fAbsorbers);
      while ((abs = (KVMaterial*) next())) {
         abs->SetThickness(thickness[i++]);
      }
      delete [] thickness;
   }
   return eloss;
}

//_______________________________________________________________________________

Double_t KVDetector::GetParticleEIncFromERes(KVNucleus* kvp, TVector3* norm)
{
   //Calculate the energy of particle 'kvn' before its passage through the detector,
   //based on the current kinetic energy, Z & A of nucleus 'kvn', supposed to be
   //after passing through the detector.
   //
   //If the optional argument 'norm' is given, it is supposed to be a vector
   //normal to the detector, oriented from the origin towards the detector.
   //In this case the effective thicknesses of the detector's absorbers 'seen' by the particle
   //depending on its direction of motion is used for the calculation.

   Double_t Einc = 0.0;
   //make 'clone' of particle
   KVNucleus* clone_part = new KVNucleus(kvp->GetZ(), kvp->GetA());
   clone_part->SetMomentum(kvp->GetMomentum());
   //composite detector - calculate losses in all layers
   KVMaterial* abs;
   TIter next(fAbsorbers, kIterBackward); //work through list backwards
   while ((abs = (KVMaterial*) next())) {

      //calculate energy of particle before current absorber
      Einc = abs->GetParticleEIncFromERes(clone_part, norm);

      //set new energy of particle
      clone_part->SetKE(Einc);
   }
   //delete clone
   delete clone_part;
   return Einc;
}


//__________________________________________________________________________________
void KVDetector::Print(Option_t* opt) const
{
   //Print info on this detector
   //if option="data" the energy loss and raw data are displayed

   if (!strcmp(opt, "data")) {
      cout << GetName() << " -- ";
      // print raw data signals
      KVDetectorSignal* sig;
      TIter it(&GetListOfDetectorSignals());
      while ((sig = (KVDetectorSignal*)it())) {
         if (sig->IsRaw()) {
            std::cout << sig->GetName() << "=" << sig->GetValue() << " | ";
         }
      }
      // print calibrated data
      it.Reset();
      while ((sig = (KVDetectorSignal*)it())) {
         if (!sig->IsRaw() && !sig->GetValueNeedsExtraParameters()) {
            std::cout << sig->GetName() << "=" << sig->GetValue() << " | ";
         }
      }
      cout << " ";
      if (BelongsToUnidentifiedParticle())
         cout << "(Belongs to an unidentified particle)";
      cout << endl;
   }
   else if (!strcmp(opt, "all")) {
      //Give full details of detector
      //
      TString option("    ");
      cout << option << ClassName() << " : " << ((KVDetector*) this)->
           GetName() << endl;
      //composite detector - print all layers
      KVMaterial* abs;
      TIter next(fAbsorbers);
      while ((abs = (KVMaterial*) next())) {
         if (GetActiveLayer() == abs)
            cout << " ### ACTIVE LAYER ### " << endl;
         cout << option << option;
         abs->Print();
         if (GetActiveLayer() == abs)
            cout << " #################### " << endl;
      }
      if (fParticles) {
         cout << option << " --- Detected particles:" << endl;
         fParticles->Print();
      }
      if (fIDTelescopes) {
         cout << option << " --- Detector belongs to the following Identification Telescopes:" << endl;
         fIDTelescopes->ls();
      }
   }
   else {
      //just print name
      cout << ClassName() << ": " << ((KVDetector*) this)->
           GetName() << endl;
   }
}


//___________________________________________________________________________________

const Char_t* KVDetector::GetArrayName()
{
   // This method is called by KVASMultiDetArray::MakeListOfDetectors
   // after the array geometry has been defined (i.e. all detectors have
   // been placed in the array). The string returned by this method
   // is used to set the name of the detector.
   //
   // Override this method in child classes in order to define a naming
   // convention for specific detectors of the array.
   //
   // By default we return the same name as KVDetector::GetName

   fFName = GetName();
   return fFName.Data();
}

//_____________________________________________________________________________________
Bool_t KVDetector::AddCalibrator(KVCalibrator* cal, const KVNameValueList& opts)
{
   // Associate a calibration with this detector (the object will be deleted by the detector)
   //
   // This will add a new signal to the list of the detector's signals
   //
   // Also sets calibrator's name to [detname]_[caltype]
   //
   // The (optional) KVNameValueList argument can be used to pass any extra parameters/options.
   // For example, if it contains a parameter `ZRange`:
   //
   //~~~~~~~~~~~~~~~
   // ZRange=1-10
   //~~~~~~~~~~~~~~~
   //
   // then the calibrator will be handled by a KVZDependentCalibratedSignal (handles several
   // calibrators which provide the same output signal, each one is used for a specific range
   // of atomic numbers)

   if (!cal) return kFALSE;
   // look for input signal
   KVDetectorSignal* in  = GetDetectorSignal(cal->GetInputSignalType());
   // input signal must exist
   if (!in) {
      Error("AddCalibrator", "No known detector signal: %s", cal->GetInputSignalType().Data());
      return kFALSE;
   }
   if (!fCalibrators)
      fCalibrators = new KVList();

   fCalibrators->Add(cal);
   cal->SetName(Form("%s_%s", GetName(), cal->GetType()));

   if (cal->GetOutputSignalType() == "") {
      Warning("AddCalibrator", "%s : output signal not defined for calibrator %s. No output signal created.",
              GetName(), cal->GetType());
   }
   else {
      KVDetectorSignal* new_cal_sig(nullptr);
      if (opts.HasParameter("ZRange")) {
         // If 'ZRange' parameter is given we need to find the KVZDependentCalibratedSignal
         // and add a new signal to it.
         KVDetectorSignal* sig = GetDetectorSignal(cal->GetOutputSignalType());
         if (!sig) {
            new_cal_sig = sig = new KVZDependentCalibratedSignal(in, cal->GetOutputSignalType());
         }
         dynamic_cast<KVZDependentCalibratedSignal*>(sig)->AddSignal(new KVCalibratedSignal(in, cal), opts.GetStringValue("ZRange"));
      }
      else {
         new_cal_sig = new KVCalibratedSignal(in, cal);
      }
      if (new_cal_sig) fDetSignals.Add(new_cal_sig);
   }
   return kTRUE;
}

Bool_t KVDetector::ReplaceCalibrator(const Char_t* type, KVCalibrator* cal, const KVNameValueList& opts)
{
   // Replace calibrator of given type with the given calibrator object
   // The calibrator object should not be shared with any other detectors: it now belongs
   // to this detector, which will delete it when necessary.
   // If an exising calibrator with the same type is already defined, it will be
   // deleted and removed from the detector's calibrator list
   //
   // Returns kFALSE in case of problems.
   //
   // The (optional) KVNameValueList argument can be used to pass any extra parameters/options.

   KVCalibrator* old_cal = GetCalibrator(type);
   if (old_cal) {
      fCalibrators->Remove(old_cal);
      remove_signal_for_calibrator(old_cal);
      delete old_cal;
   }
   return AddCalibrator(cal, opts);
}

Bool_t KVDetector::IsCalibrated(const KVNameValueList& params) const
{
   // A detector is considered to be calibrated if it has
   // a signal "Energy" available and if depending on the supplied parameters
   // this signal can be calculated

   KVCalibratedSignal* e_sig = dynamic_cast<KVCalibratedSignal*>(GetDetectorSignal("Energy"));
   return (e_sig && e_sig->IsCalibratedFor(params));
}


void KVDetector::Clear(Option_t* opt)
{
   //Set energy loss(es) etc. to zero
   //If opt="N" we do not reset acquisition parameters/raw detector signals

   SetAnalysed(kFALSE);
   fIdentP = fUnidentP = 0;
   ResetBit(kIdentifiedParticle);
   ResetBit(kUnidentifiedParticle);
   if (strncmp(opt, "N", 1)) {
      TIter it(&GetListOfDetectorSignals());
      KVDetectorSignal* ds;
      while ((ds = (KVDetectorSignal*)it())) {
         if (ds->IsRaw() && !ds->IsExpression()) ds->Reset();
      }
   }
   ClearHits();
   //reset all layers in detector
   KVMaterial* mat;
   TIter next(fAbsorbers);
   while ((mat = (KVMaterial*) next())) {
      mat->Clear();
   }
   fEResforEinc = -1.;
}

void KVDetector::AddAbsorber(KVMaterial* mat)
{
   // Add a layer of absorber material to the detector
   // By default, the first layer added is set as the "Active" layer.
   // Call SetActiveLayer to change this.
   fAbsorbers->Add(mat);
   if (!TestBit(kActiveSet))
      SetActiveLayer(mat);
   if (fAbsorbers->GetSize() > 1) fSingleLayer = kFALSE;
}

KVMaterial* KVDetector::GetAbsorber(Int_t i) const
{
   //Returns pointer to the i-th absorber in the detector (i=0 first absorber, i=1 second, etc.)

   if (i < 0) {
      //special case of reading old detectors
      //no warning
      return 0;
   }
   if (!fAbsorbers) {
      Error("GetAbsorber", "No absorber list");
      return 0;
   }
   if (fAbsorbers->GetSize() < 1) {
      Error("GetAbsorber", "Nothing in absorber list");
      return 0;
   }
   if (i >= fAbsorbers->GetSize()) {
      Error("GetAbsorber", "i=%d but only %d absorbers in list", i,
            fAbsorbers->GetSize());
      return 0;
   }

   return (KVMaterial*) fAbsorbers->At(i);
}

void KVDetector::remove_signal_for_calibrator(KVCalibrator* K)
{
   // Used when a calibrator object is removed or replaced
   // We remove and delete the corresponding output signal from the list of detector signals

   if (K->GetOutputSignalType() != "") {
      KVDetectorSignal* ds = GetDetectorSignal(K->GetOutputSignalType());
      if (ds) {
         fDetSignals.Remove(ds);
         delete ds;
      }
   }
}

void KVDetector::RemoveCalibrators()
{
   // Removes all calibrations associated to this detector: in other words, we delete all
   // the KVCalibrator objects in list fCalibrators.
   //
   // We also destroy all signals provided by these calibrators

   if (fCalibrators) {
      KVCalibrator* K;
      TIter it(fCalibrators);
      while ((K = (KVCalibrator*)it())) {
         remove_signal_for_calibrator(K);
      }
      fCalibrators->Delete();
   }
}

//___________________________________________________________________________//

void KVDetector::AddIDTelescope(TObject* idt)
{
   //Add ID telescope to list of telescopes to which detector belongs
   fIDTelescopes->Add(idt);
}

KVList* KVDetector::GetAlignedIDTelescopes()
{
   // Return list of all ID telescopes containing detectors placed in front of
   // this one.

   // temporary kludge during transition to trajectory-based reconstruction
   // ROOT-geometry-based detectors will not have fIDTelAlign filled
   if (ROOTGeo() && !fIDTelAlign->GetEntries()) {
      const KVGeoDNTrajectory* Rtr = GetGroup()->GetTrajectoryForReconstruction(
                                        (KVGeoDNTrajectory*)GetNode()->GetTrajectories()->First(),
                                        GetNode()
                                     );
      if (Rtr) fIDTelAlign->AddAll(Rtr->GetIDTelescopes());
   }
   return fIDTelAlign;
}

//___________________________________________________________________________//

TList* KVDetector::GetTelescopesForIdentification()
{
   //Returns list of identification telescopes to be used in order to try to identify
   //particles stopping in this detector. This is the same as GetAlignedIDTelescopes
   //but only including the telescopes of which this detector is a member.
   if (fIDTele4Ident) return fIDTele4Ident;
   if (!fIDTelescopes || !fIDTelAlign) return 0;
   fIDTele4Ident = new TList;
   TIter next(GetAlignedIDTelescopes());
   TObject* idt;
   while ((idt = next())) {
      if (fIDTelescopes->FindObject(idt)) fIDTele4Ident->Add(idt);
   }
   return fIDTele4Ident;
}


//______________________________________________________________________________//

Double_t KVDetector::GetCorrectedEnergy(KVNucleus* nuc, Double_t e, Bool_t transmission)
{
   // Returns the total energy loss in the detector for a given nucleus
   // including inactive absorber layers.
   // e = energy loss in active layer (if not given, we use current value)
   // transmission = kTRUE (default): the particle is assumed to emerge with
   //            a non-zero residual energy Eres after the detector.
   //          = kFALSE: the particle is assumed to stop in the detector.
   //
   // WARNING: if transmission=kTRUE, and if the residual energy after the detector
   //   is known (i.e. measured in a detector placed after this one), you should
   //   first call
   //       SetEResAfterDetector(Eres);
   //   before calling this method. Otherwise, especially for heavy ions, the
   //   correction may be false for particles which are just above the punch-through energy.
   //
   // WARNING 2: if measured energy loss in detector active layer is greater than
   // maximum possible theoretical value for given nucleus' Z & A, this may be because
   // the A was not measured but calculated from Z and hence could be false, or perhaps
   // there was an (undetected) pile-up of two or more particles in the detector.
   // In this case we return the uncorrected energy measured in the active layer
   // and we add the following parameters to the particle (in nuc->GetParameters()):
   //
   // GetCorrectedEnergy.Warning = 1
   // GetCorrectedEnergy.Detector = [name]
   // GetCorrectedEnergy.MeasuredDE = [value]
   // GetCorrectedEnergy.MaxDE = [value]
   // GetCorrectedEnergy.Transmission = 0 or 1
   // GetCorrectedEnergy.ERES = [value]

   Int_t z = nuc->GetZ();
   Int_t a = nuc->GetA();

   if (e < 0.) e = GetEnergy();
   if (e <= 0) {
      SetEResAfterDetector(-1.);
      return 0;
   }

   enum SolType solution = kEmax;
   if (!transmission) solution = kEmin;

   // check that apparent energy loss in detector is compatible with a & z
   Double_t maxDE = GetMaxDeltaE(z, a);
   Double_t EINC, ERES = GetEResAfterDetector();
   if (e > maxDE) {
      nuc->GetParameters()->SetValue("GetCorrectedEnergy.Warning", 1);
      nuc->GetParameters()->SetValue("GetCorrectedEnergy.Detector", GetName());
      nuc->GetParameters()->SetValue("GetCorrectedEnergy.MeasuredDE", e);
      nuc->GetParameters()->SetValue("GetCorrectedEnergy.MaxDE", maxDE);
      nuc->GetParameters()->SetValue("GetCorrectedEnergy.Transmission", (Int_t)transmission);
      nuc->GetParameters()->SetValue("GetCorrectedEnergy.ERES", ERES);
      return e;

   }
   if (transmission && ERES > 0.) {
      // if residual energy is known we use it to calculate EINC.
      // if EINC < max of dE curve, we change solution
      EINC = GetIncidentEnergyFromERes(z, a, ERES);
      if (EINC < GetEIncOfMaxDeltaE(z, a)) solution = kEmin;
      // we could keep the EINC value calculated using ERES, but then
      // the corrected dE of this detector would not depend on the
      // measured dE !
   }
   EINC = GetIncidentEnergy(z, a, e, solution);
   if (EINC < 0) {
      SetEResAfterDetector(-1.);
      return EINC;
   }
   ERES = GetERes(z, a, EINC);

   SetEResAfterDetector(-1.);
   return (EINC - ERES);
}

//______________________________________________________________________________//

Int_t KVDetector::FindZmin(Double_t ELOSS, Char_t mass_formula)
{
   //For particles which stop in the first stage of an identification telescope,
   //we can at least estimate a minimum Z value based on the energy lost in this
   //detector.
   //
   //This is based on the KVMaterial::GetMaxDeltaE method, giving the maximum
   //energy loss in the active layer of the detector for a given nucleus (A,Z).
   //
   //The "Zmin" is the Z of the particle which gives a maximum energy loss just greater
   //than that measured in the detector. Particles with Z<Zmin could not lose as much
   //energy and so are excluded.
   //
   //If ELOSS is not given, we use the current value of GetEnergy()
   //Use 'mass_formula' to change the formula used to calculate the A of the nucleus
   //from its Z. Default is valley of stability value. (see KVNucleus::GetAFromZ).
   //
   //If the value of ELOSS or GetEnergy() is <=0 we return Zmin=0

   ELOSS = (ELOSS < 0. ? GetEnergy() : ELOSS);
   if (ELOSS <= 0) return 0;

   UInt_t z = 40;
   UInt_t zmin, zmax;
   zmin = 1;
   zmax = 92;
   Double_t difference;
   UInt_t last_positive_difference_z = 1;
   KVNucleus particle;
   if (mass_formula > -1)
      particle.SetMassFormula((UChar_t)mass_formula);

   difference = 0.;

   while (zmax > zmin + 1) {

      particle.SetZ(z);

      difference = GetMaxDeltaE(z, particle.GetA()) - ELOSS;
      //if difference < 0 the z is too small
      if (difference < 0.0) {

         zmin = z;
         z += (UInt_t)((zmax - z) / 2 + 0.5);

      }
      else {

         zmax = z;
         last_positive_difference_z = z;
         z -= (UInt_t)((z - zmin) / 2 + 0.5);

      }
   }
   if (difference < 0) {
      //if the last z we tried actually made the difference become negative again
      //(i.e. z too small) we return the last z which gave a +ve difference
      z = last_positive_difference_z;
   }
   return z;
}

//_____________________________________________________________________________________//

Double_t KVDetector::ELossActive(Double_t* x, Double_t* par)
{
   // Calculates energy loss (in MeV) in active layer of detector, taking into account preceding layers
   //
   // Arguments are:
   //    x[0] is incident energy in MeV
   // Parameters are:
   //   par[0]   Z of ion
   //   par[1]   A of ion

   Double_t e = x[0];
   TIter next(fAbsorbers);
   KVMaterial* mat;
   mat = (KVMaterial*)next();
   while (fActiveLayer != mat) {
      // calculate energy losses in absorbers before active layer
      e = mat->GetERes(par[0], par[1], e);     //residual energy after layer
      if (e <= 0.)
         return 0.;          // return 0 if particle stops in layers before active layer
      mat = (KVMaterial*)next();
   }
   //calculate energy loss in active layer
   return mat->GetDeltaE(par[0], par[1], e);
}

//_____________________________________________________________________________________//

Double_t KVDetector::RangeDet(Double_t* x, Double_t* par)
{
   // Calculates range (in centimetres) of ions in detector as a function of incident energy (in MeV),
   // taking into account all layers of the detector.
   //
   // Arguments are:
   //   x[0]  =  incident energy in MeV
   // Parameters are:
   //   par[0] = Z of ion
   //   par[1] = A of ion

   Double_t Einc = x[0];
   Int_t Z = (Int_t)par[0];
   Int_t A = (Int_t)par[1];
   Double_t range = 0.;
   TIter next(fAbsorbers);
   KVMaterial* mat = (KVMaterial*)next();
   if (!mat) return 0.;
   do {
      // range in this layer
      Double_t this_range = mat->GetLinearRange(Z, A, Einc);
      KVMaterial* next_mat = (KVMaterial*)next();
      if (this_range > mat->GetThickness()) {
         // particle traverses layer.
         if (next_mat)
            range += mat->GetThickness();
         else // if this is last layer, the range continues to increase beyond size of detector
            range += this_range;
         // calculate incident energy for next layer (i.e. residual energy after this layer)
         Einc = mat->GetERes(Z, A, Einc);
      }
      else {
         // particle stops in layer
         range += this_range;
         return range;
      }
      mat = next_mat;
   }
   while (mat);
   // particle traverses detector
   return range;
}

//_____________________________________________________________________________________//

Double_t KVDetector::EResDet(Double_t* x, Double_t* par)
{
   // Calculates residual energy (in MeV) of particle after traversing all layers of detector.
   // Returned value is -1000 if particle stops in one of the layers of the detector.
   //
   // Arguments are:
   //    x[0] is incident energy in MeV
   // Parameters are:
   //   par[0]   Z of ion
   //   par[1]   A of ion

   Double_t e = x[0];
   TIter next(fAbsorbers);
   KVMaterial* mat;
   while ((mat = (KVMaterial*)next())) {
      Double_t eres = mat->GetERes(par[0], par[1], e);     //residual energy after layer
      if (eres <= 0.)
         return -1000.;          // return -1000 if particle stops in layers before active layer
      e = eres;
   }
   return e;
}

//____________________________________________________________________________________

KVDetector* KVDetector::MakeDetector(const Char_t* name, Float_t thickness)
{
   //Static function which will create an instance of the KVDetector-derived class
   //corresponding to 'name'
   //These are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   //   [name_of_dataset].detector_type
   //   detector_type
   // To use the dataset-dependent plugin, call this method with
   //  name = "[name_of_dataset].detector_type"
   // If not, the default plugin will be used
   //first we check if there is a special plugin for the DataSet
   //if not we take the default one
   //
   //'thickness' is passed as argument to the constructor for the detector plugin

   //check and load plugin library
   TPluginHandler* ph = nullptr;
   KVString nom(name);
   if (!nom.Contains(".") && !(ph = LoadPlugin("KVDetector", name))) return nullptr;
   if (nom.Contains(".")) {
      // name format like [dataset].[det_type]
      // in case dataset name contains "." we parse string to find detector type: assumed after the last "."
      nom.RBegin(".");
      KVString det_type = nom.RNext();
      if (!(ph = LoadPlugin("KVDetector", name))) {
         if (!(ph = LoadPlugin("KVDetector", det_type))) {
            return nullptr;
         }
      }
   }

   //execute constructor/macro for detector
   return (KVDetector*) ph->ExecPlugin(1, thickness);
}

//____________________________________________________________________________________

void KVDetector::GetVerticesInOwnFrame(TVector3* corners, Double_t depth, Double_t layer_thickness)
{
   // This will fill the array corners[8] with the coordinates of the vertices of the
   // front (corners[0],...,corners[3]) & back (corners[4],...,corners[7]) sides of the volume
   // representing either a single absorber or the whole detector.
   //
   // depth = depth of detector/absorber inside the KVTelescope it belongs to (in centimetres)
   // layer_thickness = thickness of absorber/detector (in centimetres)
   //
   // Positioning information is taken from the KVTelescope to which this detector
   // belongs; if this is not the case, nothing will be done.

   // relative distance to back of detector
   Double_t dist_to_back = depth + layer_thickness;

   // get coordinates
   KVTelescope* fTelescope = (KVTelescope*)GetParentStructure("TELESCOPE");
   if (fTelescope) {
      fTelescope->GetCornerCoordinatesInOwnFrame(corners, depth);
      fTelescope->GetCornerCoordinatesInOwnFrame(&corners[4], dist_to_back);
   }
   else {
      GetCornerCoordinatesInOwnFrame(corners, depth);
      GetCornerCoordinatesInOwnFrame(&corners[4], dist_to_back);
   }
}

//____________________________________________________________________________________

TGeoVolume* KVDetector::GetGeoVolume()
{
   // Construct a TGeoVolume shape to represent this detector in the current
   // geometry managed by gGeoManager.
   //
   // Making the volume requires:
   //    - to construct a 'mother' volume (TGeoArb8) defined by the (theta-min/max, phi-min/max)
   //      and the total thickness of the detector (all layers)
   //    - to construct and position volumes (TGeoArb8) for each layer of the detector inside the
   //      'mother' volume. Each layer is represented by a TGeoArb8 whose two parallel faces correspond
   //      to the front and back surfaces of the layer.
   //
   // If the detector is composed of a single absorber, we do not create a superfluous "mother" volume.
   //
   // gGeoManager must point to current instance of geometry manager.

   if (!gGeoManager) return NULL;

   if (fTotThickness == 0) GetTotalThicknessInCM(); // calculate sum of absorber thicknesses in centimetres
   // get from telescope info on relative depth of detector i.e. depth inside telescope

   KVTelescope* fTelescope = (KVTelescope*)GetParentStructure("TELESCOPE");
   if (fTelescope && fDepthInTelescope == 0)
      fDepthInTelescope = fTelescope->GetDepthInCM(fTelescope->GetDetectorRank(this));

   TVector3 coords[8];
   Double_t vertices[16];
   Double_t tot_thick_det = fTotThickness;
   TGeoMedium* med;
   TGeoVolume* mother_vol = 0;

   Bool_t multi_layer = fAbsorbers->GetSize() > 1;

   if (multi_layer) {
      mother_vol = gGeoManager->MakeVolumeAssembly(Form("%s_DET", GetName()));
   }

   /**** BUILD & ADD ABSORBER VOLUMES ****/
   TIter next(fAbsorbers);
   KVMaterial* abs;
   Double_t depth_in_det = 0.;
   Int_t no_abs = 1;
   while ((abs = (KVMaterial*)next())) {
      // get medium for absorber
      med = abs->GetGeoMedium();
      Double_t thick = abs->GetThickness();
      // do not include layers with zero thickness
      if (thick == 0.0) continue;
      GetVerticesInOwnFrame(coords, fDepthInTelescope + depth_in_det, thick);
      for (int i = 0; i < 8; i++) {
         vertices[2 * i] = coords[i].X();
         vertices[2 * i + 1] = coords[i].Y();
      }
      Double_t dz = thick / 2.;
      TString vol_name;
      if (abs == GetActiveLayer()) vol_name = GetName();
      else vol_name = Form("%s_%d_%s", GetName(), no_abs, abs->GetType());
      TGeoVolume* vol =
         gGeoManager->MakeArb8(vol_name.Data(), med, dz, vertices);
      vol->SetLineColor(med->GetMaterial()->GetDefaultColor());
      if (multi_layer) {
         /*** position absorber in mother ***/
         Double_t trans_z = -tot_thick_det / 2. + depth_in_det + dz; // (note: reference is CENTRE of absorber)
         TGeoTranslation* tr = new TGeoTranslation(0., 0., trans_z);
         mother_vol->AddNode(vol, 1, tr);
      }
      else {
         // single absorber: mother is absorber is detector is mother is ...
         mother_vol = vol;
      }
      depth_in_det += thick;
      no_abs++;
   }

   return mother_vol;
}

//____________________________________________________________________________________

void KVDetector::AddToGeometry()
{
   // Construct and position a TGeoVolume shape to represent this detector in the current
   // geometry managed by gGeoManager.
   //
   // Adding the detector to the geometry requires:
   //    - to construct a 'mother' volume (TGeoArb8) defined by the (theta-min/max, phi-min/max)
   //      and the total thickness of the detector (all layers)
   //    - to construct and position volumes (TGeoArb8) for each layer of the detector inside the
   //      'mother' volume
   //    - to position 'mother' inside the top-level geometry
   //
   // gGeoManager must point to current instance of geometry manager.

   if (!gGeoManager) return;

   // get volume for detector
   TGeoVolume* vol = GetGeoVolume();

   // rotate detector to orientation corresponding to (theta,phi)
   Double_t theta = GetTheta();
   Double_t phi = GetPhi();
   TGeoRotation rot1, rot2;
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-90., 0., 0.);
   // distance to detector centre = distance to telescope + depth of detector in telescope + half total thickness of detector
   KVTelescope* fTelescope = (KVTelescope*)GetParentStructure("TELESCOPE");
   Double_t dist_telescope = (fTelescope ? fTelescope->GetDistance() : 0.);
   Double_t dist = dist_telescope + fDepthInTelescope + fTotThickness / 2.;
   // translate detector to correct distance from target (note: reference is CENTRE of detector)
   Double_t trans_z = dist;

   TGeoTranslation tran(0, 0, trans_z);
   TGeoHMatrix h = rot2 * tran * rot1;
   TGeoHMatrix* ph = new TGeoHMatrix(h);

   // add detector volume to geometry
   gGeoManager->GetTopVolume()->AddNode(vol, 1, ph);
}

#ifndef WITH_CPP11
void printvec(const TVector3& v)
{
   cout << "(" << v.X() << "," << v.Y() << "," << v.Z() << ")";
};
#endif

Double_t KVDetector::GetEntranceWindowSurfaceArea()
{
   // Return surface area of first layer of detector in cm2.

   if (ROOTGeo()) {
      if (!GetEntranceWindow().GetShape()->InheritsFrom("TGeoArb8")) {
         // simple shape area
         return GetEntranceWindow().GetShape()->GetFacetArea(1);
      }
      // Monte Carlo calculation for TGeoArb8 shapes
      return GetEntranceWindow().GetSurfaceArea();
   }

   KVTelescope* fTelescope = (KVTelescope*)GetParentStructure("TELESCOPE");
   if (fTelescope && fDepthInTelescope == 0)
      fDepthInTelescope = fTelescope->GetDepthInCM(fTelescope->GetDetectorRank(this));

   TVector3 coords[4];
#ifdef WITH_CPP11
   auto printvec = [](const TVector3 & v) {
      cout << "(" << v.X() << "," << v.Y() << "," << v.Z() << ")";
   };
#endif

   if (fTelescope) fTelescope->GetCornerCoordinates(coords, fDepthInTelescope);
   else GetCornerCoordinates(coords, 0);
   cout << "DETECTOR COORDINATES (in cm):" << endl;
   cout << "=================================" << endl;
   cout << " A : ";
   printvec(coords[0]);
   cout << endl;
   cout << " B : ";
   printvec(coords[1]);
   cout << endl;
   cout << " C : ";
   printvec(coords[2]);
   cout << endl;
   cout << " D : ";
   printvec(coords[3]);
   cout << endl;

   cout << "DETECTOR DIMENSIONS (in cm):" << endl;
   cout << "================================" << endl;
   Double_t c = (coords[0] - coords[1]).Mag();
   Double_t b = (coords[1] - coords[2]).Mag();
   Double_t d = (coords[2] - coords[3]).Mag();
   Double_t a = (coords[0] - coords[3]).Mag();
   cout << " AB = " << c << endl;
   cout << " BC = " << b << endl;
   cout << " CD = " << d << endl;
   cout << " AD = " << a << endl;

   cout << "DETECTOR SURFACE AREA = ";
   Double_t surf = pow((a + b), 2.0) * (a - b + 2.0 * c) * (b - a + 2.0 * c);
   surf = sqrt(surf) / 400.0;
   cout << surf << " cm2" << endl;

   return surf;
}

TF1* KVDetector::GetEResFunction(Int_t Z, Int_t A)
{
   // Return pointer toTF1 giving residual energy after detector as function of incident energy,
   // for a given nucleus (Z,A).
   // The TF1::fNpx parameter is taken from environment variable KVDetector.ResidualEnergy.Npx

   if (!fEResF) {
      fEResF = new TF1(Form("KVDetector:%s:ERes", GetName()), this, &KVDetector::EResDet,
                       0., 1.e+04, 2, "KVDetector", "EResDet");
      fEResF->SetNpx(gEnv->GetValue("KVDetector.ResidualEnergy.Npx", 20));
   }
   fEResF->SetParameters((Double_t)Z, (Double_t)A);
   fEResF->SetRange(0., GetSmallestEmaxValid(Z, A));
   fEResF->SetTitle(Form("Residual energy [MeV] after detector %s for Z=%d A=%d", GetName(), Z, A));

   return fEResF;
}

TF1* KVDetector::GetRangeFunction(Int_t Z, Int_t A)
{
   // Return pointer toTF1 giving range (in centimetres) in detector as function of incident energy,
   // for a given nucleus (Z,A).
   // The TF1::fNpx parameter is taken from environment variable KVDetector.Range.Npx

   if (!fRangeF) {
      fRangeF = new TF1(Form("KVDetector:%s:Range", GetName()), this, &KVDetector::RangeDet,
                        0., 1.e+04, 2, "KVDetector", "RangeDet");
      fRangeF->SetNpx(gEnv->GetValue("KVDetector.Range.Npx", 20));
   }
   fRangeF->SetParameters((Double_t)Z, (Double_t)A);
   fRangeF->SetRange(0., GetSmallestEmaxValid(Z, A));
   fRangeF->SetTitle(Form("Range [cm] in detector %s for Z=%d A=%d", GetName(), Z, A));

   return fRangeF;
}

TF1* KVDetector::GetELossFunction(Int_t Z, Int_t A)
{
   // Return pointer to TF1 giving energy loss in active layer of detector as function of incident energy,
   // for a given nucleus (Z,A).
   // The TF1::fNpx parameter is taken from environment variable KVDetector.EnergyLoss.Npx

   if (!fELossF) {
      fELossF = new TF1(Form("KVDetector:%s:ELossActive", GetName()), this, &KVDetector::ELossActive,
                        0., 1.e+04, 2, "KVDetector", "ELossActive");
      fELossF->SetNpx(gEnv->GetValue("KVDetector.EnergyLoss.Npx", 20));
   }
   fELossF->SetParameters((Double_t)Z, (Double_t)A);
   fELossF->SetRange(0., GetSmallestEmaxValid(Z, A));
   fELossF->SetTitle(Form("Energy loss [MeV] in detector %s for Z=%d A=%d", GetName(), Z, A));
   return fELossF;
}

Double_t KVDetector::GetEIncOfMaxDeltaE(Int_t Z, Int_t A)
{
   // Overrides KVMaterial::GetEIncOfMaxDeltaE
   // Returns incident energy corresponding to maximum energy loss in the
   // active layer of the detector, for a given nucleus.

   return GetELossFunction(Z, A)->GetMaximumX();
}

Double_t KVDetector::GetMaxDeltaE(Int_t Z, Int_t A)
{
   // Overrides KVMaterial::GetMaxDeltaE
   // Returns maximum energy loss in the
   // active layer of the detector, for a given nucleus.

   return GetELossFunction(Z, A)->GetMaximum();
}

Double_t KVDetector::GetDeltaE(Int_t Z, Int_t A, Double_t Einc)
{
   // Overrides KVMaterial::GetDeltaE
   // Returns energy loss of given nucleus in the active layer of the detector.

   // optimization for single-layer detectors
   if (fSingleLayer) {
      return fActiveLayer->GetDeltaE(Z, A, Einc);
   }
   return GetELossFunction(Z, A)->Eval(Einc);
}

Double_t KVDetector::GetTotalDeltaE(Int_t Z, Int_t A, Double_t Einc)
{
   // Returns calculated total energy loss of ion in ALL layers of the detector.
   // This is just (Einc - GetERes(Z,A,Einc))

   return Einc - GetERes(Z, A, Einc);
}

Double_t KVDetector::GetERes(Int_t Z, Int_t A, Double_t Einc)
{
   // Overrides KVMaterial::GetERes
   // Returns residual energy of given nucleus after the detector.
   // Returns 0 if Einc<=0

   if (Einc <= 0.) return 0.;
   Double_t eres = GetEResFunction(Z, A)->Eval(Einc);
   // Eres function returns -1000 when particle stops in detector,
   // in order for function inversion (GetEIncFromEres) to work
   if (eres < 0.) eres = 0.;
   return eres;
}

Double_t KVDetector::GetIncidentEnergy(Int_t Z, Int_t A, Double_t delta_e, enum SolType type)
{
   // Overrides KVMaterial::GetIncidentEnergy
   // Returns incident energy corresponding to energy loss delta_e in active layer of detector for a given nucleus.
   // If delta_e is not given, the current energy loss in the active layer is used.
   //
   // By default the solution corresponding to the highest incident energy is returned
   // This is the solution found for Einc greater than the maximum of the dE(Einc) curve.
   // If you want the low energy solution set SolType = KVIonRangeTable::kEmin.
   //
   // WARNING: calculating the incident energy of a particle using only the dE in a detector
   // is ambiguous, as in general (and especially for very heavy ions) the maximum of the dE
   // curve occurs for Einc greater than the punch-through energy, therefore it is not always
   // true to assume that if the particle does not stop in the detector the required solution
   // is that for type=KVIonRangeTable::kEmax. For a range of energies between punch-through
   // and dE_max, the required solution is still that for type=KVIonRangeTable::kEmin.
   // If the residual energy of the particle is unknown, there is no way to know which is the
   // correct solution.
   //
   // WARNING 2
   // If the given energy loss in the active layer is greater than the maximum theoretical dE
   // for given Z & A, (dE > GetMaxDeltaE(Z,A)) then we return a NEGATIVE incident energy
   // corresponding to the maximum, GetEIncOfMaxDeltaE(Z,A)

   if (Z < 1) return 0.;

   Double_t DE = (delta_e > 0 ? delta_e : GetEnergyLoss());

   // If the given energy loss in the active layer is greater than the maximum theoretical dE
   // for given Z & A, (dE > GetMaxDeltaE(Z,A)) then we return a NEGATIVE incident energy
   // corresponding to the maximum, GetEIncOfMaxDeltaE(Z,A)
   if (DE > GetMaxDeltaE(Z, A)) return -GetEIncOfMaxDeltaE(Z, A);

   TF1* dE = GetELossFunction(Z, A);
   Double_t e1, e2;
   dE->GetRange(e1, e2);
   switch (type) {
      case kEmin:
         e2 = GetEIncOfMaxDeltaE(Z, A);
         break;
      case kEmax:
         e1 = GetEIncOfMaxDeltaE(Z, A);
         break;
   }
   int status;
   Double_t EINC = ProtectedGetX(dE, DE, status, e1, e2);
   if (status != 0) {
//      Warning("GetIncidentEnergy",
//              "In %s : Called for Z=%d A=%d with dE=%f sol_type %d",
//              GetName(), Z, A, DE, type);
//      Warning("GetIncidentEnergy",
//              "Max DeltaE in this case is %f MeV",
//              GetMaxDeltaE(Z, A));
//      Warning("GetIncidentEnergy",
//              "Between Einc limits [%f,%f] no solution found",
//              e1,e2);
//      Warning("GetIncidentEnergy",
//              "Returned value is -Einc for %s dE, %f",
//              (status>0 ? "maximum" : "minimum"), EINC);
      return -EINC;
   }
   return EINC;
}

Double_t KVDetector::GetDeltaEFromERes(Int_t Z, Int_t A, Double_t Eres)
{
   // Overrides KVMaterial::GetDeltaEFromERes
   //
   // Calculate energy loss in active layer of detGetAlignedDetector for nucleus (Z,A)
   // having a residual kinetic energy Eres (MeV)

   if (Z < 1 || Eres <= 0.) return 0.;
   Double_t Einc = GetIncidentEnergyFromERes(Z, A, Eres);
   if (Einc <= 0.) return 0.;
   return GetELossFunction(Z, A)->Eval(Einc);
}

Double_t KVDetector::GetIncidentEnergyFromERes(Int_t Z, Int_t A, Double_t Eres)
{
   // Overrides KVMaterial::GetIncidentEnergyFromERes
   //
   // Calculate incident energy of nucleus from residual energy.
   //
   // Returns -1 if Eres is out of defined range of values

   if (Z < 1 || Eres <= 0.) return 0.;
   //return GetEResFunction(Z, A)->GetX(Eres);
   Int_t status;
   Double_t einc = KVBase::ProtectedGetX(GetEResFunction(Z, A), Eres, status);
   if (status != 0) {
      // problem with inversion - value out of defined range of function
      return -1;
   }
   return einc;
}

Double_t KVDetector::GetSmallestEmaxValid(Int_t Z, Int_t A)
{
   // Returns the smallest maximum energy for which range tables are valid
   // for all absorbers in the detector, and given ion (Z,A)

   Double_t maxmin = -1.;
   TIter next(fAbsorbers);
   KVMaterial* abs;
   while ((abs = (KVMaterial*)next())) {
      if (maxmin < 0.) maxmin = abs->GetEmaxValid(Z, A);
      else {
         if (abs->GetEmaxValid(Z, A) < maxmin) maxmin = abs->GetEmaxValid(Z, A);
      }
   }
   return maxmin;
}

void KVDetector::ReadDefinitionFromFile(const Char_t* envrc)
{
   // Create detector from text file in 'TEnv' format.
   //
   // Example:
   // ========
   //
   // Layer:  Gold
   // Gold.Material:   Au
   // Gold.AreaDensity:   200.*KVUnits::ug
   // +Layer:  Gas1
   // Gas1.Material:   C3F8
   // Gas1.Thickness:   5.*KVUnits::cm
   // Gas1.Pressure:   50.*KVUnits::mbar
   // Gas1.Active:    yes
   // +Layer:  Si1
   // Si1.Material:   Si
   // Si1.Thickness:   300.*KVUnits::um

   TEnv fEnvFile(envrc);

   KVString layers(fEnvFile.GetValue("Layer", ""));
   layers.Begin(" ");
   while (!layers.End()) {
      KVString layer = layers.Next();
      KVString mat = fEnvFile.GetValue(Form("%s.Material", layer.Data()), "");
      KVString tS = fEnvFile.GetValue(Form("%s.Thickness", layer.Data()), "");
      KVString pS = fEnvFile.GetValue(Form("%s.Pressure", layer.Data()), "");
      KVString dS = fEnvFile.GetValue(Form("%s.AreaDensity", layer.Data()), "");
      Double_t thick, dens, press;
      thick = dens = press = 0.;
      KVMaterial* M = 0;
      if (pS != "" && tS != "") {
         press = (Double_t)gROOT->ProcessLineFast(Form("%s*1.e+12", pS.Data()));
         press /= 1.e+12;
         thick = (Double_t)gROOT->ProcessLineFast(Form("%s*1.e+12", tS.Data()));
         thick /= 1.e+12;
         M = new KVMaterial(mat.Data(), thick, press);
      }
      else if (tS != "") {
         thick = (Double_t)gROOT->ProcessLineFast(Form("%s*1.e+12", tS.Data()));
         thick /= 1.e+12;
         M = new KVMaterial(mat.Data(), thick);
      }
      else if (dS != "") {
         dens = (Double_t)gROOT->ProcessLineFast(Form("%s*1.e+12", dS.Data()));
         dens /= 1.e+12;
         M = new KVMaterial(dens, mat.Data());
      }
      if (M) {
         AddAbsorber(M);
         if (fEnvFile.GetValue(Form("%s.Active", layer.Data()), kFALSE)) SetActiveLayer(M);
      }
   }
}

//_________________________________________________________________________________________//

TList* KVDetector::GetAlignedDetectors(UInt_t direction)
{
   // Returns list of detectors (including this one) which are in geometrical aligment
   // with respect to the target position (assuming this detector is part of a multidetector
   // array described by KVMultiDetArray).
   //
   // By default the list is in the order starting from this detector and going towards the target
   // (direction=KVGroup::kBackwards).
   // Call with argument direction=KVGroup::kForwards to have the list of detectors in the order
   // "seen" by a particle flying out from the target and arriving in this detector.
   //
   // If this detector is not part of a KVMultiDetArray (i.e. we have no information on
   // its geometrical relation to other detectors), we return 0x0.
   //
   // The list pointers are stored in member variable fAlignedDetectors[] for rapid retrieval,
   // the lists will be deleted with this detector.
   //
   // See KVGroup::GetAlignedDetectors for more details.

   if (!GetGroup() || direction > 1) return 0x0;
   if (fAlignedDetectors[direction]) return fAlignedDetectors[direction];
   return (fAlignedDetectors[direction] = GetGroup()->GetAlignedDetectors(this, direction));
}

//_________________________________________________________________________________________//

void KVDetector::ResetAlignedDetectors(UInt_t direction)
{
   if (!GetGroup() || direction > 1) return;
   if (fAlignedDetectors[direction]) fAlignedDetectors[direction] = 0;
}

Double_t KVDetector::GetRange(Int_t Z, Int_t A, Double_t Einc)
{
   // WARNING: SAME AS KVDetector::GetLinearRange
   // Only linear range in centimetres is calculated for detectors!
   return GetLinearRange(Z, A, Einc);
}

Double_t KVDetector::GetLinearRange(Int_t Z, Int_t A, Double_t Einc)
{
   // Returns range of ion in centimetres in this detector,
   // taking into account all layers.
   // Note that for Einc > punch through energy, this range is no longer correct
   // (but still > total thickness of detector).
   return GetRangeFunction(Z, A)->Eval(Einc);
}

Double_t KVDetector::GetPunchThroughEnergy(Int_t Z, Int_t A)
{
   // Returns energy (in MeV) necessary for ion (Z,A) to punch through all
   // layers of this detector

   if (fSingleLayer) {
      // Optimize calculation time for single-layer detector
      return fActiveLayer->GetPunchThroughEnergy(Z, A);
   }
   return GetRangeFunction(Z, A)->GetX(GetTotalThicknessInCM());
}


TGraph* KVDetector::DrawPunchThroughEnergyVsZ(Int_t massform)
{
   // Creates and fills a TGraph with the punch through energy in MeV vs. Z for the given detector,
   // for Z=1-92. The mass of each nucleus is calculated according to the given mass formula
   // (see KVNucleus).

   TGraph* punch = new TGraph(92);
   punch->SetName(Form("KVDetpunchthrough_%s_mass%d", GetName(), massform));
   punch->SetTitle(Form("Simple Punch-through %s (MeV) (mass formula %d)", GetName(), massform));
   KVNucleus nuc;
   nuc.SetMassFormula(massform);
   for (int Z = 1; Z <= 92; Z++) {
      nuc.SetZ(Z);
      punch->SetPoint(Z - 1, Z, GetPunchThroughEnergy(nuc.GetZ(), nuc.GetA()));
   }
   return punch;
}

TGraph* KVDetector::DrawPunchThroughEsurAVsZ(Int_t massform)
{
   // Creates and fills a TGraph with the punch through energy in MeV/nucleon vs. Z for the given detector,
   // for Z=1-92. The mass of each nucleus is calculated according to the given mass formula
   // (see KVNucleus).

   TGraph* punch = new TGraph(92);
   punch->SetName(Form("KVDetpunchthroughEsurA_%s_mass%d", GetName(), massform));
   punch->SetTitle(Form("Simple Punch-through %s (AMeV) (mass formula %d)", GetName(), massform));
   KVNucleus nuc;
   nuc.SetMassFormula(massform);
   for (int Z = 1; Z <= 92; Z++) {
      nuc.SetZ(Z);
      punch->SetPoint(Z - 1, Z, GetPunchThroughEnergy(nuc.GetZ(), nuc.GetA()) / nuc.GetA());
   }
   return punch;
}


KVGroup* KVDetector::GetGroup() const
{
   return (KVGroup*)GetParentStructure("GROUP");
}

//_________________________________________________________________________
UInt_t KVDetector::GetGroupNumber()
{
   return (GetGroup() ? GetGroup()->GetNumber() : 0);
}

void KVDetector::AddParentStructure(KVGeoStrucElement* elem)
{
   fParentStrucList.Add(elem);
}

void KVDetector::RemoveParentStructure(KVGeoStrucElement* elem)
{
   fParentStrucList.Remove(elem);
}

KVGeoStrucElement* KVDetector::GetParentStructure(const Char_t* type, const Char_t* name) const
{
   // Get parent geometry structure element of given type.
   // Give unique name of structure if more than one element of same type is possible.
   KVGeoStrucElement* el = 0;
   if (strcmp(name, "")) {
      KVSeqCollection* strucs = fParentStrucList.GetSubListWithType(type);
      el = (KVGeoStrucElement*)strucs->FindObject(name);
      delete strucs;
   }
   else
      el = (KVGeoStrucElement*)fParentStrucList.FindObjectByType(type);
   return el;
}

void KVDetector::SetActiveLayerMatrix(const TGeoHMatrix* m)
{
   // Set ROOT geometry global matrix transformation to coordinate frame of active layer volume
   SetMatrix(m);
}

void KVDetector::SetActiveLayerShape(TGeoBBox* s)
{
   // Set ROOT geometry shape of active layer volume
   SetShape(s);
}

void KVDetector::SetEntranceWindowMatrix(const TGeoHMatrix* m)
{
   // Set ROOT geometry global matrix transformation to coordinate frame of entrance window
   fEWPosition.SetMatrix(m);
}

void KVDetector::SetEntranceWindowShape(TGeoBBox* s)
{
   // Set ROOT geometry shape of entrance window
   fEWPosition.SetShape(s);
}

void KVDetector::SetThickness(Double_t thick)
{
   // Overrides KVMaterial::SetThickness
   //
   // If ROOT geometry is defined, we modify the DZ thickness of the volume representing
   // this detector in accordance with the new thickness.
   //
   // This is only implemented for single-layer detectors with a simple shape.

   if (ROOTGeo() && fSingleLayer) {
      TGeoPhysicalNode* pn = (TGeoPhysicalNode*)gGeoManager->GetListOfPhysicalNodes()->FindObject(GetNode()->GetFullPathToNode());
      if (!pn) pn = gGeoManager->MakePhysicalNode(GetNode()->GetFullPathToNode());
      TGeoBBox* shape = (TGeoBBox*)pn->GetShape();
      TGeoShape* newshape = nullptr;
      // bad kludge - is there no better way to clone a shape and change its dZ?
      if (shape->IsA() == TGeoBBox::Class()) {
         newshape = new TGeoBBox(shape->GetDX(), shape->GetDY(), 0.5 * thick);
      }
      else if (shape->IsA() == TGeoTube::Class()) {
         TGeoTube* oldtube = static_cast<TGeoTube*>(shape);
         newshape = new TGeoTube(oldtube->GetRmin(), oldtube->GetRmax(), 0.5 * thick);
      }
      else {
         Error("SetThickness", "No implementation for %s (%s)", shape->IsA()->GetName(), GetName());
      }
      if (newshape) {
         pn->Align(nullptr, newshape);
         //Info("SetThickness", "Modified ROOT geometry for %s: new thickness=%g cm", GetName(), thick);
         gGeoManager->RefreshPhysicalNodes(kFALSE);
      }
   }
   KVMaterial::SetThickness(thick);
}

Bool_t KVDetector::HasSameStructureAs(const KVDetector* other) const
{
   // Return kTRUE if the two detectors have the same internal structure, i.e.
   //  - the same number of absorber layers
   //  - in the same order
   //  - with the same material & thickness

   int nabs = GetNumberOfAbsorberLayers();
   if (other->GetNumberOfAbsorberLayers() != nabs) return kFALSE;
   bool same = true;
   for (int iabs = 0; iabs < nabs; ++iabs) {
      KVMaterial* this_abs = GetAbsorber(iabs);
      KVMaterial* other_abs = other->GetAbsorber(iabs);
      if (!this_abs->IsType(other_abs->GetType())
            || this_abs->GetMass() != other_abs->GetMass()
            || this_abs->GetThickness() != other_abs->GetThickness())
         same = false;
   }
   return same;
}

Bool_t KVDetector::AddDetectorSignalExpression(const TString& type, const KVString& _expr)
{
   // Add a new KVDetectorSignalExpression to this detector:
   //
   // 'type' will be the name of the new signal.
   //
   // '_expr' is a mathematical expression using any of the known signals of the detector.
   //
   // If the expression is not valid, no signal will be created and method returns kFALSE.

   KVDetectorSignalExpression* ds = new KVDetectorSignalExpression(type, _expr, this);
   if (!ds->IsValid()) {
      delete ds;
      ds = nullptr;
   }
   else
      AddDetectorSignal(ds);
   return (ds != nullptr);
}
