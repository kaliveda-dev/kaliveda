#include "KVTarget.h"
#include "KVTemplateEvent.h"
#include "KVUnits.h"
#include "TMath.h"
#include "Riostream.h"

using namespace std;

ClassImp(KVTarget)

void KVTarget::init()
{
   //Default initialisations
   SetRandomized(kFALSE);
   SetIncoming(kFALSE);
   SetOutgoing(kFALSE);
   fTargets = new KVList;
   SetName("KVTarget");
   SetTitle("Target for experiment");
   fNLayers = 0;

   fNormal.SetXYZ(0, 0, 1);
   fIntPoint.SetXYZ(0, 0, 0);
}

KVTarget::KVTarget()
{
   //Default costructor
   init();
}

//________________________________________________________________________

KVTarget::KVTarget(const Char_t* material, const Double_t thick)
{
   // Just give the type & "thickness" of material for target
   // The "thickness" is the area density of the target in mg/cm**2.

   init();
   AddLayer(material, thick);
}

//________________________________________________________________________
//
KVTarget::KVTarget(const KVTarget& obj) : KVMaterial()
{
   //Copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVTarget&) obj).Copy(*this);
#endif
}

//________________________________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTarget::Copy(TObject& obj) const
#else
void KVTarget::Copy(TObject& obj)
#endif
{
   //Copy this to obj
   ((KVTarget&)obj).fTargets->Clear();// delete any existing layers
   ((KVTarget&) obj).fNLayers = 0;
   KVMaterial::Copy(obj);
   ((KVTarget&) obj).SetRandomized(IsRandomized());
   ((KVTarget&) obj).SetIncoming(IsIncoming());
   ((KVTarget&) obj).SetOutgoing(IsOutgoing());

   ((KVTarget&) obj).fNormal = fNormal;

   TIter next(GetLayers());
   KVMaterial* mat;
   while ((mat = (KVMaterial*) next())) {
      ((KVTarget&) obj).AddLayer(mat->GetName(), mat->GetAreaDensity() / KVUnits::mg);
   }
}
//________________________________________________________________________

void KVTarget::AddLayer(const Char_t* material, Double_t thick)
{
   // Add a layer to a target, with 'thickness' in mg/cm**2 (area density).
   // Sets/updates name of target with name of material.
   // In case of multi-layer target the name is
   //      material1/material2/material3/...

   fTargets->Add(new KVMaterial(thick * KVUnits::mg, material));
   fNLayers++;
   if (fNLayers == 1) {
      SetName(material);
   }
   else {
      TString _name(GetName());
      _name += "/";
      _name += material;
      SetName(_name.Data());
   }
}

//________________________________________________________________________

Double_t KVTarget::GetTotalThickness()
{
   // return sum of 'thicknesses' (area densities in mg/cm**2)
   // of all layers in target

   Float_t thick = 0.;
   TIter next(fTargets);
   KVMaterial* mat = 0;
   while ((mat = (KVMaterial*) next())) {
      thick += mat->GetAreaDensity();
   }
   return thick / KVUnits::mg;
}

//________________________________________________________________________

Double_t KVTarget::GetTotalThickness(Int_t lay1, Int_t lay2)
{
   //return sum of 'thicknesses' (area densities in mg/cm**2)
   // of layers lay1 to lay2 in target

   Double_t thick = 0.;
   for (int i = lay1; i <= lay2; i++) {
      thick += GetThickness(i);
   }
   return thick;
}

//________________________________________________________________________

void KVTarget::SetAngleToBeam(Double_t a)
{
   //Sets angle of target to incident beam direction by rotating about the +x axis (12 o'clock)
   //Angle 'a' is given in degrees.

   fNormal.SetXYZ(0, 0, 1);
   fNormal.RotateX(a * TMath::DegToRad());
}

//________________________________________________________________________

Double_t KVTarget::GetAngleToBeam()
{
   //Gives angle of target to incident beam direction in degrees

   return TMath::RadToDeg() * fNormal.Angle(TVector3(0, 0, 1));
}

//________________________________________________________________________

