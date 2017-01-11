//Created by KVClassFactory on Tue Apr 16 09:45:50 2013
//Author: John Frankland,,,

#include "KVMultiDetArray.h"
#include "Riostream.h"
#include "TROOT.h"
#include "KVDetector.h"
#include "KVDetectorEvent.h"
#include "KVReconstructedEvent.h"
#include "KVReconstructedNucleus.h"
#include "KVACQParam.h"
#include "KVRList.h"
#include "KVLayer.h"
#include "KVEvent.h"
#include "KVNucleus.h"
#include "KVGroup.h"
#include "KVRing.h"
#include "KVTelescope.h"
#include "KVMaterial.h"
#include "KVTarget.h"
#include "KVIDTelescope.h"
#include "KV2Body.h"
#include <KVString.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <KVIDGridManager.h>
#include <KVDataSetManager.h>
#include <KVUpDater.h>
#include "TPluginManager.h"
#include "KVDataSet.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "KVHashList.h"
#include "KVNameValueList.h"
#include "KVUniqueNameList.h"
#include "KVIonRangeTable.h"
#include "KVRangeTableGeoNavigator.h"
#include <KVDataAnalyser.h>
#include <KVNamedParameter.h>
#ifdef WITH_OPENGL
#include <TGLViewer.h>
#include <TVirtualPad.h>
#endif
using namespace std;

KVMultiDetArray* gMultiDetArray = 0x0;

Bool_t KVMultiDetArray::fCloseGeometryNow = kTRUE;
Bool_t KVMultiDetArray::fBuildTarget = kFALSE;

ClassImp(KVMultiDetArray)
////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVMultiDetArray</h2>
<h4>Multidetector array base class</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVMultiDetArray::KVMultiDetArray()
   : KVGeoStrucElement(), fTrajectories(kTRUE)
{
   // Default constructor
   init();
   gMultiDetArray = this;
}

void KVMultiDetArray::init()
{
   //Basic initialisation called by constructor.
   //Creates detectors list fDetectors,
   //groups list fGroups, identification telescopes list
   //fIDTelescopes
   //
   //Cleanups
   //The fDetectors and fACQParams lists contain references to objects which are
   //referenced & owned by other objects. We use the ROOT automatic garbage collection
   //to make sure that any object deleted elsewhere is removed automatically from these lists.
   //
   //The fGroups & fIDTelescopes lists contain objects owned by the multidetector array,
   //but which may be deleted by other objects (or as a result of the deletion of other
   //objects: i.e. if all the detectors in a group are deleted, the group itself is destroyed).
   //We use the ROOT automatic garbage collection to make sure that any object deleted
   //elsewhere is removed automatically from these lists.

   fIDTelescopes = new KVHashList();
   fIDTelescopes->SetOwner(kTRUE); // owns its objects
   fIDTelescopes->SetCleanup(kTRUE);

   fHitGroups = 0;

   fACQParams = 0;
   fTarget = 0;
   fCurrentRun = 0;

   fStatusIDTelescopes = 0;
   fCalibStatusDets = 0;
   fSimMode = kFALSE;

   fROOTGeometry = gEnv->GetValue("KVMultiDetArray.ROOTGeometry", kTRUE);
   fFilterType = kFilterType_Full;

//   fGeoManager = 0;
   fNavigator = 0;
   fUpDater = 0;

   if (!gIDGridManager) new KVIDGridManager;

   // all trajectories belong to us
   fTrajectories.SetOwner();
}

//___________________________________________________________________________________
KVMultiDetArray::~KVMultiDetArray()
{
   //destroy (delete) the MDA and all the associated structure, detectors etc.

   SafeDelete(fHitGroups);
   //destroy all identification telescopes
   if (fIDTelescopes && fIDTelescopes->TestBit(kNotDeleted)) {
      fIDTelescopes->Delete();
      delete fIDTelescopes;
   }
   fIDTelescopes = 0;

   //clear list of acquisition parameters
   if (fACQParams && fACQParams->TestBit(kNotDeleted)) {
      fACQParams->Clear();
      delete fACQParams;
   }
   fACQParams = 0;

   if (fTarget) {
      delete fTarget;
      fTarget = 0;
   }
   if (gMultiDetArray == this)
      gMultiDetArray = 0;

   if (fStatusIDTelescopes) {
      fStatusIDTelescopes->Delete();
      delete fStatusIDTelescopes;
      fStatusIDTelescopes = 0;
   }
   if (fCalibStatusDets) {
      fCalibStatusDets->Delete();
      delete fCalibStatusDets;
      fCalibStatusDets = 0;
   }

//   SafeDelete(fGeoManager);
   SafeDelete(fNavigator);
   SafeDelete(fUpDater);

   // Detectors belong to multidetector array. Our responsibility to delete.
   fDetectors.Delete();
}


//_______________________________________________________________________________________

void KVMultiDetArray::Build(Int_t)
{



}

//_______________________________________________________________________________________

Int_t KVMultiDetArray::GetIDTelescopes(KVDetector* de, KVDetector* e, TCollection* list)
{
   // Create one or more KVIDTelescope particle-identification objects from the two detectors
   //
   // The different ID telescopes are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   // # The KVMultiDetArray::GetIDTelescopes(KVDetector*de, KVDetector*e) method uses these plugins to
   // # create KVIDTelescope instances adapted to the specific array geometry and detector types.
   // # For each pair of detectors we look for a plugin with one of the following names:
   // #    [name_of_dataset].de_detector_type[de detector thickness]-e_detector_type[de detector thickness]
   // # Each characteristic in [] brackets may or may not be present in the name; first we test for names
   // # with these characteristics, then all combinations where one or other of the characteristics is not present.
   // # In addition, we first test all combinations which begin with [name_of_dataset].
   // # The first plugin found in this order will be used.
   // # In addition, if for one of the two detectors there is a plugin called
   // #    [name_of_dataset].de_detector_type[de detector thickness]
   // #    [name_of_dataset].e_detector_type[e detector thickness]
   // # then we add also an instance of this 1-detector identification telescope.
   //
   // This method is called by DeduceIdentificationTelescopesFromGeometry
   // in order to set up all ID telescopes of the array.
   //
   // Returns number of ID telescopes created

   Int_t ntels = 0;
   // if both detectors are not OK then stop
   if (!de->IsOK() && !e->IsOK()) return ntels;

   if (fDataSet == "" && gDataSet) fDataSet = gDataSet->GetName();

   //look for ID telescopes with only one of the two detectors
   if (de->IsOK()) ntels += try_all_singleID_telescopes(de, list);
   if (e->IsOK() && de != e) ntels += try_all_singleID_telescopes(e, list);

   if (de != e && e->IsOK() && de->IsOK()) ntels += try_all_doubleID_telescopes(de, e, list);

   return ntels;
}

Int_t KVMultiDetArray::try_all_singleID_telescopes(KVDetector* d, TCollection* l)
{
   // Attempt to find a plugin KVIDTelescope class for making a single-detector
   // ID telescope from detector *d
   // We look for plugins with the following signatures (uri):
   //
   //       [type]
   //       [type][thickness]
   //
   // where 'type' is the type of the detector in UPPER or lowercase letters
   // 'thickness' is the nearest-integer thickness of the detector as returned by d->GetThickness()
   // In addition, if a dataset is set (gDataSet!=nullptr) we try also for dataset-specific
   // plugins:
   //
   //       [dataset].[type]
   //       [dataset].[type][thickness]
   //
   // Returns number of generated telescopes

   TString uri = d->GetType();
   Int_t ntels = 0;
   if (!(ntels += try_upper_and_lower_singleIDtelescope(uri, d, l))) {
      Int_t d_thick = TMath::Nint(d->GetThickness());
      uri += d_thick;
      ntels += try_upper_and_lower_singleIDtelescope(uri, d, l);
   }
   return ntels;
}


Int_t KVMultiDetArray::try_all_doubleID_telescopes(KVDetector* de, KVDetector* e, TCollection* l)
{
   // Attempt to find a plugin KVIDTelescope class for making an ID telescope from detectors de & e.
   // We look for plugins with the following signatures (uri):
   //
   //       [de-type]-[e-type]
   //       [de-type][thickness]-[e-type]
   //       [de-type]-[e-type][thickness]
   //       [de-type][thickness]-[e-type][thickness]
   //
   // where 'type' is the type of the detector in UPPER or lowercase letters
   // 'thickness' is the nearest-integer thickness of the detector.
   // In addition, if a dataset is set (gDataSet!=nullptr) we try also for dataset-specific
   // plugins:
   //
   //       [dataset].[de-type]-[e-type]
   //       [dataset].[de-type][thickness]-[e-type]
   //       [dataset].[de-type]-[e-type][thickness]
   //       [dataset].[de-type][thickness]-[e-type][thickness]
   //
   // if no plugin is found, we return a KVIDTelescope base class object
   //
   // Returns 1 (we always generate exactly one telescope)

   TString de_type = de->GetType();
   TString e_type = e->GetType();
   TString de_thick = Form("%d", TMath::Nint(de->GetThickness()));
   TString e_thick = Form("%d", TMath::Nint(e->GetThickness()));

   TString uri = de_type + "-" + e_type;
   if (try_upper_and_lower_doubleIDtelescope(uri, de, e, l)) return 1;

   uri = de_type + de_thick + "-" + e_type;
   if (try_upper_and_lower_doubleIDtelescope(uri, de, e, l)) return 1;

   uri = de_type + "-" + e_type + e_thick;
   if (try_upper_and_lower_doubleIDtelescope(uri, de, e, l)) return 1;

   uri = de_type + de_thick + "-" + e_type + e_thick;
   if (try_upper_and_lower_doubleIDtelescope(uri, de, e, l)) return 1;

   // default id telescope object
   KVIDTelescope* idt = new KVIDTelescope;
   uri = de_type + "-" + e_type;
   idt->SetLabel(uri);
   set_up_telescope(de, e, idt, l);

   return 1;
}

bool KVMultiDetArray::try_upper_and_lower_singleIDtelescope(TString uri, KVDetector* d, TCollection* l)
{
   // Attempt to find a plugin KVIDTelescope class for making a single-detector
   // ID telescope from detector *d with the given signature/uri
   // Both original & all-upper-case versions of uri are tried.
   // uri is tried both with & without prepended dataset name (if set)
   // Returns true if successful (the new ID telescope will be added to internal
   // list fIDTelescopes and also to TCollection* l)

   if (try_a_singleIDtelescope(uri, d, l)) return true;
   uri.ToUpper();
   return try_a_singleIDtelescope(uri, d, l);
}

bool KVMultiDetArray::try_upper_and_lower_doubleIDtelescope(TString uri, KVDetector* de, KVDetector* e, TCollection* l)
{
   // Attempt to find a plugin KVIDTelescope class for making an ID telescope with the given signature/uri
   // Both original & all-upper-case versions of uri are tried.
   // uri is tried both with & without prepended dataset name (if set)
   // Returns true if successful (the new ID telescope will be added to internal
   // list fIDTelescopes and also to TCollection* l)

   if (try_a_doubleIDtelescope(uri, de, e, l)) return true;
   uri.ToUpper();
   return try_a_doubleIDtelescope(uri, de, e, l);
}

bool KVMultiDetArray::try_a_singleIDtelescope(TString uri, KVDetector* d, TCollection* l)
{
   // Attempt to find a plugin KVIDTelescope class for making a single-detector
   // ID telescope from detector *d with the given signature/uri
   // Both original & all-upper-case versions of uri are tried.
   // uri is tried both with & without prepended dataset name (if set)
   // Returns true if successful (the new ID telescope will be added to internal
   // list fIDTelescopes and also to TCollection* l)

   // dataset-specific version takes precedence over default
   TString duri = uri;
   if (gDataSet) {
      // try with dataset name
      duri.Prepend(Form("%s.", fDataSet.Data()));
      KVIDTelescope* idt;
      if ((idt = KVIDTelescope::MakeIDTelescope(duri))) {
         set_up_single_stage_telescope(d, idt, l);
         return true;
      }
   }

   // look for default version
   KVIDTelescope* idt;
   if ((idt = KVIDTelescope::MakeIDTelescope(uri))) {
      set_up_single_stage_telescope(d, idt, l);
      return true;
   }

   return false;
}

bool KVMultiDetArray::try_a_doubleIDtelescope(TString uri, KVDetector* de, KVDetector* e, TCollection* l)
{
   // Attempt to find a plugin KVIDTelescope class for making an ID telescope with the given signature/uri
   // uri is tried both with & without prepended dataset name (if set)
   // Returns true if successful (the new ID telescope will be added to internal
   // list fIDTelescopes and also to TCollection* l)

   // dataset-specific version takes precedence over default
   TString duri = uri;
   if (gDataSet) {
      // try with dataset name
      duri.Prepend(Form("%s.", fDataSet.Data()));
      KVIDTelescope* idt;
      if ((idt = KVIDTelescope::MakeIDTelescope(duri))) {
         set_up_telescope(de, e, idt, l);
         return true;
      }
   }
   // look for default version
   KVIDTelescope* idt;
   if ((idt = KVIDTelescope::MakeIDTelescope(uri))) {
      set_up_telescope(de, e, idt, l);
      return true;
   }

   return false;
}

