//Created by KVClassFactory on Mon Apr 22 14:53:13 2013
//Author: John Frankland,,,

#include "KVGeoNavigator.h"
#include "KVTemplateEvent.h"
#include "KV3DGeoTrack.h"
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include "KVGeoStrucElement.h"
#include <TVirtualPad.h>

ClassImp(KVGeoNavigator)

void KVGeoNavigator::FormatStructureName(const Char_t* type, Int_t number, KVString& name)
{
   // If a format for naming structures of given type has been defined by a call
   // to SetStructureNameFormat(const Char_t *, const Char_t *), we use it to
   // format the name in the TString.
   //
   // If no format was given, we use by default "[type]_[number]".
   //
   // If SetNameCorrespondanceList(const Char_t *) was used, we use it to translate
   // any names resulting from this formatting to their final value.

   name = "";
   if (fStrucNameFmt.HasParameter(type)) {
      KVString fmt = fStrucNameFmt.GetStringValue(type);
      fmt.Begin("$");
      while (!fmt.End()) {
         KVString bit = fmt.Next();
         if (bit.BeginsWith("type")) {
            Ssiz_t ind = bit.Index("%");
            if (ind > -1) {
               bit.Remove(0, ind);
               name += Form(bit.Data(), type);
            }
            else
               name += type;
         }
         else if (bit.BeginsWith("number")) {
            Ssiz_t ind = bit.Index("%");
            if (ind > -1) {
               bit.Remove(0, ind);
               name += Form(bit.Data(), number);
            }
            else
               name += number;
         }
         else
            name += bit;
      }
   }
   else
      name.Form("%s_%d", type, number);
   TString tmp;
   GetNameCorrespondance(name.Data(), tmp);
   name = tmp;
}

void KVGeoNavigator::FormatDetectorName(const Char_t* basename, KVString& name)
{
   // If a format for naming detectors has been defined by a call
   // to SetDetectorNameFormat(const Char_t *), we use it to
   // format the name in the TString.
   //
   // If no format was given we prefix the names of the parent structures
   // to the basename in order to generate the full name of the detector:
   //~~~~~~~~
   //    [struc1-name]_[struc2-name]_..._[detector-basename]
   //~~~~~~~~
   // If SetNameCorrespondanceList(const Char_t *) was used, we use it to translate
   // any names resulting from this formatting to their final value.

   name = "";
   if (!fCurStrucNumber) {
      // no parent structures
      name = basename;
   }
   else {
      if (fDetNameFmt == "") {
         for (int i = 0; i < fCurStrucNumber; i++) {
            KVGeoStrucElement* el = (KVGeoStrucElement*)fCurrentStructures[i];
            name += Form("%s_", el->GetName());
         }
         name += basename;
      }
      else {
         //    $det:name$             - will be replaced by the detector basename
         //    $struc:[type]:name$    - will be replaced by the name of the parent structure of given type
         //    $struc:[type]:type$    - will be replaced by the type of the parent structure of given type
         //    $struc:[type]:number$  - will be replaced by the number of the parent structure of given type
         fDetNameFmt.Begin("$");
         while (!fDetNameFmt.End()) {
            KVString bit = fDetNameFmt.Next();
            if (bit.Contains(":")) {
               bit.Begin(":");
               KVString itbit = bit.Next();
               if (itbit == "det") {
                  itbit = bit.Next();
                  if (itbit.BeginsWith("name")) {
                     Ssiz_t ind = itbit.Index("%");
                     if (ind > -1) {
                        itbit.Remove(0, ind);
                        name += Form(itbit.Data(), basename);
                     }
                     else
                        name += basename;
                  }
               }
               else if (itbit == "struc") {
                  KVString struc_typ = bit.Next();
                  KVGeoStrucElement* el = 0;
                  for (int i = 0; i < fCurStrucNumber; i++) {
                     el = (KVGeoStrucElement*)fCurrentStructures[i];
                     if (el->IsType(struc_typ)) break;
                  }
                  if (el) {
                     itbit = bit.Next();
                     if (itbit.BeginsWith("name")) {
                        Ssiz_t ind = itbit.Index("%");
                        if (ind > -1) {
                           itbit.Remove(0, ind);
                           name += Form(itbit.Data(), el->GetName());
                        }
                        else
                           name += el->GetName();
                     }
                     else if (itbit.BeginsWith("type")) {
                        Ssiz_t ind = itbit.Index("%");
                        if (ind > -1) {
                           itbit.Remove(0, ind);
                           name += Form(itbit.Data(), el->GetType());
                        }
                        else
                           name += el->GetType();
                     }
                     else if (itbit.BeginsWith("number")) {
                        Ssiz_t ind = itbit.Index("%");
                        if (ind > -1) {
                           itbit.Remove(0, ind);
                           name += Form(itbit.Data(), el->GetNumber());
                        }
                        else
                           name += el->GetNumber();
                     }
                  }
               }
            }
            else
               name += bit;
         }
      }
   }
   TString tmp;
   GetNameCorrespondance(name.Data(), tmp);
   name = tmp;
}