Double_t KVTarget::GetEffectiveThickness(KVParticle* part, Int_t ilayer)
{
   //Return effective 'thickness' (in mg/cm**2) of layer ilayer (ilayer=1, 2, ...)
   //By default ilayer=1 (i.e. for single layer target)
   //The effective thickness depends on the angle of the target (rotation about
   //x-axis => theta wrt z- (beam)-axis).
   //It also depends on the direction of motion of the incident particle.
   //If no particle is given, effective thicknesses are calculated as for
   //particles travelling in the beam direction.

   //get (or make) vector in particle direction of motion (z-direction if no particle)
   //Info("KVTarget::GetEffectiveThickness","(KVParticle * part, Int_t ilayer)");

   TVector3 p;
   if (part)
      p = part->GetMomentum();
   else
      p = TVector3(0, 0, 1);

   return GetEffectiveThickness(p, ilayer);
}

//________________________________________________________________________

Double_t KVTarget::GetEffectiveThickness(TVector3& direction,
      Int_t ilayer)
{
   //Return effective 'thickness' (in mg/cm**2) of layer ilayer (ilayer=1, 2, ...)
   //By default ilayer=1 (i.e. for single layer target)
   //The effective thickness depends on the orientation of the target (given by
   //the direction of the normal to its surface) and on the direction (e.g. direction of a particle)
   // Info("KVTarget::GetEffectiveThickness","TVector3 & direction,Int_t ilayer");
   if (ilayer < 1 || ilayer > NumberOfLayers()) {
      Error("GetEffectiveThickness(Int_t ilayer, TVector3& direction)",
            "Layer number %d is illegal. Valid values are between 1 and %d.",
            ilayer, NumberOfLayers());
      return 0.0;
   }
   return GetLayerByIndex(ilayer)->GetEffectiveAreaDensity(fNormal, direction) / KVUnits::mg;
}

//________________________________________________________________________

KVMaterial* KVTarget::GetLayer(TVector3& depth)
{
   //Returns absorber corresponding to 'depth' inside target, starting from the 'entrance'
   //layer and following the direction of 'depth'. Note: 'depth' is measured in the same
   //'thickness' units as the thickness of the different layers of the target (mg/cm2)
   //WARNING : returns 0 if no layer is found (depth is outside of target)

   return GetLayerByIndex(GetLayerIndex(depth));
}

//________________________________________________________________________

Int_t KVTarget::GetLayerIndex(TVector3& depth)
{
   //Returns absorber index corresponding to 'depth' inside target, starting from the 'entrance'
   //layer and following the direction of 'depth'. Note: 'depth' is measured in the same
   //'thickness' units as the thickness of the different layers of the target (mg/cm2)
   //WARNING : user should check returned index is >0
   //If not, this means that the given depth does not correspond to a layer inside the target

   Double_t thick = 0.0;
   Double_t D = depth.Mag();
   Int_t i = 0;

   while (thick < D && i < NumberOfLayers()) {
      //increase total thickness by effective thickness 'seen' in direction of 'depth' vector
      thick += GetEffectiveThickness(depth, ++i);
   }

   return (thick < D ? 0 : i);
}

//________________________________________________________________________

KVMaterial* KVTarget::GetLayerByDepth(Double_t depth)
{
   //Returns absorber corresponding to 'depth' inside target, starting from the 'entrance'
   //layer and following the normal direction. Note: 'depth' is measured in the same
   //'thickness' units as the thickness of the different layers of the target (mg/cm2, um, etc.)
   //WARNING : returns 0 if no layer is found (depth is outside of target)

   return GetLayerByIndex(GetLayerIndex(depth));
}

//________________________________________________________________________

Int_t KVTarget::GetLayerIndex(const Char_t* name)
{
   //Returns layer index corresponding to absorber of type 'name'.
   //WARNING : user should check returned index is >0
   //If not, this means that the given material name does not correspond to a layer inside the target
   KVMaterial* mat = GetLayer(name);
   return (mat ? GetLayers()->IndexOf(mat) + 1 : 0);
}

//________________________________________________________________________

KVMaterial* KVTarget::GetLayer(const Char_t* name)
{
   //Returns layer corresponding to absorber of type 'name'.
   return (KVMaterial*) GetLayers()->FindObjectByType(name);
}

//________________________________________________________________________

Double_t KVTarget::GetThickness(Int_t ilayer) const
{
   //'Thickness' in mg/cm**2 of layer 'ilayer' in target
   KVMaterial* lay = GetLayerByIndex(ilayer);
   return (lay ? lay->GetAreaDensity() / KVUnits::mg : 0.0);
}