void KVMultiDetArray::set_up_telescope(KVDetector* de, KVDetector* e, KVIDTelescope* idt, TCollection* l)
{
   // Set up detectors in de-e identification telescope and add to fIDTelescopes and to l

   idt->AddDetector(de);
   idt->AddDetector(e);
   if (de->GetGroup()) {
      idt->SetGroup(de->GetGroup());
   } else {
      idt->SetGroup(e->GetGroup());
   }
   // if telescope already exists, we delete this new version and add a reference to
   // the original into list l
   KVIDTelescope* p = (KVIDTelescope*)fIDTelescopes->FindObject(idt->GetName());
   if (p) {
      l->Add(p);
      delete idt;
   } else {
      fIDTelescopes->Add(idt);
      l->Add(idt);
   }
}

void KVMultiDetArray::set_up_single_stage_telescope(KVDetector* det, KVIDTelescope* idt, TCollection* l)
{
   // Set up detector in single-stage identification telescope and add to fIDTelescopes and to l

   idt->AddDetector(det);
   idt->SetGroup(det->GetGroup());
   // if telescope already exists, we delete this new version and add a reference to
   // the original into list l
   KVIDTelescope* p = (KVIDTelescope*)fIDTelescopes->FindObject(idt->GetName());
   if (p) {
      l->Add(p);
      delete idt;
   } else {
      fIDTelescopes->Add(idt);
      l->Add(idt);
   }
}
//______________________________________________________________________________________
void KVMultiDetArray::CreateIDTelescopesInGroups()
{
   fIDTelescopes->Delete();     // clear out (delete) old identification telescopes
   KVGroup* grp;
   unique_ptr<KVSeqCollection> fGroups(GetStructures()->GetSubListWithType("GROUP"));
   TIter ngrp(fGroups.get());
   while ((grp = (KVGroup*) ngrp())) {
      GetIDTelescopesForGroup(grp, fIDTelescopes);
   }
}

//_______________________________________________________________________________________

void KVMultiDetArray::RenumberGroups()
{
   //Number groups according to position in list fGroups and set fGr counter to the number
   //of groups in the list
   Int_t fGr = 0;
   KVGroup* g = 0;
   KVSeqCollection* fGroups = GetStructures()->GetSubListWithType("GROUP");
   TIter next(fGroups);
   while ((g = (KVGroup*) next())) {
      g->SetNumber(++fGr);
   }
   delete fGroups;
}

//__________________________________________________________________________________

Int_t KVMultiDetArray::FilteredEventCoherencyAnalysis(Int_t round, KVReconstructedEvent* rec_event)
{
   // Perform multi-hit and coherency analysis of filtered event.
   // round=1,2,... depending on number of times method has been called
   // Returns number of particles whose status were changed.
   // When this becomes zero, the coherency analysis is finished

   Int_t nchanged = 0;
   KVReconstructedNucleus* recon_nuc;

   while ((recon_nuc = rec_event->GetNextParticle())) {
      if (!recon_nuc->IsIdentified()) {
         int dethits = recon_nuc->GetStoppingDetector()->GetHits()->GetEntries() ;
         KVIDTelescope* idtelstop = ((KVIDTelescope*)recon_nuc->GetStoppingDetector()->GetTelescopesForIdentification()->First());
         Bool_t pileup = kFALSE;
         if (dethits > 1) {
            // if any of the other particles also stopped in the same detector, we assume identification
            // will be so false as to reject particle
            for (int j = 0; j < dethits; j++) {
               KVReconstructedNucleus* n = (KVReconstructedNucleus*)recon_nuc->GetStoppingDetector()->GetHits()->At(j);
               if (n != recon_nuc && n->GetStoppingDetector() == recon_nuc->GetStoppingDetector()) pileup = kTRUE;
            }
         }
         if (pileup) {
            nchanged++;
            recon_nuc->SetIsIdentified();//to stop looking anymore & to allow identification of other particles in same group
            if (idtelstop) idtelstop->SetIDCode(recon_nuc, idtelstop->GetBadIDCode());
            else recon_nuc->SetIsOK(kFALSE);
         } else if (recon_nuc->GetStatus() == 3) {
            //stopped in first member
            recon_nuc->SetIsIdentified();
            nchanged++;
         } else if (recon_nuc->GetStatus() == 2) {
            // pile-up in first stage of telescopes
            recon_nuc->SetIsIdentified();
            if (idtelstop && idtelstop->IsReadyForID()) idtelstop->SetIDCode(recon_nuc, idtelstop->GetMultiHitFirstStageIDCode());
            else recon_nuc->SetIsOK(kFALSE);
            nchanged++;
         } else if (recon_nuc->GetStatus() == 0) {
            // try to "identify" the particle
            TIter nxtidt(recon_nuc->GetStoppingDetector()->GetTelescopesForIdentification());
            idtelstop = (KVIDTelescope*)nxtidt();
            while (idtelstop) {
               if (idtelstop->IsReadyForID() && idtelstop->CanIdentify(recon_nuc->GetZ(), recon_nuc->GetA())
                     && idtelstop->CheckTheoreticalIdentificationThreshold(recon_nuc)) { // make sure we are above identification threshold
                  nchanged++;
                  // if this is not the first round, this particle has been 'identified' after
                  // dealing with other particles in the group
                  if (round > 1) idtelstop->SetIDCode(recon_nuc, idtelstop->GetCoherencyIDCode());
                  recon_nuc->SetIsIdentified();
                  recon_nuc->SetIsCalibrated();
                  recon_nuc->SetZMeasured();
                  recon_nuc->SetAMeasured();
                  break;
               } else {
                  Int_t nseg = recon_nuc->GetNSegDet();
                  recon_nuc->SetNSegDet(TMath::Max(nseg - 1, 0));
                  //if there are other unidentified particles in the group and NSegDet is < 2
                  //then exact status depends on segmentation of the other particles : reanalyse
                  if (recon_nuc->GetNSegDet() < 2 &&
                        KVReconstructedNucleus::GetNUnidentifiedInGroup(recon_nuc->GetGroup()) > 1) {
                     nchanged++;
                     break;
                  }
                  //if NSegDet = 0 it's hopeless
                  if (!recon_nuc->GetNSegDet()) {
                     nchanged++;
                     break;
                  }
               }
               idtelstop = (KVIDTelescope*)nxtidt();
            }
         }
      }
   }
   return nchanged;
}

