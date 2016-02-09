//Created by KVClassFactory on Mon Sep 24 16:08:22 2012
//Author: Dijon Aurore

#include "KVIDGChIoSi_e494s.h"

ClassImp(KVIDGChIoSi_e494s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDGChIoSi_e494s</h2>
<h4>Specific identification grid for e494s</h4>
<!-- */
// --> END_HTML
//
// Z-only identification grids for INDRA Ionisation chamber-Silicon telescopes.
// To identify a particle with this grid, do:
//     if( grid->IsIdentifiable(x,y) ){
//            grid->Identify(x,y,nuc);
//     }
//
// After attempting identification with method Identify, GetQualityCode() method returns
// one of the following status codes:
//
// KVIDZAGrid::kICODE0,                   OK
// KVIDZAGrid::kICODE1,                   slight ambiguity of Z, which could be larger
// KVIDZAGrid::kICODE2,                   slight ambiguity of Z, which could be smaller
// KVIDZAGrid::kICODE3,                   slight ambiguity of Z, which could be larger or smaller
// KVIDZAGrid::kICODE4,                   point is in between two lines of different Z, too far from either to be considered well-identified
// KVIDZAGrid::kICODE5,                   point is in between two lines of different Z, too far from either to be considered well-identified
// KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid
// KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid
// KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid
// KVIDGChIoSi::k_BelowPunchThrough   "warning: point below punch-through line",
// KVIDGChIoSi::k_BelowSeuilSi      "point to identify left of Si threshold line (bruit/arret ChIo?)",
// KVIDGChIoSi::k_LeftOfBragg       "point to identify below Bragg curve. Z given is a Zmin",
// KVIDGChIoSi::k_RightOfEmaxSi         "point to identify has E_Si > Emax_Si i.e. codeur is saturated. Unidentifiable",
// KVIDGChIoSi_e494s::k_BelowSeuilChIo         "point to identify was below of "ChIo threshold" line,
//
// Correctly-identified particles with no ambiguity have quality codes < KVIDZAGrid::kICODE4.
// Particles with quality code KVIDGChIoSi::k_BelowPunchThrough are also in principle correctly identified,
// but we give a warning that the point is below the line 'Punch_through' delimiting the region in which the
// identification lines can be mixed with particles punching through the silicon if no additional condition
// is placed on e.g. the CsI detector behind the silicon not firing.
// Particles with quality code KVIDGChIoSi::k_LeftOfBragg are below the Bragg curve and the Z attributed
// is a minimum value.
// Particles with code KVIDZAGrid::kICODE7 are (far) above the last line of the grid, their Z is also a minimum.
////////////////////////////////////////////////////////////////////////////////

KVIDGChIoSi_e494s::KVIDGChIoSi_e494s()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVIDGChIoSi_e494s::~KVIDGChIoSi_e494s()
{
   // Destructor
}
//________________________________________________________________

void KVIDGChIoSi_e494s::init()
{
   // Default initialisations

   fChIoSeuil = NULL;
}
//________________________________________________________________

void KVIDGChIoSi_e494s::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // After identification of the particle, we adjust the quality code
   // (if the particle was well-identified by KVIDGChIoSi::Identify, i.e. with
   // fICode<KVIDZAGrid::kICODE4) if:
   //    the particle is below the 'ChIo threshold line' => quality code KVIDGChIoSi_e494s::k_BelowSeuilChIo

   KVIDGChIoSi::Identify(x, y, idr);

   if (fICode < kICODE4) {

      if (fChIoSeuil && fChIoSeuil->WhereAmI(x, y, "below")) {
         const_cast<KVIDGChIoSi_e494s*>(this)->fICode = k_BelowSeuilChIo;
         idr->SetComment("warning: point below ChIo threshold line");
      }
      Int_t ZValidityvalue = -1;
      ZValidityvalue = const_cast<KVIDGChIoSi_e494s*>(this)->GetParameters()->GetIntValue("ZValidity");
      if (ZValidityvalue > -1 && idr->Z > ZValidityvalue) {
         const_cast<KVIDGChIoSi_e494s*>(this)->fICode = kICODE9;
      }
      idr->IDquality = fICode;
   }
}
//________________________________________________________________

void KVIDGChIoSi_e494s::Initialize()
{
   // General initialisation method for identification grid.
   // This method MUST be call once before using the grid
   // for identifications. The ID lines are stored. The natural
   // line widths of all ID lines are calculated.
   // The line with the largest Z (Zmax line) is found.
   // Pointers to different 'OK lines' are initialised.

   KVIDGChIoSi::Initialize();
   fChIoSeuil = (KVIDLine*)GetCut("Seuil_ChIo");
}
//---------------------------------------------------------------------
//  added by MFR june 2014 to take into account the ChIoCsI_seuil line
//
Bool_t KVIDGChIoSi_e494s::IsIdentifiable(Double_t x, Double_t y) const
{

   KVIDGChIoSi::IsIdentifiable(x, y);

   Bool_t can_id = kTRUE;

//must be above of arret chio/seuil csi line
   if (fChIoSeuil) {
      can_id = fChIoSeuil->WhereAmI(x, y, "above");
      if (!can_id) {
         const_cast < KVIDGChIoSi_e494s* >(this)->fICode = k_BelowSeuilChIo;
         return kFALSE;
      };
   }

   return can_id;


}
//----------------------------------------------------------------------
