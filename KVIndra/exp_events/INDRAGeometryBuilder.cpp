//Created by KVClassFactory on Wed Feb  1 11:09:41 2012
//Author: John Frankland,,,

#include "INDRAGeometryBuilder.h"
#include "TEnv.h"
#include "TMath.h"
#include "TString.h"
#include "TGeoMedium.h"
#include "TRotation.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"
#include "TGeoCompositeShape.h"
#include "TGeoEltu.h"

#include "KVINDRA.h"
#include "KVDataSet.h"

using namespace std;

ClassImp(INDRAGeometryBuilder)

void INDRAGeometryBuilder:: CorrectCoordinates(Double_t* coo, Double_t& offX, Double_t& offY)
{
   // calculate offset in X and Y
   // correct coordinates for offset
   // return offset TGeoTranslation

   offX = offY = 0;
   for (int i = 0; i < 8; i++) {
      offX += coo[2 * i];
      offY += coo[2 * i + 1];
   }
   offX /= 8.;
   offY /= 8.;
   for (int i = 0; i < 8; i++) {
      coo[2 * i] -= offX;
      coo[2 * i + 1] -= offY;
   }
}

INDRAGeometryBuilder::INDRAGeometryBuilder()
{
   // Default constructor
   fEtalonVol = 0;
}

//________________________________________________________________

INDRAGeometryBuilder::INDRAGeometryBuilder(const INDRAGeometryBuilder& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   fEtalonVol = 0;
   obj.Copy(*this);
}

INDRAGeometryBuilder::~INDRAGeometryBuilder()
{
   // Destructor
}

//________________________________________________________________

void INDRAGeometryBuilder::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable INDRAGeometryBuilder::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TObject::Copy(obj);
   //INDRAGeometryBuilder& CastedObj = (INDRAGeometryBuilder&)obj;
}

void INDRAGeometryBuilder::CalculateBackPlaneCoordinates(TVector3* frontcoords, TVector3 normal, Double_t depth, TVector3* backcoords)
{
   // Given an array of ncoords points frontcoords[ncoords] which lie in a plane
   // calculate the corresponding points on a parallel plane which is
   // at a distance from the origin which is greater by depth
   // (distance plane-origin = length of normal to plane passing through origin)
   // 'normal' is vector point of intersection plane-normal to origin

   Double_t planeDist = normal.Mag();//distance original plane-origin
   for (int i = 0; i < 4; i++) {
      Double_t rap = (planeDist + depth) / planeDist;
      backcoords[i] = rap * frontcoords[i];
   }
}
//________________________________________________________________