KVGeoNavigator::KVGeoNavigator(TGeoManager* g)
   : fGeometry(g), fCurrentStructures("KVGeoStrucElement", 50), fDetStrucNameCorrespList(nullptr),
     fDetectorPaths(kTRUE)
{
   // Constructor. Call with pointer to geometry.
   SetTracking(kFALSE);
   fDetectorPaths.SetOwner();
}

KVGeoNavigator::~KVGeoNavigator()
{
   // Destructor
   fCurrentStructures.Delete();
   SafeDelete(fDetStrucNameCorrespList);
}

void KVGeoNavigator::SetStructureNameFormat(const Char_t* type, const Char_t* fmt)
{
   // The default names for structures are taken from the node name by stripping off
   // the **STRUCT_** prefix. It is assumed that the remaining string is of the form
   //~~~~~~~~
   //    "[structure type]_[structure number]"
   //~~~~~~~~
   ///This is the name that will be used by default for the structure.
   //
   // However, this format can be change by calling this method
   //~~~~~~~~{.cpp}
   //    SetStructureNameFormat("[structure type]", "[format]")
   //~~~~~~~~
   // where format can contain any of the following tokens:
   //~~~~~~~
   //    $type$         - will be replaced by the structure type name
   //    $type%[fmt]$   - will be replaced by the structure type name using given format
   //    $number$       - will be replaced by the structure number
   //    $number%[fmt]$ - will be replaced by the structure number using given format
   //~~~~~~~
   fStrucNameFmt.SetValue(type, fmt);
}

void KVGeoNavigator::SetNameCorrespondanceList(const Char_t* listfile)
{
   // Allows to provide a list of "translations" for naming structures/detectors
   // "listfile" must be a file in TEnv format, e.g.
   //
   //~~~~~~~~
   // SI_06_1_A1: SI_0601
   // SI_06_1_A2: SI_0602
   // SI_06_1_B1: SI_0701
   // SI_06_1_B2: SI_0702
   // SI_06_2_A1: SI_0603
   // SI_06_2_A2: SI_0604
   // SI_06_2_B1: SI_0703
   //~~~~~~~~
   //
   // The name before ':' is the name of the detector or structure as deduced
   // from the geometry, including any formatting due to SetStructureNameFormat()
   // or SetDetectorNameFormat().
   //
   // The name after ':' is the name that will be used 'externally', e.g. by a
   // KVMultiDetArray created from the geometry using KVGeoImport.
   //
   // Several lists can be combined by calling this method several times.
   //
   // "listfile" can be an absolute path name; if not, we look for it in
   // `$KVROOT/KVFiles/data`, or in `$HOME`, or (finally) in `$PWD`.

   TString fullpath;
   if (!SearchKVFile(listfile, fullpath, "data")) {
      Warning("SetNameCorrespondanceList", "File %s not found", listfile);
      return;
   }
   if (!fDetStrucNameCorrespList) fDetStrucNameCorrespList = new TEnv;
   fDetStrucNameCorrespList->ReadFile(fullpath, kEnvUser);
}

void KVGeoNavigator::SetNameCorrespondanceList(const TEnv* list)
{
   // copy TEnv of name correspondances
   SafeDelete(fDetStrucNameCorrespList);
   if (list) fDetStrucNameCorrespList = (TEnv*)list->Clone();
}

Bool_t KVGeoNavigator::GetNameCorrespondance(const Char_t* name, TString& tran)
{
   // IF name correspondance lists have been set with SetNameCorrespondanceList(const Char_t*),
   // look up new name for 'name'.
   //
   // If found, returns kTRUE and 'tran' is the 'translated' name, otherwise returns kFALSE and tran=name.

   if (fDetStrucNameCorrespList) {
      tran = fDetStrucNameCorrespList->GetValue(name, "");
      if (tran == "") {
         tran = name;
         //Info("GetNameCorrespondance","...not found");
         return kFALSE;
      }
      //Info("GetNameCorrespondance","...found %s", tran.Data());
      return kTRUE;
   }
   //Info("GetNameCorrespondance","...not found");
   tran = name;
   return kFALSE;
}

