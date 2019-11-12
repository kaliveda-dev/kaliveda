//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#include "KVFAZIACOR.h"
#include "KVUnits.h"
#include "KVFAZIABlock.h"
#include "TSystem.h"
#include "KVEnv.h"

#include <TGeoMatrix.h>

ClassImp(KVFAZIACOR)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIACOR</h2>
<h4>Description of the FAZIA set up</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIACOR::KVFAZIACOR()
{
   // Default constructor
}

KVFAZIACOR::~KVFAZIACOR()
{
   // Destructor
}

void KVFAZIACOR::GetGeometryParameters()
{
   //Defined number of blocks, the distance from the target and the minimum polar angle
   fNblocks = 4;
   fFDist = 80.3;
   fFThetaMin = 2.1;
}

void KVFAZIACOR::RutherfordTelescope()
{
   // Telescope for elastic scattering monitoring
   // Two 5mm diameter silicon detectors of 525um thickness
   // placed 2m20 from the target at theta=1.84deg phi=-90deg.
   // distance between centres of detectors = 1mm

   KVMaterial silicon("Si");

   const double radius = 9 * KVUnits::mm / 2.;

   const double thick = 525 * KVUnits::um;
   const double centre_dist = 1 * KVUnits::mm;
   double total_thickness = thick + centre_dist;

   TGeoVolume* si_det = gGeoManager->MakeTube("DET_SI", silicon.GetGeoMedium(), 0., radius, thick / 2);

   TGeoVolumeAssembly* ruth_tel = gGeoManager->MakeVolumeAssembly("STRUCT_RUTH");

   ruth_tel->AddNode(si_det, 1, new TGeoTranslation(0, 0, -centre_dist / 2));
   ruth_tel->AddNode(si_det, 2, new TGeoTranslation(0, 0, centre_dist / 2));

   // front entrance of first detector at 2 metres from target
   const double distance = 2.27 * KVUnits::m + 0.5 * total_thickness;
   const double theta = 0.99;
   const double phi = -54.07;

   TGeoRotation rot1, rot2;
   rot2.SetAngles(phi + 90, theta, 0);
   rot1.SetAngles(-90, 0., 0.);
   TGeoTranslation trans(0, 0, distance);
   TGeoHMatrix h = rot2 * trans * rot1;
   gGeoManager->GetTopVolume()->AddNode(ruth_tel, 1, new TGeoHMatrix(h));
}


void KVFAZIACOR::BuildFAZIA()
{
   //Build geometry of FAZIASYM
   //All telescopes are : Si(300µm)-Si(500µm)-CsI(10cm)
   //No attempt has been made to implement real thicknesses
   //
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
//   for (Int_t bb = 0; bb < fNblocks; bb += 1) {

//      if (bb == 1)        centre.SetXYZ(-1 * (dx - centre_hole / 2), -dx - centre_hole / 2, distance_block_cible);
//      else if (bb == 2)   centre.SetXYZ(-1 * (dx + centre_hole / 2), dx - centre_hole / 2, distance_block_cible);
//      else if (bb == 3)   centre.SetXYZ(-1 * (-dx + centre_hole / 2), dx + centre_hole / 2, distance_block_cible);
//      else if (bb == 0)   centre.SetXYZ(-1 * (-dx - centre_hole / 2), -dx + centre_hole / 2, distance_block_cible);
//      else {
//         Warning("BuildFAZIA", "Block position definition is done only for %d blocks", fNblocks);
//      }
//      theta = centre.Theta() * TMath::RadToDeg();
//      phi = centre.Phi() * TMath::RadToDeg();
//      printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", bb, theta, phi);

//      rot2.SetAngles(phi + 90., theta, 0.);
//      rot1.SetAngles(-1.*phi, 0., 0.);
//      h = rot2 * trans * rot1;
//      ph = new TGeoHMatrix(h);
//      top->AddNode(block, bb, ph);
//   }
   //
   //
   // "Real" geometry of the FAZIA setup - from the various measurements done with the goniometer
   Double_t dist_target_block0 = 80.0 * KVUnits::cm ;
   Double_t dist_target_block1 = 80.3 * KVUnits::cm ;
   Double_t dist_target_block2 = 80.0 * KVUnits::cm ;
   Double_t dist_target_block3 = 80.3 * KVUnits::cm ;
   //
   // Theta and Phi angles of the blocks are deduced from the measurements of the angles of each telescopes
   Double_t theta_block0 = 5.41 ;  // in degrees
   Double_t theta_block1 = 5.43 ;  // in degrees
   Double_t theta_block2 = 5.53 ;  // in degrees
   Double_t theta_block3 = 5.52 ;  // in degrees

   Double_t phi_block0 = -13.96  ;  // in degrees
   Double_t phi_block1 = -106.57 ;  // in degrees
   Double_t phi_block2 = 164.96  ;  // in degrees
   Double_t phi_block3 = 74.93   ;  // in degrees
   //
   // Block 0
   rot1.SetAngles((-1. * phi_block0), 0., 0.) ;
   rot2.SetAngles((phi_block0 + 90.), theta_block0, 0.) ;
   trans.SetDz(dist_target_block0 + (thick_si1 / 2.)) ;
   h = rot2 * trans * rot1 ;

   ph = new TGeoHMatrix(h) ;
   top->AddNode(block, 0, ph) ;

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", 0, theta_block0, phi_block0) ;
   //
   // Block 1
   rot1.SetAngles((-1. * phi_block1), 0., 0.) ;
   rot2.SetAngles((phi_block1 + 90.), theta_block1, 0.) ;
   trans.SetDz(dist_target_block1 + (thick_si1 / 2.)) ;
   h = rot2 * trans * rot1 ;

   ph = new TGeoHMatrix(h) ;
   top->AddNode(block, 1, ph) ;

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", 1, theta_block1, phi_block1) ;
   //
   // Block 2
   rot1.SetAngles((-1. * phi_block2), 0., 0.) ;
   rot2.SetAngles((phi_block2 + 90.), theta_block2, 0.) ;
   trans.SetDz(dist_target_block2 + (thick_si1 / 2.)) ;
   h = rot2 * trans * rot1 ;

   ph = new TGeoHMatrix(h) ;
   top->AddNode(block, 2, ph) ;

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", 2, theta_block2, phi_block2) ;
   //
   // Block 3
   rot1.SetAngles((-1. * phi_block3), 0., 0.) ;
   rot2.SetAngles((phi_block3 + 90.), theta_block3, 0.) ;
   trans.SetDz(dist_target_block3 + (thick_si1 / 2.)) ;
   h = rot2 * trans * rot1 ;

   ph = new TGeoHMatrix(h) ;
   top->AddNode(block, 3, ph) ;

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", 3, theta_block3, phi_block3) ;
   //
   //
   // add telescope for elastic scattering monitoring
   RutherfordTelescope();
   // Change default geometry import angular range for rutherford telescope
   SetGeometryImportParameters(.25, 1., 1.84);
}

void KVFAZIACOR::SetNameOfDetectors(KVEnv& env)
{

   KVFAZIA::SetNameOfDetectors(env);
   env.SetValue("RUTH_SI_1", "SI1-RUTH");
   env.SetValue("RUTH_SI_2", "SI2-RUTH");

}