//________________________________________________________________________

Int_t KVTarget::GetLayerIndex(Double_t depth)
{
   //Returns absorber index corresponding to 'depth' inside target, starting from the 'entrance'
   //layer and following the normal direction. Note: 'depth' is measured in the same
   //'thickness' units as the thickness of the different layers of the target (mg/cm2)
   //WARNING : user should check returned index is >0
   //If not, this means that the given depth does not correspond to a layer inside the target

   Double_t thick = 0.0;
   Int_t i = 0;

   while (thick < depth && i < NumberOfLayers()) {
      //increase total thickness by thickness of layer
      thick += GetThickness(++i);
   }

   return (thick < depth ? 0 : i);
}

//________________________________________________________________________

Double_t KVTarget::GetTotalEffectiveThickness(KVParticle* part)
{
   //return sum of effective 'thicknesses' (mg/cm**2) of all layers in target
   //taking into account the angle of the target to the beam
   //and the direction of motion of the incident particle.
   //If no particle is given, effective thicknesses are calculated as for
   //particles travelling in the beam direction.

   TVector3 p = (part ? part->GetMomentum() : TVector3(0, 0, 1));
   return GetTotalEffectiveThickness(p);
}

//________________________________________________________________________

Double_t KVTarget::GetTotalEffectiveThickness(TVector3& dir, Int_t ilay1,
      Int_t ilay2)
{
   //return sum of effective 'thicknesses' (mg/cm**2) of layers ilay1 to ilay2 in target
   //taking into account the angle of the target to the beam
   //and the given direction.
   //
   //GetTotalEffectiveThickness(dir) --> thickness of all layers
   //GetTotalEffectiveThickness(dir,ilay1) --> thickness of layers ilay1 to last
   //GetTotalEffectiveThickness(dir,ilay1,ilay2) --> thickness of layers ilay1 to ilay2

   Double_t thick = 0.0;
   ilay2 =
      (ilay2
       ? (ilay2 <=
          NumberOfLayers() ? (ilay2 >=
                              ilay1 ? ilay2 : ilay1) : NumberOfLayers()) :
       NumberOfLayers());
   for (Int_t i = ilay1; i <= ilay2; i++) {
      thick += GetEffectiveThickness(dir, i);
   }
   return thick;
}

//________________________________________________________________________

KVTarget::~KVTarget()
{
   delete fTargets;
   fTargets = 0;
}

//________________________________________________________________________

void KVTarget::DetectParticle(KVNucleus* kvp, TVector3*)
{
   //Simulate passage of a particle through a target.
   //Energy losses are calculated and the particle is slowed down.
   //We take into account the direction of motion of the particle and an arbitrary orientation of the target.
   //
   //The' 'TVector3* dummy'argument is not used.
   //
   //If IsIncoming()=kFALSE & IsOutgoing()=kFALSE, the particle will pass through the whole of the target.
   //If IsIncoming()=kTRUE, calculate energy loss up to interaction point
   //If IsOutgoing()=kTRUE, calculate energy loss from interaction point onwards (outwards)

   if (kvp->GetKE() <= 0.)
      return;

   if (!IsIncoming() && !IsOutgoing()) {
      //calculate losses in all layers
      for (int i = 1;
            i <= NumberOfLayers() && kvp->GetKE() > 0.;
            i++) {
         GetLayerByIndex(i)->DetectParticle(kvp, &fNormal);
      }
   }
   else {

      //find starting or ending layer (where is I.P. ?)
      Int_t iplay_index = GetLayerIndex(GetInteractionPoint());
#ifdef DBG_TRGT
      cout << "IP is in layer " << iplay_index << endl;
#endif
      //particle going forwards or backwards ?
      TVector3 p = kvp->GetMomentum();
      Bool_t backwards = (p * GetNormal() < 0.0);
#ifdef DBG_TRGT
      cout << "Particle is going ";
      backwards ? cout << "backwards" : cout << "forwards";
      cout << endl;
#endif

      if (iplay_index) {

         KVMaterial* iplay = GetLayerByIndex(iplay_index);

         //effective thickness of I.P. layer is reduced because we start/stop from inside it
         //the effective thickness (measured along the normal) after the I.P. is given by the
         //sum of all thicknesses up to and including this layer minus the scalar product of
         //'depth' with the normal (i.e. the projection of 'depth' along the normal)

         Double_t e_thick_iplay = GetTotalThickness(1,
                                  iplay_index) -
                                  GetInteractionPoint() * GetNormal();
         e_thick_iplay =
            (IsIncoming() ? iplay->GetAreaDensity() / KVUnits::mg -
             e_thick_iplay : e_thick_iplay);

         if (backwards)
            e_thick_iplay = iplay->GetAreaDensity() / KVUnits::mg - e_thick_iplay;
#ifdef DBG_TRGT
         cout << "Effective thickness of IP layer is " << e_thick_iplay <<
              " (real:" << iplay->GetAreaDensity() / KVUnits::mg << ")" << endl;
#endif
         //modify effective physical thickness of layer
         Double_t thick_iplay = iplay->GetAreaDensity();// in g/cm**2
         iplay->SetAreaDensity(e_thick_iplay * KVUnits::mg);

         //first and last indices of layers to pass through
         Int_t ilay1 =
            (IsIncoming() ? (backwards ? NumberOfLayers() : 1) :
             iplay_index);
         Int_t ilay2 =
            (IsIncoming() ? iplay_index
             : (backwards ? 1 : NumberOfLayers()));

         if (backwards) {
            for (int i = ilay1;
                  i >= ilay2 && kvp->GetKE() > 0.; i--)
               GetLayerByIndex(i)->DetectParticle(kvp, &fNormal);
         }
         else {
            for (int i = ilay1;
                  i <= ilay2 && kvp->GetKE() > 0.; i++)
               GetLayerByIndex(i)->DetectParticle(kvp, &fNormal);
         }

         //reset original thickness of IP layer
         iplay->SetAreaDensity(thick_iplay);

      }
      else {
         Error("DetectParticle", "Interaction point is outside of target");
      }
   }
}