void KVMultiDetArray::DetectEvent(KVEvent* event, KVReconstructedEvent* rec_event, const Char_t* detection_frame)
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
   //different members of the group is simulated. The actual geometry and algorithms used to calculate the particle's trajectory depend on the
   //value of fROOTGeometry (modify with SetROOTGeometry(kTRUE/kFALSE):
   //       if kTRUE: we use the ROOT TGeo description of the array generated by CreateGeoManager(). In this case, CreateGeoManager()
   //           must be called once in order to set up the geometry.
   //       if kFALSE: we use the simplistic KaliVeda geometry, i.e. detectors are only defined by min/max polar & azimuthal angles.
   //
   //The detectors concerned have their fEloss members set to the energy lost by the particle when it crosses them.
   //
   //Give tags to the simulated particles via KVNucleus::AddGroup() method
   //Two general tags :
   //   - DETECTED : cross at least one active layer of one detector
   //   - UNDETECTED : go through dead zone or stopped in target
   //We add also different sub group :
   //   - For UNDETECTED particles : "NO HIT", "NEUTRON", "DEAD ZONE", "STOPPED IN TARGET" and "THRESHOLD", the last one concerned particle
   //go through the first detection stage of the multidetector array but stopped in an absorber (ie an inactive layer)
   //   - For DETECTED particles :
   //         "PUNCH THROUGH" corrresponds to particle which cross all the materials in front of it
   //         (high energy particle punh through), or which miss some detectors due to a non perfect
   //         overlap between defined telescope,
   //         "INCOMPLETE"  corresponds to particles which stopped in the first detection stage of the multidetector
   //         in a detector which can not give alone a clear identification,
   //         this correponds to status=3 or idcode=5 in INDRA data
   //
   //After the filtered process, a reconstructed event are obtain from the fired groups corresponding
   //to detection group where at least one detector havec an active layer energy loss greater than zero
   //this reconstructed event are available for the user in the KVReconstructedEvent* rec_event argument
   //This pointer is cleared and also the multidet array at the beginning of the method
   //
   //INFO to the user :
   // -    at this point the different PILE-UP, several particles going through same telescope, detector
   //      are only taken into account for filter type KVMultiDetArray::kFilterType_Full
   // -    specific cases correponding specific multi detectors are to be implemented in the child class
   //
   // === FILTER TYPES ===
   // Use gMultiDetArray->SetFilterType(...) with one of the following values:
   //    KVMultiDetArray::kFilterType_Geo               geometric filter only, particles are kept if they hit detector in the array
   //                                                                            energy losses are not calculated, particle energies are irrelevant
   //    KVMultiDetArray::kFilterType_GeoThresh    particles are kept if they have enough energy to leave the target,
   //                                                                            and enough energy to cross at least one detector of the array
   //       ---> for these two cases, accepted particles are copied into the 'reconstructed' event with their
   //             original simulated energy, charge and mass, although angles are randomized to reflect detector granularity
   //
   //    KVMultiDetArray::kFilterType_Full               full simulation of detection of particles by the array. the calibration parameters
   //                                                                           for the chosen run (call to gMultiDetArray->SetParameters(...)) are inverted in order
   //                                                                           to calculate pseudo-raw data from the calculated energy losses. the resulting pseudo-raw
   //
   // SIMULATED EVENT PARAMETERS
   // ==========================
   // The event given as input to the method may contain extra information in its parameter
   // list concerning the simulation. The parameter list of the event is copied into that of the
   // reconstructed event, therefore these informations can be accessed from the reconstructed
   // event using the method
   //     rec_event->GetParameters()
   //
   // SIMULATED PARTICLE PARAMETERS
   // ==========================
   // For filter types KVMultiDetArray::kFilterType_Geo and KVMultiDetArray::kFilterType_GeoThresh,
   // for which there is a 1-to-1 correspondance between simulated and reconstructed particles,
   // we copy the list of parameters associated to each input particle into the output particle.
   // Note that this list contains full informations on the detection of each particle
   // (see Users Guide chapter on Filtering)
   //
   // TRACKING (only with ROOT geometries)
   // ====================================
   // You can visualise the trajectories of particles for individual events in the 3D OpenGL viewer.
   // To enable this:
   //     gMultiDetArray->GetNavigator()->SetTracking(kTRUE)
   // Then you can do:
   //     gMultiDetArray->DetectEvent(...)
   //     gMultiDetArray->Draw("tracks")
   // The geometry of the array with the tracks overlaid will be displayed.

   if (!event) {
      Error("DetectEvent", "the KVEvent object pointer has to be valid");
      return;
   }
   if (!rec_event) {
      Error("DetectEvent", "the KVReconstructedEvent object pointer has to be valid");
      return;
   }

   if (IsROOTGeometry()) {
      if (!gGeoManager) {
         Error("DetectEvent", "ROOT geometry is requested, but has not been set: gGeoManager=0x0");
         return;
      }
      // set up geometry navigator
      if (!fNavigator) fNavigator = new KVRangeTableGeoNavigator(gGeoManager, KVMaterial::GetRangeTable());
      if (fNavigator->IsTracking()) {
         // clear any tracks created by last event
         gGeoManager->ClearTracks();
         fNavigator->ResetTrackID();
      }
   }

   //Clear the KVReconstructed pointer before a new filter process
   rec_event->Clear();
   //Copy any parameters associated with simulated event into the reconstructed event
   event->GetParameters()->Copy(*(rec_event->GetParameters()));
   if (!fHitGroups) {
      //Create the list where fired groups will be stored
      //for reconstruction
      fHitGroups = new KVDetectorEvent;
   } else {
      //Clear the multidetector before a new filter process
      fHitGroups->Clear();
   }

   TObjArray* toks = 0;

   // iterate through list of particles
   KVNucleus* part, *_part;
   KVNameValueList* det_stat = new KVNameValueList();
   KVNameValueList* nvl = 0;
   KVNameValueList* un = 0;
   if (fFilterType == kFilterType_Full) un = new KVNameValueList();

   while ((part = event->GetNextParticle())) {  // loop over particles

      TList* lidtel = 0;

#ifdef KV_DEBUG
      cout << "DetectEvent(): looking at particle---->" << endl;
      part->Print();
#endif
      _part = (KVNucleus*)part->GetFrame(detection_frame);
      _part->SetE0();
      det_stat->Clear();
      Double_t eLostInTarget = 0;
      KVDetector* last_det = 0;

      if (part->GetZ() && !fNavigator->CheckIonForRangeTable(part->GetZ(), part->GetA())) {
         // ignore charged particles which range table cannot handle
         det_stat->SetValue("UNDETECTED", Form("Z=%d", part->GetZ()));

         part->AddGroup("UNDETECTED");
         part->AddGroup("SUPERHEAVY");
      } else if (!fNavigator->IsTracking() && (part->GetZ() == 0)) {
         // when tracking is activated, we follow neutron trajectories
         // if not, we don't even bother trying
         det_stat->SetValue("UNDETECTED", "NEUTRON");

         part->AddGroup("UNDETECTED");
         part->AddGroup("NEUTRON");
      } else if (_part->GetKE() < 1.e-3) {
         det_stat->SetValue("UNDETECTED", "NO ENERGY");

         part->AddGroup("UNDETECTED");
         part->AddGroup("NO ENERGY");
      } else {

         //Double_t eLostInTarget=0;
         if (fTarget && part->GetZ()) {
            fTarget->SetOutgoing(kTRUE);
            //simulate passage through target material
            Double_t ebef = _part->GetKE();
            if (fFilterType != kFilterType_Geo) fTarget->DetectParticle(_part);
            eLostInTarget = ebef - _part->GetKE();
            if (_part->GetKE() < 1.e-3) {
               det_stat->SetValue("UNDETECTED", "STOPPED IN TARGET");

               part->AddGroup("UNDETECTED");
               part->AddGroup("STOPPED IN TARGET");
            }
            fTarget->SetOutgoing(kFALSE);
         }

         if ((fFilterType != kFilterType_Geo) && _part->GetKE() < 1.e-3) {
            // unless we are doing a simple geometric filter, particles which
            // do not have the energy to leave the target are not detected
         } else {
            if (!(nvl = DetectParticle(_part))) {
               if (part->GetZ() == 0) {
                  // tracking
                  det_stat->SetValue("UNDETECTED", "NEUTRON");

                  part->AddGroup("UNDETECTED");
                  part->AddGroup("NEUTRON");
               } else {
                  det_stat->SetValue("UNDETECTED", "NO HIT");

                  part->AddGroup("UNDETECTED");
                  part->AddGroup("NO HIT");
               }

            } else if (nvl->GetNpar() == 0) {

               if (part->GetZ() == 0) {
                  // tracking
                  det_stat->SetValue("UNDETECTED", "NEUTRON");

                  part->AddGroup("UNDETECTED");
                  part->AddGroup("NEUTRON");
               } else {
                  part->AddGroup("UNDETECTED");
                  part->AddGroup("DEAD ZONE");

                  det_stat->SetValue("UNDETECTED", "DEAD ZONE");
               }
               delete nvl;
               nvl = 0;
            } else {
               Int_t nbre_nvl = nvl->GetNpar();
               KVString LastDet(nvl->GetNameAt(nbre_nvl - 1));
               last_det = GetDetector(LastDet.Data());
               TList* ldet = last_det->GetAlignedDetectors();
               TIter it1(ldet);

               Int_t ntrav = 0;
               KVDetector* dd = 0;
               if (!IsROOTGeometry()) {
                  //Test de la trajectoire coherente
                  while ((dd = (KVDetector*)it1.Next())) {
                     if (dd->GetHits()) {
                        if (dd->GetHits()->FindObject(_part))
                           ntrav += 1;
                        else if (dd->IsSmallerThan(last_det))
                           ntrav += 1;
                     } else {
                        if (dd->IsSmallerThan(last_det))
                           ntrav += 1;
                     }
                  }
               } else {
                  ntrav = ldet->GetEntries();
               }

               if (ntrav != ldet->GetEntries()) {

                  // la particule a une trajectoire
                  // incoherente, elle a loupe un detecteur avec une ouverture
                  // plus large que ceux a la suite ou la particule est passe
                  // (ceci peut etre du a un pb de definition de zone morte autour de certains detecteur)
                  //
                  // on retire la particule sur tout les detecteurs
                  // ou elle est enregistree et on retire egalement
                  // sa contribution en energie
                  //
                  // on assimile en fait ces particules a
                  // des particules arretes dans les zones mortes
                  // non detectees
                  it1.Reset();
                  //Warning("DetectEvent","trajectoire incoherente ...");
                  while ((dd = (KVDetector*)it1.Next()))
                     if (dd->GetHits() && dd->GetHits()->FindObject(_part)) {
                        if (nvl->HasParameter(dd->GetName())) {
                           Double_t el = dd->GetEnergy();
                           el -= nvl->GetDoubleValue(dd->GetName());
                           dd->SetEnergyLoss(el);
                           if (dd->GetNHits() == 1)
                              dd->SetEnergyLoss(0);
                        }
                        dd->GetHits()->Remove(_part);
                     }
                  det_stat->SetValue("UNDETECTED", "GEOMETRY INCOHERENCY");

                  part->AddGroup("UNDETECTED");
                  part->AddGroup("GEOMETRY INCOHERENCY");
               } else {

                  //On recupere les telescopes d identification
                  //associe au dernier detecteur touche
                  lidtel = last_det->GetTelescopesForIdentification();
                  if (lidtel->GetEntries() == 0 && last_det->GetEnergy() <= 0) {
                     //Arret dans un absorbeur
                     det_stat->SetValue("UNDETECTED", "THRESHOLD");

                     part->AddGroup("UNDETECTED");
                     part->AddGroup("THRESHOLD");

                     //On retire la particule du detecteur considere
                     //
                     last_det->GetHits()->Remove(_part);
                     //Warning("DetectEvent","threshold ...");
                  } else {
                     part->AddGroup("DETECTED");
                     det_stat->SetValue("DETECTED", "OK");
                     fHitGroups->AddGroup(last_det->GetGroup());

                     if (lidtel->GetEntries() > 0) {
                        //Il y a possibilite d identification
                     } else if (last_det->GetEnergy() > 0) {
                        //Il n'y a pas de possibilite d'identification
                        //arret dans le premier etage de detection
                        det_stat->SetValue("DETECTED", "INCOMPLETE");
                        part->AddGroup("INCOMPLETE");
                     } else {
                        Warning("DetectEvent", "Cas non prevu ....");
                        printf("last_det->GetName()=%s, lidtel->GetEntries()=%d, last_det->GetEnergy()=%lf\n",
                               last_det->GetName(),
                               lidtel->GetEntries(),
                               last_det->GetEnergy()
                              );
                     }

                     //Test d'une energie residuelle non nulle
                     //La particule n a pas ete arrete par le detecteur
                     if (_part->GetKE() > 1.e-3) {
                        //Pour ces deux cas
                        //on a une information incomplete
                        //pour la particule
                        if (nbre_nvl != Int_t(last_det->GetGroup()->GetNumberOfDetectorLayers())) {
                           //----
                           // Fuite,
                           // la particule a loupe des detecteurs normalement aligne
                           // avec le dernier par laquelle elle est passee
                           // (ceci peut etre du a un pb de definition de la geometrie)
                           part->RemoveGroup("DETECTED");
                           det_stat->RemoveParameter("DETECTED");

                           det_stat->SetValue("UNDETECTED", "GEOMETRY INCOHERENCY");
                           part->AddGroup("UNDETECTED");
                           part->AddGroup("GEOMETRY INCOHERENCY");
                           //Warning("DetectEvent","Fuite ......");
                        } else if (nbre_nvl) {
                           //----
                           // Punch Through,
                           // La particule est trop energetique, elle a traversee
                           // tout l'appareillage de detection
                           //Warning("DetectEvent","Punch Through ......");
                           part->AddGroup("PUNCH THROUGH");
                           det_stat->SetValue("DETECTED", "PUNCH THROUGH");
                           //Warning("DetectEvent","Punch Through ......");
                        }
                     } //fin du cas ou la particule avait encore de l energie apres avoir traverser l ensemble du detecteur
                  } //fin du cas ou la particule a laisse de l energie dans un detecteur
               } //fin du cas ou la trajectoire est coherente avec la geometrie
            } //fin du cas ou la particule a touche un detecteur au sens large
         } //fin de la condition (FilterType == kFilterType_Geo) || _part->GetKE()>1.e-3
      } // Fin du cas ou une particule avec une �nergie cin�tique est sortie d'une �ventuelle cible

      //On enregistre l eventuelle perte dans la cible
      if (fTarget)
         part->GetParameters()->SetValue("TARGET Out", eLostInTarget);
      //On enregistre le detecteur ou la particule s'arrete
      if (last_det)
         part->GetParameters()->SetValue("STOPPING DETECTOR", last_det->GetName());
      //On enregistre le telescope d'identification
      if (lidtel && lidtel->GetEntries()) {
         KVIDTelescope* theIDT = 0;
         TIter nextIDT(lidtel);
         while ((theIDT = (KVIDTelescope*)nextIDT())) {
            // make sure particle passed through telescope's detectors
            Int_t ndet = theIDT->GetSize();
            Int_t ntouche = 0;
            for (int i = 1; i <= ndet; i++) {
               if (nvl && nvl->HasParameter(Form("%s", theIDT->GetDetector(i)->GetName())))
                  ntouche++;
            }
            if (ntouche < ndet) continue;
            if (fFilterType == kFilterType_Geo ||
                  (theIDT->IsReadyForID() && theIDT->CanIdentify(part->GetZ(), part->GetA()))) {
               part->GetParameters()->SetValue("IDENTIFYING TELESCOPE", theIDT->GetName());
               break;
            }
         }
      }
      //On enregistre le statut de detection
      //dans l objet KVNucleus
      for (Int_t ii = 0; ii < det_stat->GetNpar(); ii += 1) {
         part->GetParameters()->SetValue(det_stat->GetNameAt(ii), det_stat->GetStringValue(ii));
      }
      //On enregistre les differentes pertes d'energie dans les detecteurs
      //dans l objet KVNucleus
      if (nvl) {

         for (Int_t ii = 0; ii < nvl->GetNpar(); ii += 1) {
            part->GetParameters()->SetValue(nvl->GetNameAt(ii), nvl->GetDoubleValue(ii));
            //On enregistre les detecteurs touches avec le Z et A de la particule
            //Si il y a plusieurs particules, on somme les Z et A de celles ci
            //Cela servira pour deduire les parametres d acquisition
            //printf("%s %d %d\n",nvl->GetNameAt(ii),part->GetZ(),part->GetA());
            //
            //Specifique au cas fFilterType == kFilterType_Full
            //
            if (fFilterType == kFilterType_Full) {
               if (un->HasParameter(nvl->GetNameAt(ii))) {
                  TString a_z(un->GetStringValue(nvl->GetNameAt(ii)));
                  toks = a_z.Tokenize(" ");
                  Int_t zz  = part->GetZ() + ((TObjString*)toks->At(0))->GetString().Atoi();
                  Int_t aa  = part->GetA() + ((TObjString*)toks->At(1))->GetString().Atoi();
                  un->SetValue(nvl->GetNameAt(ii), Form("%d %d", zz, aa));
                  delete toks;
               } else {
                  un->SetValue(nvl->GetNameAt(ii), Form("%d %d", part->GetZ(), part->GetA()));
               }
            }
         }
         delete nvl;
         nvl = 0;
      }

      _part->SetMomentum(*_part->GetPInitial());

   }    //fin de loop over particles

   delete det_stat;

   //   Info("DetectEvent", "Finished filtering event. Event status now:");
   //   event->Print();

   // EVENT RECONSTRUCTION FOR SIMPLE GEOMETRIC FILTER
   /*
      We keep all particles EXCEPT those belonging to groups
      "NO HIT" "DEAD ZONE" or "GEOMETRY INCOHERENCY"
   */
   if (fFilterType == kFilterType_Geo) {

      KVGroup* grp_tch;
      TIter nxt_grp(fHitGroups->GetGroups());
      while ((grp_tch = (KVGroup*) nxt_grp())) {
         grp_tch->ClearHitDetectors();
      }
      while ((part = event->GetNextParticle())) {
         if (part->BelongsToGroup("DETECTED") ||
               (part->BelongsToGroup("UNDETECTED") &&
                !part->BelongsToGroup("NO HIT") &&
                !part->BelongsToGroup("DEAD ZONE") &&
                !part->BelongsToGroup("GEOMETRY INCOHERENCY") &&
                !part->BelongsToGroup("NEUTRON") &&
                !part->BelongsToGroup("NO ENERGY"))
            ) {
            KVDetector* last_det = 0;
            if (part->GetParameters()->HasParameter("STOPPING DETECTOR"))
               last_det = GetDetector(part->GetParameters()->GetStringValue("STOPPING DETECTOR"));
            if (!last_det) continue;
            KVReconstructedNucleus* recon_nuc = (KVReconstructedNucleus*)rec_event->AddParticle();
            recon_nuc->Reconstruct(last_det);
            recon_nuc->SetZandA(part->GetZ(), part->GetA());
            recon_nuc->SetE(part->GetFrame(detection_frame)->GetE());
            // copy parameter list
            part->GetParameters()->Copy(*(recon_nuc->GetParameters()));
            if (part->GetParameters()->HasParameter("IDENTIFYING TELESCOPE")) {
               KVIDTelescope* idt = GetIDTelescope(part->GetParameters()->GetStringValue("IDENTIFYING TELESCOPE"));
               if (idt) {
                  recon_nuc->SetIdentifyingTelescope(idt);
                  idt->SetIDCode(recon_nuc, idt->GetIDCode());
                  recon_nuc->SetECode(idt->GetECode());
               }
            }
            recon_nuc->GetAnglesFromStoppingDetector();
         }
      }

      // analyse all groups & particles
      nxt_grp.Reset();
      while ((grp_tch = (KVGroup*) nxt_grp())) {
         KVReconstructedNucleus::AnalyseParticlesInGroup(grp_tch);
      }
      KVReconstructedNucleus* recon_nuc;
      while ((recon_nuc = rec_event->GetNextParticle())) {
         // check for undetectable pile-ups
         if (recon_nuc->GetStatus() == KVReconstructedNucleus::kStatusOK) {
            if (recon_nuc->GetStoppingDetector()->GetNHits() > 1)
               recon_nuc->SetStatus(KVReconstructedNucleus::kStatusPileupGhost);
            else {
               KVIDTelescope* idt = recon_nuc->GetIdentifyingTelescope();
               if (idt) {
                  KVDetector* de_det = idt->GetDetector(1);
                  if (de_det->GetNHits() > 1) recon_nuc->SetStatus(KVReconstructedNucleus::kStatusPileupDE);
               }
            }
         }
         recon_nuc->SetIsOK(kFALSE);
         if (recon_nuc->GetStatus() != KVReconstructedNucleus::kStatusPileupGhost) {
            recon_nuc->SetIsIdentified();
            recon_nuc->SetIsCalibrated();
            recon_nuc->SetIsOK();
            recon_nuc->SetZMeasured();
            recon_nuc->SetAMeasured();
         }
      }
      return;
   }
   // EVENT RECONSTRUCTION FOR SIMPLE GEOMETRIC FILTER WITH THRESHOLDS
   /*
      We keep all particles belonging to "DETECTED" group
      Those in "INCOMPLETE" group are treated as Zmin particles
   */
   if (fFilterType == kFilterType_GeoThresh) {
      // before reconstruction we have to clear the list of 'hits' of each detector
      // (they currently hold the addresses of the simulated particles which were detected)
      // which will be filled with the reconstructed particles, otherwise the number of hits
      // in each detector will be 2x the real value, and coherency analysis of the reconstructed
      // events will not work
      KVGroup* grp_tch;
      TIter nxt_grp(fHitGroups->GetGroups());
      while ((grp_tch = (KVGroup*) nxt_grp())) {
         grp_tch->ClearHitDetectors();
      }
      KVReconstructedNucleus* recon_nuc;
      while ((part = event->GetNextParticle())) {
         if (part->BelongsToGroup("DETECTED")) {
            KVDetector* last_det = 0;
            if (part->GetParameters()->HasParameter("STOPPING DETECTOR"))
               last_det = GetDetector(part->GetParameters()->GetStringValue("STOPPING DETECTOR"));
            if (!last_det || !(last_det->IsOK())) continue;

            recon_nuc = (KVReconstructedNucleus*)rec_event->AddParticle();
            recon_nuc->Reconstruct(last_det);
            recon_nuc->SetZandA(part->GetZ(), part->GetA());
            recon_nuc->SetE(part->GetFrame(detection_frame)->GetE());
            // copy parameter list
            part->GetParameters()->Copy(*(recon_nuc->GetParameters()));

            if (part->GetParameters()->HasParameter("IDENTIFYING TELESCOPE")) {
               KVIDTelescope* idt = GetIDTelescope(part->GetParameters()->GetStringValue("IDENTIFYING TELESCOPE"));
               if (idt) {
                  recon_nuc->SetIdentifyingTelescope(idt);
                  // for particles which are apprently well-identified, we
                  // check that they are in fact sufficiently energetic to be identified
                  if (!part->BelongsToGroup("INCOMPLETE")
                        && !idt->CheckTheoreticalIdentificationThreshold((KVNucleus*)part->GetFrame(detection_frame))) part->AddGroup("INCOMPLETE");
                  if (!part->BelongsToGroup("INCOMPLETE")) {
                     idt->SetIDCode(recon_nuc, idt->GetIDCode());
                  } else {
                     idt->SetIDCode(recon_nuc, idt->GetZminCode());
                  }
                  recon_nuc->SetECode(idt->GetECode());
                  //recon_nuc->SetIsIdentified();
                  //recon_nuc->SetIsCalibrated();
               }
            } else { /*if(part->BelongsToGroup("INCOMPLETE"))*/
               // for particles stopping in 1st member of a telescope, there is no "identifying telescope"
               KVIDTelescope* idt = (KVIDTelescope*)last_det->GetIDTelescopes()->First();
               if (idt) idt->SetIDCode(recon_nuc, idt->GetZminCode());
            }
            recon_nuc->GetAnglesFromStoppingDetector();
         }
      }
      // analyse all groups & particles
      nxt_grp.Reset();
      while ((grp_tch = (KVGroup*) nxt_grp())) {
         KVReconstructedNucleus::AnalyseParticlesInGroup(grp_tch);
      }

      // now perform mult-hit/coherency analysis until no further changes take place
      Int_t round = 1, nchanged;
      //cout << "SIM: " << event->GetMult() << " REC: " << rec_event->GetMult() << endl;
      //for(int i=1;i<=rec_event->GetMult();i++) cout << i << " Z=" << rec_event->GetParticle(i)->GetZ()<<" status="<<rec_event->GetParticle(i)->GetStatus() <<endl;
      do {
         nchanged = FilteredEventCoherencyAnalysis(round++, rec_event);
         //cout << "Round = " << round-1 << " :  nchanged = " << nchanged << endl;
         nxt_grp.Reset();
         while ((grp_tch = (KVGroup*) nxt_grp())) {
            KVReconstructedNucleus::AnalyseParticlesInGroup(grp_tch);
         }
         //for(int i=1;i<=rec_event->GetMult();i++) cout << i << " Z=" << rec_event->GetParticle(i)->GetZ()<<" status="<<rec_event->GetParticle(i)->GetStatus() <<endl;
      } while (nchanged);

      return;
   }

   if (fFilterType == kFilterType_Full) {
      //On calcule les parametres d acquisition
      //un->Print();
      KVDetector* det = 0;
      for (Int_t nn = 0; nn < un->GetNpar(); nn += 1) {

         det = GetDetector(un->GetNameAt(nn));
         TString a_z(un->GetStringValue(nn));
         toks = a_z.Tokenize(" ");
         Int_t zz  = ((TObjString*)toks->At(0))->GetString().Atoi();
         Int_t aa  = ((TObjString*)toks->At(1))->GetString().Atoi();

         det->DeduceACQParameters(zz, aa);
         delete toks;
      }
      delete un;

      // before reconstruction we have to clear the list of 'hits' of each detector
      // (they currently hold the addresses of the simulated particles which were detected)
      // which will be filled with the reconstructed particles, otherwise the number of hits
      // in each detector will be 2x the real value, and coherency analysis of the reconstructed
      // events will not work
      KVGroup* grp_tch;
      TIter nxt_grp(fHitGroups->GetGroups());
      while ((grp_tch = (KVGroup*) nxt_grp())) {
         grp_tch->ClearHitDetectors();
      }
      //    Info("DetectEvent", "Multidetector status before event reconstruction:");
      //    fHitGroups->Print();

      // reconstruct & identify the event
      ReconstructEvent(rec_event, fHitGroups);
      rec_event->IdentifyEvent();
      // calculate particle energies
      rec_event->CalibrateEvent();
      // 'coherency'
      rec_event->SecondaryIdentCalib();
      return;
   }

}
//__________________________________________________________________________________
KVNameValueList* KVMultiDetArray::DetectParticle_TGEO(KVNucleus* part)
{
   // Use ROOT geometry to propagate particle through the array,
   // calculating its energy losses in all absorbers, and setting the
   // energy loss members of the active detectors on the way.
   //
   // It is assumed that the ROOT geometry has been generated and is
   // pointed to by gGeoManager.
   //
   // Returns a list (KVNameValueList pointer) of the crossed detectors with their name and energy loss
   //   if particle hits detector in array, 0 if not (i.e. particle
   // in beam pipe or dead zone of the multidetector)
   // INFO User has to delete the KVNameValueList after its use

   // list of energy losses in active layers of detectors
   KVNameValueList* NVL = 0;
   if (!fNavigator) {
      Error("DetectParticle_TGEO", "No existing navigator ...");
      return 0;
   }
   fNavigator->PropagateParticle(part);

   // particle missed all detectors
   if (!part->GetParameters()->GetNpar()) return NVL;
   else {
      // find detectors in array hit by particle
      // and set their energies
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
               KVDetector* curDet = GetDetector(det_name);
               if (!curDet) {
                  Error("DetectParticle_TGEO",
                        "Cannot find detector %s corresponding to particle energy loss %s",
                        det_name.Data(), pname.Data());
               } else {
                  Double_t de = param->GetDouble();
                  if (!NVL) NVL = new KVNameValueList;
                  NVL->SetValue(curDet->GetName(), de);
               }
            }
         }
      }
   }
   return NVL;
}

