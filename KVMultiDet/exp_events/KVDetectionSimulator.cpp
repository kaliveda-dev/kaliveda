//Created by KVClassFactory on Sat Oct 10 09:37:42 2015
//Author: John Frankland,,,

#include "KVDetectionSimulator.h"
#include "KVGeoNavigator.h"

ClassImp(KVDetectionSimulator)

KVDetectionSimulator::KVDetectionSimulator(KVMultiDetArray* a, Double_t e_cut_off) :
   KVBase(Form("DetectionSimulator_%s", a->GetName()),
          Form("Simulate detection of particles or events in detector array %s", a->GetTitle())),
   fArray(a), fCalcTargELoss(kTRUE)
{
   // Initialise a detection simulator
   // The detector array is put into simulation mode, and the minimum cut-off energy
   // for propagation of particles is set
   a->Clear();
   a->SetSimMode(kTRUE);
   static_cast<KVRangeTableGeoNavigator*>(a->GetNavigator())->SetCutOffKEForPropagation(e_cut_off);
}

void KVDetectionSimulator::DetectEvent(KVEvent* event, const Char_t* detection_frame)
{
   //Simulate detection of event by multidetector array.
   //
   // optional argument detection_frame(="" by default) can be used to give name of
   // inertial reference frame (defined for all particles of 'event') to be used.
   // e.g. if the simulated event's default reference frame is the centre of mass frame, before calling this method
   // you should create the 'laboratory' or 'detector' frame with KVEvent::SetFrame(...), and then give the
   // name of the 'LAB' or 'DET' frame as 3rd argument here.
   //
   //For each particle in the event we calculate first its energy loss in the target (if the target has been defined, see KVMultiDetArray::SetTarget).
   //By default these energy losses are calculated from a point half-way along the beam-direction through the target (taking into account the orientation
   //of the target), if you want random depths for each event call GetTarget()->SetRandomized() before using DetectEvent().
   //
   //If the particle escapes the target then we look for the group in the array that it will hit. If there is one, then the detection of this particle by the
   //different members of the group is simulated.
   //
   //The detectors concerned have their fEloss members set to the energy lost by the particle when it crosses them.
   //
   //Give tags to the simulated particles via KVNucleus::AddGroup() method
   //Two general tags :
   //   - DETECTED : cross at least one active layer of one detector
   //   - UNDETECTED : go through dead zone or stopped in target
   //We add also different sub group :
   //   - For UNDETECTED particles : "DEAD ZONE", "STOPPED IN TARGET" and "THRESHOLD", the last one concerned particle
   //go through the first detection stage of the multidetector array but stopped in an absorber (ie an inactive layer)
   //   - For DETECTED particles :
   //         "PUNCH THROUGH" corrresponds to particle which cross all the materials in front of it
   //         (high energy particle punh through), or which miss some detectors due to a non perfect
   //         overlap between defined telescope,
   //         "INCOMPLETE"  corresponds to particles which stopped in the first detection stage of the multidetector
   //         in a detector which can not give alone a clear identification,
   //

   // Reset detectors in array hit by any previous events
   ClearHitGroups();

   event->ResetGetNextParticle();
   KVNucleus* part;
   while ((part = event->GetNextParticle())) {  // loop over particles

      KVNucleus* _part = (KVNucleus*)part->GetFrame(detection_frame, kFALSE);

      KVNameValueList det_stat, nvl;
      Double_t eLostInTarget = 0;

      TVector3 initial_momentum = _part->GetMomentum();

      if (part->GetZ() == 0) {
         det_stat.SetValue("UNDETECTED", "NEUTRON");

         part->AddGroup("UNDETECTED");
         part->AddGroup("NEUTRON");
      }
      else if (_part->GetKE() < GetMinKECutOff()) {
         det_stat.SetValue("UNDETECTED", "NO ENERGY");

         part->AddGroup("UNDETECTED");
         part->AddGroup("NO ENERGY");
      }
      else {
         if (IncludeTargetEnergyLoss() && GetTarget()) {

            GetTarget()->SetOutgoing(kTRUE);
            //simulate passage through target material
            Double_t ebef = _part->GetKE();
            GetTarget()->DetectParticle(_part);
            eLostInTarget = ebef - _part->GetKE();
            if (_part->GetKE() < GetMinKECutOff()) {
               det_stat.SetValue("UNDETECTED", "STOPPED IN TARGET");

               part->AddGroup("UNDETECTED");
               part->AddGroup("STOPPED IN TARGET");
            }
            GetTarget()->SetOutgoing(kFALSE);
         }

         if (_part->GetKE() > GetMinKECutOff()) {

            nvl = DetectParticle(_part);

            if (nvl.IsEmpty()) {

               det_stat.SetValue("UNDETECTED", "DEAD ZONE");

               part->AddGroup("UNDETECTED");
               part->AddGroup("DEAD ZONE");

            }
            else {
               part->AddGroup("DETECTED");
            }
         }
      }

      if (IncludeTargetEnergyLoss() && GetTarget()) part->SetParameter("TARGET Out", eLostInTarget);
      if (!nvl.IsEmpty()) {
         Int_t nbre_nvl = nvl.GetNpar();
         KVString LastDet(nvl.GetNameAt(nbre_nvl - 1));
         if (part->GetE() < GetMinKECutOff() || part->GetParameters()->HasParameter("DEADZONE")) {
            part->SetParameter("STOPPING DETECTOR", LastDet.Data());
            det_stat.SetValue("DETECTED", "OK");
         }
         else {
            det_stat.SetValue("DETECTED", "PUNCHED THROUGH");
         }
         for (Int_t ii = 0; ii < nvl.GetNpar(); ++ii) {
            part->SetParameter(nvl.GetNameAt(ii), nvl.GetDoubleValue(ii));
         }
      }
      for (Int_t ii = 0; ii < det_stat.GetNpar(); ii += 1) {
         part->SetParameter(det_stat.GetNameAt(ii), det_stat.GetStringValue(ii));
      }

      _part->SetMomentum(initial_momentum);

   }

}