void INDRAGeometryBuilder::ReadDetCAO(const Char_t* detname, Int_t ring)
{
   // Read infos in CAO file for this detector.
   // We look for a TEnv file with name "detname.cao"

   TEnv infos;
   TString path = Form("%s.cao", detname);
   SearchKVFile(path.Data(), path, "data");
   infos.ReadFile(path, kEnvAll);

   Double_t unit_converter = 0.1;// distances in file given in mm: convert to cm

   fInnerFront[0].SetX(infos.GetValue("INNER.A.X", 0.0)*unit_converter);
   fInnerFront[0].SetY(infos.GetValue("INNER.A.Y", 0.0)*unit_converter);
   fInnerFront[0].SetZ(infos.GetValue("INNER.A.Z", 0.0)*unit_converter);
   fInnerFront[3].SetX(infos.GetValue("INNER.B.X", 0.0)*unit_converter);
   fInnerFront[3].SetY(infos.GetValue("INNER.B.Y", 0.0)*unit_converter);
   fInnerFront[3].SetZ(infos.GetValue("INNER.B.Z", 0.0)*unit_converter);
   fInnerFront[2].SetX(infos.GetValue("INNER.C.X", 0.0)*unit_converter);
   fInnerFront[2].SetY(infos.GetValue("INNER.C.Y", 0.0)*unit_converter);
   fInnerFront[2].SetZ(infos.GetValue("INNER.C.Z", 0.0)*unit_converter);
   fInnerFront[1].SetX(infos.GetValue("INNER.D.X", 0.0)*unit_converter);
   fInnerFront[1].SetY(infos.GetValue("INNER.D.Y", 0.0)*unit_converter);
   fInnerFront[1].SetZ(infos.GetValue("INNER.D.Z", 0.0)*unit_converter);

   // To get back to INDRA phi-coordinates, all points need to be rotated
   // by -150 degrees around the Z-axis
   for (int i = 0; i <= 3; i++) {
      fInnerFront[i].RotateZ(-150.*TMath::DegToRad());
   }
   CalculateCentre(fInnerFront, fInnerCentre);

   fOuterFront[0].SetX(infos.GetValue("OUTER.A.X", 0.0)*unit_converter);
   fOuterFront[0].SetY(infos.GetValue("OUTER.A.Y", 0.0)*unit_converter);
   fOuterFront[0].SetZ(infos.GetValue("OUTER.A.Z", 0.0)*unit_converter);
   fOuterFront[3].SetX(infos.GetValue("OUTER.B.X", 0.0)*unit_converter);
   fOuterFront[3].SetY(infos.GetValue("OUTER.B.Y", 0.0)*unit_converter);
   fOuterFront[3].SetZ(infos.GetValue("OUTER.B.Z", 0.0)*unit_converter);
   fOuterFront[2].SetX(infos.GetValue("OUTER.C.X", 0.0)*unit_converter);
   fOuterFront[2].SetY(infos.GetValue("OUTER.C.Y", 0.0)*unit_converter);
   fOuterFront[2].SetZ(infos.GetValue("OUTER.C.Z", 0.0)*unit_converter);
   fOuterFront[1].SetX(infos.GetValue("OUTER.D.X", 0.0)*unit_converter);
   fOuterFront[1].SetY(infos.GetValue("OUTER.D.Y", 0.0)*unit_converter);
   fOuterFront[1].SetZ(infos.GetValue("OUTER.D.Z", 0.0)*unit_converter);

   // To get back to INDRA phi-coordinates, all points need to be rotated
   // by -150 degrees around the Z-axis
   for (int i = 0; i <= 3; i++) {
      fOuterFront[i].RotateZ(-150.*TMath::DegToRad());
   }
   CalculateCentre(fOuterFront, fOuterCentre);

   phi0 = infos.GetValue("PHIZERO", 30.0);
   thetaMin = infos.GetValue("THETAMIN", 0.0);
   thetaMax = infos.GetValue("THETAMAX", 0.0);
   Ndets = infos.GetValue("NDETS", 1);
   deltaPhi = 360. / Ndets;

   // calculate theoretical geometry:
   // points of intersection with the plane defined by real Huguet geometry
   // and the 4 vectors to theoretical thetamin/max phimin/max
   Double_t Ph0 = fInnerCentre.Phi() * TMath::RadToDeg();
   CalculateCornersInPlane(fInnerFront, thetaMin, thetaMax,
                           Ph0 - deltaPhi / 2.,
                           Ph0 + deltaPhi / 2.,
                           fFrameFront);
   // calculate centre of frame = direction of detector
   CalculateCentre(fFrameFront, fFrameCentre);
   fFrameMat.SetMaterial(infos.GetValue("FRAME.MATERIAL", "Al"));

   fInnerPads = infos.GetValue("INNER.PADS", 1);
   fOuterPads = infos.GetValue("OUTER.PADS", 0);
   fInnerRing = infos.GetValue("INNER.RING", ring);
   fOuterRing = infos.GetValue("OUTER.RING", 0);
   fInnerDmod = infos.GetValue("INNER.DMOD", 1);
   fOuterDmod = infos.GetValue("OUTER.DMOD", 1);
   fInnerModmin = infos.GetValue("INNER.MODMIN", 1);
   fOuterModmin = infos.GetValue("OUTER.MODMIN", 1);
   fModmax = infos.GetValue("MODMAX", 24);
}

//________________________________________________________________

void INDRAGeometryBuilder::Print(Option_t*) const
{
   printf("Detector name : %s\n", fDetName.Data());
   printf("Total thickness : %f cm\n", fTotalThickness);
   printf("Frame centre:\n");
   fFrameCentre.Print();
   printf("Inner centre:\n");
   fInnerCentre.Print();
   printf("Outer centre: \n");
   fOuterCentre.Print();
   printf("Coordinates of outer pads :\n");
   for (int i = 0; i < 4; i++) fOuterFront[i].Print();
   printf("Coordinates of inner pads :\n");
   for (int i = 0; i < 4; i++) fInnerFront[i].Print();
   printf("Material of outer frame : %s\n", fFrameMat.GetTitle());
}

//________________________________________________________________

