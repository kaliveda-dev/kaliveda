//Created by KVClassFactory on Tue Sep  4 11:26:12 2012
//Author: Guilain ADEMARD

#include "KVVAMOS.h"
#include "KVVAMOSDetector.h"
#include "KVVAMOSTransferMatrix.h"
#include "KVVAMOSReconGeoNavigator.h"
#include "KVVAMOSWeightFinder.h"
#include "KVVAMOSCodes.h"
#include "KVBasicVAMOSFilter.h"
#include "KVChargeStateDistrib.h"

#include "KVGroup.h"
#include "KVDataSetManager.h"
#include "KVUpDater.h"
#include "KVFunctionCal.h"
#include "KVIDGridManager.h"
#include "KVUnits.h"

#include "TSystemDirectory.h"
#include "TPluginManager.h"
#include "TClass.h"

#include "TGeoBBox.h"

using namespace std;

ClassImp(KVVAMOS)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVVAMOS</h2>
   <h4>VAMOS: variable mode spectrometer at GANIL</h4>
   <!-- */
// --> END_HTML
//
// STRIPPING FOIL
//
// A stripping foil located at the entrance of VAMOS is used to equilibrate the
// ions charge state distribution. To define and set the stripping foil for your
// experiment use the following commands:
//
//   KVMaterial C_foil( 70*KVUnits::ug, "C" );
//   gVamos->SetStripFoil( &C_foil );
//
// or directly
//
//   gVamos->SetStripFoil( "C", 70 );
//
// In this example, the stripping foil is a Carbon foil with an area density equal
// to 70 ug/cm**2.
//
// If the stripping foil is defined, the calibrated energy of nuclei reconstructed
// in VAMOS will be corrected on the energy loss in this foil (see GetStripFoilEnergyLossCorrection(...)).
////////////////////////////////////////////////////////////////////////////////

KVVAMOS* gVamos;


KVString KVVAMOS::fACQParamTypes("0:E, 1:Q, 2:T_HF, 3:T, 9:NO_TYPE");
KVString KVVAMOS::fPositionTypes("0:X, 1:Y, 2:Z, 3:XY, 4:XZ, 5:YZ, 6:XYZ, 9:NO_TYPE");


KVVAMOS::KVVAMOS()
{
   // Default constructor
   init();
   gVamos = this;
}
//________________________________________________________________

KVVAMOS::KVVAMOS(const KVVAMOS& obj)  : KVMultiDetArray()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   init();
   obj.Copy(*this);
}
//________________________________________________________________

void KVVAMOS::init()
{
   //Basic initialisation called by constructor.
   //
   //Cleanups
   //The fDetectors list contains references to objects owned to VAMOS
   //spectrometer, but which may be deleted by other objects. Then we use
   //the ROOT automatic garbage collection to make sure that any object
   //deleted eslsewhere is removed automatically from this list.

   SetUniqueID(0);

   fFiredDets     = NULL;
   fVACQParams    = NULL;
   fCalibrators   = NULL;
   fVCalibrators  = NULL;
   fFPvolume      = NULL;
   fVAMOSvol      = NULL;
   fStripFoil     = NULL;
   fTransMatrix   = NULL;
   fZGOUBIReconstruction = NULL;
   fReconNavigator = NULL;
   fWeightFinder  = NULL;
   fRotation      = NULL;
   fFocalPos      = 0;
   fAngle         = 0;
   fBrhoRef       = -1;
   fBeamHF        = -1;
   fStripFoilPos  = 0;
   fECalibPar[0]  = fECalibPar[1] = fECalibPar[2]  = fECalibPar[3] = 0.;
   fECalibStatus  = kFALSE;
   //initalise ID grid manager
   if (!gIDGridManager)
      new KVIDGridManager;

   fFilter = NULL;
   SetToFlist_deltapath();
   Info("init", "To be implemented");
}
//________________________________________________________________

KVVAMOS::~KVVAMOS()
{
   // Destructor

   // Clear list of acquisition parameters belonging to VAMOS
   if (fVACQParams && fVACQParams->TestBit(kNotDeleted)) {
      fVACQParams->Clear();
      delete fVACQParams;
   }
   fVACQParams = NULL;

   SafeDelete(fCalibrators);
   SafeDelete(fVCalibrators);
   SafeDelete(fFiredDets);
   SafeDelete(fStripFoil);
   SafeDelete(fTransMatrix);
   SafeDelete(fZGOUBIReconstruction);
   SafeDelete(fReconNavigator);
   SafeDelete(fWeightFinder);
   SafeDelete(fFilter);

   if (gVamos == this) {
      gVamos = NULL;
      // destroy ROOT geometry associated
      delete gGeoManager;
   }
}
//________________________________________________________________

void KVVAMOS::BuildFocalPlaneGeometry(TEnv* infos)
{
   // Construction of the detector geometry at the focal plane of VAMOS for the.

   if (!fFPvolume) {
      //The FPvolume has the size of the the Focal plane detection chamber
//    TGeoMedium *Vacuum = gGeoManager->GetMedium( "Vacuum" );
//
//    if( !Vacuum ){
//       TGeoMaterial*matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
//          matVacuum->SetTitle("Vacuum");
//          Vacuum = new TGeoMedium("Vacuum", 1, matVacuum);
//    }
//
//    Double_t d  = infos->GetValue("VAMOS.FOCALCHAMBER.DEPTH" , 207.5 );
//    Double_t w  = infos->GetValue("VAMOS.FOCALCHAMBER.WIDTH" , d     );
//    Double_t h  = infos->GetValue("VAMOS.FOCALCHAMBER.HEIGHT", d     );
//
//    fFPvolume   = gGeoManager->MakeBox("FocalPlane", Vacuum,  w/2, h/2, d/2);
      fFPvolume = gGeoManager->MakeVolumeAssembly("FocalPlane");
   }

   fDetectors.R__FOR_EACH(KVVAMOSDetector, BuildGeoVolume)(infos, fFPvolume);
}
//________________________________________________________________

