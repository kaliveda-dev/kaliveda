//Created by KVClassFactory on Tue Apr 23 13:04:21 2013
//Author: John Frankland,,,

#include "KVGeoImport.h"
#include <KVMultiDetArray.h>
#include <KVIonRangeTableMaterial.h>
#include <TGeoBBox.h>
#include <TPluginManager.h>

#include <KVTemplateEvent.h>
#include <KVGroup.h>
#include <TGeoPhysicalNode.h>
#include <KVRangeTableGeoNavigator.h>
#include <KVNamedParameter.h>
#include <KVDataAnalyser.h>

ClassImp(KVGeoImport)


KVGeoImport::KVGeoImport(TGeoManager* g, KVIonRangeTable* r, KVMultiDetArray* m, Bool_t create) : KVGeoNavigator(g), fCreateArray(create), fOrigin(nullptr)
{
   // Import geometry described by TGeoManager into KVMultiDetArray object
   //
   // if create=kFALSE, we do not create any detectors etc., but just set up
   // the required links between the geometry and the existing array detectors
   //
   // We change the colours of volumes depending on their material

   fArray = m;
   fRangeTable = r;
   fCurrentTrajectory.SetAddToNodes(kFALSE);
   fCurrentTrajectory.SetPathInTitle(kFALSE);
   fCheckDetVolNames = kFALSE;

   std::map<std::string, EColor> colors;
   colors["Si"] = EColor(kGray + 1);
   colors["CsI"] = EColor(kBlue - 8);
   colors["Al"] = EColor(kGray);
   colors["Cu"] = EColor(kOrange + 2);
   TGeoVolume* vol;
   TIter next(g->GetListOfVolumes());
   while ((vol = (TGeoVolume*)next())) {
      TGeoMedium* med = vol->GetMedium();
      if (!med) continue;
      TGeoMaterial* mat = med->GetMaterial();
      if (colors.find(mat->GetName()) != colors.end()) vol->SetLineColor(colors[mat->GetName()]);
      if (mat->GetDensity() < 0.1) vol->SetTransparency(60);
   }
}

KVGeoImport::~KVGeoImport()
{
   // Destructor
   SafeDelete(fOrigin);
}

void KVGeoImport::ParticleEntersNewVolume(KVNucleus*)
{
   // All detectors crossed by the particle's trajectory are added to the multidetector

   KVDetector* detector = GetCurrentDetector();
   if (!detector) return;
   detector->GetNode()->SetName(detector->GetName());
   if (fLastDetector && detector != fLastDetector) {
      fLastDetector->GetNode()->AddBehind(detector);
      detector->GetNode()->AddInFront(fLastDetector);
      fCurrentTrajectory.AddLast(fLastDetector->GetNode());
   }
   fLastDetector = detector;
}

