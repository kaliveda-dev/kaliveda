//Created by KVClassFactory on Tue Jan 27 11:37:39 2015
//Author: ,,,

#include "KVFAZIA.h"
#include "KVGeoImport.h"
#include "KVSignal.h"
#include "KVFAZIADetector.h"
#include "KVGroup.h"
#include "KVFAZIABlock.h"
#include "KVDetectorEvent.h"
#include "KVTarget.h"
#include "TSystem.h"
#include "KVDataSet.h"
#include "KVConfig.h"

//#include "TGeoBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoEltu.h"


ClassImp(KVFAZIA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIA</h2>
<h4>Base class for description of the FAZIA set up</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
KVFAZIA* gFazia;

KVFAZIA::KVFAZIA()
{
   // Default constructor
   SetName("FAZIA");
   SetTitle(ClassName());
   fStartingBlockNumber = 0;
   gFazia = this;
   IncludeTargetInGeometry();
   fDetectorLabels = "";
   fSignalTypes = "QL1,I1,QH1,Q2,I2,Q3";
   SetGeometryImportParameters();
}

KVFAZIA::~KVFAZIA()
{
   // Destructor
}

void KVFAZIA::AddDetectorLabel(const Char_t* label)
{
   if (fDetectorLabels == "") fDetectorLabels += label;
   else if (!fDetectorLabels.Contains(label)) fDetectorLabels += Form(",%s", label);
}

void KVFAZIA::GenerateCorrespondanceFile()
{
   fDetectorLabels = "SI1,SI2,CSI";
#ifdef WITH_GNU_INSTALL
   fCorrespondanceFile.Form("%s/%s-%s.names", KVBase::WorkingDirectory(), gSystem->Getenv("USER"), ClassName());
#else
   fCorrespondanceFile.Form("%s/%s-%s.names", KVBase::GetDATADIRFilePath(), gSystem->Getenv("USER"), ClassName());
#endif
   Info("GenerateCorrespondanceFile", "Creation de %s", fCorrespondanceFile.Data());
   KVEnv env;

   SetNameOfDetectors(env);

   env.AddCommentLine(Form("Automatic generated file by %s::GenerateCorrespondanceFile", ClassName()));
   env.AddCommentLine("Make link between geometric ROOT objects and detector names");
   env.WriteFile(fCorrespondanceFile.Data());
   fDetectorLabels = "";
}

void KVFAZIA::SetNameOfDetectors(KVEnv& env)
{
   for (Int_t bb = fStartingBlockNumber; bb < fNblocks; bb += 1) {
      for (Int_t qq = 1; qq <= 4; qq += 1) {
         for (Int_t tt = 1; tt <= 4; tt += 1) {
            fDetectorLabels.Begin(",");
            while (!fDetectorLabels.End()) {
               KVString sdet = fDetectorLabels.Next();
               env.SetValue(
                  Form("BLOCK_%d_QUARTET_%d_%s-T%d", bb, qq, sdet.Data(), tt),
                  Form("%s-T%d-Q%d-B%03d", sdet.Data(), tt, qq, bb)
               );
            }
         }
      }
   }
}

void KVFAZIA::GetGeometryParameters()
{
   //Called by the Build method
   Info("GetGeometryParameters", "to be defined in child class ...");
}

void KVFAZIA::BuildFAZIA()
{
   //Called by the Build method
   Info("BuildFAZIA", "to be defined in child class ...");

}

void KVFAZIA::BuildTarget()
{

   KVMaterial target_holder_mat("Al");
   new TGeoBBox("TARGET_FRAME", 3., 3., 0.1 / 2.);
   new TGeoEltu("TARGET_HOLE", 2., 2., 0.1 / 2.);
   TGeoCompositeShape* cs = new TGeoCompositeShape("TARGET_FRAME", "TARGET_FRAME - TARGET_HOLE");
   TGeoVolume* target_frame = new TGeoVolume("TARGET_FRAME", cs, target_holder_mat.GetGeoMedium());
   gGeoManager->GetTopVolume()->AddNode(target_frame, 1);

   KVTarget* T = GetTarget();
   if (T) {
      KVMaterial* targMat = (KVMaterial*)T->GetLayers()->First();
      TGeoVolume* target = gGeoManager->MakeEltu("TARGET", targMat->GetGeoMedium(), 2., 2., targMat->GetThickness() / 2.);
      gGeoManager->GetTopVolume()->AddNode(target, 1);
   }
}

void KVFAZIA::Build(Int_t)
{
   // Build the combined INDRA & FAZIA arrays
   GetGeometryParameters();
   GenerateCorrespondanceFile();

   if (!gGeoManager) {
      new TGeoManager("FAZIA", Form("FAZIA geometry for dataset %s", gDataSet->GetName()));

      TGeoMaterial* matVacuum = gGeoManager->GetMaterial("Vacuum");
      if (!matVacuum) {
         matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
         matVacuum->SetTitle("Vacuum");
      }
      TGeoMedium* Vacuum = gGeoManager->GetMedium("Vacuum");
      if (!Vacuum) Vacuum = new TGeoMedium("Vacuum", 1, matVacuum);
      TGeoVolume* top = gGeoManager->MakeBox("WORLD", Vacuum,  500, 500, 500);
      gGeoManager->SetTopVolume(top);
   }

   BuildFAZIA();
   if (fBuildTarget)
      BuildTarget();

   KVGeoImport imp(gGeoManager, KVMaterial::GetRangeTable(), this, kTRUE);
   imp.SetDetectorPlugin(ClassName());
   imp.SetNameCorrespondanceList(fCorrespondanceFile.Data());
   // any additional structure name formatting definitions
   DefineStructureFormats(imp);

   // the following parameters are optimized for a 12-block compact
   // geometry placed at 80cm with rings 1-5 of INDRA removed.
   // make sure that the expected number of detectors get imported!
   imp.ImportGeometry(fImport_dTheta, fImport_dPhi, fImport_ThetaMin, fImport_PhiMin, fImport_ThetaMax, fImport_PhiMax);

   /*
   KVFAZIADetector* det=0;
   TIter next_d(GetDetectors());
   while ( det = (KVFAZIADetector* )next_d() ){
      printf("%s %s %d %d %d\n",det->GetName(),det->GetFAZIAType(),det->GetBlockNumber(),det->GetQuartetNumber(),det->GetTelescopeNumber());
   }
   */
   SetIdentifications();
   SortIDTelescopes();
   KVDetector* det = GetDetector("SI2-T1-Q1-B001");
   det->GetIDTelescopes()->ls();



   SetDetectorThicknesses();
   SetBit(kIsBuilt);
}

void KVFAZIA::SortIDTelescopes()
{
   KVDetector* det = 0;
   TIter next(GetDetectors());
   while ((det = (KVDetector*)next())) {
      ((KVFAZIADetector*)det)->SortIDTelescopes();
   }

}
void KVFAZIA::GetDetectorEvent(KVDetectorEvent* detev, TSeqCollection* signals)
{
   // First step in event reconstruction based on current status of detectors in array.
   // Fills the given KVDetectorEvent with the list of all groups which have fired.
   // i.e. loop over all groups of the array and test whether KVGroup::Fired() returns true or false.
   //
   // If the list of fired acquisition parameters 'signals' is given, KVMultiDetArray::GetDetectorEvent
   // is called
   //

   if (signals) {
      // list of fired acquisition parameters given
      TIter next_par(signals);

      KVSignal* par = 0;
      KVDetector* det = 0;
      KVGroup* grp = 0;
      while ((par = (KVSignal*)next_par())) {
         par->DeduceFromName();
         if ((det = GetDetector(par->GetDetectorName()))) {
            ((KVFAZIADetector*)det)->SetSignal(par, par->GetType());
            if ((grp = det->GetGroup())  && !detev->GetGroups()->FindObject(grp)) {
               detev->AddGroup(grp);
            }
         } else {
            Error("GetDetectedEvent", "Unknown detector %s !!!", par->GetDetectorName());
         }
      }
   } else {
      KVMultiDetArray::GetDetectorEvent(detev, 0);
   }

}