//________________________________________________________________

Double_t KVTarget::GetELostByParticle(KVNucleus* kvp, TVector3*)
{
   //Simulate passage of a particle through a target.
   //Energy losses are calculated but the particle's energy is not modified.
   //We take into account the direction of motion of the particle and an arbitrary
   //orientation of the target.
   //
   //The' 'TVector3* dummy'argument is not used.
   //
   //If IsIncoming()=kFALSE & IsOutgoing()=kFALSE, the particle will pass through the whole of the target.
   //If IsIncoming()=kTRUE, calculate energy loss up to interaction point
   //If IsOutgoing()=kTRUE, calculate energy loss from interaction point onwards (outwards)

   Double_t Eloss = 0.0, E0 = kvp->GetKE();

   if (E0 <= 0.)
      return E0;

   //make 'clone' of nucleus to simulate energy losses
   KVNucleus* clone_part = new KVNucleus(kvp->GetZ(), kvp->GetA());
   clone_part->SetMomentum(kvp->GetMomentum());

   if (!IsIncoming() && !IsOutgoing()) {
      //calculate losses in all layers
      for (int i = 1;
            i <= NumberOfLayers() && clone_part->GetKE() > 0.;
            i++) {
         Eloss +=
            GetLayerByIndex(i)->GetELostByParticle(clone_part, &fNormal);
         clone_part->SetKE(E0 - Eloss);
      }
   }
   else {

      //find starting or ending layer (where is I.P. ?)
      Int_t iplay_index = GetLayerIndex(GetInteractionPoint());
#ifdef DBG_TRGT
      cout << "IP is in layer " << iplay_index << endl;
#endif
      //particle going forwards or backwards ?
      TVector3 p = clone_part->GetMomentum();
      Bool_t backwards = (p * GetNormal() < 0.0);
#ifdef DBG_TRGT
      cout << "Particle is going ";
      backwards ? cout << "backwards" : cout << "forwards";
      cout << endl;
#endif

      if (iplay_index) {

         KVMaterial* iplay = GetLayerByIndex(iplay_index);

         //effective thickness of I.P. layer is reduced because we start/stop from inside it
         //the effective thickness (measured along the normal) after the I.P. is given by the
         //sum of all thicknesses up to and including this layer minus the scalar product of
         //'depth' with the normal (i.e. the projection of 'depth' along the normal)

         Double_t e_thick_iplay = GetTotalThickness(1,
                                  iplay_index) -
                                  GetInteractionPoint() * GetNormal();
         e_thick_iplay =
            (IsIncoming() ? iplay->GetAreaDensity() / KVUnits::mg -
             e_thick_iplay : e_thick_iplay);

         if (backwards)
            e_thick_iplay = iplay->GetAreaDensity() / KVUnits::mg - e_thick_iplay;
#ifdef DBG_TRGT
         cout << "Effective thickness of IP layer is " << e_thick_iplay <<
              " (real:" << iplay->GetAreaDensity() / KVUnits::mg << ")" << endl;
#endif
         //modify effective physical thickness of layer
         Double_t thick_iplay = iplay->GetAreaDensity(); // g/cm**2
         iplay->SetAreaDensity(e_thick_iplay * KVUnits::mg);

         //first and last indices of layers to pass through
         Int_t ilay1 =
            (IsIncoming() ? (backwards ? NumberOfLayers() : 1) :
             iplay_index);
         Int_t ilay2 =
            (IsIncoming() ? iplay_index
             : (backwards ? 1 : NumberOfLayers()));

         if (backwards) {
            for (int i = ilay1;
                  i >= ilay2 && clone_part->GetKE() > 0.; i--) {
               Eloss +=
                  GetLayerByIndex(i)->GetELostByParticle(clone_part,
                        &fNormal);
               clone_part->SetKE(E0 - Eloss);
            }
         }
         else {
            for (int i = ilay1;
                  i <= ilay2 && clone_part->GetKE() > 0.; i++) {
               Eloss +=
                  GetLayerByIndex(i)->GetELostByParticle(clone_part,
                        &fNormal);
               clone_part->SetKE(E0 - Eloss);
            }
         }

         //reset original thickness of IP layer
         iplay->SetAreaDensity(thick_iplay);

      }
      else {
         Error("DetectParticle", "Interaction point is outside of target");
      }
   }

   //delete clone
   delete clone_part;

   return Eloss;
}

