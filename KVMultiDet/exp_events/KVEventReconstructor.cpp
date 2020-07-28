//Created by KVClassFactory on Mon Oct 19 09:33:44 2015
//Author: John Frankland,,,

#include "KVEventReconstructor.h"
#include "KVDetectorEvent.h"
#include "KVGroupReconstructor.h"
#include "KVTarget.h"

#include <iostream>
using namespace std;

ClassImp(KVEventReconstructor)

KVEventReconstructor::KVEventReconstructor(KVMultiDetArray* a, KVReconstructedEvent* e, Bool_t)
   : KVBase("KVEventReconstructor", Form("Reconstruction of events in array %s", a->GetName())),
     fArray(a), fEvent(e), fGroupReconstructor(a->GetNumberOfGroups(), 1)
{
   // Default constructor
   // Set up group reconstructor for every group of the array.
   // It is assumed that these are all numbered uniquely & sensibly ;)
   //
   // Identification & calibration are enabled or not depending on the following
   // (possibly-dataset-dependent) variables:
   //
   //    EventReconstruction.DoIdentification
   //    EventReconstruction.DoCalibration

   fGroupReconstructor.SetOwner();
   unique_ptr<KVSeqCollection> gr_list(a->GetStructureTypeList("GROUP"));
   KVGroup* gr;
   TIter it(gr_list.get());
   unsigned int N = gr_list->GetEntries();
   while ((gr = (KVGroup*)it())) {
      UInt_t i = gr->GetNumber();
      if (i > N || i < 1) {
         Warning("KVEventReconstructor", "Groups in array %s are not numbered the way I like...%u", a->GetName(), i);
         gr->Print();
      }
      else {
         if (fGroupReconstructor[i]) {
            Warning("KVEventReconstructor", "%s array has non-unique group numbers!!!", a->GetName());
         }
         else {
            fGroupReconstructor[i] = a->GetReconstructorForGroup(gr);
            if (fGroupReconstructor[i]) {
               ((KVGroupReconstructor*)fGroupReconstructor[i])->SetReconEventClass(e->IsA());
               ((KVGroupReconstructor*)fGroupReconstructor[i])->SetGroup(gr);
            }
         }
      }
   }

   KVGroupReconstructor::SetDoIdentification(GetDataSetEnv(fArray->GetDataSet(), "EventReconstruction.DoIdentification", kTRUE));
   KVGroupReconstructor::SetDoCalibration(GetDataSetEnv(fArray->GetDataSet(), "EventReconstruction.DoCalibration", kTRUE));
   Info("KVEventReconstructor", "Initialised for %u groups of multidetector %s", N, fArray->GetName());
   if (GetDataSetEnv(fArray->GetDataSet(), "EventReconstruction.DoIdentification", kTRUE) || GetDataSetEnv(fArray->GetDataSet(), "EventReconstruction.DoCalibration", kTRUE)) {
      if (GetDataSetEnv(fArray->GetDataSet(), "EventReconstruction.DoIdentification", kTRUE)) {
         Info("KVEventReconstructor", " -- identification of events will be performed");
         fArray->PrintStatusOfIDTelescopes();
      }
      if (GetDataSetEnv(fArray->GetDataSet(), "EventReconstruction.DoCalibration", kTRUE)) {
         Info("KVEventReconstructor", " -- calibration of events will be performed");
         fArray->PrintCalibStatusOfDetectors();
      }
   }
   else
      Info("KVEventReconstructor", " -- no identification or calibration will be performed");
}

//________________________________________________________________

void KVEventReconstructor::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVEventReconstructor::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVEventReconstructor& CastedObj = (KVEventReconstructor&)obj;
}

void KVEventReconstructor::ReconstructEvent(const TSeqCollection* fired)
{
   // Reconstruct current event based on state of detectors in array
   //
   // The list pointer, if given, can be used to supply a list of fired
   // acquisition parameters to KVMultiDetArray::GetDetectorEvent

   if (GetArray()->GetTarget()) {
      // for target energy loss correction calculation
      GetArray()->GetTarget()->SetIncoming(kFALSE);
      GetArray()->GetTarget()->SetOutgoing(kTRUE);
   }

   GetEvent()->Clear("N");// No Group Reset (would not reset groups with no reconstructed particles)
   detev.Clear("N");// reset all groups (but not acquisition parameters), even if no particles were reconstructed in them

   GetArray()->GetDetectorEvent(&detev, fired);

   fNGrpRecon = 0;
   fHitGroups.clear();
   fHitGroups.reserve(detev.GetGroups()->GetEntries());
   TIter it(detev.GetGroups());
   KVGroup* group;
   while ((group = (KVGroup*)it())) {
      KVGroupReconstructor* grec = (KVGroupReconstructor*)fGroupReconstructor[group->GetNumber()];
      if (grec) {
         fHitGroups[fNGrpRecon] = group->GetNumber();
         grec->Process();
         ++fNGrpRecon;
      }
   }

   // merge resulting event fragments
   MergeGroupEventFragments();

   // copy any parameters stocked in the detector(s) during reconstruction in the reconstructed event
   GetArray()->SetReconParametersInEvent(GetEvent());
}

void KVEventReconstructor::MergeGroupEventFragments()
{
   // After processing has finished in groups, call this method to produce
   // a final merged event containing particles from all groups

   TList to_merge;

   for (int k = 0; k < fNGrpRecon; ++k) {
      int i = fHitGroups[k];
      to_merge.Add(((KVGroupReconstructor*)fGroupReconstructor[i])->GetEventFragment());
   }
   GetEvent()->MergeEventFragments(&to_merge, "N");// "N" = no group reset
}
