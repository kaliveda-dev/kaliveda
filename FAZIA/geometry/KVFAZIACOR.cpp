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
   gGeoManager->GetTopVolume()->AddNode(ruth_tel, 1,
                                        KVMultiDetArray::GetVolumePositioningMatrix(distance,
                                              theta,
                                              phi));
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

   KVFAZIABlock* block = new KVFAZIABlock;
   // "Real" geometry of the FAZIA setup - from the various measurements done with the goniometer
   Double_t dist_target_block0 = (80.0 + block->GetBrassCollimatorDepth()) * KVUnits::cm ;
   Double_t dist_target_block1 = (80.3 + block->GetBrassCollimatorDepth()) * KVUnits::cm ;
   Double_t dist_target_block2 = (80.0 + block->GetBrassCollimatorDepth()) * KVUnits::cm ;
   Double_t dist_target_block3 = (80.3 + block->GetBrassCollimatorDepth()) * KVUnits::cm ;
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
   top->AddNode(block, 0,
                KVMultiDetArray::GetVolumePositioningMatrix(block->GetNominalDistanceTargetBlockCentre(dist_target_block0),
                      theta_block0, phi_block0));

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf - DistToTarget=%1.2lf\n", 0, theta_block0, phi_block0, dist_target_block0) ;
   //
   // Block 1
   top->AddNode(block, 1,
                KVMultiDetArray::GetVolumePositioningMatrix(block->GetNominalDistanceTargetBlockCentre(dist_target_block1),
                      theta_block1, phi_block1));

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf - DistToTarget=%1.2lf\n", 1, theta_block1, phi_block1, dist_target_block1) ;
   //
   // Block 2
   top->AddNode(block, 2,
                KVMultiDetArray::GetVolumePositioningMatrix(block->GetNominalDistanceTargetBlockCentre(dist_target_block2),
                      theta_block2, phi_block2));

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf - DistToTarget=%1.2lf\n", 2, theta_block2, phi_block2, dist_target_block2) ;
   //
   // Block 3
   top->AddNode(block, 3,
                KVMultiDetArray::GetVolumePositioningMatrix(block->GetNominalDistanceTargetBlockCentre(dist_target_block3),
                      theta_block3, phi_block3));

   printf("BLK #%d => theta=%1.2lf - phi=%1.2lf - DistToTarget=%1.2lf\n", 3, theta_block3, phi_block3, dist_target_block3) ;
   //
   //
   // add telescope for elastic scattering monitoring
   RutherfordTelescope();
   // Change default geometry import angular range for rutherford telescope
   SetGeometryImportParameters(.25, 1., .1);
}

void KVFAZIACOR::SetNameOfDetectors(KVEnv& env)
{

   KVFAZIA::SetNameOfDetectors(env);
//   env.SetValue("RUTH_SI_1", "SI1-RUTH");
//   env.SetValue("RUTH_SI_2", "SI2-RUTH");

}