//____________________________________________________________________________________________
void KVMultiDetArray::ReplaceDetector(const Char_t*,
                                      KVDetector*)
{
   //Replace (and destroy) the named detector in the array with a detector based on the prototype
   //given by the pointer new_kvd.

//    KVDetector *kvd = GetDetector(name);
//    if (!kvd) {
//        Warning("ReplaceDetector", "Detector %s not found", name);
//        return;
//    }
//    //get telescope of detector, so we know where to put the replacement
//    KVTelescope *tel = kvd->GetTelescope();
//    if (!tel) {
//        Error("ReplaceDetector", "Detector %s is not in a telescope", name);
//        return;
//    }
//    tel->ReplaceDetector(kvd, new_kvd);
   Warning("ReplaceDetector", "Needs reimplementing");
}

//____________________________________________________________________________________________

KVTelescope* KVMultiDetArray::GetTelescope(const Char_t*) const
{
   // Return pointer to telescope in array with name given by "name"
   return 0;
}

//____________________________________________________________________________________________
KVIDTelescope* KVMultiDetArray::GetIDTelescope(const Char_t* name) const
{
   //Return pointer to DeltaE-E ID Telescope with "name"

   return (KVIDTelescope*) fIDTelescopes->FindObject(name);
}


//_______________________________________________________________________________________

KVGroup* KVMultiDetArray::GetGroupWithDetector(const Char_t* name)
{
   //return pointer to group in array which contains detector or telescope
   //with name "name"

   KVTelescope* tel = 0;
   KVDetector* det = 0;
   KVGroup* grp = 0;
   if ((tel = GetTelescope(name))) {
      grp = (KVGroup*)tel->GetParentStructure("GROUP");
      return grp;
   }
   if ((det = GetDetector(name))) {
      grp = det->GetGroup();
   }
   return grp;
}

KVGroup* KVMultiDetArray::GetGroup(const Char_t* name)
{
   // Return pointer to group with name
   return (KVGroup*)GetStructure("GROUP", name);
}



//________________________________________________________________________________________
void KVMultiDetArray::MakeListOfDetectors()
{
   AbstractMethod("MakeListOfDetectors");
}

//_________________________________________________________________________________
void KVMultiDetArray::Clear(Option_t*)
{
   //Reset all groups (lists of detected particles etc.)
   //and detectors in groups (energy losses, ACQparams etc. etc.)
   //and the target if there is one

   unique_ptr<KVSeqCollection> fGroups(GetStructures()->GetSubListWithType("GROUP"));

   TIter next(fGroups.get());
   KVGroup* grp;
   while ((grp = (KVGroup*) next())) {
      grp->Reset();
   }
   if (GetTarget())
      GetTarget()->Clear();
}

//_________________________________________________________________________________________
void KVMultiDetArray::AddACQParam(KVACQParam* par)
{
   //Add an acquisition parameter corresponding to a detector of the array.
   //The fACQParams list is added to the list of cleanups (gROOT->GetListOfCleanups).
   //Each acq-param has its kMustCleanup bit set.
   //Thus, if the acq-param is deleted (e.g. by the detector which owns it), it is
   //automatically removed from the fACQParams list by ROOT.

   if (!fACQParams) {
      fACQParams = new KVHashList;
      fACQParams->SetName(Form("List of acquisition parameters for multidetector array %s", GetName()));
      fACQParams->SetOwner(kFALSE);
      fACQParams->SetCleanup(kTRUE);
   }
   if (par) {
      fACQParams->Add(par);
   } else
      Warning("AddACQParam", "Null pointer passed as argument");
}

//_________________________________________________________________________________________