//_______________________________________________________________________________________

void KVTarget::DetectEvent(KVEvent* event)
{
   //Simulate passage of particles from some simulation through the target material.
   //The particles will be slowed down according to their calculated energy losses.
   //First we SetOutgoing(): for a simulated event, energy losses are only calculated from some
   //interaction point inside the target to the outside. This interaction point will be taken half-way
   //through the target (by default) or at some random depth in the target if SetRandomized() is
   //called first.

   SetOutgoing();
   KVNucleus* part;
   while ((part = (KVNucleus*) event->GetNextParticle())) {     // loop over particles
      DetectParticle(part, &GetInteractionPoint());
   }
}

//_________________________________________________________________________________________

void KVTarget::Print(Option_t* opt) const
{
   cout << "Target " << GetName() << ", " << GetTitle() << endl;
   fTargets->Print(opt);
}

//________________________________________________________________________

void KVTarget::Clear(Option_t* opt)
{
   KVMaterial::Clear(opt);
   fTargets->Execute("Clear", Form("%s", opt));
}

//______________________________________________________________________________________________________

TVector3& KVTarget::GetInteractionPoint(KVParticle* part)
{
   //Returns last known interaction point (if part=0) or generates a new one if part!=0.
   //
   //if IsRandomized()=kTRUE the generated interaction point is at a random distance along the
   //direction 'of the incident particle's trajectory through target.
   //if IsRandomized()=kFALSE the generated interaction point is half way along the direction.
   //
   //If no interaction point is set by the user (i.e. GetInteractionPoint never called with
   //the address of a KVParticle), the default is to generate an interaction point using the
   //beam (+ve Z-)direction.

   TVector3 dir;
   if (!part) {
      //check fIntPoint is valid
      if (fIntPoint.Mag() > 0)
         return fIntPoint;
      //set default direction - beam direction
      dir.SetXYZ(0, 0, 1);
   }
   else {
      dir = part->GetMomentum();
   }
   Double_t e_eff = GetTotalEffectiveThickness(dir);
   Double_t depth;
   depth = (IsRandomized() ? gRandom->Uniform(e_eff) : 0.5 * e_eff);
   fIntPoint = depth * (dir.Unit());
   return fIntPoint;
}

//______________________________________________________________________________________________________

