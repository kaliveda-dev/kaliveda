//Created by KVClassFactory on Fri Feb 26 17:11:15 2016
//Author: bonnet,,,

#include "KVFAZIANFS.h"
#include "KVUnits.h"
#include "KVMaterial.h"
#include "TMath.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"

ClassImp(KVFAZIANFS)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIANFS</h2>
<h4>configuration of FAZIA telescopes for NFS experiments</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVFAZIANFS::GetGeometryParameters()
{
   //Defined number of blocks, the distance from the target and the minimum polar angle
   fNblocks = 0;
   fFDist = 25.0;
   fFThetaMin = 15.0;
   fBuildTarget = kTRUE;
}

KVFAZIANFS::KVFAZIANFS()
   : KVFAZIA()
{
   // Default constructor
   SetTitle(ClassName());
}

KVFAZIANFS::~KVFAZIANFS()
{
   // Destructor
}

void KVFAZIANFS::BuildFAZIA()
{
   // Telescope for elastic scattering monitoring
   // Two 5mm diameter silicon detectors of 525um thickness
   // placed 2m20 from the target at theta=1.84deg phi=-90deg.
   // distance between centres of detectors = 1mm
   KVMaterial mat_csi("CsI");
   TGeoMedium* CesiumIodide = mat_csi.GetGeoMedium();

   KVMaterial silicon("Si");
   Double_t distance_si2_si1 = 0.220;
   Double_t distance_csi_si2 = 0.434;

   Double_t side_si = 2;
   Double_t side_csi_front = 2.050;
   Double_t side_csi_back = 2.272;

   Double_t thick_si1 = 300 * KVUnits::um;
   Double_t thick_si2 = 500 * KVUnits::um;
   Double_t thick_csi = 10;
   const double centre_dist = 1 * KVUnits::mm;
   double total_thickness = thick_si1 + centre_dist;

   TGeoVolumeAssembly* tel = gGeoManager->MakeVolumeAssembly("STRUCT_TEL");
   TGeoTranslation* tr = 0;
   TGeoVolume* si = 0;
   TGeoVolume* csi = 0;
   si = gGeoManager->MakeBox(Form("DET_SI"), silicon.GetGeoMedium(), side_si / 2, side_si / 2, thick_si1 / 2.);
   tr = new TGeoTranslation(0, 0, thick_si1 / 2.);
   tel->AddNode(si, 1, tr);

   si = gGeoManager->MakeBox(Form("DET_SI"), silicon.GetGeoMedium(), side_si / 2, side_si / 2, thick_si2 / 2.);
   tr = new TGeoTranslation(0, 0, thick_si2 / 2. + distance_si2_si1);
   tel->AddNode(si, 2, tr);

   csi = gGeoManager->MakeTrd2(Form("DET_CSI"), CesiumIodide, side_csi_front / 2, side_csi_back / 2, side_csi_front / 2, side_csi_back / 2, thick_csi / 2.);
   tr = new TGeoTranslation(0, 0, thick_csi / 2. + distance_csi_si2);
   tel->AddNode(csi, 3, tr);

   // front entrance of first detector at 2 metres from target
   const double distance = 0.25 * KVUnits::m + 0.5 * total_thickness;
   Double_t tmin = 25;
   Double_t tmax = 65;
   Double_t dt = 20;

   Double_t pmin = -90;

   TGeoRotation rot1, rot2;
   TGeoTranslation trans(0, 0, distance);
   TGeoHMatrix h;

   //Six premiers telescopes
   //25 45 65°
   Int_t nt = 1;
   Double_t ph = pmin;
   Double_t th =  0;
   for (th = tmin; th <= tmax; th += dt) {
      rot1.SetAngles(ph, 0., 0.);
      rot2.SetAngles(ph + 90, th, 0);
      h = rot2 * trans * rot1;
      gGeoManager->GetTopVolume()->AddNode(tel, nt++, new TGeoHMatrix(h));

      Double_t phi_comp = ph * -1;
      Double_t th_comp = 90 - th;
      rot1.SetAngles(phi_comp, 0., 0.);
      rot2.SetAngles(phi_comp + 90, th_comp, 0);

      printf("nt=%d th=%lf th_comp=%lf\n", nt, th, th_comp);

      h = rot2 * trans * rot1;
      gGeoManager->GetTopVolume()->AddNode(tel, nt++, new TGeoHMatrix(h));
   }

   //Si 8 telescopes
   //on le met a 30-60
   /*
   th = 30;
   rot1.SetAngles(ph, 0., 0.);
   rot2.SetAngles(ph + 90, th, 0);
   h = rot2 * trans * rot1;
   gGeoManager->GetTopVolume()->AddNode(tel, nt++, new TGeoHMatrix(h));

   Double_t phi_comp = ph * -1;
   Double_t th_comp = 90 - th;
   rot1.SetAngles(phi_comp, 0., 0.);
   rot2.SetAngles(phi_comp + 90, th_comp, 0);

   printf("nt=%d th=%lf th_comp=%lf\n", nt, th, th_comp);

   h = rot2 * trans * rot1;
   gGeoManager->GetTopVolume()->AddNode(tel, nt++, new TGeoHMatrix(h));
   */

   SetGeometryImportParameters(dt / 2, TMath::Abs(2 * pmin), tmin, pmin, tmax + dt, -1 * pmin);

}


void KVFAZIANFS::BuildTarget()
{

   KVMaterial target_holder_mat("NE102");

   TGeoVolume* target = gGeoManager->MakeEltu("TARGET", target_holder_mat.GetGeoMedium(), 4., 4., 30 * KVUnits::um / 2.);
   TGeoTranslation* tr = new TGeoTranslation(0, 0, -0.1);   //recule la cible pour qu elle ne soit pas vu qd on filtre
   gGeoManager->GetTopVolume()->AddNode(target, 1, tr);

}

void KVFAZIANFS::SetNameOfDetectors(KVEnv& env)
{

   for (Int_t tt = 1; tt <= 8; tt += 1) {
      env.SetValue(Form("TEL_%d_SI_1", tt), Form("SI1-T%d", tt));
      env.SetValue(Form("TEL_%d_SI_2", tt), Form("SI2-T%d", tt));
      env.SetValue(Form("TEL_%d_CSI_3", tt), Form("CSI-T%d", tt));
   }

}