void KVMultiDetArray::SetACQParams()
{
   // Set up acquisition parameters in all detectors of the array + any acquisition parameters which are not
   // directly related to a detector.
   //
   // Override the method SetArrayACQParams() in order to add any acquisition parameters not directly
   // related to a detector.
   //
   // For the detector acquisition parameters, we loop over all detectors of the array and call each detector's
   // SetACQParams() method, if it has not already been done (i.e. if the detector has no associated parameters).
   // Each specific implementation of a KVDetector class should redefine the KVDetector::SetACQParams()
   // method in order to give the detector in question the necessary acquisition parameters (KVACQParam objects).
   //
   // The list of acquisition parameters of each detector is then used to
   //   1) add to fACQParams list of all acquisition parameters of the array
   //   2) set as "not working" the acquisition parameters for which environment variables such as
   //        [dataset name].KVACQParam.[acq par name].Working:    NO
   //       are set in a .kvrootrc file.
   //   3) set bitmask for each detector used to determine which acquisition parameters are
   //       taken into account by KVDetector::Fired based on the environment variables
   //          [classname].Fired.ACQParameterList.[type]: PG,GG,T
   //   where [classname]=KVDetector by default, or the name of some class
   //   derived from KVDetector which calls the method KVDetector::SetKVDetectorFiredACQParameterListFormatString()
   //   in its constructor.

   if (fACQParams) {
      fACQParams->Clear();
   }

   SetArrayACQParams();

   TIter next(GetDetectors());
   KVDetector* det;
   while ((det = (KVDetector*) next())) {
      KVSeqCollection* l = det->GetACQParamList();
      if (!l) {
         //detector has no acq params
         //set up acqparams in detector
         det->SetACQParams();
         l = det->GetACQParamList();
      }
      //loop over acqparams and add them to fACQParams list, checking
      //their status (working or not working ?)
      TIter next_par(l);
      KVACQParam* par;
      while ((par = (KVACQParam*) next_par())) {
         AddACQParam(par);
         par->SetWorking(gDataSet->GetDataSetEnv(Form("KVACQParam.%s.Working", par->GetName()), kTRUE));
      }
      // set bitmask
      KVString inst;
      inst.Form(det->GetFiredACQParameterListFormatString(), det->GetType());
      KVString lpar = gDataSet->GetDataSetEnv(inst);
      det->SetFiredBitmask(lpar);
   }
}

//_________________________________________________________________________________

void KVMultiDetArray::SetArrayACQParams()
{
   // Method called by SetACQParams() in order to define any acquisition parameters which are not
   // directly related to any detectors of the array.
   // This implementation does nothing: override it in derived classes if needed.
}

//_________________________________________________________________________________

void KVMultiDetArray::SetCalibrators()
{
   //Set up calibrators in all detectors of the array
   //Note that this only initialises the calibrator objects associated to each
   //detector (defined in each detector class's SetCalibrators method),
   //it does not set the parameters of the calibrations: this is done by
   //SetParameters or SetRunCalibrationParameters

   const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetCalibrators)();
}

//_________________________________________________________________________________

void KVMultiDetArray::UpdateCalibrators()
{
   //This method can be used to update the calibrations of the detectors of a KVMultiDetArray
   //object which has been read back from a ROOT file, and was possibly written with an old
   //version of the class(es) which is now obsolete.
   //The existing calibrator objects (read in from the file) are first removed from the array's
   //detectors, and then SetCalibrators() is called in order to set new calibrators as defined
   //in the current version of the class(es).
   //In order to set the parameters of the new calibrators for a given run,
   //SetParameters or SetRunCalibrationParameters must be called after this method.

   const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, RemoveCalibrators)();
   SetCalibrators();
}

//_________________________________________________________________________________

void KVMultiDetArray::GetDetectorEvent(KVDetectorEvent* detev, TSeqCollection* fired_params)
{
   // First step in event reconstruction based on current status of detectors in array.
   // Fills the given KVDetectorEvent with the list of all groups which have fired.
   // i.e. loop over all groups of the array and test whether KVGroup::Fired() returns true or false.
   //
   // If the list of fired acquisition parameters 'fired_params' is given, then we use this list
   // to find, first, the associated fired detectors, then, the associated groups. This is possible when
   // reading raw data using an object derived from KVRawDataReader:
   //
   //     KVRawDataReader *run = ... ; // base pointer to object used to read data
   //     if( run->GetNextEvent() )  // read an event and test that all is well
   //      {
   //           KVSeqCollection* fired = run->GetFiredDataParameters(); // get fired acquisition parameters
   //           gMultiDetArray->GetDetectorEvent(detev, fired); // build list of fired groups
   //       }
   //
   // The KVDetectorEvent object can then be used by KVReconstructedEvent::ReconstructEvent
   // in order to generate a list of particles (KVReconstructedNucleus).
   //
   // Call method detev->Clear() before reading another event in order to reset all of the hit groups
   // (including all detectors etc.) and emptying the list.

   if (fired_params) {
      // list of fired acquisition parameters given
      TIter next_par(fired_params);
      KVACQParam* par = 0;
      KVDetector* det = 0;
      KVGroup* grp = 0;
      while ((par = (KVACQParam*)next_par())) {
         if ((det = par->GetDetector())) {
            if ((grp = det->GetGroup()) && grp->GetParents()->Contains(this)) detev->AddGroup(grp);
         }
      }
   } else {
      //loop over groups
      TSeqCollection* fGroups = GetStructures()->GetSubListWithType("GROUP");

      TIter next_grp(fGroups);
      KVGroup* grp;
      while ((grp = (KVGroup*) next_grp())) {
         if (grp->Fired()) {
            //if (!fHitGroups->FindObject(grp))
            // grp->Print();
            //add new group to list of hit groups
            detev->AddGroup(grp);
         }
      }
      delete fGroups;
   }
}

void KVMultiDetArray::ReconstructEvent(KVReconstructedEvent* recev, KVDetectorEvent* kvde)
{
   // Use the KVDetectorEvent (list of hit groups) in order to fill the
   // KVReconstructedEvent with reconstructed nuclei
   //
   // Reconstruction of detected particles
   //
   // - loop over last stage of group telescopes
   // - if any detector is hit, construct "particle" from energy losses in all detectors
   //   directly in front of it.
   // - loop over next to last stage...if any detector hit NOT ALREADY IN A "PARTICLE"
   //   construct "particle" etc. etc.

   KVGroup* grp_tch;
   TIter nxt_grp(kvde->GetGroups());
   while ((grp_tch = (KVGroup*) nxt_grp())) {
      AnalyseGroupAndReconstructEvent(recev, grp_tch);
   }
}

//________________________________________________________________________//

void KVMultiDetArray::AnalyseGroupAndReconstructEvent(KVReconstructedEvent* recev, KVGroup* grp)
{
   // Loop over detectors in group, starting from the furthest from the target,
   // and working inwards. Calls KVReconstructedEvent::AnalyseDetectors

   for (Int_t il = grp->GetNumberOfDetectorLayers(); il > 0; il--) {
      TList* dets = grp->GetDetectorsInLayer(il);
      recev->AnalyseDetectors(dets);
      delete dets;
   }
   //perform first-order coherency analysis (set fAnalStatus for each particle)
   KVReconstructedNucleus::AnalyseParticlesInGroup(grp);
}

//_________________________________________________________________________________


void KVMultiDetArray::SetTarget(const Char_t* material,
                                const Float_t thickness)
{
   //Define the target used for a given experimental set-up. For material names, see KVMaterial.
   //The thickness is in mg/cm2.
   //Use SetTarget(0) to remove the existing target.

   if (!material) {
      if (fTarget) {
         delete fTarget;
         fTarget = 0;
      }
      return;
   }
   if (!GetTarget())
      fTarget = new KVTarget(material, thickness);

}

void KVMultiDetArray::SetTarget(KVTarget* targ)
{
   //Adopt KVTarget object for use as experimental target i.e. we make a clone of the object pointed to by 'targ'.
   //Therefore, any subsequent modifications to the target should be made to the object whose pointer is returned by GetTarget().
   //This object will be deleted with the detector array, or when the target is changed.
   //
   //Calling SetTarget(0) will remove any existing target.

   if (fTarget) {
      delete fTarget;
      fTarget = 0;
   }
   if (!targ)
      return;
   fTarget = (KVTarget*) targ->Clone();
}

void KVMultiDetArray::SetTargetMaterial(const Char_t* material)
{
   //Define the target material used for a given experimental set-up.
   //For material names, see KVDetector.

   if (!GetTarget())
      fTarget = new KVTarget(material, 0.0);
}

void KVMultiDetArray::SetTargetThickness(const Float_t thickness)
{
   //Define the target thickness (mg/cm2) used for a given experimental set-up.
   //Need to define material first
   if (GetTarget()) {
      if (GetTarget()->GetLayerByIndex(1))
         GetTarget()->GetLayerByIndex(1)->SetThickness(thickness);
   }
}

void KVMultiDetArray::RemoveGroup(KVGroup*)
{
   //Remove (i.e. destroy) all the telescopes belonging to a given group
//    if (grp) {
//        grp->Destroy();
//    }
   Warning("RemoveGroup", "Needs reimplementing");
}

void KVMultiDetArray::RemoveGroup(const Char_t* name)
{
   //Remove (i.e. destroy) all the telescopes belonging to the group in array
   //which contains detector or telescope with name "name"
   KVGroup* grp = GetGroupWithDetector(name);
   if (grp) {
      RemoveGroup(grp);
   }
}

//_________________________________________________________________________________

void KVMultiDetArray::DetectParticleIn(const Char_t* detname,
                                       KVNucleus* kvp)
{
   //Given the name of a detector, simulate detection of a given particle
   //by the complete corresponding group. The particle's theta and phi are set
   //at random within the limits of detector entrance window

   KVDetector* kvd = GetDetector(detname);
   if (kvd) {
      KVNameValueList* nvl = 0;
      kvp->SetMomentum(kvp->GetEnergy(), kvd->GetRandomDirection("random"));
      if ((nvl = DetectParticle(kvp))) delete nvl;
   } else {
      Error("DetectParticleIn", "Detector %s not found", detname);
   }
}

//_________________________________________________________________________________

void KVMultiDetArray::SetPedestals(const Char_t* filename)
{
   //Set pedestals for all acquisition parameters whose name appears in the file 'filename'
   //The file should contain a line for each parameter beginning with:
   //parameter_name   pedestal_value
   //(anything else on the line will not be read)
   //Begin comment lines with '#'

   ifstream pedfile(filename);
   if (!pedfile.good()) {
      Error("SetPedestals", "File %s cannot be opened", filename);
      return;
   }

   KVString s;

   while (pedfile.good()) {

      s.ReadLine(pedfile);

      if (pedfile.good() && !s.BeginsWith("#")) {       //skip comments

         //break line into parts
         TObjArray* toks = s.Tokenize(" ");
         if (toks->GetSize() > 0) {
            TString name =
               ((TObjString*) toks->At(0))->GetString().
               Strip(TString::kBoth);
            KVString value(((TObjString*) toks->At(1))->GetString());

            KVACQParam* par = GetACQParam(name.Data());
            if (par)
               par->SetPedestal(value.Atof());
         }
         delete toks;
      }
   }
   pedfile.close();
}

//_________________________________________________________________________________

KVMultiDetArray* KVMultiDetArray::MakeMultiDetector(const Char_t* dataset_name, Int_t run, TString classname)
{
   //Static function which will create and 'Build' the multidetector object corresponding to
   //a given run of dataset 'dataset_name'. Any previously existing multidetector will be
   //deleted.
   //We first activate the given dataset if not already done
   //
   //Multidetector arrays are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   //
   //Plugin.KVMultiDet:    [dataset_name]    [classname]     [library]    "[constructor]()"
   //
   //The constructors/macros are always without arguments
   //
   //Dataset name is stored in fDataSet

   if (gMultiDetArray) {
      printf("Info in <KVMultiDetArray::MakeMultiDetector>: Deleting existing array %s\n", gMultiDetArray->GetName());
      delete gMultiDetArray;
      gMultiDetArray = nullptr;
      if (gIDGridManager) {
         delete gIDGridManager;
         gIDGridManager = nullptr;
      }
   }
   if (!gDataSet || gDataSet != gDataSetManager->GetDataSet(dataset_name))
      gDataSetManager->GetDataSet(dataset_name)->cd();

   // Creation of database when dataset is selected for first time may
   // include creation of multidetector array (by calling this method)
   KVMultiDetArray* mda = nullptr;
   if (!gMultiDetArray) {
      TPluginHandler* ph;
      if (!(ph = LoadPlugin(classname.Data(), dataset_name)))
         return nullptr;

      //execute constructor/macro for multidetector - assumed without arguments
      mda = (KVMultiDetArray*) ph->ExecPlugin(0);

      mda->fDataSet = dataset_name;
      mda->Build(run);
      // if ROOT geometry is requested by default (KVMultiDetArray.ROOTGeometry: yes)
      if (mda->fROOTGeometry) mda->CheckROOTGeometry();
   } else {
      mda = gMultiDetArray;
      // database creation may not have set the right run
      if (run > -1) mda->SetParameters(run);
   }
   return mda;
}

KVUpDater* KVMultiDetArray::GetUpDater()
{
   // Return pointer to KVUpDater defined by dataset for this multidetector
   // Will create a new KVUpDater if none exists

   if (!fUpDater) {
      fUpDater = KVUpDater::MakeUpDater(fDataSet);
   }
   return fUpDater;
}

//_________________________________________________________________________________

void KVMultiDetArray::SetParameters(UShort_t run)
{
   //Set identification and calibration parameters for run.
   //This can only be done if gDataSet has been set i.e. a dataset has been chosen
   //Otherwise this just has the effect of setting the current run number

   fCurrentRun = run;
   KVDataSet* ds = gDataSet;
   if (!ds) {
      if (gDataSetManager)
         ds = gDataSetManager->GetDataSet(fDataSet.Data());
   }
   if (ds) {
      GetUpDater()->SetParameters(run);
      SetBit(kParamsSet);
   }
}

//_________________________________________________________________________________