void KVGeoNavigator::PropagateEvent(KVEvent* TheEvent, TVector3* TheOrigin)
{
   // Propagate a set of particles through the geometry.
   //
   // By default, propagates particles from (0,0,0) (world coordinates),
   // unless a different origin is given.

   ResetTrackID();
   KVNucleus* part;
   while ((part = TheEvent->GetNextParticle())) {
      PropagateParticle(part, TheOrigin);
   }
}

void KVGeoNavigator::ParticleEntersNewVolume(KVNucleus*)
{
   // User-overridable method, to be redefined in child classes.
   //
   // This method is called every time that a propagated particle enters a new volume
   // in the geometry.
   // The user then has access to the following informations:
   //
   // - the TGeoVolume the particle is now entering (GetCurrentVolume());
   // - the node in the geometry this volume occupies (GetCurrentNode());
   // - the distance the particle will have to travel in this volume before
   //    leaving it (GetStepSize());
   // - the position of the particle on the boundary as it enters this volume
   //    (GetEntryPoint()).
   // - the position of the particle on the boundary as it leaves this volume
   //    (GetExitPoint()).
   //
   // If required, propagation of the particle can be stopped at any time by calling
   //      SetStopPropagation()

   AbstractMethod("ParticleEntersNewVolume");
}

const TGeoHMatrix* KVGeoNavigator::GetCurrentMatrix() const
{
   // Returns pointer to internal copy of current global transformation matrix
   return &fCurrentMatrix;
}

TGeoVolume* KVGeoNavigator::GetCurrentDetectorNameAndVolume(KVString& detector_name, Bool_t& multilayer)
{
   // Returns the name of the current detector (if we are inside a detector)
   // and whether it is a multilayer or simple detector.
   //
   // Returns nullptr if we are not inside a detector volume.
   //
   // **N.B.** the returned volume corresponds to the *whole* detector (even if it has several layers).
   // For a multilayer detector, GetCurrentVolume() returns the volume for the current layer.
   //
   // See ExtractDetectorNameFromPath() for details on detector name formatting.

   multilayer = kFALSE;
   fCurrentDetectorNode = 0;
   TString volNom = GetCurrentVolume()->GetName();
   TGeoVolume* detector_volume = 0;
   if (volNom.BeginsWith("DET_")) {
      // simple detector
      fCurrentDetectorNode = GetCurrentNode();
      detector_volume = GetCurrentVolume();
   }
   else {
      // have we hit 1 layer of a multilayer detector?
      TGeoVolume* mother_vol = GetCurrentNode()->GetMotherVolume();
      if (mother_vol) {
         TString mom = mother_vol->GetName();
         if (mom.BeginsWith("DET_")) {
            // it *is* a multilayer detector (youpi! :-)
            if (fMotherNode) { // this is the node corresponding to the whole detector,
               fCurrentDetectorNode = fMotherNode;
               detector_volume = mother_vol;
               multilayer = kTRUE;
            }
         }
      }
   }
   if (detector_volume) ExtractDetectorNameFromPath(detector_name);
   return detector_volume;
}

TGeoNode* KVGeoNavigator::GetCurrentDetectorNode() const
{
   // Returns the node corresponding to the current detector volume
   //
   // **N.B.** the returned node corresponds to the *whole* detector (even if it has several layers).
   return fCurrentDetectorNode;
}