void INDRAGeometryBuilder::MakeFrame(TString det_type, Int_t ring_num)
{
   // Create the TGeoVolume corresponding to the outer casing
   // (dead zone) of the detector

   TVector3 fFrameBack[4];
   TVector3 fFrameBackCentre;
   //CalculateBackPlaneCoordinates(fFrameFront, fFrameCentre, fTotalThickness, fFrameBack);
   // make deadzone be 1um thick
   Double_t dz = 5.e-5;
   CalculateBackPlaneCoordinates(fFrameFront, fFrameCentre, 2 * dz, fFrameBack);
   CalculateCentre(fFrameBack, fFrameBackCentre);

   TVector3 corners[8]; // 8 vertices of the volume
   TransformToOwnFrame(fFrameFront, fFrameCentre, corners);
   TransformToOwnFrame(fFrameBack, fFrameBackCentre, &corners[4]);

   Double_t vertices[16];
   for (int i = 0; i < 8; i++) {
      vertices[2 * i] = corners[i].X();
      vertices[2 * i + 1] = corners[i].Y();
   }

   //Double_t dz = 0.99 * fTotalThickness / 2.;
   TString vol_name;
   vol_name.Form("STRUCT_%s_%02d", det_type.Data(), ring_num);
   fFrameVolume = gGeoManager->MakeVolumeAssembly(vol_name);
   fFrameVolume->SetMedium(gGeoManager->GetMedium("Vacuum"));
   vol_name.Form("DEADZONE_%s_%02d", det_type.Data(), ring_num);
   TGeoMedium* med = fFrameMat.GetGeoMedium();
   Double_t offX, offY;
   CorrectCoordinates(vertices, offX, offY);
   // place the deadzone 1um (=1.e-4 cm) behind the detector entrance window
   TGeoTranslation* offset  = new TGeoTranslation(offX, offY, -(0.5 * fTotalThickness - (1.e-4 + dz)));
   TGeoVolume* frame = gGeoManager->MakeArb8(vol_name.Data(), med, dz, vertices);
   frame->SetLineColor(med->GetMaterial()->GetDefaultColor());
   fFrameVolume->AddNode(frame, 1, offset);
}

//________________________________________________________________

void INDRAGeometryBuilder::TransformToOwnFrame(TVector3* orig, TVector3& centre, TVector3* ownframe)
{
   // takes the four points (TVector3) in the array and transforms them
   // to a reference frame in which the +ve z-axis goes through their centre

   TRotation rot_to_frame;
   rot_to_frame.SetYEulerAngles(-centre.Phi(), -centre.Theta(), 0.);
   TVector3 displZ(0, 0, centre.Mag());
   for (int i = 0; i < 4; i++) {
      ownframe[i] = rot_to_frame * orig[i] - displZ;
   }
}


//________________________________________________________________

void INDRAGeometryBuilder::PlaceFrame(Double_t phi, Int_t copy_no)
{
   // position frame (dead zone) volume in geometry

   Double_t theta = fFrameCentre.Theta() * TMath::RadToDeg();
   TGeoRotation rot1, rot2;
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-90., 0., 0.);
   // distance to frame centre
   Double_t trans_z = fFrameCentre.Mag() + fTotalThickness / 2.;

   TGeoTranslation tran(0, 0, trans_z);
   TGeoHMatrix h = rot2 * tran * rot1;
   TGeoHMatrix* ph = new TGeoHMatrix(h);

   // add detector volume to geometry
   gGeoManager->GetTopVolume()->AddNode(fFrameVolume, copy_no, ph);
}

//________________________________________________________________

void INDRAGeometryBuilder::PlaceDetector()
{
   // position detector inside frame

   fFrameVolume->AddNode(fDetVolume, 1, fDetectorPosition);
   fFrameVolume->GetNode(Form("%s_1", fDetVolume->GetName()))->SetName(fDetVolume->GetName());
}
//________________________________________________________________

Bool_t INDRAGeometryBuilder::CheckDetectorPresent(TString detname)
{
   KVDetector* thisdetector = gIndra->GetDetector(detname);
   if (!thisdetector || !thisdetector->IsPresent()) {
      //Info("CheckDetectorPresent", "%s is absent", detname.Data());
      return kFALSE;
   }
   if (!fLayers) {
      // get info on detector structure from first detector which is present on ring
      fProtoDetector = thisdetector;
      fLayers = thisdetector->GetListOfAbsorbers();
      fActiveLayer = 1 + thisdetector->GetListOfAbsorbers()->IndexOf(thisdetector->GetActiveLayer());
      fTotalThickness = thisdetector->GetTotalThicknessInCM();
   }
   if ((thisdetector != fProtoDetector) && !thisdetector->HasSameStructureAs(fProtoDetector)) {
      // current detector does not have same structure as prototype
      fProtoDetector = thisdetector;
      fLayers = thisdetector->GetListOfAbsorbers();
      fActiveLayer = 1 + thisdetector->GetListOfAbsorbers()->IndexOf(thisdetector->GetActiveLayer());
      fTotalThickness = thisdetector->GetTotalThicknessInCM();
   }
   return kTRUE;
}