Bool_t KVVAMOS::BuildGeoVolume(TEnv* infos)
{
   // Build all the other volumes wich are not associated to the focal
   // plane detectors. Informations for the construction are read in
   // the TEnv object 'infos' ( see BuildVAMOSGeometry() ).
   //
   // Example of volumes built here:
   //              -vamos assembly volume
   //              -dipole and quadrupole
   //                  ....

   Info("BuildGeoVolume", "Method to be completed");

   TGeoVolume* top = gGeoManager->GetTopVolume();

   // this volume assembly allow the rotation of VAMOS around the
   // target
   fVAMOSvol = gGeoManager->MakeVolumeAssembly("VAMOS");
// fVAMOSvol = gGeoManager->MakeBox("VAMOS", Vacuum,  200, 200, 500);//TO BE CHANGE

   if (fRotation) fRotation->Clear();
   else {
      fRotation =  new TGeoRotation("VAMOSrotation");
      fRotation->RotateY(GetAngle());
   }
   top->AddNode(fVAMOSvol, 1, fRotation);

   // place the focal plane detection chamber from target position.
   fFocalPos =  infos->GetValue("VAMOS.FOCALPOS", 0.);

   // For the moment we place it 10cm just after the target
   TGeoMatrix* matrix = new TGeoTranslation("focal_pos", 0., 0., 10 + ((TGeoBBox*)fFPvolume->GetShape())->GetDZ()); // TO BE CHANGED
   fVAMOSvol->AddNode(fFPvolume, 1, matrix);

   return kTRUE;
}
//________________________________________________________________

void KVVAMOS::BuildVAMOSGeometry()
{

   // Construction of the geometry of VAMOS spectrometer.
   // The informations used for building the VAMOS geometry are read in
   // all the files with the .cao extension in $KVROOT/KVFiles/<DataSet>/VAMOSgeometry
   // directory.
   //
   // This method will create an instance of TGeoManager (any previous existing geometry gGeoManager
   // will be automatically deleted) and initialise it with the full geometry of VAMOS
   //Every detector at the focal plane will be represented in the resulting geometry.
   //
   // For information on using the ROOT geometry package, see TGeoManager and related classes,
   // as well as the chapter "The Geometry Package" in the ROOT Users' Guide.
   //
   // The half-lengths of the "world"/"top" volumei, into which all the detectors
   //  are placed, are equal to 500 cm. This should be big enough so that all detectors.
   // This "world" is a cube 1000cmx1000cmx1000cm (with sides going from -500cm to +500cm on each axis).

   TEnv infos;
   if (!LoadGeoInfosIn(&infos)) {
      Error("BuildVAMOSGeometry", "No information found to build VAMOS geometry");
      return;
   }

   CreateGeoManager();

   BuildFocalPlaneGeometry(&infos);
   BuildGeoVolume(&infos);

   // The option "i" is important to give unique ID for each existing
   // Node. This is done in InitGeometry()
   gGeoManager->CloseGeometry("i");

}
//________________________________________________________________

void KVVAMOS::InitGeometry()
{
   // Initialize the geometry of VAMOS and of the detectors
   // placed at the focal plane.

   TGeoNode*   node = NULL;
   TString     vname;

   // we give the unique ID for each node to simplify navigation
   // inside the geometry. We set the same ID for the volume
   // having the same name of the node (we assume that each
   // volume is associated to one node only)
   for (Int_t i = 0; i < gGeoManager->GetNNodes(); i++) {
      gGeoManager->CdNode(i);
      node = gGeoManager->GetCurrentNode();
      node->SetUniqueID(gGeoManager->GetCurrentNodeId());
      node->GetVolume()->SetUniqueID(gGeoManager->GetCurrentNodeId());
   }

   fFocalToTarget.SetName("focal_to_target");
   GeoModified();
   UpdateGeometry();

}
//________________________________________________________________

Int_t KVVAMOS::LoadGeoInfosIn(TEnv* infos)
{
   // Load in a TEnv object the informations concerning the geometry of VAMOS.
   // These informations  are read in all the files with the .cao extension in
   // $KVROOT/KVFiles/<DataSet>/VAMOSgeometry directory.
   // Returns the number of files read.

   // keep the current workind directory
   TString old_dir = gSystem->WorkingDirectory();

   // Reading geometry iformations in .cao files
   const Char_t dirname[] = "VAMOSgeometry";
   TString path(KVBase::GetDATADIRFilePath());
   path += "/";
   if (gDataSet) {
      path += gDataSet->GetName();
      path += "/";
   }
   path += dirname;
   TSystemDirectory dir(dirname, path.Data());
   TList* lfiles = dir.GetListOfFiles();
   TIter nextfile(lfiles);
   TSystemFile* file = NULL;
   Int_t Nfiles = 0;
   while ((file = (TSystemFile*)nextfile())) {
      path.Form("%s/%s", file->GetTitle(), file->GetName());
      if (!path.EndsWith(".cao")) continue;
      infos->ReadFile(path.Data(), kEnvAll);
      Nfiles++;
   }
   delete lfiles;

   // The call of TSystemDirectory::GetListOfFiles() can
   // change the working directory then we come back to the
   // previous one
   gSystem->ChangeDirectory(old_dir.Data());

   return Nfiles;
}
//________________________________________________________________

void KVVAMOS::MakeListOfDetectors()
{
   // Build detectors of VAMOS.
   // The detectors are defined and associated to their TGeoVolume's
   // which compose it.

   TString envname = Form("%s.DetectorList", ClassName());

   KVString list = gDataSet->GetDataSetEnv(envname.Data());
   list.Begin(" ");
   KVNumberList numlist;
   Int_t det_class_num = 1;
   // Loop over each detector of the spectrometer
   while (!list.End()) {
      KVString detname = list.Next(kTRUE);
      TClass* detcl = TClass::GetClass(detname.Data());
      if (!detcl) {
         Error("KVVAMOS", "class %s not found in the dictionary", detname.Data());
         continue;
      }
      else if (!detcl->InheritsFrom("KVVAMOSDetector")) {
         Error("KVVAMOS", "class %s does not inherits from KVVAMOSDetector", detname.Data());
      }

      envname.Form("%s.%s.Number", ClassName(), detname.Data());
      numlist.SetList(gDataSet->GetDataSetEnv(envname.Data()));
      numlist.Begin();
      // Loop over detectors with same type.
      // Different by their number
      while (!numlist.End()) {
         Int_t num = numlist.Next();

         // Making the detector
         KVSpectroDetector* det = (KVSpectroDetector*) detcl->New();
         det->SetNumber(num);
         det->SetUniqueID(10 * num + (det_class_num));
         det->SetName(det->GetArrayName());
         fDetectors.Add(det);
      }
      det_class_num++;
   }
}
//________________________________________________________________

