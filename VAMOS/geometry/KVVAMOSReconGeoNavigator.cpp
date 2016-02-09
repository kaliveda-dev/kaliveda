//Created by KVClassFactory on Mon Jul  8 15:40:35 2013
//Author: Guilain ADEMARD

#include "KVVAMOSReconGeoNavigator.h"
#include "KVVAMOSReconNuc.h"
#include "KVVAMOSDetector.h"
#include "KVIonRangeTableMaterial.h"
#include "KVTarget.h"

ClassImp(KVVAMOSReconGeoNavigator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSReconGeoNavigator</h2>
<h4>Propagate nuclei through the geometry of VAMOS for their reconstruction/calibrate/identification</h4>
<!-- */
// --> END_HTML
// Given a valid ROOT geometry, we propagate the nuclei in the VAMOS focal-plane
// detection area and, every time a nucleus traverses a volume made of a TGeoMaterial
// with a name corresponding to a material known by this range table,
// we calculate the step through the material (STEP) and the energy losse of the nucleus,
// and store it in the nucleus's list KVParticle::fParameters in the form
//
//   "STEP:[detector name]" = [step in material in cm]
//   "DE:[detector name]"   = [energy losse in material in MeV]
//
// In the case of multilayer detectors, "[detector name]" in the previous
// examples is replaced by "[detector name]/[layer name]"
//
//
// For the calibration of the times of flight, we also store the distance (DPATH in cm) between the intersection point at the focal plane
// and the point at the entrance of the volume if it is the first active volume of
// a detector
//
//   "DPATH:[detector name]" = [distance FP point to active volume entrance point]
//
// DPATH has the sign + if the volume is behind the focal plane or - is it
// is at the front of it.
//
// The time of flight between the center of the target and the point at the entrance of the first active volume of each detector
//
//   "TOF:[detector name]" = [time of flight from target to active volume in cm/ns]
//
// This class is used for the reconstruction in KVVAMOSReconNuc. It can be used
// also for the calibration (time and energy) of detectors.
// The propagation of the nucleus is first done from the focal-plane point
// toward the inverse direction of the reconstructed trajectory (KVVAMOSReconNuc::fRT.dirFP direction) to found the starting point.
// Then a second propagation is done from this starting point, in the in the inverse direction
// to perform the calculations.
// If the trajectory is not correctly reconstructed then a central trajectory
// is used (i.e. Theta = Phi = ThetaF = PhiF = 0 degree and Xf = Yf = 0 cm ).
// In this case a warning will be displayed.
//
// DPATH is used to calculate the correct flight distance of the nucleus.
// For example if the time TSED1_HF is used for a time-of-flight
// measurement then the real flight distance is the reconstructed path, from
// the target point to the focal plane, plus the value of DPATH:SED1/ACTIVE_SED2_Myl
//
//
// EXAMPLE
// =======
//
// In this case the VAMOS geometry is made up of 2 SeDs, 1 HARPEE ionization chamber and 1 silicon detector.
// The ionization chamber has 4 layers (window + gas + gas + gas), with an "ACTIVE" gas layer
//
// For a well reconstructed nucleus in VAMOS, this navigator is used and then
// you can access to the result of the propagation with:
//
// nuc->GetParameters()->Print()
//
// KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0xa163800)
//  <STEP:SED1/ACTIVE_SED1_Myl=0.000126196>
//  <DPATH:SED1/ACTIVE_SED1_Myl=-71.6663>
//  <STEP:SED2/ACTIVE_SED2_Myl=0.000126196>
//  <DPATH:SED2/ACTIVE_SED2_Myl=16.0801>
//  <STEP:CHI/CHI_Myl=0.000150153>
//  <STEP:CHI/CHI_C4H10=2.07011>
//  <STEP:CHI/ACTIVE_CHI_C4H10=10.4677>
//  <DPATH:CHI/ACTIVE_CHI_C4H10=56.8245>
//  <STEP:CHI/CHI_C4H10_1=1.27931>
//  <STEP:SI_17=0.0500511>
//  <DPATH:SI_17=68.575>
//
// here 'nuc' is a KVVAMOSReconNuc object.
////////////////////////////////////////////////////////////////////////////////

//________________________________________________________________

KVVAMOSReconGeoNavigator::KVVAMOSReconGeoNavigator(TGeoManager* g, KVIonRangeTable* r) : KVGeoNavigator(g)
{
   fRangeTable = r;
   fDoNothing  = kFALSE;
   fCalib      = kNoCalib;
   fE          = 0.;
}
//________________________________________________________________

KVVAMOSReconGeoNavigator::~KVVAMOSReconGeoNavigator()
{
   // Destructor
}
//________________________________________________________________

void KVVAMOSReconGeoNavigator::ParticleEntersNewVolume(KVNucleus* nuc)
{
   // Overrides method in KVGeoNavigator base class.
   // Every time a particle enters a new volume, we check the material to see
   // if it is known (i.e. contained in the range table fRangeTable).
   // If so, then we calculate the step through the material (STEP) of the nucleus
   // and the distance (DPATH in cm) between the intersection point at the focal plane
   // and the point at the entrance of the volume if it is the first active volume of a detector.
   // DPATH has the sign + if the volume is behind the focal plane or - if it
   // is at the front of it.
   //

   KVVAMOSReconNuc* rnuc = (KVVAMOSReconNuc*)nuc;

   // stop the propagation if the current volume is the stopping detector
   // of the nucleus but after the process of this volume
   if (rnuc->GetStoppingDetector()) {
      TGeoVolume* stopVol = (TGeoVolume*)((KVVAMOSDetector*)rnuc->GetStoppingDetector())->GetActiveVolumes()->Last();

      if (GetCurrentVolume() == stopVol) SetStopPropagation();
   }

   if (fDoNothing) return;


   TGeoMaterial* material = GetCurrentVolume()->GetMaterial();
   KVIonRangeTableMaterial* irmat = 0;

   // skip the process if the current material is unkown
   if ((irmat = fRangeTable->GetMaterial(material))) {

      KVString dname;
      Bool_t multi;
      TString absorber_name;
      Bool_t is_active = kFALSE;
      if (GetCurrentDetectorNameAndVolume(dname, multi)) {
         is_active = kTRUE;
         if (multi) {
            absorber_name.Form("%s/%s", dname.Data(), GetCurrentNode()->GetName());
            is_active = absorber_name.Contains("ACTIVE_");
         } else absorber_name = dname;
      } else
         absorber_name = irmat->GetName();

      // Coordinates of the vector between the intersection point at the
      // focal plane and the point at the entrance of the current detector
      Double_t X = GetEntryPoint().X() - fOrigine.X();
      Double_t Y = GetEntryPoint().Y() - fOrigine.Y();
      Double_t Z = GetEntryPoint().Z() - fOrigine.Z();

      // Norm of this vector. The signe gives an infomation about the detector position
      // (1: behind; -1: in front of) with respect to the focal plane.
      Double_t Delta = TMath::Sign(1., Z) * TMath::Sqrt(X * X + Y * Y + Z * Z);

      if ((fCalib & kECalib) || (fCalib & kTCalib)) {
         if (fE > 1e-3) {

            // velocity before material
            Double_t Vi = nuc->GetVelocity().Mag();

            // energy lost in the material
            Double_t DE = irmat->GetLinearDeltaEOfIon(
                             nuc->GetZ(), nuc->GetA(), fE, GetStepSize(), 0.,
                             material->GetTemperature(),
                             material->GetPressure());
            fE -= DE;
            nuc->SetEnergy(fE);

            //set flag to say that particle has been slowed down
            nuc->SetIsDetected();

            // velocity after material
            Double_t Vf = nuc->GetVelocity().Mag();

            if (fCalib & kTCalib) {
               //from current start point to the entrance point
               fTOF += (Delta - fStartPath) / Vi;
               fStartPath = Delta;

               //nuc->GetParameters()->SetValue(Form("TOF:%s",absorber_name.Data()), fTOF);
               if (is_active) nuc->GetParameters()->SetValue(Form("TOF:%s", dname.Data()), fTOF);
               else if ((fCalib & kFullTCalib) == kFullTCalib) nuc->GetParameters()->SetValue(Form("TOF:%s", absorber_name.Data()), fTOF);


               // from the entrance to the exit of the material
               Double_t step = GetStepSize();
               fTOF += CalculateLinearDeltaT(Vi, Vf, step);
               fStartPath += step;
            }

            if (fCalib & kECalib) {
               if (is_active) nuc->GetParameters()->SetValue(Form("DE:%s", dname.Data()), DE);
               else if ((fCalib & kFullECalib) == kFullECalib) nuc->GetParameters()->SetValue(Form("DE:%s", absorber_name.Data()), DE);
            }
         }
      }

      if (is_active)  nuc->GetParameters()->SetValue(Form("DPATH:%s", dname.Data()), Delta);
      else if ((fCalib & kFullTCalib) == kFullTCalib) nuc->GetParameters()->SetValue(Form("DPATH:%s", absorber_name.Data()), Delta);
      nuc->GetParameters()->SetValue(Form("STEP:%s", absorber_name.Data()), GetStepSize());

   }
}
//________________________________________________________________

void KVVAMOSReconGeoNavigator::PropagateNucleus(KVVAMOSReconNuc* nuc, ECalib cal)
{
   // Propagates the VAMOS reconstructed nucleus along its focal plane
   // direction in the focal-plane detection area.
   // Calculate the calibration for a well know calibration
   // nucleus (Z, A, E) i.e. energy losses, steps, in each crossed detector, in the
   // stripping foil and in the target (if it exists) as well as
   // times of flight are calculated. We assume that you have set Z, A
   // and energy of the nucleus before calling this method (see SetZandA(...)
   // and SetEnergy(...) ).
   //
   // The calculated quantities are stored  in the nucleus's list KVParticle::fParameters ( see main class description ).

   //Set the calibration choice
   fCalib = cal;

   //If this is the first absorber that the particle crosses, we set a "reminder" of its
   //initial energy
   nuc->SetE0();
   fE =  nuc->GetEnergy();

   // For gGeoManager the origin is the target point.
   // Starting point has to be set from this origin.

   // intersection point between reconstructed trajectory and the Focal
   // Plane in the focal plane frame of reference
   Double_t XYZ[3];

   //propagation direction at the focal plane
   Double_t XYZdir[3];

   // Take reconstructed path if the trajector is correctly reconstructed
   // or take the central trajectory otherwise
   if (!nuc->GetCodes().TestFPCode(kFPCode0) && (nuc->GetPath() > 0)) {
      fStartPath = -nuc->GetPath();
      XYZ[0] = nuc->GetXf();
      XYZ[1] = nuc->GetYf();
      XYZ[2] = 0.;
      nuc->GetFocalPlaneDirection().GetXYZ(XYZdir);
      gVamos->FocalToTargetVect(XYZdir, XYZdir);
   } else {
      fStartPath = -gVamos->GetFocalPlanePosition();
      XYZ[0] = XYZ[1] = XYZ[2] = 0.;
      XYZdir[0] = XYZdir[1] = 0.;
      XYZdir[2] = 1.;
      Warning("PropagateNucleus", "A central trajectory is used to propagate the nucleus (FPCode%d, path= %f cm)", nuc->GetCodes().GetFPCodeIndex(), nuc->GetPath());
   }
   // intersection point between reconstructed trajectory and the Focal
   // Plane in the frame of reference centered at the target point
   gVamos->FocalToTarget(XYZ, XYZ);
   fOrigine.SetXYZ(XYZ[0], XYZ[1], XYZ[2]);

   //  propagation direction = direction of the trajectory at the focal plane
   gVamos->FocalToTargetVect(XYZdir, XYZdir);
   TVector3 direction(XYZdir);

   // Calculate energy losses in target and in tripping foil if the option
   // kECalib or kFullCalib is chosen
   if ((fCalib & kECalib) || (fCalib & kTCalib)) {

      //    nuc->SetEnergy( fE );
      Double_t DE = 0.;
      TVector3 norm(0., 0., 1.); //vector normal to the target and to the stripping foil
      Double_t nz = nuc->GetMomentum().Unit().Z();
      fTOF = 0.;

      //target, we assume that the nucleus is not slow down in the target
      if ((fE > 1e-3) && gMultiDetArray->GetTarget()) {
         gMultiDetArray->GetTarget()->SetIncoming(kFALSE);
         gMultiDetArray->GetTarget()->SetOutgoing(kTRUE);
         DE = gMultiDetArray->GetTarget()->GetELostByParticle(nuc, &norm);
         fE -= DE;
         nuc->SetEnergy(fE);

         if (fCalib & kECalib) nuc->GetParameters()->SetValue(Form("DE:TARGET_%s", gMultiDetArray->GetTarget()->GetName()), DE);

      }

      //stripping foil
      if ((fE > 1e-3) && gVamos->GetStripFoil()) {

         // velocity before stripping foil
         Double_t Vi = nuc->GetVelocity().Mag();

         DE = gVamos->GetStripFoil()->GetELostByParticle(nuc, &norm);
         fE -= DE;
         nuc->SetEnergy(fE);

         // velocity after stripping foil
         Double_t Vf = nuc->GetVelocity().Mag();

         //target to stripping foil
         if (fCalib & kTCalib) {
            Double_t path = gVamos->GetStripFoilPosition() / TMath::Max(nz, 1.e-03);
            fTOF += path / Vi;
            fStartPath += path;

            if ((fCalib & kFullTCalib) == kFullTCalib) {
               nuc->GetParameters()->SetValue(Form("TOF:STRIPFOIL_%s", gVamos->GetStripFoil()->GetName()), fTOF);
               nuc->GetParameters()->SetValue(Form("DPATH:STRIPFOIL_%s", gVamos->GetStripFoil()->GetName()), fStartPath);
            }

            // from the entrance to the exit of the stripping foil
            Double_t step = gVamos->GetStripFoil()->GetThickness() / TMath::Max(nz, 1.e-03);
            fTOF += CalculateLinearDeltaT(Vi, Vf, step);
            fStartPath += step;
            nuc->GetParameters()->SetValue(Form("STEP:%s", gVamos->GetStripFoil()->GetName()), step);
         }

         if ((fCalib & kFullECalib) == kFullECalib) nuc->GetParameters()->SetValue(Form("DE:STRIPFOIL_%s", gVamos->GetStripFoil()->GetName()), DE);
      }
   }

   // stop the E and T calculation if the nucleus does not have energy anymore
   // but continue the propagation
   if (fE < 1e-3) {
      fCalib = kNoCalib;
      fE = 1; // set a non null energy to continue the propagation
   }

   // Backward propagation to find the second origine point located
   // just before the first detector on this direction
   fDoNothing = kTRUE;
   direction = -direction;
   nuc->SetMomentum(direction);
   PropagateParticle(nuc, &fOrigine);

   // Forward propagation
   fDoNothing = kFALSE;
   direction = -direction;

   nuc->SetMomentum(fE, direction);
   const Double_t* posi = GetGeometry()->GetCurrentPoint();
   XYZ[0] = (posi[0] + GetExitPoint().X()) / 2.;
   XYZ[1] = (posi[1] + GetExitPoint().Y()) / 2.;
   XYZ[2] = (posi[2] + GetExitPoint().Z()) / 2.;
   fFOrigine.SetXYZ(XYZ[0], XYZ[1], XYZ[2]);
   // Info("PropagateNucleus","FOrigine vol. name: %s",GetCurrentVolume()->GetName());
   // cout<<"FORIGINE POINT: "; fFOrigine.Print();
   PropagateParticle(nuc, &fFOrigine);

   //set the initial momentum/energy  to the nucleus
   nuc->ResetEnergy();

}
//________________________________________________________________

Double_t KVVAMOSReconGeoNavigator::CalculateLinearDeltaT(Double_t vi, Double_t vf, Double_t step) const
{
   // Calculates the time spent for crossing a material for a given step.
   // We assume that the velocity evolves linearly from the initial velocity 'vi'
   // to the final velocity 'vf' reached after a distance 'step' i.e.
   //      v(x) = a x + b = dx/dt    for x between 0 and step
   //      where
   //      a = (vf-vi)/step     and  b = vi
   //
   // Finally, the returned value is   DeltaT = [ln(a*step+b)-ln(b)]/a
   //

   if ((step <= 0) || (vi <= 0)) return 0.;
   if (vi == vf) return step / vi;

   Double_t a = (vf - vi) / step;
   Double_t b = vi;

   return (TMath::Log(a * step + b) - TMath::Log(b)) / a;
}
