
///
/// @file MEDetectorStack.cpp
///
/// @section Description
///
///   A simple Silicon-Isobutane-CsI detector stack, used in the A value
///   estimation algorithms - see SiliconEnergyMinimiser. Allows one to calculate
///   the "delta" between the measured and simulated silicon detector energies in
///   order to determine the most likely candidate for the A value (ideally
///   "delta" would be zero so we look for the minimum value of "delta").
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 14:12:39  2016
///

#include "MEDetectorStack.h"

ClassImp(MEDetectorStack)

MEDetectorStack::MEDetectorStack() :
#if __cplusplus < 201103L
   si_(NULL),
   isobutane_(NULL),
   csi_(NULL),
   sim_nucleus_(NULL),
   calibrator_(NULL),
#else
   si_(nullptr),
   isobutane_(nullptr),
   csi_(nullptr),
   sim_nucleus_(nullptr),
   calibrator_(nullptr),
#endif
   kInitialised_(kFALSE),
   absolute_delta_(kTRUE)
{

}

MEDetectorStack::~MEDetectorStack()
{
#if __cplusplus < 201103L
   if (csi_) {
      delete csi_;
      csi_ = NULL;
   }

   if (isobutane_) {
      delete isobutane_;
      isobutane_ = NULL;
   }

   if (si_) {
      delete si_;
      si_ = NULL;
   }

   if (sim_nucleus_) {
      delete sim_nucleus_;
      sim_nucleus_ = NULL;
   }
#endif
}

Bool_t MEDetectorStack::Init()
{
   if (kInitialised_) {
      return kTRUE;
   }

   // The following silicon thickness is just a placeholder - this value is set
   // properly in the SetIDTelescope function.

#if __cplusplus < 201103L
   sim_nucleus_ = new KVNucleus();
   si_ = new KVMaterial("Si", 530 * KVUnits::um);
   isobutane_ = new KVMaterial("C4H10", 136.5 * KVUnits::mm);
   csi_ = new KVMaterial("CsI", 1.*KVUnits::cm);
#else
   sim_nucleus_.reset(new KVNucleus());
   si_.reset(new KVMaterial("Si", 530 * KVUnits::um));
   isobutane_.reset(new KVMaterial("C4H10", 136.5 * KVUnits::mm));
   csi_.reset(new KVMaterial("CsI", 1.*KVUnits::cm));
#endif

   isobutane_->SetPressure(40.*KVUnits::mbar);
   kInitialised_ = kTRUE;

   return kTRUE;
}