Bool_t KVVAMOS::ReadDetectorGroupFile(ifstream& ifile)
{
   //Reads the groups of detectors to build for VAMOS from file loaded
   //in ifile and stores them as a structure.
   //Before reading, all old groups are deleted. A number is set
   //to each new group.
   //
   //The comment lines begin with '#'.
   //
   //Each line correspond to a list with the name of each detector which
   //can be punshed through by a same trajectory. Each name is
   //separated by a space.
   //A detector can only belong to one group.

   ClearStructures("GROUP");          // clear out (delete) old groups

   KVString sline, detname;
   while (ifile.good()) {         //reading the file
      sline.ReadLine(ifile);

      if (sline.IsNull()) continue;
      // Skip comment line
      if (sline.BeginsWith("#")) continue;

      KVGroup*           cur_grp  = NULL;
      KVGroup*           det_grp  = NULL;
      KVSpectroDetector* det      = NULL;
      KVSpectroDetector* lastDet  = NULL;

      sline.ReplaceAll("\t", " ");
      sline.Begin(" ");
      while (!sline.End()) {

         detname = sline.Next(kTRUE);
         det     = (KVSpectroDetector*)GetDetector(detname.Data());

         if (det) {
            det_grp = det->GetGroup();

            if (!cur_grp) {
               if (det_grp) cur_grp = det_grp;
               else {
                  cur_grp = new KVGroup;
                  cur_grp->SetNumber(++fGr);
                  cur_grp->Add(det);
                  Add(cur_grp);
               }
            }
            else {
               if (det_grp && det_grp != cur_grp)
                  Warning("ReadDetectorGroupFile",
                          "Detector %s : already belongs to %s, now seems to be in %s",
                          det->GetName(), det_grp->GetName(),
                          cur_grp->GetName());
               else cur_grp->Add(det);
            }

            det->GetNode()->SetName(det->GetName());

            if (lastDet && det != lastDet) {
               lastDet->GetNode()->AddBehind(det);
               det->GetNode()->AddInFront(lastDet);
            }
            lastDet = det;

            // Sort the detector list of each group
            // from the the closest from the target to the furthest.
            cur_grp->SortDetectors(kSortAscending);
         }
         else Error("ReadDetectorGroupFile", "Detector %s not found", detname.Data());
      }
   }
   return kTRUE;
}
//________________________________________________________________

void KVVAMOS::SetArrayACQParams()
{
   // Add acquisition parameters which are not
   // associated to a detector. The list of ACQ parameters is
   // defined in environment variables such as
   // [dataset name].KVVAMOS.ACQParameterList: TSI_HF TSED1_SED2 ...
   // in the .kvrootrc file.

   if (fVACQParams) fVACQParams->Clear();
   TString envname = Form("%s.ACQParameterList", ClassName());

   TString list = gDataSet->GetDataSetEnv(envname.Data());
   TObjArray* tok = list.Tokenize(" ");
   TIter nextparam(tok);
   TObject* obj = NULL;
   Info("SetArrayACQParams", "List of ACQ Parameters belonging to %s:", GetName());

   KVEBYEDAT_ACQParam* par = NULL;
   Short_t num = 1;
   while ((obj = nextparam())) {
      TString param(obj->GetName());
      cout << param.Data() << " ";
      // VAMOS is the owner of the acq param. (kTRUE)
      par = new KVEBYEDAT_ACQParam(param.Data());
      par->SetNumber(num++);
      if (param.BeginsWith("T")) {
         if (param.EndsWith("HF")) {
            par->SetType("T_HF");
            par->SetLabel("HF");
         }
         else par->SetType("T");
      }
      par->SetWorking(gDataSet->GetDataSetEnv(Form("KVACQParam.%s.Working", par->GetName()), kTRUE));
      AddACQParam(par, kTRUE);
   }
   cout << endl;
   delete tok;
}
//________________________________________________________________

void KVVAMOS::SetCalibrators()
{
   // Note that this only initialises the calibrators objects associated
   // to each detector (defined in each detector class's SetCalibrators method) and to VAMOS.
   // It does not set the parameters of the calibrations: this is done
   // by SetParameters.

   TIter nextdet(GetDetectors());
   KVDetector* det = NULL;
   while ((det = (KVDetector*)nextdet())) {
      det->SetCalibrators();

      KVSeqCollection* l = det->GetListOfCalibrators();
      TIter nextcal(l);
      KVCalibrator* cal  = NULL;
      while ((cal = (KVCalibrator*)nextcal())) {
         if (!AddCalibrator(cal, kFALSE))
            Error("SetCalibrators", "Duplicated calibrator %s of detector %s in VAMOS's list", cal->GetTitle(), det->GetName());
      }
   }

   // For ACQ parameters belonging to VAMOS.
   // We set only calibrators for time of flight ACQ parameters,
   // we assume that their name begin with the character 'T'.
   // Their calibrator will have 'channel->ns' type.

   TString calibtype;
   KVEBYEDAT_ACQParam*    par  = NULL;
   KVFunctionCal* c    = NULL;
   TF1*           func = NULL;

   TIter nextpar(fVACQParams);
   while ((par = (KVEBYEDAT_ACQParam*)nextpar())) {
      if ((*par->GetType()) != 'T') continue;
      calibtype = "channel->ns";
      calibtype.Append(" ");
      calibtype.Append(par->GetName());

      func = new TF1(calibtype.Data(), "pol1", 0., 16384.);
      c = new KVFunctionCal(func);
      c->SetType(calibtype.Data());
      c->SetLabel(par->GetLabel());
      c->SetUniqueID(par->GetUniqueID());
      c->SetACQParam(par);
      c->SetStatus(kFALSE);
      if (!AddCalibrator(c, kTRUE)) delete c;
   }
}
//________________________________________________________________

void KVVAMOS::SetGroupsAndIDTelescopes()
{
   //Build groups of detectors (KVGroup) from the file given by
   //the environment variable:
   //[dataset name]. KVVAMOS.DetectorGroupFile: ...
   //( See ReadDetectorGroupFile(...) method )
   //
   //In each group, the list of detectors is sorted in order to list
   //them from the farest one to the closest one.
   //
   //Also creates all ID telescopes (DeltaE-E, Fast-Slow ) used for the Z-identification
   //in VAMOS and stores them in fIDTelescopes.
   //
   //Any previous groups/idtelescopes are deleted beforehand.
   //
   //As any ID grids stored in gIDGridManager will have been associated to the
   //old ID telescopes (whose addresses now become obsolete), make sure that
   //you deleted  all grids associated to VAMOS before to call this method.
   //You should therefore follow this with a call to SetIdentifications()
   //in order to reinitialize all that.

   TString envname, filename;
   envname.Form("%s.DetectorGroupFile", ClassName());
   filename = gDataSet->GetDataSetEnv(envname.Data());
   ifstream ifile;
   if (!SearchAndOpenKVFile(filename.Data(), ifile, gDataSet->GetName())) {
      Error("SetGroupsAndIDTelescopes", "Could not open file %s", filename.Data());
      return;
   }

   Info("SetGroupsAndIDTelescopes", "Reading file with the list of detector groups of VAMOS: %s", filename.Data());

   if (!ReadDetectorGroupFile(ifile)) Error("SetGroupsAndIDTelescopes", "Bad structure inside the file %s", filename.Data());

   ifile.close();

   //now read list of groups and create list of ID telescopes
   CreateIDTelescopesInGroups();

   //Set uniqueID for each ID telescopes
   TIter next(GetListOfIDTelescopes());
   TObject* idt = NULL;
   Int_t     id = 0;
   while ((idt = next())) idt->SetUniqueID(id++);
}
//________________________________________________________________

