// Author: Peter C. Wigg <peter.wigg.314159@gmail.com>
// Created Wed 20 Jan 13:58:10  2016

///////////////////////////////////////////////////////////////////////////////
//
// Corrections_e503.cpp
//
// Description
//
// A set of non-member, non-friend functions which together act as a wrapper
// for the standard KaliVeda identification functions and enable the
// identified nucleus to be modified in accordance with the implemented
// identification corrections.
//
// Peter C. Wigg <peter.wigg.314159@gmail.com>
// Wed 20 Jan 13:58:10  2016
//
///////////////////////////////////////////////////////////////////////////////

#include "Corrections_e503.h"

namespace idc {

   //___________________________________________________________________________

   void VAMOSIdentifyHack(KVVAMOSReconNuc* const n)
   {
      assert(n);

      n->Identify(); // KVReconstructedNucleus::Identify()

      KVIdentificationResult partID;
      Bool_t ok(kFALSE);

      UInt_t num_iterations(0);
      UInt_t max_iterations(10);

      // For all nuclei we take the first identification which gives IDOK==kTRUE
      Int_t id_no(1);
      KVIdentificationResult* pid(n->GetIdentificationResult(id_no));

      while (pid->IDattempted && (num_iterations < max_iterations)) {

         if (pid->IDOK) {
            ok = kTRUE;
            partID = *pid;
            break;
         }

         ++id_no;
         pid = n->GetIdentificationResult(id_no);
         ++num_iterations;
      }

      if (ok) {
         n->SetIsIdentified();
         KVIDTelescope* idt(
            static_cast<KVIDTelescope*>(
               n->GetIDTelescopes()->FindObjectByType(partID.GetType())
            )
         );
         if (!idt) {
            Warning("Identify", "cannot find ID telescope with type %s",
                    partID.GetType());
            n->GetIDTelescopes()->ls();
            partID.Print();
         }
         n->SetIdentifyingTelescope(idt);
         n->SetIdentification(&partID);
      } else {
         /******* UNIDENTIFIED PARTICLES *******/
         /*** general ID code for non-identified particles ***/
         if (num_iterations == max_iterations) {
            std::cerr << "\nVAMOSIdentifyHack: Exceeded max iterations!"
                      << std::endl;
         }
         n->SetIDCode(kIDCode14);
      }

   };

   //___________________________________________________________________________