void KVGeoImport::ImportGeometry(Double_t dTheta, Double_t dPhi,
                                 Double_t ThetaMin, Double_t PhiMin, Double_t ThetaMax, Double_t PhiMax)
{
   // Scan the geometry in order to find all detectors and detector alignments.
   // This is done by sending out "particles" from (0,0,0) or (x,y,z) (if SetOrigin(x,y,z) was called)
   // in all directions between (ThetaMin,ThetaMax) - with respect to Z-axis - and (PhiMin,PhiMax) - cylindrical
   // angle in the (X,Y)-plane, over a grid of step dTheta in Theta and dPhi in Phi.

   Int_t ndets0 = fArray->GetDetectors()->GetEntries();

   KVNucleusEvent EVT;
   KVEvent* evt = &EVT;
   KVNucleus* nuc = evt->AddParticle();
   nuc->SetZAandE(1, 1, 1);

   Info("ImportGeometry",
        "Importing geometry in angular ranges : Theta=[%f,%f:%f] Phi=[%f,%f:%f]", ThetaMin, ThetaMax, dTheta, PhiMin, PhiMax, dPhi);
   if (fOrigin)
      Info("ImportGeometry",
           "Origin for geometry = (%f,%f,%f)", fOrigin->x(), fOrigin->y(), fOrigin->z());
   Int_t count = 0;
   if (!KVDataAnalyser::IsRunningBatchAnalysis())
      std::cout << "\xd" << "Info in <KVGeoImport::ImportGeometry>: tested " << count << " directions" << std::flush;
   for (Double_t theta = ThetaMin; theta <= ThetaMax; theta += dTheta) {
      for (Double_t phi = PhiMin; phi <= PhiMax; phi += dPhi) {
         nuc->SetTheta(theta);
         nuc->SetPhi(phi);
         fLastDetector = nullptr;
         PropagateEvent(evt, fOrigin);
         count++;
         if (!KVDataAnalyser::IsRunningBatchAnalysis())
            std::cout << "\xd" << "Info in <KVGeoImport::ImportGeometry>: tested " << count << " directions" << std::flush;
      }
   }
   if (KVDataAnalyser::IsRunningBatchAnalysis())
      std::cout << "Info in <KVGeoImport::ImportGeometry>: tested " << count << " directions" << std::endl;
   else
      std::cout << std::endl;

   Info("ImportGeometry",
        "Imported %d detectors into array", fArray->GetDetectors()->GetEntries() - ndets0);

   // make sure detector nodes are correct
   TIter next(fArray->GetDetectors());
   KVDetector* d;
   while ((d = (KVDetector*)next())) {
      d->GetNode()->RehashLists();
      d->SetNameOfArray(fArray->GetName());
   }
   // set up all detector node trajectories
   fArray->AssociateTrajectoriesAndNodes();
   // create all groups
   fArray->DeduceGroupsFromTrajectories();

   if (fCreateArray) {
      fArray->SetGeometry(GetGeometry());
      // Set up internal navigator of array with all informations on detector/
      // structure name formatting, correspondance lists, etc.
      KVGeoNavigator* nav = fArray->GetNavigator();
      nav->SetDetectorNameFormat(fDetNameFmt);
      for (int i = 0; i < fStrucNameFmt.GetEntries(); i++) {
         KVNamedParameter* fmt = fStrucNameFmt.GetParameter(i);
         nav->SetStructureNameFormat(fmt->GetName(), fmt->GetString());
      }
      nav->SetNameCorrespondanceList(fDetStrucNameCorrespList);
      nav->AbsorbDetectorPaths(this);
      fArray->CalculateDetectorSegmentationIndex();
      fArray->DeduceIdentificationTelescopesFromGeometry();
      fArray->CalculateReconstructionTrajectories();
   }
}

void KVGeoImport::SetLastDetector(KVDetector* d)
{
   fLastDetector = d;
}

void KVGeoImport::PropagateParticle(KVNucleus* nuc, TVector3* TheOrigin)
{
   // Override KVGeoNavigator method
   // We build the list of all trajectories through the array

   fCurrentTrajectory.Clear();

   KVGeoNavigator::PropagateParticle(nuc, TheOrigin);

   if (fLastDetector && fLastDetector->GetNode()) {
      if (fCurrentTrajectory.GetN()) {
         if (!fCurrentTrajectory.Contains(fLastDetector->GetNode())) {
            fCurrentTrajectory.AddLast(fLastDetector->GetNode());
         }
         fCurrentTrajectory.ReverseOrder();
      }
      else {
         fCurrentTrajectory.AddLast(fLastDetector->GetNode());
      }
      if (!fArray->GetTrajectories()->FindObject(fCurrentTrajectory.GetName())) {
         KVGeoDNTrajectory* tr = new KVGeoDNTrajectory(fCurrentTrajectory);
         fArray->AddTrajectory(tr);
      }
   }
}

void KVGeoImport::AddAcceptedDetectorName(const char* name)
{
   // Add to list of acceptable detector names when scanning geometry
   // Each detector volume name will be tested; if it doesn't contain
   // any of the (partial) detector names in the list, it will be ignored

   fCheckDetVolNames = kTRUE;
   fAcceptedDetectorNames.SetValue(name, 1);
}

