//Created by KVClassFactory on Fri Feb 26 14:22:01 2016
//Author: bonnet,,,

#include "KVFAZIETO.h"
#include "KVUnits.h"
#include "KVFAZIABlock.h"

ClassImp(KVFAZIETO)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIETO</h2>
<h4>description of the FAZIA-12B demonstrator</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIETO::KVFAZIETO()
   : KVFAZIA()
{
   // Default constructor
   SetTitle(ClassName());
}

KVFAZIETO::~KVFAZIETO()
{
   // Destructor
}

void KVFAZIETO::GetGeometryParameters()
{
   //defined configuration of the blocks
   fNblocks = 12;
   fFDist = 100.0;
   fFThetaMin = 2.0;
}


//________________________________________________________________

void KVFAZIETO::BuildFAZIA()
{
   Info("BuildFAZIA", "Compact geometry, %f cm from target",
        fFDist);

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

   TVector3 centre;
   for (Int_t bb = 0; bb < fNblocks; bb += 1) {

      //for FAZIASYM ordering
      if (bb == 1)      centre.SetXYZ(dx     - centre_hole / 2, -dx    - centre_hole / 2,  distance_block_cible);
      else if (bb == 2) centre.SetXYZ(dx     + centre_hole / 2, dx     - centre_hole / 2,  distance_block_cible);
      else if (bb == 3) centre.SetXYZ(-dx    + centre_hole / 2, dx     + centre_hole / 2,  distance_block_cible);
      else if (bb == 0) centre.SetXYZ(-dx    - centre_hole / 2, -dx    + centre_hole / 2,  distance_block_cible);

      else if (bb == 4) centre.SetXYZ(3 * dx   - centre_hole / 2,  -dx   - centre_hole / 2,  distance_block_cible);
      else if (bb == 5) centre.SetXYZ(dx     - centre_hole / 2,  -3 * dx - centre_hole / 2,  distance_block_cible);
      else if (bb == 6) centre.SetXYZ(3 * dx   + centre_hole / 2,  dx    - centre_hole / 2,  distance_block_cible);
      else if (bb == 7) centre.SetXYZ(dx     + centre_hole / 2,  3 * dx  - centre_hole / 2,  distance_block_cible);

      else if (bb == 8) centre.SetXYZ(-dx    + centre_hole / 2,  3 * dx  + centre_hole / 2,  distance_block_cible);
      else if (bb == 9) centre.SetXYZ(-3 * dx  + centre_hole / 2,  dx    + centre_hole / 2,  distance_block_cible);
      else if (bb == 10)centre.SetXYZ(-3 * dx  - centre_hole / 2, -dx    + centre_hole / 2,  distance_block_cible);
      else if (bb == 11)centre.SetXYZ(-dx    - centre_hole / 2,  -3 * dx + centre_hole / 2,  distance_block_cible);


      else {
         Warning("BuildFAZIA", "Block position definition is done only for %d blocks", fNblocks);
      }
      theta = centre.Theta() * TMath::RadToDeg();
      phi = centre.Phi() * TMath::RadToDeg();
      printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", bb, theta, phi);

      rot2.SetAngles(phi + 90., theta, 0.);
      rot1.SetAngles(-1.*phi, 0., 0.);
      h = rot2 * trans * rot1;
      ph = new TGeoHMatrix(h);
      top->AddNode(block, bb, ph);
   }

}
/*
TVector3 cent_b1(dx - centre_hole / 2, -dx - centre_hole / 2, distance_block_cible); //position centre of block 1
TVector3 cent_b2(dx + centre_hole / 2 , dx - centre_hole / 2, distance_block_cible); //position centre
TVector3 cent_b3(-dx + centre_hole / 2, dx + centre_hole / 2, distance_block_cible); //position centre of block 3
TVector3 cent_b4(-dx - centre_hole / 2, -dx + centre_hole / 2, distance_block_cible); //position centre of block 4

TVector3 cent_b5(3*dx   - centre_hole/2,  -dx   - centre_hole / 2,            distance_block_cible); //position centre of block 5
TVector3 cent_b6(dx     - centre_hole/2,  -3*dx - centre_hole / 2,      distance_block_cible); //position centre of block 6
TVector3 cent_b7(3*dx   + centre_hole/2,  dx    - centre_hole / 2,         distance_block_cible); //position centre of block 2
TVector3 cent_b8(dx     + centre_hole/2,  3*dx  - centre_hole / 2,   distance_block_cible); //position centre of block 2

TVector3 cent_b9 (-dx   + centre_hole/2,  3*dx + centre_hole/2,   distance_block_cible); //position centre of block 3
TVector3 cent_b10(-3*dx + centre_hole/2,  dx + centre_hole/2,     distance_block_cible); //position centre of block 3
TVector3 cent_b11(-3*dx - centre_hole/2, -dx + centre_hole/2,     distance_block_cible); //position centre of block 4
TVector3 cent_b12(-dx   - centre_hole/2,  -3*dx + centre_hole/2,  distance_block_cible); //position centre of block 4
*/