void KVTarget::SetInteractionLayer(Int_t ilayer, TVector3& dir)
{
   //Sets the interaction point inside the layer with index 'ilayer'
   //
   //if IsRandomized()=kTRUE the generated interaction point is at a random distance along the
   //direction 'dir' inside layer (e.g. incident particle's trajectory through layer).
   //if IsRandomized()=kFALSE the generated interaction point is half way along the direction 'dir'
   //inside the layer

   //total effective thickness (along 'dir') of all layers before 'ilayer'
   Double_t e_eff =
      (ilayer > 1 ? GetTotalEffectiveThickness(dir, 1, ilayer - 1) : 0.0);
#ifdef DBG_TRGT
   cout <<
        "total effective thickness (along 'dir') of all layers before 'ilayer'="
        << e_eff << endl;
#endif
   //effective depth inside layer (along 'dir')
   Double_t e_eff_ilayer = GetEffectiveThickness(dir, ilayer);
   Double_t depth =
      (IsRandomized() ? gRandom->Uniform(e_eff_ilayer) : 0.5 *
       e_eff_ilayer);
#ifdef DBG_TRGT
   cout << "dpeth inside interaction layer=" << depth << endl;
#endif
   fIntPoint = (e_eff + depth) * (dir.Unit());
#ifdef DBG_TRGT
   cout << "generated IP vector:" << endl;
   fIntPoint.Print();
#endif
}

//______________________________________________________________________________________________________

void KVTarget::SetInteractionLayer(const Char_t* name, TVector3& dir)
{
   //Sets the interaction point inside the layer made of absorber type 'name'
   //
   //if IsRandomized()=kTRUE the generated interaction point is at a random distance along the
   //direction 'dir' inside layer (e.g. incident particle's trajectory through layer).
   //if IsRandomized()=kFALSE the generated interaction point is half way along the direction 'dir'
   //inside the layer

   Int_t ilayer = GetLayerIndex(name);
   if (!ilayer) {
      Error("SetInteractionLayer",
            "No layer in target with material type %s", name);
   }
   SetInteractionLayer(ilayer, dir);
}

//______________________________________________________________________________________________________

void KVTarget::SetInteractionLayer(const Char_t* name, KVParticle* part)
{
   //Sets the interaction point inside the layer made of absorber type 'name'
   //
   //if IsRandomized()=kTRUE the generated interaction point is at a random distance along the
   //direction along the incident particle's trajectory through layer.
   //if IsRandomized()=kFALSE the generated interaction point is half way along the direction
   //inside the layer

   TVector3 dir = part->GetMomentum();
   SetInteractionLayer(name, dir);
}

//______________________________________________________________________________________________________

void KVTarget::SetMaterial(const Char_t* type)
{
   // Set material of first layer
   if (GetLayerByIndex(1))
      GetLayerByIndex(1)->SetMaterial(type);
}

//______________________________________________________________________________________________________

void KVTarget::SetLayerThickness(Float_t thick, Int_t ilayer)
{
   // Set 'thickness' in mg/cm**2 of a layer, by default this is the first layer
   if (GetLayerByIndex(ilayer))
      GetLayerByIndex(ilayer)->SetAreaDensity(thick * KVUnits::mg);
}

//______________________________________________________________________________________________________

Double_t KVTarget::GetAtomsPerCM2() const
{
   //Calculates total number of atoms per square centimetre of the target.
   //For a multilayer target, the area densities for each layer are summed up.
   Double_t atom_cib = 0;
   for (int i = 1; i <= NumberOfLayers(); i++) {
      //N_atoms = N_Avogadro * target_thickness (mg/cm**2) * 1.e-3 / atomic_mass_of_target
      atom_cib +=
         TMath::Na() * GetThickness(i) * 1.e-3 /
         GetLayerByIndex(i)->GetMass();
   }
   return atom_cib;
}

//________________________________________________________________