void KVVAMOS::UpdateGeometry()
{
   // Update the geometry of VAMOS.
   // This method has to be called when the geometry is modified and
   // before using of methods giving information about geometry.
   //
   // The focal-plane to target matrix is calculated again and set
   // to each detector.


   if (!IsGeoModified()) return;

   Int_t prev_id = gGeoManager->GetCurrentNodeId();

   // To be sure that the matrices will be calculated again
   gGeoManager->CdTop();
   // Focal-plane to target matrix
   gGeoManager->CdNode(fFPvolume->GetUniqueID());
   fFocalToTarget = *gGeoManager->GetCurrentMatrix();

   // Initialize the geometry of the detectors
   TIter next_det(GetDetectors());
   KVVAMOSDetector* det = NULL;
   while ((det = (KVVAMOSDetector*)next_det())) {
      det->SetFocalToTargetMatrix(&fFocalToTarget);
   }

   ResetBit(kGeoModified);

   // just to not create problems if this method is called during a tracking
   gGeoManager->CdNode(prev_id);
}
//________________________________________________________________

void KVVAMOS::AddACQParam(KVEBYEDAT_ACQParam* par, Bool_t owner)
{
   // Add an acquisition parameter corresponding to a detector
   // at the focal plane of the spectrometer. The fACQParams and fVACQParams
   // lists are added to the list of cleanups (gROOT->GetListOfCleanups).
   // Each acqisition parameter has its kMustCleanup bit set.
   // Thus, if this acq. parameter is deleted, it is automatically
   // removed from the lists by ROOT.

   if (!par) {
      Warning("AddACQParam", "Null pointer passed as argument");
      return;
   }

   // Add ACQ param. in global list
   KVMultiDetArray::AddACQParam(par);

   // Add ACQ param. in list of VAMOS if it is owner
   if (owner) {
      if (!fVACQParams) {
         fVACQParams = new KVList;
         fVACQParams->SetName(Form("List of ACQ param. belonging to %s", GetName()));
         fVACQParams->SetCleanup(kTRUE);
      }
//    par->SetDetector( this );
      fVACQParams->Add(par);
      par->SetUniqueID(CalculateUniqueID(par));
   }
   else par->SetUniqueID(CalculateUniqueID(par, (KVVAMOSDetector*)par->GetDetector()));
}
//________________________________________________________________

Bool_t KVVAMOS::AddCalibrator(KVCalibrator* cal, Bool_t owner)
{
   //Associate a calibration with  VAMOS. owner = true means that
   //the calibrator is associated to an ACQ Parameter belonging to VAMOS
   //( for example all the calibrators of times of flight).
   //owner = false means the calibrator is associate to an ACQ parameter
   //belonging to a detector at the focal plane (Energies, charges, ...).
   //If the calibrator object has the same class and type
   //as an existing object in the list (see KVCalibrator::IsEqual),
   //it will not be added to the list
   //(avoids duplicate calibrators) and the method returns kFALSE.
   //
   if (!fCalibrators) {
      fCalibrators = new KVHashList();
      fCalibrators->SetOwner(kFALSE);
   }
   if (fCalibrators->FindObject(cal)) return kFALSE;
   fCalibrators->Add(cal);

   if (owner) {
      if (!fVCalibrators) {
         fVCalibrators = new KVList();
      }
      fVCalibrators->Add(cal);
   }

   return kTRUE;
}
//________________________________________________________________

void KVVAMOS::Build(Int_t run)
{
   // Build the VAMOS spectrometer: detectors, geometry, identification
   // telescopes, acquisition parameters and calibrators.
   //
   // After geometry building, the detectors are sorted in the
   // list fDetectors from the Z coordinate of their first active volume,
   // in the focal Plan reference frame: from the lowest Z to the highest Z.

   IGNORE_UNUSED(run);

   if (IsBuilt()) return;

   SetName("VAMOS");
   SetTitle("VAMOS spectrometer");
   MakeListOfDetectors();
   BuildVAMOSGeometry();
   InitGeometry();
   fDetectors.Sort(kSortDescending);
   SetGroupsAndIDTelescopes();
   SetACQParams();
   SetCalibrators();
   SetIdentifications();
   Initialize();
   SetBit(kIsBuilt);
}
//________________________________________________________________

void KVVAMOS::Clear(Option_t* opt)
{
   // Call "Clear" method of each and every detector in VAMOS,
   // to reset energy loss and KVDetector::IsAnalysed() state
   // plus ACQ parameters set to zero

   IGNORE_UNUSED(opt);

   fDetectors.R__FOR_EACH(KVDetector, Clear)();

   if (fVACQParams) {
      TIter next(fVACQParams);
      KVEBYEDAT_ACQParam* par = NULL;
      while ((par = (KVEBYEDAT_ACQParam*) next())) par->Clear();
   }
}
//________________________________________________________________

void KVVAMOS::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVVAMOS::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVMultiDetArray::Copy(obj);
   //KVVAMOS& CastedObj = (KVVAMOS&)obj;
}
//________________________________________________________________

