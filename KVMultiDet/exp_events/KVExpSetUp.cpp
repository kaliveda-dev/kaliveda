//Created by KVClassFactory on Thu Feb 25 14:59:59 2016
//Author: bonnet,,,

#include "KVExpSetUp.h"
#include "KVDataSetManager.h"
#include "TClass.h"
#include "KVReconstructedNucleus.h"

#include <KVGroup.h>
#include <KVRangeTableGeoNavigator.h>

#ifdef WITH_MFM
#include "KVMFMDataFileReader.h"
#endif

ClassImp(KVExpSetUp)

void KVExpSetUp::init()
{
   fBuildTarget = kFALSE;
   fCloseGeometryNow = kFALSE;
   //modification of the owner mode compare to Mother classes
   SetOwnsDaughters(kFALSE);
   SetOwnsDetectors(kFALSE);
   fIDTelescopes->SetOwner(kFALSE);
}


KVExpSetUp::KVExpSetUp()
{
   // Default constructor
   init();
}

KVExpSetUp::~KVExpSetUp()
{
   // Destructor
   fCloseGeometryNow = kTRUE;
}


void KVExpSetUp::Build(Int_t run)
{
   // Build the combined arrays
   //
   // The name of the setup will be "[det1]-[det2]-..."

   // default ROOT geometry for all arrays
   gEnv->SetValue("KVMultiDetArray.ROOTGeometry", "yes");

   CreateGeoManager();

   Info("Build", "navigator=%p", GetNavigator());

   KVRangeTableGeoNavigator* gnl = new KVRangeTableGeoNavigator(gGeoManager, KVMaterial::GetRangeTable());
   SetNavigator(gnl);

   TString myname;

   KVMultiDetArray* tmp(nullptr);
   Int_t group_offset = 0;// for renumbering groups
   lmultidetarrayclasses = GetDataSetEnv(fDataSet, "DataSet.ExpSetUp.ClassList", IsA()->GetName());
   lmultidetarrayclasses.Begin(" ");

   // VERY IMPORTANT: deactivate 'SetParameters(run)' being called by MakeMultiDetector for sub-arrays!
   fMakeMultiDetectorSetParameters = kFALSE;

   while (!lmultidetarrayclasses.End()) {
      KVString sname = lmultidetarrayclasses.Next();
      Info("Build", "Build %s %s\n", fDataSet.Data(), sname.Data());
      gMultiDetArray = nullptr; //otherwise MakeMultiDetector will delete any previously built array
      tmp = MakeMultiDetector(fDataSet, run, sname.Data());
      if (tmp) {
         fMDAList.Add(tmp);
         if (myname != "") myname += "-";
         myname += tmp->GetName();
      }
      else {
         Error("Build", "NULL pointer returned by MakeMultiDetector");
      }
   }

   // VERY IMPORTANT: reactivate 'SetParameters(run)' so it is called for us when we return
   // to the MakeMultiDetector method which called us
   fMakeMultiDetectorSetParameters = kTRUE;

   gGeoManager->CloseGeometry();

   TIter nxt_mda(&fMDAList);
   while ((tmp = (KVMultiDetArray*)nxt_mda())) {
      tmp->PerformClosedROOTGeometryOperations();
      unique_ptr<KVSeqCollection> groups(tmp->GetStructureTypeList("GROUP"));
      if (group_offset) {
         // renumber all groups to keep unique names/numbers
         TIter next(groups.get());
         KVGroup* group;
         while ((group = (KVGroup*)next())) {
            Int_t group_number = group->GetNumber();
            group->SetNumber(group_number + group_offset);
         }
      }
      else
         group_offset += groups->GetEntries();

      fDetectors.AddAll((KVUniqueNameList*)tmp->GetDetectors());
      fStructures.AddAll((KVUniqueNameList*)tmp->GetStructures());
      fIDTelescopes->AddAll(tmp->GetListOfIDTelescopes());

      // retrieve correspondance list node path<->detector
      gnl->AbsorbDetectorPaths(tmp->GetNavigator());
   }

   SetGeometry(gGeoManager);

   gMultiDetArray = this;
   SetBit(kIsBuilt);
   SetName(myname);
}

void KVExpSetUp::Clear(Option_t* opt)
{
   // call Clear(opt) for each multidetector in the setup
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->Clear(opt);
   }
}

void KVExpSetUp::FillDetectorList(KVReconstructedNucleus* rnuc, KVHashList* DetList, const KVString& DetNames)
{
   // Call FillDetectorList for each array in turn, until DetList gets filled
   // This is because some arrays may override the KVMultiDetArray::FillDetectorList method

   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->FillDetectorList(rnuc, DetList, DetNames);
      if (!DetList->IsEmpty()) break;
   }
}