   UChar_t Identify(
      KVVAMOSReconNuc* const n,
      AbsorberEnergies* const energy_data
   )
   {
      if (!n) {
         Error("idc::Identify", "Supplied nucleus is a null pointer");
         return kNullNucleusSupplied;
      }

      // Test to make sure the nucleus has been initialised properly, if it
      // hasn't been detected anywhere then we're not really interested in it.
      if (!n->IsDetected()) {
         Error("Idc::Identify", "Supplied nucleus has not been detected");
         return kNucleusNotDetected;
      }

      KVVAMOSCodes codes(n->GetCodes());
      if (codes.GetTCode() == kTCode0) {
         // Badly calibrated time
         return kBadTimeCal;
      }

      // KVVAMOSReconNuc::Identify() is no longer implemented! Instead we must
      // call idc::VAMOSIdentifyHack() which re-implements this missing
      // function.
      //
      // WARNING: This is not a long term solution, it is a hack!
      VAMOSIdentifyHack(n);

      if (!n->IsIdentified()) return kNotIdentified;

      KVIDTelescope* idt(n->GetIdentifyingTelescope());
      if (!idt) return kNoIdTelescope;

      KVIdentificationResult* idr(n->GetIdentificationResult(idt));
      if (!idr) return kNoIdResult;
      if (!idr->IDOK) return kBasicIdNotOk;
      if (!idr->Zident) return kZIdNotOk;
      if (idr->Z < 1) return kUnCharged;;

      const Bool_t kICSi(idt->InheritsFrom("KVIDHarpeeICSi_e503"));
      const Bool_t kSiCsI(idt->InheritsFrom("KVIDHarpeeSiCsI_e503"));

      if (!kICSi && !kSiCsI) {
         // The telescope does not inherit from either of the expected classes
         return kNotInherited;
      }

      if (kICSi && kSiCsI) {
         // You can't inherit from both telescopes! What's going on?!
         return kBothInherited;
      }

      if ((idr->IDcode != kIDCode3) && (idr->IDcode != kIDCode4)) {
         // The identification result does not have either of the expected
         // identification codes
         return kInvalidIdCode;
      }

      Double_t total_energy(0.);

      KVVAMOSDetector* stopping_detector(
         static_cast<KVVAMOSDetector*>(n->GetStoppingDetector())
      );

      assert(stopping_detector);

      KVVAMOSDetector* residual_detector(
         static_cast<KVVAMOSDetector*>(idt->GetDetector(2))
      );

      assert(residual_detector);

      // We need the stopping detector to be sure we have the complete energy
      // loss information, otherwise the simulation will give bad results.

      if (stopping_detector != residual_detector) return kNotStoppingDetector;

      KVNucleus sim_nucleus;

      if (idr->Zident && idr->Aident) {
         sim_nucleus.SetZandA(idr->Z, idr->A);
      } else {
         // A Value is automatically calculated.
         sim_nucleus.SetZ(idr->Z);
      }

      assert((sim_nucleus.GetZ() > 0) && (sim_nucleus.GetZ() < 120));
      assert((sim_nucleus.GetA() > 0) && (sim_nucleus.GetA() < 300));

      // A list of all the detectors which are aligned with and between the
      // residual energy detector and the target.
      TList* detector_list(
         residual_detector->GetAlignedDetectors(KVGroup::kBackwards)
      );

      // Iterate over the list of aligned detectors (the list is in reverse
      // order) and add up the energies of all the absorbers, making our way
      // back towards the target. We must do this explicitly as the VAMOS
      // definition of "calibrated detector" may differ from the standard
      // KaliVeda definition.  All we care about here is that the detector is
      // calibrated in energy.

      Double_t detector_eloss(0.);
      Double_t absorber_eloss(0.);

      if (energy_data) {
         energy_data->Reset();
      }

#if __cplusplus < 201103L
      KVVAMOSDetector* detector(NULL);
#else
      KVVAMOSDetector* detector(nullptr);
#endif

      TIter next_detector(detector_list);
      while ((detector = static_cast<KVVAMOSDetector*>(next_detector()))) {

         const Bool_t kDetectorIsDC(detector->InheritsFrom("KVDriftChamber"));
         const Bool_t kDetectorIsIC(detector->InheritsFrom("KVHarpeeIC"));
         const Bool_t kDetectorIsSi(detector->InheritsFrom("KVHarpeeSi"));
         const Bool_t kDetectorIsCsI(detector->InheritsFrom("KVHarpeeCsI"));

         KVList* absorbers(detector->GetListOfAbsorbers());

         // For each detector we iterate IN REVERSE over the absorbers (making
         // our way towards the target) and sum either the calculated energy
         // loss in the dead layers or the calibrated energy in the active
         // layer.

         detector_eloss = 0.;

#if __cplusplus < 201103L
         KVMaterial* abs(NULL);
#else
         KVMaterial* abs(nullptr);
#endif
         TIter next_absorber(absorbers, kIterBackward);
         while ((abs = static_cast<KVMaterial*>(next_absorber()))) {

            absorber_eloss = 0.;

            if (abs == detector->GetActiveLayer()) {

               // The absorber is the active layer of the detector and so we
               // can simply call the detector's GetCalibE function, or in the
               // case of the CsI detector (which requires the Z and A values)
               // GetCorrectedEnergy(). Also the drift chamber is not
               // calibrated in energy and must be calculated.

               if (kDetectorIsCsI) {

                  KVHarpeeCsI* csi(static_cast<KVHarpeeCsI*>(detector));
                  assert(csi);

                  // XXX: Be careful: This only works because the energy of
                  // the nucleus is not used in GetCorrectedEnergy(), should
                  // this change then we will need to rethink.

                  absorber_eloss = csi->GetCorrectedEnergy(
                                      &sim_nucleus, -1., 0
                                   );

               } else if (kDetectorIsDC) {

                  // Calculate energy loss (not calibrated).
                  KVMaterial* drift_active(static_cast<KVMaterial*>(abs));
                  assert(drift_active);

                  absorber_eloss = drift_active->GetDeltaEFromERes(
                                      sim_nucleus.GetZ(),
                                      sim_nucleus.GetA(),
                                      total_energy
                                   );

                  if (energy_data) {
                     if (detector->GetNumber() == 1) {
                        energy_data->dc1_active = absorber_eloss;
                     } else if (detector->GetNumber() == 2) {
                        energy_data->dc2_active = absorber_eloss;
                     }
                  }

               } else {

                  // Other calibrated deterctor
                  absorber_eloss = detector->GetCalibE();

                  if (energy_data) {
                     if (kDetectorIsIC) {
                        energy_data->ic_active = absorber_eloss;
                     }
                  }

               }

               detector_eloss += absorber_eloss;
               total_energy += absorber_eloss;

               continue;
            }

            if (total_energy <= 0.) {

               // The total_energy should, in principle, already be set by
               // either the silicon or CsI detectors (both have only one layer
               // and it is active) so we should not encounter this condition.

               return kResidualEnergyFail;
            }

            // Total energy is the residual energy since we are working towards
            // the target from the stopping detector
            Double_t incident_energy(
               abs->GetIncidentEnergyFromERes(
                  sim_nucleus.GetZ(),
                  sim_nucleus.GetA(),
                  total_energy
               )
            );

            // Vector normal to absorber surface, pointing from the origin
            // toward the material - in this case simply the z direction (beam
            // direction)
            TVector3 norm(0, 0, 1);

            sim_nucleus.SetEnergy(incident_energy);
            sim_nucleus.SetTheta(n->GetThetaL());
            sim_nucleus.SetPhi(n->GetPhiL());

            absorber_eloss = abs->GetELostByParticle(&sim_nucleus, &norm);

            detector_eloss += absorber_eloss;
            total_energy += absorber_eloss;

         }

         if (energy_data) {
            if (kDetectorIsCsI) {
               energy_data->csi = detector_eloss;

            } else if (kDetectorIsSi) {
               energy_data->si = detector_eloss;

            } else if (kDetectorIsDC) {

               if (detector->GetNumber() == 1)
                  energy_data->dc1 = detector_eloss;

               else if (detector->GetNumber() == 2)
                  energy_data->dc2 = detector_eloss;

            } else if (kDetectorIsIC) {
               energy_data->ic = detector_eloss;
            }
         }

         if (kDetectorIsSi && kSiCsI) {

            // Si->CsI dead layer correction.
            //
            // Be careful here! This calculation relies upon the silicon
            // detector having only one absorber, which is calibrated. If the
            // silicon detector has any dead layers then this calculation will
            // not be accurate, as we only make use of the calibrated energy.
            //
            // The alternative would be to use detector_eloss in the incident
            // energy calculation but that is also problematic, as you introduce
            // a cyclic dependency on the calculated total energy! We end up in
            // this situation as the residual energy from the CsI detector is
            // zero and so reliable calculations of the CsI incident energy are
            // not possible.

            // silicon detector incident energy
            assert((detector->GetCalibE() >= 0.) &&
                   (detector->GetCalibE() < 3000.));

            Double_t incident_energy(
               detector->GetEResFromDeltaE(
                  sim_nucleus.GetZ(),
                  sim_nucleus.GetA(),
                  detector->GetCalibE()
               )
            );

            // Vector normal to dead-layer surface, pointing from the origin
            // toward the material - in this case simply the z direction (beam
            // direction)

            TVector3 norm(0, 0, 1);
            KVMaterial isobutane("C4H10", 13.65 * KVUnits::cm);
            isobutane.SetPressure(40.*KVUnits::mbar);

            sim_nucleus.SetEnergy(incident_energy);
            sim_nucleus.SetTheta(n->GetThetaL());
            sim_nucleus.SetPhi(n->GetPhiL());

            absorber_eloss = isobutane.GetELostByParticle(&sim_nucleus, &norm);
            total_energy += absorber_eloss;

            if (energy_data) {
               energy_data->iso_sicsi = absorber_eloss;
            }

            // The silicon detector losses have already been taken into account
            // above.
            continue;

         }

         if (kDetectorIsDC && (detector->GetNumber() == 2)) {

            // Make the SED correction
            //
            // We calculate the incident energy to the second drift chamber
            // from its residual energy and use it as the basis for calculating
            // the energy loss in the sed. Care is needed to set the correct
            // effective thickness of the material for the particle.

            // Vector normal to SED surface, pointing from the origin toward the
            // material - in this case 45 degrees (the beam [0,0,1] sees the
            // thickness as 1.273um)
            TVector3 norm(0, -1, 1);
            KVMaterial sed("Myl", 0.9 * KVUnits::um);

            // total_energy at this point includes the second drift chamber
            // energy losses and so this needs to be subtracted again to get the
            // residual energy.
            Double_t drift_2_incident_energy(
               detector->GetIncidentEnergyFromERes(
                  sim_nucleus.GetZ(),
                  sim_nucleus.GetA(),
                  total_energy - detector_eloss
               )
            );

            // Nucleus is defined just prior to entering the second drift
            // chamber and is used only to evaluate the direction in which it is
            // travelling.

            sim_nucleus.SetEnergy(drift_2_incident_energy);
            sim_nucleus.SetTheta(n->GetThetaL());
            sim_nucleus.SetPhi(n->GetPhiL());

            // Normalised!
            TVector3 momentum(sim_nucleus.GetMomentum());
            Double_t mag(momentum.Mag());

            TVector3 dir;
            dir.SetX(momentum.X() / mag);
            dir.SetY(momentum.Y() / mag);
            dir.SetZ(momentum.Z() / mag);

            Double_t effective_thickness(sed.GetEffectiveThickness(norm, dir));

            // Set the linear thickness of the SED to the correct effective
            // thickness.
            sed.SetThickness(effective_thickness);

            absorber_eloss = sed.GetDeltaEFromERes(
                                sim_nucleus.GetZ(),
                                sim_nucleus.GetA(),
                                drift_2_incident_energy
                             );

            total_energy += absorber_eloss;

            if (energy_data) {
               energy_data->sed = absorber_eloss;
            }

            // The drift chamber energy losses have already been accounted for
            // above.
            continue;
         }

      }

      // Final corrections to the energy for the strip foil and the target

      KVMaterial strip_foil(20.*KVUnits::ug, "C");

      absorber_eloss = strip_foil.GetDeltaEFromERes(
                          sim_nucleus.GetZ(),
                          sim_nucleus.GetA(),
                          total_energy
                       );
      total_energy += absorber_eloss;

      if (energy_data) {
         energy_data->strip_foil = absorber_eloss;
      }

      // FIXME: Target is not being set
      //KVTarget* target(gVamos->GetTarget());

      //if (!target) {
      //   return kNoTarget;
      //}

      //absorber_eloss = target->GetDeltaEFromERes(
      //                    sim_nucleus.GetZ(),
      //                    sim_nucleus.GetA(),
      //                    total_energy
      //                 );
      //total_energy += absorber_eloss;

      //if (energy_data) {
      //   energy_data->target = absorber_eloss;
      //}

      assert((total_energy >= 0.) && (total_energy < 3000.));

      Double_t real_z(idr->PID);

      Double_t real_a(
         n->CalculateRealA(
            sim_nucleus.GetZ(),
            total_energy,
            //n->GetBetaFromToF()
            n->GetBeta("TSI_HF")
         )
      );

      Double_t pid(real_z + 0.1 * (real_a  - 2. * real_z));

      Int_t z_value(sim_nucleus.GetZ());
      Int_t a_value(sim_nucleus.GetA());

      // TODO: Just occasionally (1/20,000) something weird happens with the
      // energy calculation and we end up with A = 0. Set a flag for now and
      // come back and analyse what is going on another time - I couldn't find
      // anything immediately obvious. I think it's something to do with the
      // nucleus stating that it has stopped in the silicon but there's still
      // some energy detected in the CsI behind it. So essentially we have some
      // missing energy still to account for, I'm not sure why the CsI does not
      // register as being hit...

      if (a_value < 1) {
         return kUnChargedResult;
      }

      idc::CorrectionData data;
      data.idt = idt;
      data.z_value = z_value;
      data.a_value = a_value;
      data.z_real = real_z;
      data.a_real = real_a;
      data.pid = pid;
      data.energy = total_energy;

      if (!idc::ApplyCorrections(n, &data)) {
         return kCorrectionsFailed;
      }

      return kAllOK;
   }