KVNameValueList* KVVAMOS::DetectParticle(KVNucleus* part)
{

   if (!fFilter) {
      //Build Filter
      TString fullpath;
      SearchKVFile("KVBasicVAMOSFilter.root", fullpath, gDataSet->GetName());
      TFile file(fullpath.Data());
      if (file.IsZombie()) {
         Error("DetectParticle", "Impossible to open file %s with VAMOS filter", fullpath.Data());
         return NULL;
      }
      else {
         fFilter = (KVBasicVAMOSFilter*)file.Get("BasicVAMOSfilter");
         Info("DetectParticle", "VAMOS filter is Loaded from file %s", fullpath.Data());
         fFilter->Print();
      }
   }

   static KVChargeStateDistrib csd;

   Int_t Q = part->GetParameters()->GetIntValue("Q");
   Q = (Q > 0 ? Q : Int_t(csd.GetRandomQ(part)));
   if (Q <= 0) return NULL;

   Double_t  Brho = part->GetMomentum().Mag() / (KVNucleus::C() * 10.*Q);
   Double_t  delta = Brho / GetBrhoRef();

   // spherical angles in VAMOS reference frame
   Double_t  th_l = part->Theta();               //rad
   Double_t  ph_l = part->Phi() + TMath::Pi() / 2; //rad

   // normalized cartesian coordinates in VAMOS reference frame
   Double_t  x    = TMath::Sin(th_l) * TMath::Cos(ph_l);
   Double_t  y    = TMath::Sin(th_l) * TMath::Sin(ph_l);
   Double_t  z    = TMath::Cos(th_l);

   // optical angles in VAMOS reference frame
   Double_t  th_v = TMath::RadToDeg() * TMath::ATan(x / z); //degree
   Double_t  ph_v = TMath::RadToDeg() * TMath::ASin(y);  //degree

   KVNameValueList* NVL = 0;
   if (fFilter->IsTrajectoryAccepted(GetAngle(), delta, th_v, ph_v)) {

      if (!NVL) NVL = new KVNameValueList;
      Double_t de = 0.;
      NVL->SetValue("VAMOS", de);
      part->GetParameters()->SetValue("Q", Q);
      part->GetParameters()->SetValue("Brho", Brho);
      part->GetParameters()->SetValue("Delta", delta);
      part->GetParameters()->SetValue("ThetaV", th_v);
      part->GetParameters()->SetValue("PhiV", ph_v);
   }


   return NVL;
}
//________________________________________________________________

void KVVAMOS::FocalToTarget(const Double_t* focal, Double_t* target)
{
   // Convert the point coordinates from focal plane reference to target reference system.
   GetFocalToTargetMatrix().LocalToMaster(focal, target);
}
//________________________________________________________________

void KVVAMOS::FocalToTargetVect(const Double_t* focal, Double_t* target)
{
   // Convert the vector coordinates from focal plane reference to target reference system.
   GetFocalToTargetMatrix().LocalToMasterVect(focal, target);
}
//________________________________________________________________

Bool_t KVVAMOS::Calibrate(KVReconstructedNucleus* nuc)
{
   // Calculate and set the energy of a reconstructed nucleus from
   // the energy measured in the detectors listed in the detector
   // list of this nucleus. A calibration function is used:
   // Ecal = C_0 + C_1*E_CHI + C_2*E_SI + C_3*E_CSI;
   // where C_i are calibration parameters and E_i calibrated
   // energy of the detector i (KVVAMOSDetector::GetEnergy()).
   // To set these parameters use SetECalibParameters(...) method.
   //status code

   nuc->SetECode(kECode0);
   if (!GetECalibStatus()) return kFALSE;

   TIter next(nuc->GetDetectorList());
   KVVAMOSDetector* det = NULL;
   KVVAMOSDetector* csi, *si, *chi;
   csi = si = chi = NULL;
   while ((det = (KVVAMOSDetector*)next())) {
      if (det->IsType("CSI")) csi = det;
      else if (det->IsType("SI")) si = det;
      else if (det->IsType("CHI")) chi = det;
   }

   Double_t E =  fECalibPar[0]
                 + (chi ? fECalibPar[1] * chi->GetEnergy() : 0.)
                 + (si  ? fECalibPar[2] * si->GetEnergy() : 0.)
                 + (csi ? fECalibPar[3] * csi->GetEnergy() : 0.);

   if (E <= 0) return kFALSE;

   nuc->SetEnergy(E);
   nuc->SetECode(kECode1);
   nuc->SetIsCalibrated();

   return kTRUE;
}
//________________________________________________________________

KVList* KVVAMOS::GetFiredDetectors(Option_t* opt)
{
   //Fills 'list' with the fired detectors. The option 'opt' is
   //set to the method KVSpectroDetector::Fired( opt ) used to know
   //if a detector is fired. The list is not the owner of its content.
   //It is a member variable fFiredDets of the class and will be deleted
   //with this KVVAMOS object.

   if (fFiredDets) fFiredDets->Clear();
   else fFiredDets = new KVList(kFALSE);

   TIter next(&fDetectors);
   KVSpectroDetector* det = NULL;
   while ((det = (KVSpectroDetector*)next())) {
      if (det->Fired(opt)) fFiredDets->Add(det);
   }
   return fFiredDets;
}
//________________________________________________________________
Int_t KVVAMOS::GetIDTelescopes(KVDetector* de, KVDetector* e,
                               TCollection* idtels)
{
   //Overwrite the same method of KVMultiDetArray in order to use another
   //format for the URI of the plugins associated to VAMOS.
   //Create a KVIDTelescope from the two detectors and add it to the list.
   //If no detector is segmented (i.e. KVDetector::GetSegment()<1) then no
   //KVIDTelescope is created.
   //
   // # For each pair of detectors we look for now a plugin with one of the following names:
   // #    [name_of_dataset].name_of_vamos.de_detector_type[de detector thickness]-e_detector_type[de detector thickness]
   // # Each characteristic in [] brackets may or may not be present in the name; first we test for names
   // # with these characteristics, then all combinations where one or other of the characteristics is not present.
   // # In addition, we first test all combinations which begin with [name_of_dataset].
   // # The first plugin found in this order will be used.
   // # In addition, if for one of the two detectors there is a plugin called
   // #    [name_of_dataset].name_of_vamos.de_detector_type[de detector thickness]
   // #    [name_of_dataset].name_of_vamos.e_detector_type[e detector thickness]
   // # then we add also an instance of this 1-detector identification telescope.
   //
   //This method is called by KVGroup in order to set up all ID telescopes
   //of the array.
   //
   // Returns number of ID telescopes created

   if (!(de->IsOK() && e->IsOK())) return 0;

   if ((de->GetSegment() < 1) && (e->GetSegment() < 1)) return 0;

   KVIDTelescope* idt = NULL;

   if (fDataSet == "" && gDataSet) fDataSet = gDataSet->GetName();

   Int_t ntels(0);

   //first we look for ID telescopes specific to current dataset
   //these are ID telescopes formed from two distinct detectors
   TString uri;

   // prefix of the URI
   TString prefixes[5];
   TString prefix;
   // for E-DE identification (Z)
   prefixes[0].Form("%s.%s.", fDataSet.Data(), GetName());
   prefixes[1].Form("%s.", GetName());
   // for Q and A identification with A-A/Q or Q-A/Q maps
   prefixes[2].Form("%s.%s.QA.", fDataSet.Data(), GetName());
   prefixes[3].Form("%s.QA.", GetName());
   prefixes[4] = "";

   //look for ID telescopes with only one of the two detectors
   KVDetector* dets[3] = { de, e, NULL };
   KVDetector* det     = NULL;

   for (UChar_t i = 0; (det = dets[i]);  i++) {
      for (UChar_t j = 0; !((prefix = prefixes[j]).IsNull()); j++) {

         uri.Form("%s%s%d", prefix.Data(), det->GetType(),
                  TMath::Nint(det->GetThickness()));
         if ((idt = KVIDTelescope::MakeIDTelescope(uri.Data()))) {
            set_up_single_stage_telescope(det, idt, idtels);
            ++ntels;
            break;
         }

         uri.Form("%s%s", prefix.Data(), det->GetType());
         if ((idt = KVIDTelescope::MakeIDTelescope(uri.Data()))) {
            set_up_single_stage_telescope(det, idt, idtels);
            ++ntels;
            break;
         }
      }
   }

   //look for ID telescopes with the two detectors
   if (de == e) return ntels;

   Int_t de_thick = TMath::Nint(de->GetThickness());
   Int_t e_thick  = TMath::Nint(e->GetThickness());

   for (UChar_t j = 0; !((prefix = prefixes[j]).IsNull()); j++) {

      uri.Form("%s%s%d-%s%d", prefix.Data(), de->GetType(),
               de_thick, e->GetType(), e_thick);
      if ((idt = KVIDTelescope::MakeIDTelescope(uri.Data()))) {
         set_up_telescope(de, e, idt, idtels);
         ++ntels;
         break;
      }

      uri.Form("%s%s%d-%s", prefix.Data(), de->GetType(),
               de_thick, e->GetType());
      if ((idt = KVIDTelescope::MakeIDTelescope(uri.Data()))) {
         set_up_telescope(de, e, idt, idtels);
         ++ntels;
         break;
      }

      uri.Form("%s%s-%s%d", prefix.Data(), de->GetType(), e->GetType(),
               e_thick);
      if ((idt = KVIDTelescope::MakeIDTelescope(uri.Data()))) {
         set_up_telescope(de, e, idt, idtels);
         ++ntels;
         break;
      }

      uri.Form("%s%s-%s", prefix.Data(), de->GetType(), e->GetType());
      if ((idt = KVIDTelescope::MakeIDTelescope(uri.Data()))) {
         set_up_telescope(de, e, idt, idtels);
         ++ntels;
         break;
      }
   }

   if (!idt) {
      // Make a generic de-e identification telescope
      uri.Form("%s%s-%s", prefixes[1].Data(), de->GetType(), e->GetType());
      idt = new KVIDTelescope;
      set_up_telescope(de, e, idt, idtels);
      idt->SetLabel(uri);
      ++ntels;
   }

   return ntels;
}
//________________________________________________________________
void KVVAMOS::set_up_telescope(KVDetector* de, KVDetector* e, KVIDTelescope* idt, TCollection* idtels)
{
   // Set up detectors in de-e identification telescope and add to idtels
   idt->AddDetector(de);
   idt->AddDetector(e);
   if (de->GetGroup()) {
      idt->SetGroup(de->GetGroup());
   }
   else {
      idt->SetGroup(e->GetGroup());
   }
   if (idtels->FindObject(idt->GetName())) {
      delete idt;
   }
   else {
      idtels->Add(idt);
   }
}