void KVGeoNavigator::ExtractDetectorNameFromPath(KVString& detname)
{
   // We analyse the current path in order to construct the full (unique) name
   // of the detector, i.e. if the current path is
   //
   //~~~~~~~
   // /TOP_1/STRUCT_BLOCK_2/CHIO_WALL_1/DET_CHIO_2/WINDOW_1
   //~~~~~~~
   //
   // then the default name of the detector will be **BLOCK_2_CHIO_2**
   // (see below to override this).
   //
   // This method also fills the fCurrentStructures array with elements
   // deduced from the path, e.g. if the path is
   //
   //~~~~~~~
   // /TOP_1/STRUCT_BLOCK_2/STRUCT_QUARTET_1/DET_SI1-T1
   //~~~~~~~
   //
   // then by default
   //~~~~~~~{.cpp}
   //  fCurrentStructures[0] = KVGeoStrucElement(name = "BLOCK_2", type = "BLOCK", number = 2)
   //  fCurrentStructures[1] = KVGeoStrucElement(name = "QUARTET_1", type = "QUARTET", number = 1)
   //~~~~~~~
   //
   // and the default name of the detector will be **BLOCK_2_QUARTET_1_SI1-T1**
   //
   // ### STRUCTURE & DETECTOR NAME FORMATTING ###
   // #### Structures ####
   // The default names for structures are taken from the node name by stripping off
   // the **STRUCT_** prefix. It is assumed that the remaining string is of the form
   //~~~~~~~~~~
   //    "[structure type]_[structure number]"
   //~~~~~~~~~~
   // (the structure number is always taken after the last occurence of '_' in the
   // node name). This is the name that will be used by default for the structure.
   //
   // However, this format can be change by calling method
   //~~~~~{.cpp}
   //    SetStructureNameFormat("[structure type]", "[format]")
   //~~~~~
   // where format can contain any of the following tokens:
   //~~~~~
   //    $type$         - will be replaced by the structure type name
   //    $type%[fmt]$   - will be replaced by the structure type name using given format
   //    $number$       - will be replaced by the structure number
   //    $number%[fmt]$ - will be replaced by the structure number using given format
   //~~~~~
   //
   // Example: to change the name of the block in the previous example to "B-02":
   //~~~~~~{.cpp}
   //   SetStructureNameFormat("BLOCK", "$type%.1s$-$number%02d$")
   //~~~~~~
   //
   // #### Detectors ####
   // The default base names for detectors are taken from the node name by stripping off
   // the **DET_** prefix. In order to ensure that all detectors have unique names,
   // by default we prefix the names of the parent structures to the basename in
   // order to generate the full name of the detector:
   //~~~~~~~
   //    [struc1-name]_[struc2-name]_..._[detector-basename]
   //~~~~~~~
   // However, this format can be changed by calling method
   //~~~~~{.cpp}
   //    SetDetectorNameFormat("[format]")
   //~~~~~
   // where format can contain any of the following tokens:
   //~~~~~~
   //    $det:name$             - will be replaced by the detector basename
   //    $struc:[type]:name$    - will be replaced by the name of the parent structure of given type
   //    $struc:[type]:type$    - will be replaced by the type of the parent structure of given type
   //    $struc:[type]:number$  - will be replaced by the number of the parent structure of given type
   //~~~~~~
   // plus additional formatting information as for SetStructureNameFormat() (see above).
   //
   // Example: to change the name of the **SI1-T1** detector in the previous example to
   //  **SI1-T1-Q1-B2**:
   //~~~~~~{.cpp}
   //   SetDetectorNameFormat("$det:name$-Q$struc:QUARTET:number$-B$struc:BLOCK:number$");
   //~~~~~~
   // Or if you also change the format of the structure names:
   //~~~~~~
   //   SetStructureNameFormat("BLOCK", "$type%.1s$$number$");
   //   SetStructureNameFormat("QUARTET", "$type%.1s$$number$");
   //   SetDetectorNameFormat("$det:name$-$struc:QUARTET:name$-$struc:BLOCK:name$");
   //~~~~~~


   KVString path = GetCurrentPath();
   path.Begin("/");
   detname = "";
   fCurrentStructures.Clear("C");
   fCurStrucNumber = 0;
   while (!path.End()) {
      KVString elem = path.Next();
      if (elem.BeginsWith("STRUCT_")) {
         // structure element. strip off "STRUCT_" and extract type and number of structure.
         KVString struc_name(elem(7, elem.Length() - 7));
         KVGeoStrucElement* gel = (KVGeoStrucElement*)fCurrentStructures.ConstructedAt(fCurStrucNumber++);
         Ssiz_t last_ = struc_name.Last('_'); // find last '_' in structure name
         TString type = struc_name(0, last_);
         TString nums = struc_name(last_ + 1, struc_name.Length() - last_ - 1);
         Int_t number = nums.Atoi();
         KVString name;
         FormatStructureName(type, number, name);
         gel->SetNameTitle(name, type);
         gel->SetNumber(number);
      }
      else if (elem.BeginsWith("DET_")) {
         // detector name. strip off "DET_" and use rest as basename
         KVString basename(elem(4, elem.Length() - 4));
         FormatDetectorName(basename, detname);
      }
   }
}