   //___________________________________________________________________________

   Bool_t ApplyCorrections(
      KVVAMOSReconNuc* const n,
      const CorrectionData* const data
   )
   {
      assert(n);
      assert(data);

      const KVIDTelescope* const idt(data->idt);
      assert(idt);

      Bool_t status(kFALSE);

      if (idt->InheritsFrom("KVIDHarpeeICSi_e503")) {
         const KVIDHarpeeICSi_e503* const icsi(
            static_cast<const KVIDHarpeeICSi_e503* const>(idt)
         );
         assert(icsi);

         status = ApplyIcSiCorrections(n, icsi, data);

      } else if (idt->InheritsFrom("KVIDHarpeeSiCsI_e503")) {
         const KVIDHarpeeSiCsI_e503* const sicsi(
            static_cast<const KVIDHarpeeSiCsI_e503* const>(idt)
         );
         assert(sicsi);

         status = ApplySiCsiCorrections(n, sicsi, data);

      }

      return status;
   }

   //___________________________________________________________________________

   Bool_t ApplyIcSiCorrections(
      KVVAMOSReconNuc* const n,
      const KVIDHarpeeICSi_e503* const idt,
      const CorrectionData* const data
   )
   {
      // Apply corrections for the Ionisation Chamber:Silicon Telescopes TODO:
      // Can any of this code be consoldiated? There is a lot of duplication
      // between ICSi and SiCsI functions

      assert(n);
      assert(idt);
      assert(data);

      const KVList* parameters(idt->GetIDCorrectionParameters());
      assert(parameters);

      KVHarpeeIC* ic(static_cast<KVHarpeeIC*>(idt->GetDetector(1)));
      assert(ic);

      // A/Q Straightening parameters
      KVDBParameterSet* a_over_q_straight(
         static_cast<KVDBParameterSet*>(
            parameters->FindObject(
               Form("straight_fct_chiosi_chio%d",
                    ic->GetFiredSegNumber()
                   )
            )
         )
      );

      if (!a_over_q_straight) {
         // No straightening parameters
         return kFALSE;
      }

      Double_t uncorrected_a_over_q(n->GetRealAoverQ("TSI_HF"));
      Double_t corrected_a_over_q(
         CorrectAoverQ(a_over_q_straight, uncorrected_a_over_q, data->pid)
      );

      IGNORE_UNUSED(corrected_a_over_q);

      // More Parameters
      //Correct();

      // Finally, modify the nucleus:

      // Note: This sets kinetic energy which must be accessed in your selector
      // class by KVVAMOSReconNuc::GetKE();

      n->SetZAandE(data->z_value, data->a_value, data->energy);
      n->SetRealZ(data->z_real);
      n->SetRealA(data->a_real);

      // NOTE: KVVAMOSReconNuc::Calibrate() is extremely slow (lots of TF1::Eval
      // going on) and it reduces the event rate. I don't think I need it anyway
      // (I *think*) as I'm setting the energy manually in the code above. As a
      // result the nucleus will not appear calibrated, but the nucleus itself
      // should be OK.

      // n->Calibrate();
      // if (!n->IsCalibrated()) return kNotCalibrated;

      return kTRUE;
   }