void KVVAMOS::set_up_single_stage_telescope(KVDetector* det, KVIDTelescope* idt, TCollection* idtels)
{
   // Set up detector in single-stage identification telescope and add to idtels

   idt->AddDetector(det);
   idt->SetGroup(det->GetGroup());
   if (idtels->FindObject(idt->GetName())) {
      delete idt;
   }
   else {
      idtels->Add(idt);
   }
}
Double_t KVVAMOS::GetStripFoilEnergyLossCorrection(KVReconstructedNucleus* nuc)
{
   // Calculate the energy loss in the stripping foil
   // for the reconstructed charged nucleus 'nuc', assuming that the current
   // energy and momentum of this nucleus correspond to its state on
   // leaving the foil.
   //
   // The stripping foil is assumed to be placed vertically i.e. the vector normal
   // to the surface of the foil is oriented towards the beam axis ( Z-axis ).
   //
   // The returned value is the energy lost in the stripping foil in MeV.
   // The energy/momentum of 'nuc' are not affected.

   TVector3 norm(0, 0, 1);
   if (fStripFoil && nuc) return (fStripFoil->GetParticleEIncFromERes(nuc, &norm) - nuc->GetEnergy());
   return 0;
}
//________________________________________________________________

KVVAMOSReconGeoNavigator* KVVAMOS::GetReconNavigator()
{
   //Returns the geometry navigator used to progate nuclei for their
   //reconstruction in VAMOS. Method used by KVVAMOSReconNuc;

   if (!fReconNavigator) fReconNavigator = new KVVAMOSReconGeoNavigator(GetGeometry(), KVMaterial::GetRangeTable());
   return fReconNavigator;
}

//________________________________________________________________

KVVAMOSTransferMatrix* KVVAMOS::GetTransferMatrix()
{
   //Returns the transformation matrix allowing to map the measured
   //coordinates at the focal plane back to the target. If no matrix
   //exists then a new matrix is built from coefficient files found
   //in the directory of the current dataset ( see the method
   //KVVAMOSTransferMatrix::ReadCoefInDataSet() ).

   if (fTransMatrix) return fTransMatrix;
   return (fTransMatrix = new KVVAMOSTransferMatrix(kTRUE));
}
//________________________________________________________________

KVZGOUBIReconstruction* KVVAMOS::GetZGOUBIReconstruction()
{
   //Returns the transformation matrix allowing to map the measured
   //coordinates at the focal plane back to the target. If no matrix
   //exists then a new matrix is built from coefficient files found
   //in the directory of the current dataset ( see the method
   //KVVAMOSTransferMatrix::ReadCoefInDataSet() ).
   if (fZGOUBIReconstruction) return fZGOUBIReconstruction;
   return (fZGOUBIReconstruction = new KVZGOUBIReconstruction(true));
}
//________________________________________________________________

KVVAMOSWeightFinder* KVVAMOS::GetWeightFinder()
{
   //Returns the weight estimator for events normalisation.
   if (fWeightFinder) return fWeightFinder;
   return (fWeightFinder = new KVVAMOSWeightFinder((Int_t) fCurrentRun));
}
//________________________________________________________________


void KVVAMOS::Initialize()
{
   // Initialize data members of the VAMOS detectors and of VAMOS
   // itself. This method has to be called each time you look at a
   // new event.
   fDetectors.R__FOR_EACH(KVVAMOSDetector, Initialize)();
}
//________________________________________________________________