void KVMultiDetArray::SetRunIdentificationParameters(UShort_t run)
{
   //Set identification parameters for run.
   //This can only be done if gDataSet has been set i.e. a dataset has been chosen
   //Otherwise this just has the effect of setting the current run number

   fCurrentRun = run;
   KVDataSet* ds = gDataSet;
   if (!ds) {
      if (gDataSetManager)
         ds = gDataSetManager->GetDataSet(fDataSet.Data());
   }
   if (ds) {
      GetUpDater()->SetIdentificationParameters(run);
      SetBit(kIDParamsSet);
   }
}

//_________________________________________________________________________________

void KVMultiDetArray::SetRunCalibrationParameters(UShort_t run)
{
   //Set calibration parameters for run.
   //This can only be done if gDataSet has been set i.e. a dataset has been chosen
   //Otherwise this just has the effect of setting the current run number

   fCurrentRun = run;
   KVDataSet* ds = gDataSet;
   if (!ds) {
      if (gDataSetManager)
         ds = gDataSetManager->GetDataSet(fDataSet.Data());
   }
   if (ds) {
      GetUpDater()->SetCalibrationParameters(run);
      SetBit(kCalParamsSet);
   }
}

//_________________________________________________________________________________

void KVMultiDetArray::SetIdentifications()
{
   //Initialisation of all ACTIVE identification telescopes in the array, i.e. those appearing in a line
   //in the .kvrootrc file such as this:
   //
   //# [dataset name].ActiveIdentifications:    [type1]  [type2] ...
   //
   //The 'types' given correspond to the value given by KVIDTelescope::GetLabel(), these are the
   //identifiers used to retrieve the different plugin classes in GetIDTelescopes(KVDetector*,KVDetector*,KVList*).
   //
   //For each type of identification in the list, we retrieve the first identification telescope with this
   //label from the list of all ID telescopes, in order to call its KVIDTelescope::SetIdentificationParameters() method.
   //This method (when rederived in child classes of KVIDTelescope) initialises the identification objects
   //for ALL of the ID telescopes of the same type (class) in the array.
   //
   //Note that, in general, the parameters of the identifications for a given run are not
   //set until SetParameters or SetRunIdentificationParameters is called.

   TString id_labels = gDataSet->GetDataSetEnv("ActiveIdentifications");
   if (id_labels == "") {
      Info("SetIdentifications", "No active identifications");
      return;
   }
   //split list of labels
   TObjArray* toks = id_labels.Tokenize(' ');
   TIter next_lab(toks);
   TObjString* lab;
   //loop over labels/identification 'types'
   while ((lab = (TObjString*)next_lab())) {

      //get first telescope in list with right label
      KVIDTelescope* idt = (KVIDTelescope*)GetListOfIDTelescopes()->FindObjectByLabel(lab->String().Data());
      //set ID parameters for all telescopes of this 'type'
      if (idt) {
         Info("SetIdentifications", "Initialising %s identifications...", idt->GetLabel());
         if (idt->SetIdentificationParameters(this))
            Info("SetIdentifications", "OK");
      }

   }
   delete toks;
}

//_________________________________________________________________________________

void KVMultiDetArray::UpdateIdentifications()
{
   //This method can be used to update the identifications of a KVMultiDetArray
   //object which has been read back from a ROOT file, and was possibly written with an old
   //version of the identification parameters which are now obsolete.
   //We remove/destroy the existing identification parameters and replace them with the current versions.
   //In order to set the parameters of the new identifications for a given run,
   //SetParameters or SetRunIdentificationParameters must be called after this method.

   //remove existing identification objects/parameters
   fIDTelescopes->R__FOR_EACH(KVIDTelescope, RemoveIdentificationParameters)();

   //reset identifications
   SetIdentifications();
}

//_________________________________________________________________________________

void KVMultiDetArray::UpdateIDTelescopes()
{
   //This method can be used to update the identification telescopes of a KVMultiDetArray
   //object which has been read back from a ROOT file, and was possibly written with an old
   //version of the classes which are now obsolete, or for which new KVIDTelescope classes
   //are now available.
   //In order to set the parameters of the new identifications for a given run,
   //SetParameters or SetRunIdentificationParameters must be called after this method.

   //destroy old ID telescopes
   fIDTelescopes->Delete();
   //now read list of groups and create list of ID telescopes
   KVGroup* grp;
   KVSeqCollection* fGroups = GetStructures()->GetSubListWithType("GROUP");

   TIter ngrp(fGroups);
   while ((grp = (KVGroup*) ngrp())) {
      GetIDTelescopesForGroup(grp, fIDTelescopes);
   }
   delete fGroups;
   //reset identifications
   SetIdentifications();
}

//_________________________________________________________________________________

void KVMultiDetArray::InitializeIDTelescopes()
{
   // Calls Initialize() method of each identification telescope (see KVIDTelescope
   // and derived classes). This is essential before identification of particles is attempted.

   fIDTelescopes->R__FOR_EACH(KVIDTelescope, Initialize)();
}

//_________________________________________________________________________________

void KVMultiDetArray::PrintStatusOfIDTelescopes()
{
   // Print full status report on ID telescopes in array, using informations stored in
   // fStatusIDTelescopes (see GetStatusOfIDTelescopes).

   if (!GetCurrentRunNumber()) {
      Info("PrintStatusOfIDTelescopes", "Cannot know status without knowing RUN NUMBER");
      return;
   }

   cout << endl << "-----STATUS OF IDENTIFICATION TELESCOPES FOR RUN "
        << GetCurrentRunNumber() << "------" << endl << endl;
   //get list of active telescopes
   TString id_labels = gDataSet->GetDataSetEnv("ActiveIdentifications");
   if (id_labels == "") {
      cout << " *** No active identifications *** " << endl;
      return;
   }
   //split list of labels
   TObjArray* toks = id_labels.Tokenize(' ');

   //update status infos
   GetStatusOfIDTelescopes();

   TIter next_type(fStatusIDTelescopes);
   TList* id_type_list = 0;
   while ((id_type_list = (TList*)next_type())) {

      cout << " *** " << id_type_list->GetName() << " Identifications -------------------" << endl;
      if (!toks->FindObject(id_type_list->GetName())) {
         cout << "      [NOT ACTIVE]" << endl;
      }
      TList* ok_list = (TList*)id_type_list->FindObject("OK");
      TList* notok_list = (TList*)id_type_list->FindObject("NOT OK");
      TList* print_list = 0;
      Int_t Nok = ok_list->GetEntries();
      Int_t Notok = notok_list->GetEntries();
      if (Nok && Notok) {
         if (Nok < Notok) print_list = ok_list;
         else print_list = notok_list;
      }
      if (Nok && (!Notok)) cout << "    ALL telescopes are OK" << endl;
      else if (Notok && (!Nok)) cout << "    NO telescopes are OK" << endl;
      else {
         cout << "    " << ok_list->GetEntries() << " telescopes are OK, "
              << notok_list->GetEntries() << " telescopes are NOT OK" << endl;
         cout << "    " << print_list->GetName() << " :" << endl;
         print_list->ls();
      }
      cout << endl;

   }
   delete toks;
}

//_________________________________________________________________________________

TList* KVMultiDetArray::GetStatusOfIDTelescopes()
{
   // Fill and return pointer to list fStatusIDTelescopes which contains
   // a list for each type of ID telescope in the array, each list contains a list
   // "OK" with the ID telescopes which have IsReadyForID()=kTRUE, and
   // a list "NOT OK" with the others.
   //
   // The returned TList object must not be deleted (it belongs to the KVMultiDetArray).

   if (!fStatusIDTelescopes) {
      fStatusIDTelescopes = new TList;
      fStatusIDTelescopes->SetOwner(kTRUE);
   } else {
      fStatusIDTelescopes->Delete();
   }
   if (!fIDTelescopes || !fIDTelescopes->GetEntries()) return fStatusIDTelescopes;
   TIter next(fIDTelescopes);
   KVIDTelescope* idt = 0;
   while ((idt = (KVIDTelescope*)next())) {

      TString id_type = idt->GetLabel();
      TList* id_type_list = (TList*)fStatusIDTelescopes->FindObject(id_type.Data());
      if (!id_type_list) {
         id_type_list = new TList;
         id_type_list->SetOwner(kTRUE);
         id_type_list->SetName(id_type.Data());
         fStatusIDTelescopes->Add(id_type_list);
         id_type_list->Add(new TList);
         ((TList*)id_type_list->At(0))->SetName("OK");
         id_type_list->Add(new TList);
         ((TList*)id_type_list->At(1))->SetName("NOT OK");
      }
      if (idt->IsReadyForID())
         ((TList*)id_type_list->FindObject("OK"))->Add(idt);
      else
         ((TList*)id_type_list->FindObject("NOT OK"))->Add(idt);
   }
   return fStatusIDTelescopes;
}

//_________________________________________________________________________________

KVList* KVMultiDetArray::GetIDTelescopeTypes()
{
   // Create, fill and return pointer to a list of TObjString containing the name of each type
   // of ID telescope in the array.
   //
   // Delete the KVList after use (it owns the TObjString objects)

   KVList* type_list = new KVList;
   if (!fIDTelescopes || !fIDTelescopes->GetEntries()) return type_list;
   TIter next(fIDTelescopes);
   KVIDTelescope* idt = 0;
   while ((idt = (KVIDTelescope*)next())) {
      if (!type_list->FindObject(idt->GetLabel())) {
         type_list->Add(new TObjString(idt->GetLabel()));
      }
   }
   return type_list;
}

//_________________________________________________________________________________

KVSeqCollection* KVMultiDetArray::GetIDTelescopesWithType(const Char_t* type)
{
   // Create, fill and return pointer to a list of KVIDTelescopes with
   // the given type in the array.
   // WARNING! - check pointer is not zero (we return NULL if ID telescopes
   // list is not defined or empty)
   //
   // Delete the KVList after use (it does not own the KVIDTelescopes).

   if (!fIDTelescopes || !fIDTelescopes->GetEntries()) return NULL;
   return fIDTelescopes->GetSubListWithLabel(type);
}

//_________________________________________________________________________________

TList* KVMultiDetArray::GetCalibrationStatusOfDetectors()
{
   // Fill and return pointer to list fCalibStatusDets which contains
   // a list for each type of detector in the array, each list contains a list
   // "OK" with the detectors which are calibrated, and
   // a list "NOT OK" with the others.
   //
   // The returned TList object must not be deleted (it belongs to the KVMultiDetArray).

   if (!fCalibStatusDets) {
      fCalibStatusDets = new TList;
      fCalibStatusDets->SetOwner(kTRUE);
   } else {
      fCalibStatusDets->Delete();
   }
   if (!GetDetectors()->GetEntries()) return fCalibStatusDets;
   TIter next(GetDetectors());
   KVDetector* det = 0;
   while ((det = (KVDetector*)next())) {

      TString type = det->GetType();
      TList* type_list = (TList*)fCalibStatusDets->FindObject(type.Data());
      if (!type_list) {
         type_list = new TList;
         type_list->SetOwner(kTRUE);
         type_list->SetName(type.Data());
         fCalibStatusDets->Add(type_list);
         type_list->Add(new TList);
         ((TList*)type_list->At(0))->SetName("OK");
         type_list->Add(new TList);
         ((TList*)type_list->At(1))->SetName("NOT OK");
      }
      if (det->IsCalibrated())
         ((TList*)type_list->FindObject("OK"))->Add(det);
      else
         ((TList*)type_list->FindObject("NOT OK"))->Add(det);
   }
   return fCalibStatusDets;
}

//_________________________________________________________________________________

void KVMultiDetArray::PrintCalibStatusOfDetectors()
{
   // Print full status report on calibration of detectors in array, using informations stored in
   // fCalibStatusDets (see GetCalibrationStatusOfDetectors).

   if (!GetCurrentRunNumber()) {
      Info("PrintCalibStatusOfDetectors", "Cannot know status without knowing RUN NUMBER");
      return;
   }

   cout << endl << "-----------STATUS OF CALIBRATIONS FOR RUN "
        << GetCurrentRunNumber() << "------------" << endl << endl;

   //update status infos
   GetCalibrationStatusOfDetectors();

   TIter next_type(fCalibStatusDets);
   TList* id_type_list = 0;
   while ((id_type_list = (TList*)next_type())) {

      cout << " *** " << id_type_list->GetName() << " Detectors -------------------" << endl;
      TList* ok_list = (TList*)id_type_list->FindObject("OK");
      TList* notok_list = (TList*)id_type_list->FindObject("NOT OK");
      TList* print_list = 0;
      Int_t Nok = ok_list->GetEntries();
      Int_t Notok = notok_list->GetEntries();
      if (Nok && Notok) {
         if (Nok < Notok) print_list = ok_list;
         else print_list = notok_list;
      }
      if (Nok && (!Notok)) cout << "    ALL calibrations are OK" << endl;
      else if (Notok && (!Nok)) cout << "    NO calibrations are OK" << endl;
      else {
         cout << "    " << ok_list->GetEntries() << " calibrations are OK, "
              << notok_list->GetEntries() << " calibrations are NOT OK" << endl;
         cout << "    " << print_list->GetName() << " :" << endl;
         print_list->ls();
      }
      cout << endl;

   }
}

//_________________________________________________________________________________