//__________________________________________________________________________________

KVNameValueList KVDetectionSimulator::DetectParticle(KVNucleus* part)
{
   // Simulate detection of a single particle
   //
   // Propagate particle through the array,
   // calculating its energy losses in all absorbers, and setting the
   // energy loss members of the active detectors on the way.
   //
   // Returns a list containing the name and energy loss of each
   // detector hit in array (list is empty if none i.e. particle
   // in beam pipe or dead zone of the multidetector)

   fArray->GetNavigator()->PropagateParticle(part);

   // particle missed all detectors
   if (part->GetParameters()->IsEmpty()) return KVNameValueList();

   // list of energy losses in active layers of detectors
   KVNameValueList NVL;

   // find detectors in array hit by particle
   KVDetector* last_detector = nullptr;
   TIter next(part->GetParameters()->GetList());
   KVNamedParameter* param;
   while ((param = (KVNamedParameter*)next())) {
      KVString pname(param->GetName());
      pname.Begin(":");
      KVString pn2 = pname.Next();
      KVString pn3 = pname.Next();
      if (pn2 == "DE") {
         pn3.Begin("/");
         KVString det_name = pn3.Next();
         if (pn3.End() || pn3.Next().BeginsWith("ACTIVE")) {
            // energy loss in active layer of detector
            KVDetector* curDet = last_detector = fArray->GetDetector(det_name);
            if (!curDet) {
               Error("DetectParticle",
                     "Cannot find detector %s corresponding to particle energy loss %s",
                     det_name.Data(), pname.Data());
            }
            else {
               Double_t de = param->GetDouble();
               NVL.SetValue(curDet->GetName(), de);
            }
         }
      }
   }

   // add hit group to list if not already in it
   if (last_detector) fHitGroups.AddGroup(last_detector->GetGroup());

   return NVL;
}

KVNameValueList KVDetectionSimulator::DetectParticleIn(const Char_t* detname, KVNucleus* kvp)
{
   // Given the name of a detector, simulate detection of a given particle
   // by the complete corresponding group. The particle's theta and phi are set
   // at random within the limits of detector entrance window
   //
   // Returns a list containing the name and energy loss of each
   // detector hit in array

   KVDetector* kvd = GetArray()->GetDetector(detname);
   if (kvd) {
      kvp->SetMomentum(kvp->GetEnergy(), kvd->GetRandomDirection("random"));
      return DetectParticle(kvp);
   }
   else {
      Error("DetectParticleIn", "Detector %s not found", detname);
   }
   return KVNameValueList();
}