Double_t KVTarget::GetParticleEIncFromERes(KVNucleus* kvp, TVector3*)
{
   // Calculate initial energy of particle from its current (residual) energy, assumed
   // to correspond to the state of the particle after passage through all or some part
   // of the target, taking into account the particle's direction of motion and an arbitrary
   // orientation of the target.
   //
   // The 'TVector3*' argument is not used.
   //
   // If IsIncoming()=kFALSE & IsOutgoing()=kFALSE, we assume the particle passed through the whole of the target.
   // If IsIncoming()=kTRUE, assume current energy is energy on reaching interaction point;
   //       we calculate energy of particle before entering target
   // If IsOutgoing()=kTRUE, assume current energy is energy on exiting from target;
   //       we calculate energy of particle at interactio point

   Double_t ERes = 0.0;

   //make 'clone' of nucleus to simulate energy losses
   KVNucleus* clone_part = new KVNucleus(kvp->GetZ(), kvp->GetA());
   clone_part->SetMomentum(kvp->GetMomentum());

   if (!IsIncoming() && !IsOutgoing()) {

      //correct for losses in all layers
      for (int i = NumberOfLayers(); i > 0; i--) {
         ERes = GetLayerByIndex(i)->GetParticleEIncFromERes(clone_part, &fNormal);
         clone_part->SetKE(ERes);
      }
      delete clone_part;
      return ERes;

   }
   else {

      //find starting or ending layer (where is I.P. ?)
      Int_t iplay_index = GetLayerIndex(GetInteractionPoint());
      //particle going forwards or backwards ?
      TVector3 p = clone_part->GetMomentum();
      Bool_t backwards = (p * GetNormal() < 0.0);

      if (iplay_index) {

         KVMaterial* iplay = GetLayerByIndex(iplay_index);

         //effective thickness of I.P. layer is reduced because we start/stop from inside it
         //the effective thickness (measured along the normal) after the I.P. is given by the
         //sum of all thicknesses up to and including this layer minus the scalar product of
         //'depth' with the normal (i.e. the projection of 'depth' along the normal)

         Double_t e_thick_iplay = GetTotalThickness(1,
                                  iplay_index) -
                                  GetInteractionPoint() * GetNormal();
         e_thick_iplay =
            (IsIncoming() ? iplay->GetAreaDensity() / KVUnits::mg -
             e_thick_iplay : e_thick_iplay);

         if (backwards)
            e_thick_iplay = iplay->GetAreaDensity() / KVUnits::mg - e_thick_iplay;
         //modify effective physical thickness of layer
         Double_t thick_iplay = iplay->GetAreaDensity();
         iplay->SetAreaDensity(e_thick_iplay * KVUnits::mg);

         //first and last indices of layers to pass through
         Int_t ilay1 =
            (IsIncoming() ? (backwards ? NumberOfLayers() : 1) :
             iplay_index);
         Int_t ilay2 =
            (IsIncoming() ? iplay_index
             : (backwards ? 1 : NumberOfLayers()));

         //correct for losses in different layers
         if (backwards) {

            for (int i = ilay2;
                  i <= ilay1; i++) {
               ERes =
                  GetLayerByIndex(i)->GetParticleEIncFromERes(clone_part,
                        &fNormal);
               clone_part->SetKE(ERes);
            }

         }
         else {

            for (int i = ilay2;
                  i >= ilay1 ; i--) {
               ERes =
                  GetLayerByIndex(i)->GetParticleEIncFromERes(clone_part,
                        &fNormal);
               clone_part->SetKE(ERes);
            }

         }

         //reset original thickness of IP layer
         iplay->SetAreaDensity(thick_iplay);

      }
      else {
         Error("GetParticleEIncFromERes", "Interaction point is outside of target");
      }
   }

   //delete clone
   delete clone_part;

   return ERes;
}

//__________________________________________________________________________

Double_t KVTarget::GetIncidentEnergyFromERes(Int_t Z, Int_t A, Double_t Eres)
{
   // Calculate initial energy of nucleus (Z,A) from given residual energy Eres, assumed
   // to correspond to the state of the particle after passage through all or some part
   // of the target, taking into account an arbitrary orientation of the target.
   //
   //         *** WARNING ***
   //      Obviously we cannot know the particle's direction of motion,
   //      therefore we assume it to be travelling in the beam direction (0,0,1)
   //       Normally you should use GetParticleEIncFromERes
   //
   // The' 'TVector3*' argument is not used.
   //
   // If IsIncoming()=kFALSE & IsOutgoing()=kFALSE, we assume the particle passed through the whole of the target.
   // If IsIncoming()=kTRUE, assume current energy is energy on reaching interaction point;
   //       we calculate energy of particle before entering target
   // If IsOutgoing()=kTRUE, assume current energy is energy on exiting from target;
   //       we calculate energy of particle at interactio point

   //Fake nucleus
   KVNucleus dummy(Z, A);
   dummy.SetKE(Eres);
   return GetParticleEIncFromERes(&dummy);
}