//void INDRAGeometryBuilder::MakeRing(TGeoVolume* MOTHER, const Char_t* det, int ring)
void INDRAGeometryBuilder::MakeRing(const Char_t* det, int ring)
{

   if (!strcmp(det, "PHOS"))
      fDetName.Form("%s_01", det);
   else
      fDetName.Form("%s_%02d01", det, ring);
   ReadDetCAO(fDetName, ring);
   //Print();
   Double_t phi = phi0;
   Int_t innerMod = fInnerModmin;
   Int_t outerMod = fOuterModmin;
   int i = 1;

   cout << fDetName.Data()  << "  " << fInnerCentre.Mag() << "  " <<
        0.5 * (fInnerFront[0].Theta() + fInnerFront[3].Theta())*TMath::RadToDeg() << "  " <<
        0.5 * (fInnerFront[1].Theta() + fInnerFront[2].Theta())*TMath::RadToDeg() << "  " <<
        0.5 * (fInnerFront[2].Phi() + fInnerFront[3].Phi())*TMath::RadToDeg() << "  " <<
        0.5 * (fInnerFront[0].Phi() + fInnerFront[1].Phi())*TMath::RadToDeg() << endl;

// check presence of all detectors of ring
   Int_t npresent = 0;
   Int_t ntotal = 0;
   fLayers = nullptr;//will force CheckDetectorPresent() to get infos from first detector present
   fProtoDetector = nullptr;
   for (i = 1; i <= Ndets; i++) {
      if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
      else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
      ntotal++;
      npresent += (Int_t)CheckDetectorPresent(fDetName);

      innerMod += fInnerDmod;

      if (fInnerPads == 3) {
         if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
         else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
         ntotal++;
         npresent += (Int_t)CheckDetectorPresent(fDetName);
         innerMod += fInnerDmod;
         if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
         else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
         ntotal++;
         npresent += (Int_t)CheckDetectorPresent(fDetName);
         innerMod += fInnerDmod;
      }
      else {
         if (fInnerPads == 2) {
            if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
            else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
            ntotal++;
            npresent += (Int_t)CheckDetectorPresent(fDetName);
            innerMod += fInnerDmod;
         }
         if (fOuterPads) {
            if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, outerMod);
            else fDetName.Form("%s_%02d%02d", det, fOuterRing, outerMod);
            ntotal++;
            npresent += (Int_t)CheckDetectorPresent(fDetName);
            outerMod += fOuterDmod;
            if (fOuterPads == 2) {
               if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, outerMod);
               else fDetName.Form("%s_%02d%02d", det, fOuterRing, outerMod);
               ntotal++;
               npresent += (Int_t)CheckDetectorPresent(fDetName);
               outerMod += fOuterDmod;
            }
         }
      }
   }
   if (!npresent) {
      Info("MakeRing", "Detector type %s  ring number %d : ABSENT", det, ring);
      return;
   }
   Info("MakeRing", "Detector type %s  ring number %d : %d/%d detectors present", det, ring, npresent, ntotal);

   i = 1;

   innerMod = fInnerModmin;
   outerMod = fOuterModmin;

   fLayers = nullptr;//will force CheckDetectorPresent() to get infos from first detector present
   fProtoDetector = nullptr;
   for (i = 1; i <= Ndets; i++) {

      // check if prototype information changes
      KVDetector* oldProtoDet = fProtoDetector;

      // check absence of whole block
      ntotal = 0;
      npresent = 0;
      if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
      else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
      ntotal++;
      npresent += (Int_t)CheckDetectorPresent(fDetName);

      innerMod += fInnerDmod;

      if (fInnerPads == 3) {
         if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
         else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
         ntotal++;
         npresent += (Int_t)CheckDetectorPresent(fDetName);
         innerMod += fInnerDmod;
         if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
         else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
         ntotal++;
         npresent += (Int_t)CheckDetectorPresent(fDetName);
         innerMod += fInnerDmod;
      }
      else {
         if (fInnerPads == 2) {
            if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, innerMod);
            else fDetName.Form("%s_%02d%02d", det, fInnerRing, innerMod);
            ntotal++;
            npresent += (Int_t)CheckDetectorPresent(fDetName);
            innerMod += fInnerDmod;
         }
         if (fOuterPads) {
            if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, outerMod);
            else fDetName.Form("%s_%02d%02d", det, fOuterRing, outerMod);
            ntotal++;
            npresent += (Int_t)CheckDetectorPresent(fDetName);
            outerMod += fOuterDmod;
            if (fOuterPads == 2) {
               if (!strcmp(det, "PHOS")) fDetName.Form("%s_%02d", det, outerMod);
               else fDetName.Form("%s_%02d%02d", det, fOuterRing, outerMod);
               ntotal++;
               npresent += (Int_t)CheckDetectorPresent(fDetName);
               outerMod += fOuterDmod;
            }
         }
      }
      if (!npresent) Info("MakeRing", "\tBlock %d: ABSENT", i);
      else if (npresent < ntotal) {
         Info("MakeRing", "\tBlock %d: %d/%d detectors present", i, npresent, ntotal);
         Info("MakeRing", "\tCASE NOT TREATED. ALL DETECTORS PRESENT.");
      }
      if (npresent) {

         if (fProtoDetector != oldProtoDet) {
            // Prototype information has changed since last group/block
            // We must create a new prototype

            Info("MakeRing", "DET:%s RING:%d  Making new prototype group for i=%d/%d",
                 det, ring, i, Ndets);

            /* build and add frame */
            MakeFrame(det, ring);

            // make pads in inner ring
            MakeDetector("A1", fInnerFront, fInnerCentre);
            PlaceDetector();
            if (fInnerPads == 3) {
               MakeDetector("A2", fOuterFront, fOuterCentre);
               PlaceDetector();
               TVector3 reflex[4];
               ReflectPad(fInnerFront, fFrameCentre.Phi(), reflex);
               TVector3 refcent;
               CalculateCentre(reflex, refcent);
               MakeDetector("A3", reflex, refcent);
               PlaceDetector();
            }
            else {
               if (fInnerPads == 2) {
                  TVector3 reflex[4];
                  ReflectPad(fInnerFront, fFrameCentre.Phi(), reflex);
                  TVector3 refcent;
                  CalculateCentre(reflex, refcent);
                  MakeDetector("A2", reflex, refcent);
                  PlaceDetector();
               }
               if (fOuterPads) {
                  // make pads in outer ring
                  MakeDetector("B1", fOuterFront, fOuterCentre);
                  PlaceDetector();
                  if (fOuterPads == 2) {
                     TVector3 reflex[4];
                     ReflectPad(fOuterFront, fFrameCentre.Phi(), reflex);
                     TVector3 refcent;
                     CalculateCentre(reflex, refcent);
                     MakeDetector("B2", reflex, refcent);
                     PlaceDetector();
                  }
               }
            }
         }

         PlaceFrame(phi, i);
      }

      if (((ring > 9 && ring < 13) && i == 1) || (ring > 12 && i == 2)) {
         fEtalonTheta[ring - 10] = fFrameCentre.Theta() * TMath::RadToDeg();
         fEtalonPhi[ring - 10] = phi;
         fEtalonTheta[ring - 9] = fFrameCentre.Theta() * TMath::RadToDeg();
         fEtalonPhi[ring - 9] = phi;
      }
      phi += deltaPhi;
   }

}

