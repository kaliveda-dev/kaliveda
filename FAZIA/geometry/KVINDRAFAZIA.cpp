//Created by KVClassFactory on Fri Feb 14 09:48:39 2014
//Author: John Frankland,,,

#include "KVINDRAFAZIA.h"
#include "KVFAZIABlock.h"
#include "INDRAGeometryBuilder.h"
#include "KVMaterial.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "KVGeoImport.h"
#include "TROOT.h"
#include "KVUnits.h"
#include "TMath.h"
#include "KVIDTelescope.h"

ClassImp(KVINDRAFAZIA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAFAZIA</h2>
<h4>INDRA-FAZIA setups</h4>
<!-- */
// --> END_HTML
// Change default geometry with .kvrootrc variables:
//
// INDRAFAZIA.GeoType                  [geometry type = "compact", ...]
// INDRAFAZIA.DistanceFAZIA            [distance FAZIA-target in centimetres]
// INDRAFAZIA.ThetaMinCompactGeo       [minimum polar angle in degrees for compact geometry]
////////////////////////////////////////////////////////////////////////////////

KVINDRAFAZIA::KVINDRAFAZIA()
{
   // Default constructor
   SetName("INDRAFAZIA");
   SetTitle("INDRA-FAZIA combined array");

   fFGeoType = gEnv->GetValue("INDRAFAZIA.GeoType", "compact");
   fFDist    = gEnv->GetValue("INDRAFAZIA.DistanceFAZIA", 100.0);
   fFThetaMin = gEnv->GetValue("INDRAFAZIA.ThetaMinCompactGeo", 0.5);
}

KVINDRAFAZIA::~KVINDRAFAZIA()
{
   // Destructor
}

void KVINDRAFAZIA::Build(Int_t)
{
   // Build the combined INDRA & FAZIA arrays

   KVINDRA::Build();
   INDRAGeometryBuilder igb;
   igb.Build(kFALSE, kFALSE);
   if (fFGeoType == "compact") BuildFAZIACompact();
   else {
      Fatal("Build", "Unknown geometry type: %s. Perhaps you should implement it?",
            fFGeoType.Data());
   }
   SetROOTGeometry();
   KVGeoImport imp(gGeoManager, KVMaterial::GetRangeTable(), this);
   imp.SetNameCorrespondanceList("FAZIA.names");
   imp.SetNameCorrespondanceList("INDRA.names");
   // the following parameters are optimized for a 12-block compact
   // geometry placed at 80cm with rings 1-5 of INDRA removed.
   // make sure that the expected number of detectors get imported!
   imp.ImportGeometry(0.25, 1, 2., 0, 14);
}

void KVINDRAFAZIA::BuildFAZIACompact()
{
   Info("BuildFAZIA", "Compact geometry, %f cm from target, theta-min=%f deg.",
        fFDist, fFThetaMin);

   TGeoVolume* top = gGeoManager->GetTopVolume();

   Double_t distance_block_cible = fFDist * KVUnits::cm;
   Double_t thick_si1 = 300 * KVUnits::um;
   TGeoTranslation trans;
   trans.SetDz(distance_block_cible + thick_si1 / 2.);

   KVFAZIABlock* block = new KVFAZIABlock;

   TGeoRotation rot1, rot2;
   TGeoHMatrix h;
   TGeoHMatrix* ph = 0;
   Double_t theta = 0;
   Double_t phi = 0;

   Double_t theta_min = fFThetaMin;//smallest lab polar angle in degrees
   Double_t centre_hole = 2.*tan(theta_min * TMath::DegToRad()) * distance_block_cible;
   Double_t dx = (block->GetTotalSideWithBlindage()) / 2.;

   /*BLOCK 1*/
   TVector3 cent_b1(dx - centre_hole / 2, -(dx + centre_hole) + centre_hole / 2, distance_block_cible); //position centre of block 1
   theta = cent_b1.Theta() * TMath::RadToDeg();
   phi = cent_b1.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 1, ph);
///*BLOCK 2*/
   TVector3 cent_b2(dx + centre_hole - centre_hole / 2, dx - centre_hole + centre_hole / 2, distance_block_cible); //position centre of block 2
   theta = cent_b2.Theta() * TMath::RadToDeg();
   phi = cent_b2.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 2, ph);
   /*BLOCK 3*/
   TVector3 cent_b3(-dx + centre_hole - centre_hole / 2, dx + centre_hole / 2, distance_block_cible); //position centre of block 3
   theta = cent_b3.Theta() * TMath::RadToDeg();
   phi = cent_b3.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 3, ph);
   /*BLOCK 4*/
   TVector3 cent_b4(-dx - centre_hole / 2, -dx + centre_hole / 2, distance_block_cible); //position centre of block 4
   theta = cent_b4.Theta() * TMath::RadToDeg();
   phi = cent_b4.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 4, ph);
   /*BLOCK 5 -above block 1*/
   TVector3 cent_b5(2 * dx + dx - centre_hole / 2, -(dx + centre_hole) + centre_hole / 2, distance_block_cible); //position centre of block 5
   theta = cent_b5.Theta() * TMath::RadToDeg();
   phi = cent_b5.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 5, ph);
   /*BLOCK 6 -to left of block 1*/
   TVector3 cent_b6(dx - centre_hole / 2, -2.*dx - (dx + centre_hole) + centre_hole / 2, distance_block_cible); //position centre of block 6
   theta = cent_b6.Theta() * TMath::RadToDeg();
   phi = cent_b6.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 6, ph);
   /*BLOCK 7 - above block 2*/
   TVector3 cent_b7(2 * dx + dx + centre_hole - centre_hole / 2, dx - centre_hole + centre_hole / 2, distance_block_cible); //position centre of block 2
   theta = cent_b7.Theta() * TMath::RadToDeg();
   phi = cent_b7.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 7, ph);
   /*BLOCK 8 - right of block 2*/
   TVector3 cent_b8(dx + centre_hole - centre_hole / 2, 2 * dx + dx - centre_hole + centre_hole / 2, distance_block_cible); //position centre of block 2
   theta = cent_b8.Theta() * TMath::RadToDeg();
   phi = cent_b8.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 8, ph);
   /*BLOCK 9 - right of block 3*/
   TVector3 cent_b9(-dx + centre_hole - centre_hole / 2, 2 * dx + dx + centre_hole / 2, distance_block_cible); //position centre of block 3
   theta = cent_b9.Theta() * TMath::RadToDeg();
   phi = cent_b9.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 9, ph);
   /*BLOCK 10 - below block 3*/
   TVector3 cent_b10(-2 * dx - dx + centre_hole - centre_hole / 2, dx + centre_hole / 2, distance_block_cible); //position centre of block 3
   theta = cent_b10.Theta() * TMath::RadToDeg();
   phi = cent_b10.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 10, ph);
   /*BLOCK 11 - below block 4*/
   TVector3 cent_b11(-2 * dx - dx - centre_hole / 2, -dx + centre_hole / 2, distance_block_cible); //position centre of block 4
   theta = cent_b11.Theta() * TMath::RadToDeg();
   phi = cent_b11.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 11, ph);
   /*BLOCK 12 - left of block 4*/
   TVector3 cent_b12(-dx - centre_hole / 2, -2 * dx - dx + centre_hole / 2, distance_block_cible); //position centre of block 4
   theta = cent_b12.Theta() * TMath::RadToDeg();
   phi = cent_b12.Phi() * TMath::RadToDeg();
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-1.*phi, 0., 0.);
   h = rot2 * trans * rot1;
   ph = new TGeoHMatrix(h);
   top->AddNode(block, 12, ph);

   gGeoManager->CloseGeometry();
   gGeoManager->DefaultColors();
}
