//Author: Diego Gruyer,,,

#include "KVIDSiCsI_e613.h"
#include "KVINDRACodeMask.h"
#include "KVIdentificationResult.h"

using namespace std;

ClassImp(KVIDSiCsI_e613)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSiCsI_e613</h2>
<h4>Si-CsI id with grids for INDRA_e613</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSiCsI_e613::KVIDSiCsI_e613()
{
   // Default constructor

   fGGgrid = 0;
   fPGgrid = 0;
   fSi     = 0;
   fCsI    = 0;
}

KVIDSiCsI_e613::~KVIDSiCsI_e613()
{
   // Destructor
}

//___________________________________________________________________________________________

void KVIDSiCsI_e613::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 grid, we set fCanIdentify = kTRUE
   // "Natural" line widths are calculated for KVIDZAGrids.

   fSi  = (KVSilicon*) GetDetector(1);
   fSiPGPedestal = fSi->GetPedestal("PG");
   fSiGGPedestal = fSi->GetPedestal("GG");
   fCsI = (KVCsI*) GetDetector(2);
   fCsIRPedestal = fCsI->GetPedestal("R");
   fCsILPedestal = fCsI->GetPedestal("L");

   fGGgrid = fPGgrid = 0;

   KVIDGraph* grid = 0;
   TIter next_grid(GetListOfIDGrids());
   while ((grid = (KVIDGraph*)next_grid())) {
      if (!strcmp(grid->GetVarY(), "SI_GG"))      fGGgrid = (KVIDZAGrid*)grid;
      else if (!strcmp(grid->GetVarY(), "SI_PG")) fPGgrid = (KVIDZAGrid*)grid;
   }

   if (fGGgrid) {
      SetBit(kReadyForID);
      fGGgrid->Initialize();
      if (fPGgrid) fPGgrid->Initialize();
      fPIEDESTAL = (KVIDCutLine*)fGGgrid->GetCut("PIEDESTAL");
   }
   else ResetBit(kReadyForID);

}


Double_t KVIDSiCsI_e613::GetIDMapX(Option_t*)
{
   //Calculates current X coordinate for identification.
   //It is the CsI detector's total light output calculated from current values of 'R' and 'L'.
   //'opt' has no effect.

   Double_t y = fCsI->GetLumiereTotale();
   return y;
}

//__________________________________________________________________________//

Double_t KVIDSiCsI_e613::GetIDMapY(Option_t* opt)
{
   //Calculates current Y coordinate for identification.
   //It is the silicon's current grand gain (if opt="GG") or petit gain (opt != "GG")
   //coder data, with pedestal correction.
   Double_t si = -1.;
   if (!strcmp(opt, "GG")) si = (Double_t)fSi->GetGG() - fSiGGPedestal;
   else if (!strcmp(opt, "PG")) si = (Double_t)fSi->GetPG() - fSiPGPedestal;
   return si;
}

//________________________________________________________________________________________//

Bool_t KVIDSiCsI_e613::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   // Particle identification and code setting using identification grids.
   // perform identification in Si(GG) - CsI(H) map
   // Sets idr->deltaEpedestal according to position in GG map

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   Double_t sigg = (y < 0. ? GetIDMapY("GG") : y);
   Double_t lumtot = (x < 0. ? GetIDMapX() : x);

   KVIDZAGrid* TheGrid = 0;

   fGGgrid->Identify(lumtot, sigg, idr);
   // check if silicon-GG is in pedestal region (possible neutron)
   if (fPIEDESTAL) {
      if (fPIEDESTAL->TestPoint(lumtot, sigg)) idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_NO;
      else idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_YES;
   }
   else {
      idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_UNKNOWN;
   }

   TheGrid = (KVIDZAGrid*) fGGgrid;

   //Info("Identify","OK=%d Z=%d Zmax=%d GG=%d\n",idr->IDOK,idr->Z,TheGrid->GetZmax(),TMath::Nint(sigg));
   if (idr->IDOK && idr->Z == TheGrid->GetZmax()) {
      //Gestion des saturations GG
      //on teste l identification PG
      if (fPGgrid) {
         Double_t sipg = (y < 0. ? GetIDMapY("PG") : y);
         fPGgrid->Identify(lumtot, sipg, idr);
         //on garde l identification PG si celle ci renvoie un code
         //de 0 a 4 ou 7
         if (idr->Zident) {
            //Info("Identify","On passe de %d a %d",Zgg,Zpg);
            TheGrid = (KVIDZAGrid*) fPGgrid;
         }
      }
   }

   if (TheGrid == fGGgrid) {
      if (fGGgrid->GetQualityCode() > KVIDZAGrid::kICODE6 && fPGgrid) { //we have to try PG grid (if there is one)
         Double_t sipg = (y < 0. ? GetIDMapY("PG") : y);
         fPGgrid->Identify(lumtot, sipg, idr);
         TheGrid = (KVIDZAGrid*) fPGgrid;
      }
   }
   if (TheGrid->GetQualityCode() == KVIDZAGrid::kICODE8) {
      // only if the final quality code is kICODE8 do we consider that it is
      // worthwhile looking elsewhere. In all other cases, the particle has been
      // "identified", even if we still don't know its Z and/or A (in this case
      // we consider that we have established that they are unknowable).

      return kFALSE;
   }

   if (TheGrid->GetQualityCode() == KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE7 (above last line in grid) then the estimated
      // Z is only a minimum value (Zmin)
      idr->IDcode = kIDCode5;

      return kTRUE;
   }

   if (TheGrid->GetQualityCode() > KVIDZAGrid::kICODE3 && TheGrid->GetQualityCode() < KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE4, kICODE5 or kICODE6 then this "nucleus"
      // corresponds to a point which is inbetween the lines, i.e. noise

      idr->IDcode = kIDCode10;
      return kTRUE;
   }

   // set general ID code Si-CsI
   idr->IDcode = kIDCode3;
   return kTRUE;
}