//________________________________________________________________

void INDRAGeometryBuilder::MakeDetector(const Char_t* det, TVector3* som, TVector3 cen)
{

   // make volume corresponding to the actual detector

   TVector3 corners[8]; // 8 vertices of the volume
   Double_t vertices[16];

   // Check whether this is a real multi-layer detector
   // i.e. make sure all "layers" have non-zero thickness
   Bool_t multi_layer = fLayers->GetSize() > 1;
   if (multi_layer) {
      TIter next(fLayers);
      KVMaterial* abs;
      Int_t no_abs = 0;
      while ((abs = (KVMaterial*)next())) {
         Double_t thick = abs->GetThickness();
         if (thick > 0.0) no_abs++;
      }
      multi_layer = (no_abs > 1);
   }
   if (multi_layer) {
      fDetVolume = gGeoManager->MakeVolumeAssembly(Form("DET_%s", det));
      fDetVolume->SetMedium(gGeoManager->GetMedium("Vacuum"));
   }
   TVector3 frontPlane[4], backPlane[4], frontCentre, backCentre;
   // front plane of first absorber is front plane of detector
   for (int i = 0; i < 4; i++) frontPlane[i] = som[i];
   frontCentre = cen;

   /**** BUILD & ADD LAYERS ****/
   TIter next(fLayers);
   KVMaterial* abs;
   Double_t depth_in_det = 0.;
   Int_t no_abs = 1;

   Double_t offX, offY; //offset of each layer
   fDetectorPosition = 0;

   while ((abs = (KVMaterial*)next())) {
      // get medium for absorber
      TGeoMedium* med = abs->GetGeoMedium();
      Double_t thick = abs->GetThickness();

      if (thick == 0.0) {
         no_abs++;   // ignore zero thickness layers
         continue;
      }

      // calculate coordinates of back plane
      CalculateBackPlaneCoordinates(frontPlane, frontCentre, thick, backPlane);
      // calculate centre of back plane
      CalculateCentre(backPlane, backCentre);
      // calculate corners of volume
      TransformToOwnFrame(frontPlane, fFrameCentre, corners);
      TransformToOwnFrame(backPlane, fFrameCentre, &corners[4]);
      for (int i = 0; i < 8; i++) {
         vertices[2 * i] = corners[i].X();
         vertices[2 * i + 1] = corners[i].Y();
      }

      Double_t dz = thick / 2.;
      TString vol_name;
      if (multi_layer) {
         if (no_abs == fActiveLayer) vol_name = Form("ACTIVE_%s", det);
         else vol_name = Form("%s_%d_%s", det, no_abs, abs->GetName());
      }
      else
         vol_name = Form("DET_%s", det);
      CorrectCoordinates(vertices, offX, offY);
      TGeoVolume* vol =
         gGeoManager->MakeArb8(vol_name.Data(), med, dz, vertices);
      vol->SetLineColor(med->GetMaterial()->GetDefaultColor());
      if (multi_layer) {
         /*** position absorber in mother ***/
         Double_t trans_z = -fTotalThickness / 2. + depth_in_det + dz; // (note: reference is CENTRE of absorber)
         TGeoTranslation* tr = new TGeoTranslation(offX, offY, trans_z);
         fDetVolume->AddNode(vol, no_abs, tr);
      }
      else {
         // single absorber: mother is absorber is detector is mother is ...
         fDetVolume = vol;
         fDetectorPosition = new TGeoTranslation(offX, offY, 0);
      }

      depth_in_det += thick;
      no_abs++;

      // front of next absorber is back of current absorber
      for (int i = 0; i < 4; i++) frontPlane[i] = backPlane[i];
      frontCentre = backCentre;
   }
}

