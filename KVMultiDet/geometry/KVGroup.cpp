#include "Riostream.h"
#include "KVGroup.h"
#include "KVNucleus.h"
#include "KVList.h"
#include "KVDetector.h"
#include "KVLayer.h"
#include "TROOT.h"
#include "KVNameValueList.h"

using namespace std;

ClassImp(KVGroup)


KVGroup::KVGroup()
{
   init();
}

//_________________________________________________________________________________

void KVGroup::init()
{
   // Default initialisation
   // KVGroup does not own the structures which it groups together

   fReconstructedNuclei = 0;
   SetType("GROUP");
   SetOwnsDaughters(kFALSE);
}

//_____________________________________________________________________________________

KVGroup::~KVGroup()
{
   if (fReconstructedNuclei && fReconstructedNuclei->TestBit(kNotDeleted)) {
      fReconstructedNuclei->Clear();
      delete fReconstructedNuclei;
      fReconstructedNuclei = 0;
   }
   fReconstructedNuclei = 0;
}

UInt_t KVGroup::GetNumberOfDetectorLayers()
{
   // The number of detector layers is the maximum number of detectors in the
   // group which are placed one in front of the other, i.e. we interrogate
   // each detector as to how many detectors there are in front of it

   UInt_t max = 0;
   TIter next(GetDetectors());
   KVDetector* d;
   while ((d = (KVDetector*)next())) {
      UInt_t e = d->GetAlignedDetectors()->GetEntries();
      if (e > max) max = e;
   }
   return max;
}

TList* KVGroup::GetDetectorsInLayer(UInt_t lay)
{
   // lay=1 : create and fill list with detectors closest to target
   // lay=GetNumberOfDetectorLayers() : detectors furthest from target

   TList* dets = new TList;
   TIter next(GetDetectors());
   KVDetector* d;

   while ((d = (KVDetector*)next())) {
      if (lay == (UInt_t)d->GetAlignedDetectors()->GetEntries()) dets->Add(d);
   }
   return dets;
}

//______________________________________________________________________________

void KVGroup::Reset()
{
   //Reset the group, i.e. wipe the list of reconstructed nuclei and call "Clear" method of
   //each and every detector in the group.
   //Info("Reset","Call %s",GetName());
   if (fReconstructedNuclei && fReconstructedNuclei->GetSize()) {
      fReconstructedNuclei->Clear();
   }
   //reset energy loss and KVDetector::IsAnalysed() state
   //plus ACQParams set to zero
   const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, Clear)();
}

//_________________________________________________________________________________

void KVGroup::AddHit(KVNucleus* kvd)
{
   if (!fReconstructedNuclei) {
      fReconstructedNuclei = new KVList(kFALSE);
      fReconstructedNuclei->SetCleanup();
   }
   fReconstructedNuclei->Add(kvd);
}

//_________________________________________________________________________________

void KVGroup::RemoveHit(KVNucleus* kvd)
{
   //Remove reconstructed nucleus from group's list of reconstructed
   //particles.
   if (fReconstructedNuclei) {
      fReconstructedNuclei->Remove(kvd);
      if (fReconstructedNuclei->GetSize() == 0) {
         delete fReconstructedNuclei;
         fReconstructedNuclei = 0;
      }
   } else {
      Warning("RemoveHit", "No reconstructed nuclei in this group");
   }
}

TList* KVGroup::GetAlignedDetectors(KVDetector* det, UChar_t dir)
{
   //Fill TList with all detectors aligned with "det" which are closer to the target.
   //These are the detectors through which any particle stopping in "det" will have
   //to pass. By default (dir=KVGroup::kBackwards) the list starts with "det" and
   //goes towards the target. Use dir=KVGroup::kForwards to have the list in the
   //order seen by an impinging particle.
   //
   //Delete TList after use.

   TList* tmp = new TList;

   while (det) {
      tmp->Add(det);
      KVGeoDetectorNode* node = det->GetNode();
      KVSeqCollection* infront = node->GetDetectorsInFront();
      if (!infront) break;
      if (infront->GetEntries() > 1) {
         Warning("GetAlignedDetectors",
                 "No unique solution. There are %d detectors in front of %s.",
                 infront->GetEntries(), det->GetName());
         infront->ls();
      }
      det = (KVDetector*)infront->First();
   }

   if (dir == kForwards) {
      TIter next(tmp, kIterBackward);
      TList* tmp2 = new TList;
      while ((det = (KVDetector*)next())) tmp2->Add(det);
      delete tmp;
      tmp = tmp2;
   }
   return tmp;
}

//_________________________________________________________________________________



void KVGroup::ClearHitDetectors()
{
   // Loop over all detectors in group and clear their list of 'hits'
   // i.e. the lists of particles which hit each detector
   const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, ClearHits)();
}