KVVAMOS* KVVAMOS::MakeVAMOS(const Char_t* name)
{
   // Static function which will create and 'Build' the VAMOS spectrometer
   // object corresponding to 'name'.
   // These are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   //
   //Plugin.KVVAMOS:    INDRA_e494s    KVVAMOS     KVVamos    "KVVAMOS()"
   //+Plugin.KVVAMOS:    INDRA_e503    KVVAMOS     KVVamos    "KVVAMOS()"
   //
   //The 'name' ("INDRA_e464s" etc.) corresponds to the name of a dataset in $KVROOT/KVFiles/manip.list
   //The constructors/macros are always without arguments
   //
   //This name is stored in fDataSet

   //check and load plugin library
   TPluginHandler* ph;
   if (!(ph = LoadPlugin("KVVAMOS", name)))
      return 0;

   //execute constructor/macro for multidetector - assumed without arguments
   KVVAMOS* vamos = (KVVAMOS*) ph->ExecPlugin(0);

   vamos->fDataSet = name;
   //call Build() method
   vamos->Build();
   return vamos;
}
//________________________________________________________________

void KVVAMOS::ResetParameters()
{
   // Reset all the calibration parameters and the configuration parameters
   // which could depend on the run.
   //
   // This will reset:
   //      the angle of VAMOS around the target (default: 0)
   //      the high frequency of the beam (default: -1)
   //      remove the stripping foil
   //      the reference magnetic rigidity (default: -1)
   //      the calibration parameters

   //Reset all calibrators of all detectors and of VAMOS

   KVCalibrator* kvc = NULL;
   TIter next_cal(GetListOfCalibrators());
   while ((kvc = (KVCalibrator*) next_cal())) kvc->Reset();

   SetAngle(0);
   SetBrhoRef(-1);
   SetBeamHF(-1);
   SetStripFoil(0);
   fECalibPar[0]  = fECalibPar[1] = fECalibPar[2]  = fECalibPar[3] = 0.;
   SetECalibStatus(kFALSE);
}
//________________________________________________________________

void  KVVAMOS::SetECalibParameters(Double_t c_0, Double_t c_chi, Double_t c_si, Double_t c_csi)
{
   //Set the fECalibPar parameters for the energy calibration of a reconstructed
   //nucleus (see method Calibrate(...)).
   //Once the parameters have been set with this method, the status of the
   //for the calibration becomes 'OK' or 'Ready'
   fECalibPar[0] = c_0;
   fECalibPar[1] = c_chi;
   fECalibPar[2] = c_si;
   fECalibPar[3] = c_csi;
   SetECalibStatus(kTRUE);
}
//________________________________________________________________

void KVVAMOS::SetPedestal(const Char_t* name, Float_t ped)
{
   // Set value of pedestal associated to parameter with given name.

   KVEBYEDAT_ACQParam* par = GetACQParam(name);
   if (par) {
      par->SetPedestal(ped);
   }
}
//________________________________________________________________

void  KVVAMOS::SetStripFoil(KVMaterial* foil, Double_t pos)
{
   //Adopt KVMaterial object for use as stripping foil i.e. we make a clone of the object
   //pointed to by 'foil'.
   //Therefore, any subsequent modifications to the stripping foil should be made to the object whose
   //pointer is returned by GetStripFoil().
   //This object will be deleted with the detector array, or when the stripping foil is changed.
   //
   //Calling SetStripFoil(0) will remove any existing stripping foil.
   //
   //The stripping foil is assumed to be placed vertically behind the target,
   //at the distance 'pos' (in cm) from the target point and we suppose that
   //all the nuclei measured in VAMOS have punched through this foil.

   if (fStripFoil) {
      delete fStripFoil;
      fStripFoil = 0;
   }
   if (foil) fStripFoil = (KVMaterial*) foil->Clone();
   if (fStripFoil && pos)  fStripFoilPos = pos;
   else fStripFoilPos = 0;
}
//________________________________________________________________

void KVVAMOS::SetStripFoil(const Char_t* material, const Float_t area_density, Double_t pos)
{
   //Define the stripping foil used for a given experimental set-up. For material names, see KVMaterial.
   //The area density is in ug/cm2.
   //The foil is placed vertically behin the target at the distance 'pos' (in cm).
   //Use SetStripFoil(0) to remove the existing stripping foil.

   if (!material) SetStripFoil(0);
   else {
      KVMaterial foil(area_density * KVUnits::ug, material);
      SetStripFoil(&foil, pos);
   }
}
//________________________________________________________________

void KVVAMOS::SetTransferMatrix(KVVAMOSTransferMatrix* mat)
{
   //Set the transformation matrix allowing to map the measured
   //coordinates at the focal plane back to the target. If a matrix
   //already exists then it is deleted first to set the new one.

   if (!mat) return;
   if (fTransMatrix) SafeDelete(fTransMatrix);
   fTransMatrix = mat;
}
//________________________________________________________________

void KVVAMOS::SetZGOUBIReconstruction(KVZGOUBIReconstruction* mat)
{
   //Set the transformation matrix allowing to map the measured
   //coordinates at the focal plane back to the target. If a matrix
   //already exists then it is deleted first to set the new one.

   if (!mat) return;
   if (fZGOUBIReconstruction) SafeDelete(fZGOUBIReconstruction);
   fZGOUBIReconstruction = mat;
}
//________________________________________________________________

void KVVAMOS::TargetToFocal(const Double_t* target, Double_t* focal)
{
   // Convert the point coordinates from  target reference to focal plane reference system.
   GetFocalToTargetMatrix().MasterToLocal(target, focal);
}

//________________________________________________________________

void KVVAMOS::TargetToFocalVect(const Double_t* target, Double_t* focal)
{
   // Convert the vector coordinates from  target reference to focal plane reference system.
   GetFocalToTargetMatrix().MasterToLocalVect(target, focal);
}
//________________________________________________________________

UInt_t KVVAMOS::CalculateUniqueID(KVBase* param, KVVAMOSDetector* det)
{
   UInt_t uid = param->GetNumber();
   if (det) {
      uid += 1000   * det->GetACQParamTypeIdx(param->GetType());
      uid += 10000  * det->GetPositionTypeIdx(param->GetLabel());
      uid += 100000 * det->GetUniqueID();
   }
   else {
      uid += 1000   * GetACQParamTypeIdx(param->GetType());
      uid += 10000  * GetPositionTypeIdx(param->GetLabel());
   }
   return uid;
}
//________________________________________________________________

UChar_t KVVAMOS::GetACQParamTypeIdx(const Char_t* type, KVVAMOSDetector* det)
{
   KVString* types = det ? &det->GetACQParamTypes() : &GetACQParamTypes();
   Ssiz_t i = types->Index(Form(":%s,", type));
   return (i < 0 ? 9 : types->Data()[i - 1] - '0');
}
//________________________________________________________________