Bool_t MEDetectorStack::Simulate(
   const struct me::SimulationParameters* const parameters,
   struct me::SimulationResult* const result
)
{
   assert(kInitialised_);

   assert(parameters);
   assert(result);

   // Check for sensible input parameters
   assert((parameters->z > 0) && (parameters->z < 120));
   assert((parameters->a > 0) && (parameters->a < 300));
   assert((parameters->si_energy > 0.) && (parameters->si_energy < 3000.));

   // We now need to calculate the incident energy of the nucleus (so that we
   // can simulate its passage through the idtelescope).

   // We have the csi_light, so let's work out the csi_energy first using the
   // light->energy calibration function defined for this caesium iodide
   // detector.

   if (!calibrator_) return kFALSE;
   if (!calibrator_->GetStatus()) return kFALSE;

   Double_t csi_energy(
      calibrator_->Compute(
         parameters->z,
         parameters->a,
         parameters->csi_light
      )
   );

   if ((csi_energy <= 0.) || (csi_energy > 3000.)) {
      // Out of bounds csi energy, discard this event. For some reason there
      // are occasionally energies of 0. MeV returned for non-zero csi light,
      // I'm not sure whether that is a fault in the code or if it is the
      // default return value when TF1::GetX() fails. To see these events
      // enable the assertions and uncomment the above assertion.

      return kFALSE;
   }

   // We then calculate the incident energy for the isobutane layer, which
   // resides behind the silicon layer and in front of the caesium iodide. We
   // calculate this by finding the residual energy just after the silicon
   // layer (using the energy measured in the silicon).

   Double_t isobutane_incident_energy(
      si_->GetEResFromDeltaE(
         parameters->z,
         parameters->a,
         parameters->si_energy
      )
   );

   if (isobutane_incident_energy <= 0.) {
      // Particle does not punch through the silicon material and so we can
      // not identify it in the SI-CSI telescope.

      return kFALSE;
   }

   if (isobutane_incident_energy > 3000.) {
      // This generally happens when the silicon energy (from the silicon
      // calibration) is too low, this results in an over-estimation of the
      // isobutane incident energy. This event is rejected as this energy loss
      // is far greater than that of the available beam energy (realistically
      // we expect less than 2000 MeV)

      return kFALSE;
   }

   Double_t isobutane_energy(
      isobutane_->GetDeltaE(
         parameters->z,
         parameters->a,
         isobutane_incident_energy
      )
   );

   assert((isobutane_energy > 0.) && (isobutane_energy < 3000.));

   // We now have all the data we need to calculate the incident energy of the
   // nucleus prior to entering the silicon layer

   Double_t incident_energy(
      parameters->si_energy +
      isobutane_energy +
      csi_energy
   );

   // Now we can simulate the passage of the nucleus through the idtelescope
   // and calculate 'delta' (the absolute difference between the measured
   // silicon energy and that simulated for this nucleus)

   assert((incident_energy > 0.) && (incident_energy < 3000.));

   sim_nucleus_->Clear();
   sim_nucleus_->SetZAandE(parameters->z, parameters->a, incident_energy);

#if __cplusplus < 201103L
   si_->DetectParticle(sim_nucleus_);
   isobutane_->DetectParticle(sim_nucleus_);
   csi_->DetectParticle(sim_nucleus_);
#else
   si_->DetectParticle(sim_nucleus_.get());
   isobutane_->DetectParticle(sim_nucleus_.get());
   csi_->DetectParticle(sim_nucleus_.get());
#endif

   Double_t delta(0.);

   if (absolute_delta_) {
      // We've specified absolute delta (default)
      delta = TMath::Abs(parameters->si_energy - si_->GetEnergyLoss());
   } else {
      // Otherwise simply return the difference
      delta = parameters->si_energy - si_->GetEnergyLoss();
   }

   // Pack the result struct

   result->z = parameters->z;
   result->a = parameters->a;
   result->si_energy = si_->GetEnergyLoss();
   result->gap_energy = isobutane_energy;
   result->csi_energy = csi_energy;
   result->delta = delta;

   // Clear the materials for the next simulation
   si_->Clear();
   isobutane_->Clear();
   csi_->Clear();

   return kTRUE;
}

Bool_t MEDetectorStack::SetIDTelescope(const TString& name)
{
   assert(kInitialised_);

   KVIDTelescope* idt(gVamos->GetIDTelescope(name.Data()));

   if (!idt) return kFALSE;
   if (!idt->InheritsFrom("KVIDHarpeeSiCsI_e503")) return kFALSE;

   // ----------------------------------
   // Set the silicon material thickness
   // ----------------------------------

   KVDetector* si_detector(idt->GetDetector(1));
   assert(si_detector);

   KVMaterial* si_absorber(si_detector->GetAbsorber(0));
   assert(si_absorber);

   si_->SetThickness(si_absorber->GetThickness());

   // --------------------------------------
   // Set the CsI light -> energy calibrator
   // --------------------------------------

   KVDetector* csi(idt->GetDetector(2));
   assert(csi);

   const TString calibrator_name(
      Form("Light->MeV %s", csi->GetName())
   );

   KVLightEnergyCsIVamos* cal(
      dynamic_cast<KVLightEnergyCsIVamos*>(
         csi->GetCalibrator(calibrator_name.Data())
      )
   );

   if (!cal) return kFALSE;
   if (!cal->GetStatus()) return kFALSE;

   calibrator_ = cal;

   return kTRUE;
}

void MEDetectorStack::SetAbsoluteDelta(Bool_t status)
{
   absolute_delta_ = status;
}

Bool_t MEDetectorStack::IsInitialised() const
{
   return kInitialised_;
}