KVDetector* KVGeoImport::GetCurrentDetector()
{
   // Returns pointer to KVDetector corresponding to current location
   // in geometry. Detector is created and added to array if needed.
   // We also set up any geometry structure elements (from nodes beginning with "STRUCT_")
   // Any detector volume with a name not recognised by comparison with the list
   // fAcceptedDetectorNames (if defined) will be ignored.

   KVString detector_name;
   Bool_t multilay;
   TGeoVolume* detector_volume = GetCurrentDetectorNameAndVolume(detector_name, multilay);
   // failed to identify current volume as part of a detector

   if (!detector_volume) return 0;
   // check volume belongs to us
   if (fCheckDetVolNames) {
      int nn = fAcceptedDetectorNames.GetNpar();
      bool reject_volume = true;
      for (int i = 0; i < nn; ++i) {
         if (detector_name.Contains(fAcceptedDetectorNames.GetNameAt(i))) {
            reject_volume = false;
            break;
         }
      }
      if (reject_volume) return 0;
   }

   // has detector already been built ? if not, do it now
   KVDetector* det = fArray->GetDetector(detector_name);
   if (!fCreateArray) {
      if (det) {
         // set matrix & shape for entrance window if not done yet
         if (!det->GetEntranceWindow().ROOTGeo()) {
            det->SetEntranceWindowMatrix(GetCurrentMatrix());
            det->SetEntranceWindowShape((TGeoBBox*)GetCurrentVolume()->GetShape());
         }
         TString vol_name(GetCurrentVolume()->GetName());
         if (!multilay || vol_name.BeginsWith("ACTIVE_")) {
            // set matrix & shape for active layer
            det->SetActiveLayerMatrix(GetCurrentMatrix());
            det->SetActiveLayerShape((TGeoBBox*)GetCurrentVolume()->GetShape());
            // set full path to physical node as title of detector's node (KVGeoDetectorNode)
            det->GetNode()->SetTitle(GetCurrentPath());
         }
         // add entry to correspondance list between physical nodes and detectors (all layers)
         fDetectorPaths.Add(new KVGeoDetectorPath(GetCurrentPath(), det));
      }
   }
   else {
      if (!det) {
         det = BuildDetector(detector_name, detector_volume);
         if (det) {
            // Setting the entrance window shape and matrix
            // ============================================
            // for consistency, the matrix and shape MUST correspond
            // i.e. we cannot have the matrix corresponding to the entrance window
            // of a multilayer detector and the shape corresponding to the
            // whole detector (all layers) - otherwise, calculation of points
            // on detector entrance window will be false!
//                Info("GetCurrentDetector","Setting EW matrix to current matrix:");
//                GetCurrentMatrix()->Print();
            det->SetEntranceWindowMatrix(GetCurrentMatrix());
            det->SetEntranceWindowShape((TGeoBBox*)GetCurrentVolume()->GetShape());
            TString vol_name(GetCurrentVolume()->GetName());
            if (!multilay || vol_name.BeginsWith("ACTIVE_")) {
               // first layer of detector (or only layer) is also active layer
//                    Info("GetCurrentDetector","and also setting active layer matrix to current matrix:");
//                    GetCurrentMatrix()->Print();
               det->SetActiveLayerMatrix(GetCurrentMatrix());
               det->SetActiveLayerShape((TGeoBBox*)GetCurrentVolume()->GetShape());
               // set full path to physical node as title of detector's node (KVGeoDetectorNode)
               det->GetNode()->SetTitle(GetCurrentPath());
            }
            // add entry to correspondance list between physical nodes and detectors (all layers)
            fDetectorPaths.Add(new KVGeoDetectorPath(GetCurrentPath(), det));
            fArray->Add(det);
            Int_t nstruc = CurrentStructures().GetEntries();
            if (nstruc) {
               // Build and add geometry structure elements
               KVGeoStrucElement* ELEM = fArray;
               for (int i = 0; i < nstruc; i++) {
                  KVGeoStrucElement* elem = (KVGeoStrucElement*)CurrentStructures()[i];
                  KVGeoStrucElement* nextELEM = ELEM->GetStructure(elem->GetName());
                  if (!nextELEM) {
                     // make new structure
                     nextELEM = new KVGeoStrucElement(elem->GetName(), elem->GetType());
                     nextELEM->SetNumber(elem->GetNumber());
                     ELEM->Add(nextELEM);
                  }
                  ELEM = nextELEM;
               }
               // add detector to last structure
               ELEM->Add(det);
            }
         }
      }
      else {
         // Detector already built, are we now in its active layer ?
         TString vol_name(GetCurrentVolume()->GetName());
         if (!multilay || vol_name.BeginsWith("ACTIVE_")) {
//                Info("GetCurrentDetector","Setting active layer matrix to current matrix:");
//                GetCurrentMatrix()->Print();
            det->SetActiveLayerMatrix(GetCurrentMatrix());
            det->SetActiveLayerShape((TGeoBBox*)GetCurrentVolume()->GetShape());
            // set full path to physical node as title of detector's node (KVGeoDetectorNode)
            det->GetNode()->SetTitle(GetCurrentPath());
         }
         // add entry to correspondance list between physical nodes and detectors (all layers)
         fDetectorPaths.Add(new KVGeoDetectorPath(GetCurrentPath(), det));
      }
   }
   return det;
}