void INDRAGeometryBuilder::CalculateCentre(TVector3* corners, TVector3& inter)
{
   // TVector3 corners[4] array of corner coordinates in usual order:
   //       corners[3] : theta-min, phi-min
   //       corners[2] : theta-max, phi-min
   //       corners[1] : theta-max, phi-max
   //       corners[0] : theta-min, phi-max
   //
   // Calculate the intersection of the normal to the plane passing through the origin.

   TVector3 a10 = corners[1] - corners[0];
   TVector3 a21 = corners[2] - corners[1];
   TVector3 normal = a10.Cross(a21);
   Double_t d = (corners[3] * normal) / normal.Mag2();
   inter = normal * d;
}


void INDRAGeometryBuilder::CalculateCornersInPlane(TVector3* plane, Double_t thetamin,
      Double_t thetamax, Double_t phimin, Double_t phimax, TVector3* corners)
{
   // fill TVector3 corners[4] array in usual order:
   //       corners[3] : theta-min, phi-min
   //       corners[2] : theta-max, phi-min
   //       corners[1] : theta-max, phi-max
   //       corners[0] : theta-min, phi-max
   // theta, phi in degrees
   // These points lie in the plane defined by the equivalent TVEctor3 plane[4] corners

   TVector3 a10 = plane[1] - plane[0];
   TVector3 a21 = plane[2] - plane[1];
   TVector3 normal = a10.Cross(a21);
   Double_t d0 = plane[3] * normal;

   TVector3 l;
   l.SetMagThetaPhi(1., thetamin * TMath::DegToRad(), phimin * TMath::DegToRad());
   corners[3] = l * (d0 / (l * normal));
   l.SetMagThetaPhi(1., thetamax * TMath::DegToRad(), phimin * TMath::DegToRad());
   corners[2] = l * (d0 / (l * normal));
   l.SetMagThetaPhi(1., thetamax * TMath::DegToRad(), phimax * TMath::DegToRad());
   corners[1] = l * (d0 / (l * normal));
   l.SetMagThetaPhi(1., thetamin * TMath::DegToRad(), phimax * TMath::DegToRad());
   corners[0] = l * (d0 / (l * normal));
}

void INDRAGeometryBuilder::CloseAndDraw()
{
   gGeoManager->CloseGeometry();
   //gGeoManager->GetTopVolume()->SetVisContainers();
   gGeoManager->DefaultColors();
   gGeoManager->GetTopVolume()->Draw("ogl");
}

void INDRAGeometryBuilder::ReflectPad(TVector3* orig, Double_t phicentre, TVector3* newpad)
{
   for (int i = 0; i < 4; i++) {
      newpad[3 - i].SetMagThetaPhi(orig[i].Mag(), orig[i].Theta(), 2.*phicentre - orig[i].Phi());
   }
}

void INDRAGeometryBuilder::BuildTarget()
{
   //TO DO: multi-layer targets + target with angle to beam

   // target holder
   KVMaterial target_holder_mat("Al");
   new TGeoBBox("TARGET_FRAME", 3., 3., 0.1 / 2.);
   new TGeoEltu("TARGET_HOLE", 2., 2., 0.1 / 2.);
   TGeoCompositeShape* cs = new TGeoCompositeShape("TARGET_FRAME", "TARGET_FRAME - TARGET_HOLE");
   TGeoVolume* target_frame = new TGeoVolume("TARGET_FRAME", cs, target_holder_mat.GetGeoMedium());
   gGeoManager->GetTopVolume()->AddNode(target_frame, 1);

   // add target
   // only single-layer target treated correctly
   KVTarget* T = gIndra->GetTarget();
   if (T) {
      KVMaterial* targMat = (KVMaterial*)T->GetLayers()->First();
      TGeoVolume* target = gGeoManager->MakeEltu("TARGET", targMat->GetGeoMedium(), 2., 2., targMat->GetThickness() / 2.);
      gGeoManager->GetTopVolume()->AddNode(target, 1);
   }
}