void KVGeoNavigator::PropagateParticle(KVNucleus* part, TVector3* TheOrigin)
{
   // Propagate a particle through the geometry in the direction of its momentum,
   // until we reach the boundary of the geometry, or until fStopPropagation is set to kFALSE.
   //
   // Propagation will also stop if we encounter a volume whose name begins with "DEADZONE"

   // Define point of origin of particles
   if (TheOrigin) fGeometry->SetCurrentPoint(TheOrigin->X(), TheOrigin->Y(), TheOrigin->Z());
   else fGeometry->SetCurrentPoint(0., 0., 0.);

   // unit vector in direction of particle's momentum
   TVector3 v = part->GetMomentum().Unit();
   // use particle's momentum direction
   fGeometry->SetCurrentDirection(v.x(), v.y(), v.z());
   fGeometry->FindNode();

   fCurrentVolume = fGeometry->GetCurrentVolume();
   fCurrentNode = fGeometry->GetCurrentNode();
   fMotherNode = fGeometry->GetMother();
   fCurrentMatrix = *(fGeometry->GetCurrentMatrix());
   fCurrentPath = fGeometry->GetPath();
   // move along trajectory until we hit a new volume
   fGeometry->FindNextBoundaryAndStep();
   fStepSize = fGeometry->GetStep();
   TGeoVolume* newVol = fGeometry->GetCurrentVolume();
   TGeoNode* newNod = fGeometry->GetCurrentNode();
   TGeoNode* newMom = fGeometry->GetMother();
   TGeoHMatrix* newMatx = fGeometry->GetCurrentMatrix();
   TString newPath = fGeometry->GetPath();

   Double_t XX, YY, ZZ;
   XX = YY = ZZ = 0.;

   // reset user flag for stopping propagation of particle
   SetStopPropagation(kFALSE);

//    if(IsTracking()) Info("PropagateParticle","Beginning: i am in %s on node %s with path %s",
//         fCurrentVolume->GetName(),fCurrentNode->GetName(),fCurrentPath.Data());

   if (IsTracking() && fGeometry->IsOutside()) {
      const Double_t* posi = fGeometry->GetCurrentPoint();
      AddPointToCurrentTrack(posi[0], posi[1], posi[2]);
      return;
   }

   // track particle until we leave the geometry or until fStopPropagation
   // becomes kTRUE
   while (!fGeometry->IsOutside()) {

      const Double_t* posi = fGeometry->GetCurrentPoint();
      fEntryPoint.SetXYZ(XX, YY, ZZ);
      XX = posi[0];
      YY = posi[1];
      ZZ = posi[2];
      fExitPoint.SetXYZ(XX, YY, ZZ);

      TString vn = GetCurrentVolume()->GetName();
      if (vn.BeginsWith("DEADZONE")) {
         part->GetParameters()->SetValue("DEADZONE", Form("%s/%s", GetCurrentVolume()->GetName(), GetCurrentNode()->GetName()));
         break;
      }

//        if(IsTracking()) Info("PropagateParticle","just before ParticleEntersNewVolume\nnow i am in %s on node %s with path %s",
//             fCurrentVolume->GetName(),fCurrentNode->GetName(),fCurrentPath.Data());

      ParticleEntersNewVolume(part);

      if (StopPropagation()) break;

      fCurrentVolume = newVol;
      fCurrentNode = newNod;
      fMotherNode = newMom;
      fCurrentMatrix = *newMatx;
      fCurrentPath = newPath;

//       if(IsTracking()) Info("PropagateParticle","after ParticleEntersNewVolume\nnow i am in %s on node %s with path %s",
//             fCurrentVolume->GetName(),fCurrentNode->GetName(),fCurrentPath.Data());

      // move on to next volume crossed by trajectory
      fGeometry->FindNextBoundaryAndStep();
      fStepSize = fGeometry->GetStep();
      newVol = fGeometry->GetCurrentVolume();
      newNod = fGeometry->GetCurrentNode();
      newMom = fGeometry->GetMother();
      newMatx = fGeometry->GetCurrentMatrix();
      newPath = fGeometry->GetPath();
   }
   if (IsTracking() && fGeometry->IsOutside()) {
      const Double_t* posi = fGeometry->GetCurrentPoint();
      AddPointToCurrentTrack(posi[0], posi[1], posi[2]);
   }
}

void KVGeoNavigator::DrawTracks(KVNumberList* zlist)
{
   // When using ROOT geometry, after calling PropagateEvent() to simulate detection of some particles,
   // you can call this method to overlay the tracks of the corresponding particles on the 3D
   // geometry of the array
   //
   // If KVNumberList pointer is given, use it to limit Z of drawn tracks

   TIter next_track(fGeometry->GetListOfTracks());
   TVirtualGeoTrack* track;
   bool first = true;
   while ((track = (TVirtualGeoTrack*)next_track())) {
      KVNucleus nuc(track->GetName());
      if (!zlist || zlist->Contains(nuc.GetZ())) {
         KV3DGeoTrack* gtrack = new KV3DGeoTrack(track);
         if (first) {
            if (gPad) gtrack->Draw("same");
            else gtrack->Draw("ogl");
            first = false;
         }
         else gtrack->Draw("same");
      }
   }
}