UChar_t KVVAMOS::GetPositionTypeIdx(const Char_t* type, KVVAMOSDetector* det)
{
   KVString* types = det ? &det->GetPositionTypes() : &GetPositionTypes();
   Ssiz_t i = types->Index(Form(":%s,", type));
   return (i < 0 ? 9 : types->Data()[i - 1] - '0');
}
//________________________________________________________________

Bool_t KVVAMOS::IsUsedToMeasure(const Char_t* type, KVVAMOSDetector* det)
{
   // Returns true if VAMOS (det=NULL) or if a detector (det) is used
   // for the measurment of a quantity (time, position, energy ...).
   // 'type' can be: E, T, T_HF, Q, X, Y, ...
   // The quantities measured by VAMOS/detector are given by
   // GetACQParamTypes or GetPositionTypes

   KVString* types = det ? &det->GetACQParamTypes() : &GetACQParamTypes();
   Ssiz_t i = types->Index(Form(":%s,", type));
   if (i >= 0) return kTRUE;

   types = det ? &det->GetPositionTypes() : &GetPositionTypes();
   i = types->Index(Form(":%s,", type));
   return i >= 0;
}

//________________________________________________________________

void KVVAMOS::SetToFlist_deltapath()
{
   //Set list of distance between FP and detectors in z by using .cao files in VAMOSgeometry

   ifstream geometryfile;

   TString fullpath;

   TString svalue;
   TString sline;
   Ssiz_t pos = 0;
   Float_t position = 0;
   Float_t positionref = 0;
   Bool_t flagvalue = false;
   Bool_t flagref = false;
   TString dataset(gDataSet->GetName());
   TString VAMOSgeometryfolder = dataset + "/VAMOSgeometry";


   if (gDataSet->SearchKVFile("CHI.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("CHI.Z")) {
                  pos = sline.Index(("CHI.Z:")) ;
                  pos = pos + 7 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("CHI.REF.Z")) {
                  pos = sline.Index(("CHI.REF.Z:")) ;
                  pos = pos + 11 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }
      if (flagvalue == true || flagref == true) {
         ToFlist_deltapath.SetValue("CHI", position - positionref);
      }
      std::cout << "CHI:" << ToFlist_deltapath.GetDoubleValue("CHI") << std::endl;
      geometryfile.close();
   }

   if (gDataSet->SearchKVFile("SED.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("SED1.Z")) {
                  pos = sline.Index(("SED1.Z:")) ;
                  pos = pos + 8 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("SED1.REF.Z")) {
                  pos = sline.Index(("SED1.REF.Z:")) ;
                  pos = pos + 12 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }
      if (flagvalue == true || flagref == true) {
         ToFlist_deltapath.SetValue("SED1", position - positionref);
         std::cout << "SED1:" << ToFlist_deltapath.GetDoubleValue("SED1") << std::endl;
      }
      geometryfile.close();
   }

   if (gDataSet->SearchKVFile("SED.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("SED2.Z")) {
                  pos = sline.Index(("SED2.Z:")) ;
                  pos = pos + 8 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("SED2.REF.Z")) {
                  pos = sline.Index(("SED2.REF.Z:")) ;
                  pos = pos + 12 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }
      if (flagvalue == true || flagref == true) {
         ToFlist_deltapath.SetValue("SED2", position - positionref);
         std::cout << "SED2:" << ToFlist_deltapath.GetDoubleValue("SED2") << std::endl;
      }
      geometryfile.close();
   }

   if (gDataSet->SearchKVFile("SI.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("SI.Z")) {
                  pos = sline.Index(("SI.Z:")) ;
                  pos = pos + 6 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("SI.REF.Z")) {
                  pos = sline.Index(("SI.REF.Z:")) ;
                  pos = pos + 10 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }
      if (flagvalue == true || flagref == true) {
         for (int i = 1; i <= 18; i++) {
            ToFlist_deltapath.SetValue(Form("SI_%.2i", i), position - positionref);
         }
      }
      std::cout << "SI:" << ToFlist_deltapath.GetDoubleValue("SI_01") << std::endl;
      geometryfile.close();
   }

   if (gDataSet->SearchKVFile("DC.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("DC1.Z")) {
                  pos = sline.Index(("DC1.Z:")) ;
                  pos = pos + 7 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("DC1.REF.Z")) {
                  pos = sline.Index(("DC1.REF.Z:")) ;
                  pos = pos + 10 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }

      if (flagvalue == true || flagref == true) {
         ToFlist_deltapath.SetValue("DC1", position - positionref);
      }
      std::cout << "DC1:" << ToFlist_deltapath.GetDoubleValue("DC1") << std::endl;
      geometryfile.close();
   }

   if (gDataSet->SearchKVFile("DC.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("DC2.Z")) {
                  pos = sline.Index(("DC2.Z:")) ;
                  pos = pos + 7 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("DC2.REF.Z")) {
                  pos = sline.Index(("DC2.REF.Z:")) ;
                  pos = pos + 10 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }

      if (flagvalue == true || flagref == true) {
         ToFlist_deltapath.SetValue("DC2", position - positionref);
      }
      std::cout << "DC2:" << ToFlist_deltapath.GetDoubleValue("DC2") << std::endl;
      geometryfile.close();
   }

   if (gDataSet->SearchKVFile("CSI.cao", fullpath, VAMOSgeometryfolder.Data())) {
      svalue = "";
      sline = "";
      pos = 0;
      position = 0;
      positionref = 0;
      flagvalue = false;
      flagref = false;
      geometryfile.open(fullpath, ifstream::in);
      while (geometryfile.good()) {
         sline.ReadLine(geometryfile);

         if (!geometryfile.eof()) {
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               if (sline.BeginsWith("CSI.Z")) {
                  pos = sline.Index(("CSI.Z:")) ;
                  pos = pos + 7 ;
                  flagvalue = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &position);
                  }
               }

               if (sline.BeginsWith("CSI.REF.Z")) {
                  pos = sline.Index(("CSI.REF.Z:")) ;
                  pos = pos + 11 ;
                  flagref = true;
                  if (sline.Tokenize(svalue, pos, "|")) {
                     sscanf(svalue.Data(), "%f", &positionref);
                  }
               }
            }
         }
      }
      if (flagvalue == true || flagref == true) {
         for (int i = 1; i <= 80; i++) {
            ToFlist_deltapath.SetValue(Form("CSI%.2i", i), position - positionref);
         }
      }
      std::cout << "CSI:" << ToFlist_deltapath.GetDoubleValue("CSI01") << std::endl;
      geometryfile.close();
   }


}

KVNameValueList KVVAMOS::GetToFlist_deltapath()
{
   return ToFlist_deltapath;
}