   //___________________________________________________________________________

   Bool_t ApplySiCsiCorrections(
      KVVAMOSReconNuc* const n,
      const KVIDHarpeeSiCsI_e503* const idt,
      const CorrectionData* const data
   )
   {
      // Apply corrections for the Silicon:Caesium Iodide Telescopes

      assert(n);
      assert(idt);
      assert(data);

      const KVList* parameters(idt->GetIDCorrectionParameters());
      assert(parameters);

      KVHarpeeSi* si(static_cast<KVHarpeeSi*>(idt->GetDetector(1)));
      assert(si);

      // A/Q Straightening parameters
      KVDBParameterSet* a_over_q_straight(
         static_cast<KVDBParameterSet*>(
            parameters->FindObject(
               Form("straight_fct_si%d",
                    si->GetNumber()
                   )
            )
         )
      );

      if (!a_over_q_straight) {
         // No straightening parameters
         return kFALSE;
      }

      Double_t uncorrected_a_over_q(n->GetRealAoverQ("TSI_HF"));
      Double_t corrected_a_over_q(
         CorrectAoverQ(a_over_q_straight, uncorrected_a_over_q, data->pid)
      );

      IGNORE_UNUSED(corrected_a_over_q);

      // More Parameters
      //Correct();

      // Finally, modify the nucleus:

      // Note: This sets kinetic energy which must be accessed in your selector
      // class by KVVAMOSReconNuc::GetKE();

      n->SetZAandE(data->z_value, data->a_value, data->energy);
      n->SetRealZ(data->z_real);
      n->SetRealA(data->a_real);

      // NOTE: KVVAMOSReconNuc::Calibrate() is extremely slow (lots of TF1::Eval
      // going on) I don't think I need it anyway (I *think*) as I'm
      // setting the energy manually in the code above. As a result the nucleus
      // will not appear calibrated, but the nucleus itself should be OK.

      //n->Calibrate();
      //if (!n->IsCalibrated()) return kNotCalibrated;

      return kTRUE;
   }

   //___________________________________________________________________________

   Double_t CorrectAoverQ(
      KVDBParameterSet* const parameters,
      Double_t uncorrected,
      Double_t pid
   )
   {
      assert(parameters);
      assert(parameters->GetParamNumber() == 3);
      assert(uncorrected > 0.);
      assert(pid > 0.);

      Double_t corrected(
         uncorrected - (
            parameters->GetParameter(0) +
            (parameters->GetParameter(1) * pid) +
            (parameters->GetParameter(2) * TMath::Power(pid, 2.))
         )
      );

      assert((corrected >= 0.) && (corrected < 10.));

      return corrected;
   }

} // namespace idc