KVDetector* KVGeoImport::BuildDetector(TString det_name, TGeoVolume* det_vol)
{
   // Create a KVDetector with given name for the given volume
   //
   // #### Detector definition in geometry ####
   //  1. All detector volumes & nodes must have names which begin with **DET_**
   //
   //  2. They must be made of materials which are known by the range table fRangeTable.
   //
   //  3. For multi-layer detectors, the "active" layer volume/node must have a name beginning with **ACTIVE_**
   //
   //  4. The "thickness" of the detector or any layer inside a multilayer detector will be taken as the size of the volume's
   //     shape along its Z-axis (so make sure that you define your detector volumes in this way).
   //
   //  5. It is assumed that the natural length units of the geometry are centimetres.
   //
   //  6. The name of the KVDetector object created and added to the array will be taken
   //     from the unique full path of the node corresponding to the geometrical positioning
   //     of the detector, see KVGeoNavigator::ExtractDetectorNameFromPath()
   //
   //  7. The 'type' of the detector will be set to the name of the material
   //     in the detector's active layer i.e. if active layer material name is "Si",
   //     detector type will be 'Si'
   //
   //  8. Default class for all detectors is KVDetector. If you want to use another class
   //     you need to define it using SetDetectorPlugin() method and put the
   //     associated line in your .kvrootrc configuration file.


   KVDetector* d = 0;
   TPluginHandler* ph = NULL;
   if (fDetectorPlugin == "" || !(ph = LoadPlugin("KVDetector", fDetectorPlugin))) {
      d = new KVDetector;
   }
   else {
      d = (KVDetector*)ph->ExecPlugin(0);
   }

   d->SetName(det_name);

   Int_t nlayer = det_vol->GetNdaughters();
   if (nlayer) {
      for (int i = 0; i < nlayer; i++) {
         AddLayer(d, det_vol->GetNode(i)->GetVolume());
      }
   }
   else
      AddLayer(d, det_vol);
   TString type = d->GetActiveLayer()->GetName();
   //type.ToUpper();
   d->SetType(type);
   return d;
}

void KVGeoImport::AddLayer(KVDetector* det, TGeoVolume* vol)
{
   // Add an absorber layer to the detector.
   //
   // Volumes representing 'active' layers in detectors must have names
   // which begin with **ACTIVE_**.

   TString vnom = vol->GetName();
   // exclude dead zone layers
   if (vnom.BeginsWith("DEADZONE")) return;
   TGeoMaterial* material = vol->GetMaterial();
   KVIonRangeTableMaterial* irmat = fRangeTable->GetMaterial(material);
   if (!irmat) {
      Warning("AddLayer", "Unknown material %s/%s used in layer %s of detector %s",
              material->GetName(), material->GetTitle(), vol->GetName(), det->GetName());
      return;
   }
   TGeoBBox* sh = dynamic_cast<TGeoBBox*>(vol->GetShape());
   if (!sh) {
      Warning("AddLayer", "Unknown shape class %s used in layer %s of detector %s",
              vol->GetShape()->ClassName(), vol->GetName(), det->GetName());
      return; // just in case - for now, all shapes derive from TGeoBBox...
   }
   Double_t width = 2.*sh->GetDZ(); // thickness in centimetres
   KVMaterial* absorber;
   if (irmat->IsGas()) {
      Double_t p = material->GetPressure();
      Double_t T = material->GetTemperature();
      absorber = new KVMaterial(irmat->GetType(), width, p, T);
   }
   else
      absorber = new KVMaterial(irmat->GetType(), width);
   det->AddAbsorber(absorber);
   if (vnom.BeginsWith("ACTIVE_")) det->SetActiveLayer(absorber);
}