Double_t KVMultiDetArray::GetTargetEnergyLossCorrection(KVReconstructedNucleus* ion)
{
   // Calculate the energy loss in the current target of the multidetector
   // for the reconstructed charged particle 'ion', assuming that the current
   // energy and momentum of this particle correspond to its state on
   // leaving the target.
   //
   // WARNING: for this correction to work, the target must be in the right 'state':
   //
   //      gMultiDetArray->GetTarget()->SetIncoming(kFALSE);
   //      gMultiDetArray->GetTarget()->SetOutgoing(kTRUE);
   //
   // (see KVTarget::GetParticleEIncFromERes).
   //
   // The returned value is the energy lost in the target in MeV.
   // The energy/momentum of 'ion' are not affected.

   if (fTarget && ion) return (fTarget->GetParticleEIncFromERes(ion) - ion->GetEnergy());
   return 0;
}

//_________________________________________________________________________________

TGeoManager* KVMultiDetArray::GetGeometry() const
{
   // Return pointer to the (ROOT) geometry of the array.
   return gGeoManager;
}

KVGeoNavigator* KVMultiDetArray::GetNavigator() const
{
   return fNavigator;
}

void KVMultiDetArray::SetDetectorThicknesses()
{
   // Look for a file in the dataset directory with the name given by .kvrootrc variable:
   //
   //    KVMultiDetArray.DetectorThicknesses:
   // or
   //    dataset.KVMultiDetArray.DetectorThicknesses:
   //
   // and, if it exists, we use it to set the real thicknesses of the detectors.
   // Any detector which is not in the file will be left with its nominal thickness.
   //
   // EXAMPLE FILE:
   //
   //# thickness of detector DET01 in default units
   //DET01: 56.4627
   //
   //# DET03 has several layers
   //DET03.Abs0: 61.34
   //DET03.Abs1: 205.62
   //
   // !!! WARNING !!!
   // Single-layer detectors: The units are those defined by default for the detector's
   //         Get/SetThickness methods.
   // Multi-layer: Each layer is a KVMaterial object. The thickness MUST be given in centimetres
   //         (default thickness unit for KVMaterial).

   TString filename = gDataSet->GetDataSetEnv("KVMultiDetArray.DetectorThicknesses", "");
   if (filename == "") {
      Error("SetDetectorThicknesses", "*.KVMultiDetArray.DetectorThicknesses not defined in .kvrootrc");
      return;
   }
   TString fullpath;
   if (!SearchKVFile(filename.Data(), fullpath, gDataSet->GetName())) {
      Info("SetDetectorThicknesses", "File %s not found", filename.Data());
      return;
   }
   TEnv thickdat;
   if (thickdat.ReadFile(fullpath, kEnvUser) != 0) {
      Error("SetDetectorThicknesses", "Problem opening file %s", fullpath.Data());
      return;
   }
   Info("SetDetectorThicknesses", "Setting thicknesses of detectors from file %s", filename.Data());
   TIter next(GetDetectors());
   KVDetector* det;
   while ((det = (KVDetector*)next())) {
      if (thickdat.Defined(det->GetName())) {
         // simple single layer detector
         Double_t thick = thickdat.GetValue(det->GetName(), 0.0);
         det->SetThickness(thick);
         //Info("SetDetectorThicknesses", "Set thickness of %s to %f", det->GetName(), thick);
      } else {
         Char_t i = 0;
         TString absname;
         absname.Form("%s.Abs%d", det->GetName(), (Int_t)i);
         if (thickdat.Defined(absname.Data())) {
            // detector with several layers
            KVMaterial* abs = 0;
            while ((abs = det->GetAbsorber(i))) {
               Double_t thick = thickdat.GetValue(absname.Data(), 0.0);
               abs->SetThickness(thick);
               //Info("SetDetectorThicknesses", "Set thickness of %s.Abs%d to %f", det->GetName(), (Int_t)i, thick);
               i++;
               absname.Form("%s.Abs%d", det->GetName(), (Int_t)i);
               if (!thickdat.Defined(absname.Data())) break;
            }
         }
      }
   }
}

void KVMultiDetArray::SetGeometry(TGeoManager* g)
{
   // Define the geometry of the array with a valid ROOT geometry (TGeoManager instance)
   // The name and title of the TGeoManager object will be used for the array.
   // ROOT geometry will be used by default from now on.
   //fGeoManager = g;
   SetNameTitle(g->GetName(), g->GetTitle());
   SetROOTGeometry();
}

Double_t KVMultiDetArray::GetPunchThroughEnergy(const Char_t* detector, Int_t Z, Int_t A)
{
   // Calculate incident energy of particle (Z,A) required to punch through given detector,
   // taking into account any detectors which the particle must first cross in order to reach it.

   KVDetector* theDet = GetDetector(detector);
   if (!theDet) {
      Error("GetPunchThroughEnergy", "Detector %s not found in array", detector);
      return -1.0;
   }
   Double_t E0 = theDet->GetPunchThroughEnergy(Z, A);
   TIter alDets(theDet->GetAlignedDetectors());
   // first detector in list is theDet
   alDets.Next();
   KVDetector* D;
   while ((D = (KVDetector*)alDets.Next())) {
      Double_t E1 = D->GetIncidentEnergyFromERes(Z, A, E0);
      E0 = E1;
   }
   return E0;
}

TGraph* KVMultiDetArray::DrawPunchThroughEnergyVsZ(const Char_t* detector, Int_t massform)
{
   // Creates and fills a TGraph with the punch through energy in MeV vs. Z for the given detector,
   // for Z=1-92. The mass of each nucleus is calculated according to the given mass formula
   // (see KVNucleus).

   TGraph* punch = new TGraph(92);
   punch->SetName(Form("KVMultiDetpunchthrough_%s_mass%d", detector, massform));
   punch->SetTitle(Form("Array Punch-through %s (MeV) (mass formula %d)", detector, massform));
   KVNucleus nuc;
   nuc.SetMassFormula(massform);
   for (int Z = 1; Z <= 92; Z++) {
      nuc.SetZ(Z);
      punch->SetPoint(Z - 1, Z, GetPunchThroughEnergy(detector, nuc.GetZ(), nuc.GetA()));
   }
   return punch;
}

TGraph* KVMultiDetArray::DrawPunchThroughEsurAVsZ(const Char_t* detector, Int_t massform)
{
   // Creates and fills a TGraph with the punch through energy in MeV/nucleon vs. Z for the given detector,
   // for Z=1-92. The mass of each nucleus is calculated according to the given mass formula
   // (see KVNucleus).

   TGraph* punch = new TGraph(92);
   punch->SetName(Form("KVMultiDetpunchthroughEsurA_%s_mass%d", detector, massform));
   punch->SetTitle(Form("Array Punch-through %s (AMeV) (mass formula %d)", detector, massform));
   KVNucleus nuc;
   nuc.SetMassFormula(massform);
   for (int Z = 1; Z <= 92; Z++) {
      nuc.SetZ(Z);
      punch->SetPoint(Z - 1, Z, GetPunchThroughEnergy(detector, nuc.GetZ(), nuc.GetA()) / nuc.GetA());
   }
   return punch;
}

TGraph* KVMultiDetArray::DrawPunchThroughZVsVpar(const Char_t* detector, Int_t massform)
{
   // Creates and fills a TGraph with the Z vs. the punch-through velocity paralllel to the beam
   // direction in cm/ns for the given detector,
   // for Z=1-92. The mass of each nucleus is calculated according to the given mass formula
   // (see KVNucleus).

   TGraph* punch = new TGraph(92);
   punch->SetName(Form("KVMultiDetpunchthroughV_%s_mass%d", detector, massform));
   punch->SetTitle(Form("Array Punch-through %s (cm/ns) (mass formula %d)", detector, massform));
   KVNucleus nuc;
   nuc.SetMassFormula(massform);
   for (int Z = 1; Z <= 92; Z++) {
      nuc.SetZ(Z);
      nuc.SetE(GetPunchThroughEnergy(detector, nuc.GetZ(), nuc.GetA()));
      nuc.SetTheta(GetDetector(detector)->GetTheta());
      punch->SetPoint(Z - 1, nuc.GetVpar(), Z);
   }
   return punch;
}

void KVMultiDetArray::SetROOTGeometry(Bool_t on)
{
   // Call with on=kTRUE if array uses ROOT geometry for tracking
   // Call SetGeometry(TGeoManager*) first with a valid geometry.

   fROOTGeometry = on;
   if (on && !gGeoManager) {
      Error("SetROOTGeometry", "ROOT geometry is requested, but has not been set: gGeoManager=0x0,\n Call CreateGeoManager() method first");
      return;
   }

   // set up geometry navigator
   if (on && !fNavigator) fNavigator = new KVRangeTableGeoNavigator(gGeoManager, KVMaterial::GetRangeTable());
   else if (!on) SafeDelete(fNavigator);
}

void KVMultiDetArray::CalculateDetectorSegmentationIndex()
{
   // *** Set 'segmentation' index of detectors ***
   // This is essential for particle reconstruction, judging whether particles can be identified
   // independently of any others in the same group etc.
   // Basically, any detector with >1 detector placed directly behind it has a seg. index = 0
   // if <=1 detector is directly behind, the seg. index = 1
   // This method is used for arrays imported from ROOT geometries.

   TIter next(GetDetectors());
   KVDetector* d;
   while ((d = (KVDetector*)next())) {
      if (d->GetNode()->GetNDetsBehind() > 1) d->SetSegment(0);
      else d->SetSegment(1);
   }
}

//___________________________________________________________________________//

void KVMultiDetArray::GetAlignedIDTelescopesForDetector(KVDetector* det, TCollection* list)
{
   //Create and add to list all ID telescopes made of this detector
   //and the aligned detectors placed in front of it.
   //
   //If list=0 then we store pointers to the ALREADY EXISTING ID telescopes
   //in det->fIDTelAlign. (first clear det->fIDTelAlign)

   if (!(det->IsOK())) return;

   TList* aligned = det->GetAlignedDetectors();

   Bool_t list_zero = kFALSE;

   if (!list) {
      list_zero = kTRUE;
      //the IDTelescopes created will be destroyed at the end,
      //once we have used their names to find pointers to the corresponding
      //telescopes in gMultiDetArray
      list = new KVList;
      //clear any existing list of aligned telescopes
      det->GetAlignedIDTelescopes()->Clear();
   }

   if (aligned->GetSize() > 1) {
      //pairwise looping through list
      for (int i = 0; i < (aligned->GetSize() - 1); i++) {
         KVDetector* det1 = (KVDetector*) aligned->At(i + 1);
         KVDetector* det2 = (KVDetector*) aligned->At(i);

         GetIDTelescopes(det1, det2, list);
      }
   } else {
      //The following line is in case there are no detectors aligned
      //with 'det', but 'det' acts as an IDTelescope all by itself.
      //In this case we expect KVMultiDetArray::GetIDTelescopes
      //to define the appropriate ID telescope whenever one of the
      //two detector arguments (or both!) corresponds to 'det''s type.
      GetIDTelescopes(det, det, list);
   }

   if (list_zero) {
      //now we use the created ID telescopes to find pointers to the already
      //existing identical telescopes in gMultiDetArray, stock them in
      //det->fIDTelAlign
      TIter next_tel(list);
      KVIDTelescope* tel;
      while ((tel = (KVIDTelescope*) next_tel())) {
         KVIDTelescope* trash = GetIDTelescope(tel->GetName());
         if (trash) {
            det->GetAlignedIDTelescopes()->Add(trash);
         }
      }
      //destroy the superfluous copy telescopes we just created
      list->Delete();
      delete list;
   }

}

void KVMultiDetArray::GetIDTelescopesForGroup(KVGroup* grp, TCollection* tel_list)
{
   //Identify all the ways of identifying particles possible from the detectors
   //in the group, create the appropriate KVIDTelescope objects and add them to
   //the list pointed to by tel_list.
   //USER'S RESPONSIBILITY TO DELETE CONTENTS OF LIST!!
   //
   //Starting from each detector in the group,
   //we build ID telescopes from all pairs of aligned detectors.
   //
   //For each pair of detectors, it is KVMultiDetArray::GetIDTelescopes
   //which determines which KVIDTelescope class to use (specialise this method
   //in KVMultiDetArray child classes). It must also make sure that
   //each IDTelescope is added only once (i.e. check it is not already in the list).

   TIter next_det(grp->GetDetectors());
   KVDetector* det;

   while ((det = (KVDetector*) next_det())) {
      if (det->IsOK()) {
         //1st call: create ID telescopes, they will be added to the
         //gMultiDetArray list of IDTelescopes
         GetAlignedIDTelescopesForDetector(det, tel_list);
         //2nd call: set up in the detector a list of pointers to the
         //ID telescopes made up of it and all aligned detectors in front
         //of it
         GetAlignedIDTelescopesForDetector(det, 0);
      }
   }
}