void INDRAGeometryBuilder::Build(Bool_t withTarget, Bool_t closeGeometry)
{
   if (!gIndra) {
      Error("Build", "You must build the geometry with gDataSet->BuildMultiDetector() before calling this method");
      return;
   }

   if (!gGeoManager) {
      Error("Build", "You must defined gGeoManager with KVMultiDetArray::CreateGeoManager before calling this method");
      return;
   }

   if (gIndra->GetDetector("PHOS_01")) MakeRing("PHOS", 1);
   else {
      MakeRing("SI", 1);
      MakeRing("CSI", 1);
   }
   for (int ring = 2; ring <= 16; ring += 2) {
      MakeRing("CI", ring);
      if (ring > 1 && ring < 10)MakeRing("SI", ring);
      MakeRing("CSI", ring);
      if (ring == 12) {
         MakeRing("CI", ring + 1);
         MakeRing("CSI", ring + 1);
      }
   }
   for (int ring = 10; ring <= 17; ring++) {
      if (gIndra->GetDetector(Form("SI75_%d", ring))) MakeEtalon(ring);
   }
   if (withTarget) BuildTarget();
   if (closeGeometry) {
      gGeoManager->DefaultColors();
      gGeoManager->CloseGeometry();
   }

}
void INDRAGeometryBuilder::Build(const KVNumberList& rings, const KVNameValueList& detectors)
{
   // Build geometry, using only the ring numbers in the KVNumberList, and
   // only the detectors which are in the KVNameValueList:
   //
   // KVNameValueList dets;
   // dets.SetValue("CI",1);
   // dets.SetValue("SI",1);
   // igb.Build("2-9", dets); // use INDRAGeometryBuilder object 'igb' to build
   //                         // ChIo and Silicon detectors of rings 2-9
   //
   // Possible detector types are "CI", "SI", "CSI", "PHOS"
   // If you want to see the target in the previous example, use dets.SetValue("TARGET",1)
   //
   // Include "ring+100" in number list to build etalon telescope for "ring"
   //
   // This method will not destroy any pre-existing geometry (gGeoManager),
   // but will create one if none exists.
   // Also it will not close the geometry, leaving the possibility to add ancillary
   // detectors.

   if (!gIndra) {
      Error("Build", "You must build the geometry with gDataSet->BuildMultiDetector() before calling this method");
      return;
   }

   if (!gGeoManager) {
      Error("Build", "You must defined gGeoManager with KVMultiDetArray::CreateGeoManager before calling this method");
      return;
   }

   if (rings.Contains(1)) {
      if (detectors.HasParameter("PHOS")) MakeRing("PHOS", 1);
      if (detectors.HasParameter("SI"))MakeRing("SI", 1);
      if (detectors.HasParameter("CSI"))MakeRing("CSI", 1);
   }
   for (int ring = 2; ring <= 16; ring += 2) {
      if (rings.Contains(ring) && detectors.HasParameter("CI")) MakeRing("CI", ring);
      if (ring > 1 && ring < 10) {
         if (rings.Contains(ring) && detectors.HasParameter("SI"))MakeRing("SI", ring);
      }
      if (rings.Contains(ring) && detectors.HasParameter("CSI"))MakeRing("CSI", ring);
      if (ring == 12) {
         if (rings.Contains(ring + 1) && detectors.HasParameter("CI"))MakeRing("CI", ring + 1);
         if (rings.Contains(ring + 1) && detectors.HasParameter("CSI"))MakeRing("CSI", ring + 1);
      }
   }
   for (int ring = 10; ring <= 17; ring++) {
      if (rings.Contains(ring + 100)) MakeEtalon(ring);
   }

   if (detectors.HasParameter("TARGET"))
      BuildTarget();
}