void KVExpSetUp::AcceptParticleForAnalysis(KVReconstructedNucleus* NUC) const
{
   // Overrides KVMultiDetArray method
   // We use the "ARRAY" parameter of the reconstructed particle (if set)
   // to know which array of the setup detected it.

   if (NUC->GetParameters()->HasStringParameter("ARRAY")) {
      KVMultiDetArray* whichArray = GetArray(NUC->GetParameters()->GetStringValue("ARRAY"));
      if (whichArray) {
         whichArray->AcceptParticleForAnalysis(NUC);
      }
      else
         Warning("AcceptParticleForAnalysis", "ReconstructedNucleus has ARRAY=%s but no KVMultiDetArray with this name in set-up",
                 NUC->GetParameters()->GetStringValue("ARRAY"));
   }
   else {
      Warning("AcceptParticleForAnalysis", "ReconstructedNucleus has no ARRAY parameter: cannot determine correct KVMultiDetArray");
   }
}

void KVExpSetUp::GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* fired_params)
{
   // Override KVMultiDetArray method
   // Call each array in turn and add fired groups to the KVDetectorEvent

   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->GetDetectorEvent(detev, fired_params);
   }
}

KVGroupReconstructor* KVExpSetUp::GetReconstructorForGroup(const KVGroup* g) const
{
   // Override KVMultiDetArray method
   // Call each array in turn to get reconstructor for group
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   KVGroupReconstructor* gr(nullptr);
   while ((mda = (KVMultiDetArray*)next_array())) {
      if ((gr = mda->GetReconstructorForGroup(g))) break;
   }
   return gr;
}

void KVExpSetUp::SetRawDataFromReconEvent(KVNameValueList& l)
{
   prepare_to_handle_new_raw_data();
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->SetRawDataFromReconEvent(l);
   }
}

void KVExpSetUp::SetReconParametersInEvent(KVReconstructedEvent* e) const
{
   // Add contents of fReconParameters of each sub-array to the event parameter list
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      //*(e->GetParameters()) += mda->GetReconParameters();
      mda->SetReconParametersInEvent(e);
   }
}

void KVExpSetUp::GetArrayMultiplicities(KVReconstructedEvent* e, KVNameValueList& m, Option_t* opt)
{
   // Calculate multiplicities of particles in each array of the setup.
   // They will appear in the KVNameValueList as parameters with the name of the array.
   // e.g.
   //
   //    "INDRA" = 21
   //    "FAZIA" = 3
   //
   // Any option given will be used to determine the type of event iterator used -
   // see KVEvent::GetNextParticleIterator(Option_t*).

   m.Clear();
   for (KVEvent::Iterator it = e->GetNextParticleIterator(opt); it != e->end(); ++it) {
      m.IncrementValue((*it).GetParameters()->GetStringValue("ARRAY"), 1);
   }
}

void KVExpSetUp::MakeCalibrationTables(KVExpDB* db)
{
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   TString orig_dbtype = db->GetDBType();
   while ((mda = (KVMultiDetArray*)next_array())) {
      db->SetDBType(Form("%sDB", mda->GetName()));
      mda->MakeCalibrationTables(db);
   }
   db->SetDBType(orig_dbtype);
}

void KVExpSetUp::SetCalibratorParameters(KVDBRun* r, const TString&)
{
   // Set calibrators for all detectors for the run
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->SetCalibratorParameters(r, mda->GetName());
   }
}

void KVExpSetUp::InitialiseRawDataReading(KVRawDataReader* R)
{
   // Calls InitialiseRawDataReading() for each array of the setup in turn
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->InitialiseRawDataReading(R);
   }
}

#ifdef WITH_MFM
Bool_t KVExpSetUp::handle_raw_data_event_mfmframe(const MFMCommonFrame& mfmframe)
{
   // Handle single (not merged) MFM frames of raw data. It is assumed
   // that each frame type corresponds to a different detector array.
   // Therefore as soon as one of them treats the data in the frame,
   // we return kTRUE.

   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      if (mda->handle_raw_data_event_mfmframe(mfmframe)) {
         mda->fHandledRawData = true;
         return kTRUE;
      }
   }
   return kFALSE;
}
#endif

Bool_t KVExpSetUp::HandleRawDataEvent(KVRawDataReader* rawdata)
{
   // Set fRawDataReader pointer in each sub-array and call prepare_to_handle_new_raw_data()
   // for each sub-array, before treating raw data.
   Info("KVExpSetUp::HandleRawDataEvent", "called");
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      mda->fRawDataReader = rawdata;
      mda->prepare_to_handle_new_raw_data();
   }
   if (KVMultiDetArray::HandleRawDataEvent(rawdata)) {
      // copy fired signals & detectors of sub-arrays to main list
      while ((mda = (KVMultiDetArray*)next_array())) {
         fFiredDetectors.AddAll(&mda->fFiredDetectors);
         fFiredSignals.AddAll(&mda->fFiredSignals);
      }
      return true;
   }
   return false;
}

void KVExpSetUp::copy_fired_parameters_to_recon_param_list()
{
   TIter next_array(&fMDAList);
   KVMultiDetArray* mda;
   while ((mda = (KVMultiDetArray*)next_array())) {
      if (mda->HandledRawData()) mda->copy_fired_parameters_to_recon_param_list();
   }
}