void KVMultiDetArray::PrepareModifGroup(KVGroup* grp, KVDetector* dd)
{
   //Casse tous les liens entre les detecteurs d un meme groupe
   //Retire de la liste gMultiDetArray->GetListOfIDTelescopes() les
   //telescopes associes et les efface apres les avoir
   //retires des grilles auxquelles ils etaient associees
   //pour preparer l ajout ou le retrait d un detecteur
   //voir KVDetector::SetPresent()
   //
   //
   KVNameValueList nv;

   KVDetector* det = 0;
   KVIDTelescope* id = 0;

   const KVSeqCollection* lgrdet = grp->GetDetectors();
   TIter nextdet(lgrdet);
   while ((det = (KVDetector*)nextdet())) {
      //Info("PrepareModif","On retire les detecteurs alignes pour %s",det->GetName());
      det->ResetAlignedDetectors(KVGroup::kForwards);
      det->ResetAlignedDetectors(KVGroup::kBackwards);
      Int_t ntel = det->GetIDTelescopes()->GetEntries();
      for (Int_t ii = 0; ii < ntel; ii += 1) {
         id = (KVIDTelescope*)det->GetIDTelescopes()->At(0);
         if (id->GetDetectors()->FindObject(dd)) {
            nv.SetValue(id->GetName(), "");
            det->GetIDTelescopes()->RemoveAt(0);
         }
      }
   }

   KVHashList* lidtel = (KVHashList*)GetListOfIDTelescopes();
   for (Int_t ii = 0; ii < nv.GetEntries(); ii += 1) {
      id = (KVIDTelescope*)lidtel->FindObject(nv.GetNameAt(ii));
      //Info("PrepareModif","On retire et on detruit l'ID tel %s",id->GetName());

      if (id->GetListOfIDGrids()) {
         KVIDGraph* idg = 0;
         for (Int_t kk = 0; kk < id->GetListOfIDGrids()->GetEntries(); kk += 1) {
            idg = (KVIDGraph*)id->GetListOfIDGrids()->At(kk);
            idg->RemoveIDTelescope(id);
         }
      }

      //Info("PrepareModif","Je retire et j efface le idtel %s %s",id->GetName(),id->ClassName());
      delete lidtel->Remove(id);
   }
   nv.Clear();

}

void KVMultiDetArray::SetPresent(KVDetector* det, Bool_t present)
{
   //
   // If present=kTRUE (default), detector is present
   // If present=kFALSE, detector has been removed
   // This method does nothing is required state is already current state.
   //
   // Methode applicable seulement pour un detecteur
   // etant le seul dans un telescope
   // ex les ChIo pour Indra
   //
   // This method as always to be call before
   // call the SetDetecting() method

   if (present == det->IsPresent())
      return;

   det->SetPresent(present);

   //On passe l etat d un detecteur de present a absent
   //
   KVTelescope* fTelescope = (KVTelescope*)det->GetParentStructure("TELESCOPE");
   if (!det->IsPresent()) {

      //Le detecteur etait l unique d un KVTelescope
      //on retire directement le KVTelescope
      if (fTelescope->GetDetectors()->GetEntries() == 1) {
         KVGroup* gr = (KVGroup*)fTelescope->GetParentStructure("GROUP");

         PrepareModifGroup(gr, det);

         gr->Remove(fTelescope);
         gr->Remove(det);
         GetIDTelescopesForGroup(gr, GetListOfIDTelescopes());

      } else {
         Warning("SetPresent", "Method implemented only in case detector is alone in telescope");
      }
   }
   //On remet le detecteur dans le groupe auquel il appartenait
   else {

      if (!det->GetGroup()) {
         KVGroup* gr = GetGroupWithAngles(det->GetTheta(), det->GetPhi());

         PrepareModifGroup(gr, det);

         gr->Add(fTelescope);
         gr->Add(det);
         gr->Sort();
         gr->CountLayers();
         GetIDTelescopesForGroup(gr, GetListOfIDTelescopes());

      }

   }

}

void KVMultiDetArray::SetDetecting(KVDetector* det, Bool_t detecting)
{

   // This method has effect only if detector is present
   //
   // If detecting=kTRUE (default), detector is detecting
   // If detecting=kFALSE, detector has been switch off or no gas or just dead
   // Does nothing if:
   //    - detector already marked as absent
   //    - required state is already current state

   if (!det->IsPresent())
      return;

   if (detecting == det->IsDetecting())
      return;

   det->SetDetecting(detecting);
   if (!det->IsDetecting()) {
      KVGroup* gr = det->GetGroup();
      PrepareModifGroup(gr, det);
      GetIDTelescopesForGroup(gr, GetListOfIDTelescopes());
   } else {
      KVGroup* gr = GetGroupWithAngles(det->GetTheta(), det->GetPhi());
      PrepareModifGroup(gr, det);
      GetIDTelescopesForGroup(gr, GetListOfIDTelescopes());
   }

}

void KVMultiDetArray::SetGridsInTelescopes(UInt_t run)
{
   // For each grid which is valid for this run, we call the KVIDTelescope::SetIDGrid method
   // of each associated ID telescope.
   TIter next(gIDGridManager->GetGrids());
   KVIDGraph* gr = 0;
   while ((gr = (KVIDGraph*) next())) {
      if (gr->GetRuns().Contains((Int_t) run)) {

         //Info("SetGridsInTelescopes","la grille %s contient le numero de run %d",gr->GetName(),run);
         TIter nxtid(gr->GetIDTelescopes());
         KVIDTelescope* idt;
         while ((idt = (KVIDTelescope*) nxtid())) {
            //Info("SetGridsInTelescopes","%s %s",idt->GetName(),idt->ClassName());
            idt->SetIDGrid(gr);
         }
      }
   }
}
void KVMultiDetArray::FillListOfIDTelescopes(KVIDGraph* gr) const
{
   // Fill list of ID telescopes with which this grid is associated
   // from list of names read from ascii file.

   gr->ClearListOfTelescopes();
   if (gr->GetParameters()->HasParameter("IDTelescopes")) {
      KVString tel_list = gr->GetParameters()->GetStringValue("IDTelescopes");
      tel_list.Begin(",");
      while (!tel_list.End()) {
         TString tel_name = tel_list.Next();
         KVIDTelescope* idt = GetIDTelescope(tel_name.Data()) ;
         if (idt) gr->AddIDTelescope(idt);
      }
   }
}

void KVMultiDetArray::Draw(Option_t* option)
{
   // Use OpenGL viewer to view multidetector geometry (only for ROOT geometries)
   //
   // If option="tracks" we draw any tracks corresponding to the last simulated
   // event whose detection was simulated with DetectEvent
   // If option="tracks:[numberlist]" with a list of numbers,
   // it will be interpreted as a KVNumberList containing the Z of tracks to be drawn
   //    e.g. option="tracks:1-92" draw only tracks with 1<=Z<=92 (no neutrons)
   //         option="tracks:2"    draw only helium isotopes
   if (IsROOTGeometry()) {
      GetGeometry()->GetTopVolume()->Draw("ogl");
      KVString opt(option);
      opt.Begin(":");
      if (opt.Next() == "tracks") {
         if (!opt.End()) {
            KVNumberList zlist(opt.Next());
            GetNavigator()->DrawTracks(&zlist);
         } else
            GetNavigator()->DrawTracks();
      }
#ifdef WITH_OPENGL
      TGLViewer* view = (TGLViewer*)gPad->GetViewer3D();
      view->SetCurrentCamera(TGLViewer::kCameraPerspYOZ);
      view->SetStyle(TGLRnrCtx::kOutline);
      view->SetSmoothLines(kTRUE);
      view->SetSmoothPoints(kTRUE);
#endif
   } else Error("Draw", "Only ROOT geometries can be viewed");

}

void KVMultiDetArray::SetNavigator(KVGeoNavigator* geo)
{
   fNavigator = (KVRangeTableGeoNavigator*)geo;
}

void KVMultiDetArray::MakeHistogramsForAllIDTelescopes(KVSeqCollection* list, Int_t dimension)
{
   // Create TH2F histograms for all IDTelescopes of the array
   // They will be added to the list
   // histograms will have resolution of dimension*dimension

   TIter it(GetListOfIDTelescopes());
   KVIDTelescope* idt;
   while ((idt = (KVIDTelescope*)it())) {
      TString name(idt->GetName());
      name.ReplaceAll("-", "_");
      list->Add(new TH2F(name, Form("Hits in %s", idt->GetName()), dimension, 0., 0., dimension, 0., 0.));
   }
}

void KVMultiDetArray::FillHistogramsForAllIDTelescopes(KVSeqCollection* list)
{
   // Fill TH2F histograms for all IDTelescopes of the array

   TIter it(GetListOfIDTelescopes());
   KVIDTelescope* idt;
   while ((idt = (KVIDTelescope*)it())) {
      TString name(idt->GetName());
      name.ReplaceAll("-", "_");
      TH2F* h = (TH2F*)list->FindObject(name);
      if (h) h->Fill(idt->GetIDMapX(), idt->GetIDMapY());
   }
}

void KVMultiDetArray::SetDetectorTransparency(Char_t t)
{
   // Modify the transparency of detector volumes in OpenGL view

   TIter itV(GetGeometry()->GetListOfVolumes());
   TGeoVolume* vol;
   while ((vol = (TGeoVolume*)itV())) vol->SetTransparency(t);
}

void KVMultiDetArray::CalculateReconstructionTrajectories()
{
   // Calculate all possible (sub-)trajectories
   // for particle reconstruction (GetReconTrajectories())

   unique_ptr<KVSeqCollection> groups(GetStructureTypeList("GROUP"));
   TIter it(groups.get());
   KVGroup* group;
   Int_t ntr = 0;
   Info("CalculateReconstructionTrajectories", "Calculating trajectories for particle reconstruction:");
   if (!KVDataAnalyser::IsRunningBatchAnalysis())
      std::cout << "\xd" << " -- calculated " << ntr << " reconstruction trajectories" << std::flush;
   while ((group = (KVGroup*)it())) {
      ntr += group->CalculateReconstructionTrajectories();
      if (!KVDataAnalyser::IsRunningBatchAnalysis())
         std::cout << "\xd" << " -- calculated " << ntr << " reconstruction trajectories" << std::flush;
   }
   if (KVDataAnalyser::IsRunningBatchAnalysis())
      std::cout << " -- calculated " << ntr << " reconstruction trajectories" << std::endl;
   else
      std::cout << std::endl;
}

void KVMultiDetArray::DeduceIdentificationTelescopesFromGeometry()
{
   // Track over all possible particle trajectories calling
   //   GetIDTelescopes(KVDetector*,KVDetector*)
   // for each pair of (present & functioning) detectors.
   // This will create all possible KVIDTelescope identification
   // objects and put them in list fIDTelescopes

   fIDTelescopes->Delete();
   TIter next_traj(GetTrajectories());
   KVGeoDNTrajectory* traj;
   Int_t count = 0;
   Info("DeduceIdentificationTelescopesFromGeometry", "Calculating...");
   if (!KVDataAnalyser::IsRunningBatchAnalysis())
      std::cout << "\xd" << " -- created " << count << " telescopes" << std::flush;
   while ((traj = (KVGeoDNTrajectory*)next_traj())) {   // loop over all trajectories

      traj->IterateFrom();   // from furthest-out to closest-in detector

      KVGeoDetectorNode* N;
      while ((N = traj->GetNextNode())) {
         KVGeoDetectorNode* Nplus1 = traj->GetNodeInFront(N);

         if (Nplus1) {
            count += GetIDTelescopes(Nplus1->GetDetector(), N->GetDetector(), traj->AccessIDTelescopeList());
            if (!KVDataAnalyser::IsRunningBatchAnalysis())
               std::cout << "\xd" << " -- created " << count << " telescopes" << std::flush;
         }
      }
   }
   if (KVDataAnalyser::IsRunningBatchAnalysis())
      std::cout << " -- created " << count << " telescopes" << std::endl;
   else
      std::cout << std::endl;
}

void KVMultiDetArray::AssociateTrajectoriesAndNodes()
{
   // Eliminate any trajectories which are just sub-trajectories of others
   // For each trajectory in list fTrajectories, we add a reference to the trajectory to each node on the trajectory
   // We also fill the trajectory list of each group

   TIter it(&fTrajectories);
   KVGeoDNTrajectory* tr;
   KVList duplicates;
   // look for duplicate sub-trajectories
   while ((tr = (KVGeoDNTrajectory*)it())) {
      int len_tr = tr->GetN();
      TIter it2(&fTrajectories);
      KVGeoDNTrajectory* tr2;
      while ((tr2 = (KVGeoDNTrajectory*)it2())) {
         if ((tr2 != tr) && (len_tr < tr2->GetN()) && (tr2->ContainsPath(tr))) {
            duplicates.Add(tr);
            break;
         }
      }
   }
   // remove duplicates
   if (duplicates.GetEntries()) {
      TIter it_dup(&duplicates);
      while ((tr = (KVGeoDNTrajectory*)it_dup())) {
         fTrajectories.Remove(tr);
      }
      Info("AssociateTrajectoriesAndNodes", "Removed %d duplicated sub-trajectories", duplicates.GetEntries());
   }
   Info("AssociateTrajectoriesAndNodes", "Calculated %d particle trajectories", fTrajectories.GetEntries());
   it.Reset();
   while ((tr = (KVGeoDNTrajectory*)it())) {
      tr->AddToNodes();
      tr->GetNodeAt(0)->GetDetector()->GetGroup()->AddTrajectory(tr);
   }
}