void INDRAGeometryBuilder::BuildEtalonVolumes()
{
   KVMaterial mat_si("Si");
   KVMaterial mat_li("Li");
   KVMaterial mat_al("Al");
   TGeoMedium* Silicon = mat_si.GetGeoMedium();
   TGeoMedium* Lithium = mat_li.GetGeoMedium();
   TGeoMedium* Alu = mat_al.GetGeoMedium();

   Double_t sili_diameter_total = 2.54;
   Double_t holder_thickness = 0.05;
   Double_t holder_length = 1.0;
   Double_t sili_diameter_active = 2.36;
   Double_t sili_silicon_thickness = 0.22;
   Double_t sili_lithium_thickness = 0.0044;
   Double_t si75_thickness = 0.008;
   Double_t si75_diameter_active = 2.2;

   fEtalonVol = new TGeoVolumeAssembly("STRUCT_ETALON");
   fEtalonVol->SetMedium(gGeoManager->GetMedium("Vacuum"));

   TGeoVolume* holder = gGeoManager->MakeTube("DEADZONE_ETALON_HOLDER", Alu,
                        (sili_diameter_total) / 2., (sili_diameter_total + 2 * holder_thickness) / 2., holder_length / 2.);
   holder->SetLineColor(holder->GetMaterial()->GetDefaultColor());
   fEtalonVol->AddNode(holder, 1);

   Double_t w = sili_silicon_thickness + sili_lithium_thickness;
   TGeoVolume* sili_dz = gGeoManager->MakeTube("DEADZONE_SILI", Alu,
                         sili_diameter_active / 2., (sili_diameter_total) / 2., w / 2.);
   sili_dz->SetLineColor(sili_dz->GetMaterial()->GetDefaultColor());

   fEtalonVol->AddNode(sili_dz, 1, new TGeoTranslation(0, 0, holder_length / 4.));

   TGeoVolumeAssembly* sili = new TGeoVolumeAssembly("DET_SILI");
   sili->SetMedium(gGeoManager->GetMedium("Vacuum"));
   TGeoVolume* sili_si = gGeoManager->MakeTube("ACTIVE_SILI", Silicon,
                         0., (sili_diameter_active) / 2., (sili_silicon_thickness) / 2.);
   TGeoVolume* sili_li = gGeoManager->MakeTube("SILI_LI", Lithium,
                         0., (sili_diameter_active) / 2., (sili_lithium_thickness) / 2.);
   sili_si->SetLineColor(sili_si->GetMaterial()->GetDefaultColor());
   sili_li->SetLineColor(sili_li->GetMaterial()->GetDefaultColor());
   sili->AddNode(sili_si, 1, new TGeoTranslation(0, 0, -(w / 2. - sili_silicon_thickness / 2.)));
   sili->AddNode(sili_li, 1, new TGeoTranslation(0, 0, w / 2. - sili_lithium_thickness / 2.));
   fEtalonVol->AddNode(sili, 1, new TGeoTranslation(0, 0, holder_length / 4.));

   TGeoVolume* si75 = gGeoManager->MakeTube("DET_SI75", Silicon,
                      0., (si75_diameter_active) / 2., (si75_thickness) / 2.);
   TGeoVolume* si75_dz = gGeoManager->MakeTube("DEADZONE_SI75", Alu,
                         (si75_diameter_active) / 2., (sili_diameter_total) / 2., (si75_thickness) / 2.);

   fEtalonVol->AddNode(si75_dz, 1);
   fEtalonVol->AddNode(si75, 1);

}

void INDRAGeometryBuilder::MakeEtalon(int RING)
{
   // Build and add etalon telescope for ring
   //Double_t theta[] = {51.075000,63.340000,79.435000,100.685000,118.235000,133.905000,149.790000,166.435000};
   //Double_t phi[] = {37.500000,37.500000,37.500000,90.000000,78.750000,78.750000,90.000000,90.000000};
   Double_t dist[] = {17.4005, 17.4005, 17.4005, 16.7005, 16.7005, 16.7005, 17.4005, 17.4005,};

   if (!fEtalonVol) BuildEtalonVolumes();

   cout << "Etalons " << RING << " : " << fEtalonTheta[RING - 10] << "  " << fEtalonPhi[RING - 10] << endl;
   TGeoTranslation trans;
   trans.SetDz(dist[RING - 10]);
   TGeoRotation rot1, rot2;
   TGeoHMatrix h;
   TGeoHMatrix* ph = 0;

//    rot2.SetAngles(phi[RING-10]+90., theta[RING-10], 0.);
//    rot1.SetAngles(-1.*phi[RING-10], 0., 0.);
   rot2.SetAngles(fEtalonPhi[RING - 10] + 90., fEtalonTheta[RING - 10], 0.);
   rot1.SetAngles(-1.*fEtalonPhi[RING - 10], 0., 0.);
//    if(RING==13){
//       TGeoTranslation p(4.5,0,0);
//       h = p * rot2 * trans * rot1;
//    }
   if (RING == 10) {
      TGeoTranslation p(1.5, 2.5, 0);
      h = rot2 * trans * p * rot1;
   }
   else if (RING == 11) {
      TGeoTranslation p(1.5, -1.5, 0);
      h = rot2 * trans * p * rot1;
   }
   else if (RING == 12) {
      TGeoTranslation p(1.5, 0, 0);
      h = rot2 * trans * p * rot1;
   }
   else if (RING == 13) {
      TGeoTranslation p(-4, -0.5, 0);
      h = rot2 * trans * p * rot1;
   }
   else if (RING == 14) {
      TGeoTranslation p(-1.7, 1.75, 0);
      h = rot2 * trans * p * rot1;
   }
   else if (RING == 15) {
      TGeoTranslation p(-1.7, -3, 0);
      h = rot2 * trans * p * rot1;
   }
   else if (RING == 16) {
      TGeoTranslation p(0, 2.5, 0);
      h = rot2 * trans * p * rot1;
   }
   else {
      TGeoTranslation p(0, -2.25, 0);
      h = rot2 * trans * p * rot1;
   }
   ph = new TGeoHMatrix(h);
   gGeoManager->GetTopVolume()->AddNode(fEtalonVol, RING, ph);
}
