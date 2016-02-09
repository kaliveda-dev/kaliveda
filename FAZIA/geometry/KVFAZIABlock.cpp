//Created by KVClassFactory on Thu May 30 10:13:17 2013
//Author: Eric Bonnet

#include "KVFAZIABlock.h"
#include "KVUnits.h"
#include "KVMaterial.h"
#include "TMath.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"

ClassImp(KVFAZIABlock)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIABlock</h2>
<h4>Block of 16 telescopes FAZIA</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIABlock::KVFAZIABlock() : TGeoVolumeAssembly("STRUCT_BLOCK")
{
   // Default constructor

   SetMedium(gGeoManager->GetMedium("Vacuum"));//to avoid warnings about STRUCT_BLOCK has dummy medium

   KVMaterial mat_si("Si");
   TGeoMedium* Silicon = mat_si.GetGeoMedium();

   KVMaterial mat_csi("CsI");
   TGeoMedium* CesiumIodide = mat_csi.GetGeoMedium();

   KVMaterial mat_plomb("Lead");
   TGeoMedium* Plomb = mat_plomb.GetGeoMedium();

   TGeoVolumeAssembly* quartet = gGeoManager->MakeVolumeAssembly("STRUCT_QUARTET");
   quartet->SetMedium(gGeoManager->GetMedium("Vacuum"));//to avoid warnings about STRUCT_QUARTET has dummy medium

   TGeoVolume* si = 0;
   TGeoVolume* csi = 0;

   Double_t distance_si2_si1 = 0.220;
   Double_t distance_csi_si2 = 0.434;

   Double_t side_si = 2;
   Double_t side_csi_front = 2.050;
   Double_t side_csi_back = 2.272;

   Double_t inter_si = 0.24;

   Double_t thick_si1 = 300 * KVUnits::um;
   Double_t thick_si2 = 500 * KVUnits::um;
   Double_t thick_csi = 10;

   Double_t adjust_csi = 0.0165;

   Int_t ndet = 1;;
   TGeoTranslation* tr = 0;

   Double_t shift = side_si / 2 + inter_si / 2;
   //printf("%lf\n", shift);

   Double_t coefx[4] = { -1., -1., 1., 1.};
   Double_t coefy[4] = {1., -1., -1., 1.};

   for (Int_t nt = 1; nt <= 4; nt += 1) {

      shift = side_si / 2 + inter_si / 2;

      si = gGeoManager->MakeBox(Form("DET_SI1-T%d", nt), Silicon, side_si / 2, side_si / 2, thick_si1 / 2.);
      tr = new TGeoTranslation(coefx[nt - 1]*shift, coefy[nt - 1]*shift, thick_si1 / 2.);

      quartet->AddNode(si, ndet++, tr);
      ((TGeoNodeMatrix*)quartet->GetNodes()->Last())->SetName(Form("DET_SI1-T%d", nt));

      si = gGeoManager->MakeBox(Form("DET_SI2-T%d", nt), Silicon, side_si / 2, side_si / 2, thick_si2 / 2.);
      tr = new TGeoTranslation(coefx[nt - 1]*shift, coefy[nt - 1]*shift, thick_si2 / 2. + distance_si2_si1);
      quartet->AddNode(si, ndet++, tr);
      ((TGeoNodeMatrix*)quartet->GetNodes()->Last())->SetName(Form("DET_SI2-T%d", nt));

      shift = side_si / 2 + inter_si / 2 + adjust_csi;

      csi = gGeoManager->MakeTrd2(Form("DET_CSI-T%d", nt), CesiumIodide, side_csi_front / 2, side_csi_back / 2, side_csi_front / 2, side_csi_back / 2, thick_csi / 2.);
      tr = new TGeoTranslation(coefx[nt - 1]*shift, coefy[nt - 1]*shift, thick_csi / 2. + distance_csi_si2);
      quartet->AddNode(csi, ndet++, tr);
      ((TGeoNodeMatrix*)quartet->GetNodes()->Last())->SetName(Form("DET_CSI-T%d", nt));

   }

   Int_t nbl = 1;
   TGeoVolume* blindage = 0;
//Double_t thick_bld = thick_si1+distance_si2_si1+thick_si2;
   /* l'epaisseur du si1 est compris dans la distance_si2_si1 */
   Double_t thick_bld = distance_si2_si1 + thick_si2;
   Double_t shift_bld = (side_si + inter_si) / 2.;
///Croix inter quartet
//
// Separation des 4 télescopes
//
//
   blindage = gGeoManager->MakeBox("DEADZONE_BLINDAGE_1", Plomb, inter_si / 2, (side_si + inter_si / 2), thick_bld / 2.);
   //printf("%s\n", blindage->GetMaterial()->GetTitle());
   tr = new TGeoTranslation(0, 0, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);

   blindage = gGeoManager->MakeBox("DEADZONE_BLINDAGE_2", Plomb, (side_si / 2), inter_si / 2, thick_bld / 2.);
   tr = new TGeoTranslation(-1 * shift_bld, 0, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);
   tr = new TGeoTranslation(+1 * shift_bld, 0, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);

///Contour de l ensemble du quartet
//
//Délimiation des bords exterieurs
//
//
   shift_bld = (side_si + inter_si);

   blindage = gGeoManager->MakeBox("DEADZONE_BLINDAGE_3", Plomb, (side_si + inter_si / 2), inter_si / 2, thick_bld / 2.);
   tr = new TGeoTranslation(0, shift_bld, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);
   tr = new TGeoTranslation(0, -1 * shift_bld, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);
///
   blindage = gGeoManager->MakeBox("DEADZONE_BLINDAGE_4", Plomb, inter_si / 2, (side_si + inter_si * 1.5), thick_bld / 2.);
   tr = new TGeoTranslation(shift_bld, 0, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);
   tr = new TGeoTranslation(-1 * shift_bld, 0, thick_bld / 2.);
   quartet->AddNode(blindage, nbl++, tr);

   fTotSidWBlind = 4 * side_si + 5 * inter_si;
//Coordonnées extraite des côtes données par Yvan M.
//vecteur pointant le milieu d un quartet
//X=-2.231625
//Y=-2.230525
//Z=99.950350
// Mag=100.000139
// Theta=1.808104
// Phi = -135.014124
   TVector3* placement = new TVector3(-2.231625, -2.230525, 99.950350);
   TVector3* Centre = new TVector3();

   TGeoRotation rot1, rot2;
   TGeoTranslation trans;
   TGeoTranslation invZtrans(0, 0, -100);

   TGeoHMatrix h;
   TGeoHMatrix* ph = 0;

//Boucle sur les 4 quartets d un block
   Double_t tx[4] = {1, -1, -1, 1};
   Double_t ty[4] = {1, 1, -1, -1};

   Double_t theta = 0;
   Double_t phi = 0;
   Double_t trans_z = 0;

   for (Int_t nq = 1; nq <= 4; nq += 1) {

      Centre->SetXYZ(placement->X()*tx[nq - 1], placement->Y()*ty[nq - 1], placement->Z());

      theta = Centre->Theta() * TMath::RadToDeg();
      phi = Centre->Phi() * TMath::RadToDeg();

      trans_z = Centre->Mag() + thick_si1 / 2.;

      rot2.SetAngles(phi + 90., theta, 0.);
      rot1.SetAngles(-1.*phi, 0., 0.);
      trans.SetDz(trans_z);

      h = invZtrans * rot2 * trans * rot1;
      ph = new TGeoHMatrix(h);

      AddNode(quartet, nq, ph);
   }
}

KVFAZIABlock::~KVFAZIABlock()
{
   // Destructor
}

