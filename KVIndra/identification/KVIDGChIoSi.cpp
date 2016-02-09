/*
$Id: KVIDGChIoSi.cpp,v 1.35 2009/05/05 15:57:52 franklan Exp $
$Revision: 1.35 $
$Date: 2009/05/05 15:57:52 $
$Author: franklan $
*/

#include "KVIDGChIoSi.h"
#include "KVIDZALine.h"
#include "KVIDCutLine.h"
#include "KVChIo.h"
#include "KVSilicon.h"
#include "KVINDRARRMValidator.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TROOT.h"
#include "KVIdentificationResult.h"
#include "KVIDTelescope.h"

ClassImp(KVIDGChIoSi)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KVIDGChIoSi
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
//
// Correctly-identified particles with no ambiguity have quality codes < KVIDZAGrid::kICODE4.
// Particles with quality code KVIDGChIoSi::k_BelowPunchThrough are also in principle correctly identified,
// but we give a warning that the point is below the line 'Punch_through' delimiting the region in which the
// identification lines can be mixed with particles punching through the silicon if no additional condition
// is placed on e.g. the CsI detector behind the silicon not firing.
// Particles with quality code KVIDGChIoSi::k_LeftOfBragg are below the Bragg curve and the Z attributed
// is a minimum value.
// Particles with code KVIDZAGrid::kICODE7 are (far) above the last line of the grid, their Z is also a minimum.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
KVIDGChIoSi::KVIDGChIoSi()
{
   //Default constructor
   init();
}

KVIDGChIoSi::KVIDGChIoSi(TString pattern)
{
   //constructor with a pattern
   init();
   SetPattern(pattern);
}

void KVIDGChIoSi::init()
{
   //default initialisations
   //SetOnlyZId(kTRUE) is called

   fBragg = fPunch = fSeuil = fEmaxSi = 0;
   SetOnlyZId(kTRUE);
}


KVIDGChIoSi::~KVIDGChIoSi()
{
   //Dtor.
}

//______________________________________________________________________________________________________//

Bool_t KVIDGChIoSi::IsIdentifiable(Double_t x, Double_t y) const
{
   // Sets fStatus for particle depending on its position in the grid.
   //
   // Returns kTRUE if the particle corresponding to point (x=esi,y=echio) in the ChIo-Si map
   // is identifiable i.e. if it is contained within the region delimited by
   //      - the line indicating the limit of particles stopping in the ChIo (KVIDCutLine:"Seuil_Si", accpeted:"right")
   //      - the line indicating the maximum canal/energy Silicium before saturation (KVIDCutLine:"Emax_Si", accepted:"left")
   // Particles which do not satisfy one of the following cuts:
   //      - the line indicating the lower limit of Z identification, or Bragg curve (KVIDCutLine:"Bragg_line", accepted:"right")
   //      - the line indicating the limit of particles punching through to the CsI (KVIDCutLine:"Punch_through", accepted:"above")
   // are identified with a quality code warning.

   Bool_t can_id = kTRUE;
   //must be right of arret chio/seuil si line
   if (fSeuil) {
      can_id = fSeuil->WhereAmI(x, y, "right");
      if (!can_id) {
         const_cast < KVIDGChIoSi* >(this)->fICode = k_BelowSeuilSi;
         return kFALSE;
      };
   }
   //must be left of Emax_Si line
   if (fEmaxSi) {
      can_id =  fEmaxSi->WhereAmI(x, y, "left");
      if (!can_id) {
         const_cast < KVIDGChIoSi* >(this)->fICode = k_RightOfEmaxSi;
         return kFALSE;
      };
   }
   return can_id;
}

//___________________________________________________________________________________

void KVIDGChIoSi::Initialize()
{
   // General initialisation method for identification grid.
   // This method MUST be called once before using the grid for identifications.
   // The ID lines are sorted.
   // The natural line widths of all ID lines are calculated.
   // The line with the largest Z (Zmax line) is found.
   // Pointers to different 'OK lines' are initialised

   KVIDZAGrid::Initialize();
   fSeuil = (KVIDLine*)GetCut("Seuil_Si");
   fBragg = (KVIDLine*)GetCut("Bragg_line");
   fPunch = (KVIDLine*)GetCut("Punch_through");
   fEmaxSi = (KVIDLine*)GetCut("Emax_Si");
}

//___________________________________________________________________________________

void KVIDGChIoSi::BackwardsCompatibilityFix()
{
   // Called after reading a grid from an ascii file.
   // Tries to convert information written by an old version of the class:
   //
   //<PARAMETER> Ring min=...  ----> <PARAMETER> IDTelescopes=...
   //<PARAMETER> Ring max=...
   //<PARAMETER> Mod min=...
   //<PARAMETER> Mod max=...

   KVIDZAGrid::BackwardsCompatibilityFix();
   if (fPar->HasParameter("IDTelescopes")) return;

   Warning("BackwardsCompatibilityFix",
           "This fix no longer works correctly. Dummy ID telescopes will be associated with this grid. There will be problems.");
   if (fPar->HasParameter("Rings")) { // && gIndra ) <== SHOULD NOT DEPEND ON KVINDRA!!!
      KVNumberList Rings(fPar->GetStringValue("Rings"));
      KVNumberList Modules(fPar->GetStringValue("Modules"));
      int r, m;
      Rings.Begin();
      while (!Rings.End()) {
         Modules.Begin();
         r = Rings.Next();
         while (!Modules.End()) {
            m = Modules.Next();
            KVIDTelescope* id = new KVIDTelescope();
            id->SetName(Form("CI_SI_%02d%02d", r, m)); // gIndra->GetIDTelescope( Form("CI_SI_%02d%02d", r,m) ); <== SHOULD NOT DEPEND ON KVINDRA!!!
            if (id) AddIDTelescope(id);
         }
      }
      WriteParameterListOfIDTelescopes();
      fPar->RemoveParameter("Rings");
      fPar->RemoveParameter("Modules");
   }
   fSeuil = (KVIDLine*)GetCut("Seuil_Si");
   fBragg = (KVIDLine*)GetCut("Bragg_line");
   fPunch = (KVIDLine*)GetCut("Punch_through");
   fEmaxSi = (KVIDLine*)GetCut("Emax_Si");
   if (fSeuil)((KVIDCutLine*)fSeuil)->SetAcceptedDirection("right");
   if (fBragg)((KVIDCutLine*)fBragg)->SetAcceptedDirection("right");
   if (fPunch)((KVIDCutLine*)fPunch)->SetAcceptedDirection("above");
   if (fEmaxSi)((KVIDCutLine*)fEmaxSi)->SetAcceptedDirection("left");
   SetVarY("CI-E/PG/GG");
   SetVarX("SI-E/PG/GG");
   SetOnlyZId();
}

//______________________________________________________________________________

void KVIDGChIoSi::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDGChIoSi.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 6) {
         R__v = R__b.ReadVersion(&R__s, &R__c);// read version of KVIDZGrid
         if (R__v != 1) {
            Warning("Streamer", "Reading KVIDZGrid with version=%d", R__v);
         }
         KVIDGrid::Streamer(R__b);
         R__b >> fZMax;
         KVINDRARRMValidator rrm;
         rrm.Streamer(R__b);
         fPattern.Streamer(R__b);
         R__b >> fBragg;
         R__b >> fPunch;
         R__b >> fSeuil;
         R__b >> fEmaxSi;
         Bool_t fIgnorePunchThrough;
         R__b >> fIgnorePunchThrough;
      } else {
         KVIDGChIoSi::Class()->ReadBuffer(R__b, this, R__v, R__s, R__c);
      }
   } else {
      KVIDGChIoSi::Class()->WriteBuffer(R__b, this);
   }
}

void KVIDGChIoSi::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // After identification of the particle, we adjust the quality code
   // (if the particle was well-identified by KVIDZAGrid::Identify, i.e. with
   // fICode<KVIDZAGrid::kICODE4) if:
   //    the particle is below the 'Bragg_line' => quality code KVIDGChIoSi::k_LeftOfBragg
   //           in this case the Z given is a minimum value
   //    the particle is below the 'Punch_through' line
   //          => quality code KVIDGChIoSi::k_BelowPunchThrough, but the particle
   //            is in principle well identified, if a condition has been applied
   //            to make sure that the detector behind the silicon (i.e. CsI) did not
   //            fire.

   KVIDZAGrid::Identify(x, y, idr);
   // check Bragg & punch through for well identified particles
   if (fICode < KVIDZAGrid::kICODE4) {
      //identified particles below (left of) Bragg line : Z is a Zmin
      if (fBragg && fBragg->WhereAmI(x, y, "left")) {
         const_cast<KVIDGChIoSi*>(this)->fICode = k_LeftOfBragg;
         idr->SetComment("Point to identify below Bragg curve. Z given is a Zmin");
      }
      //if a particle is well-identified (i.e. not too far from the identification lines)
      //but it lies below the 'Punch_through' line, we give it a warning code
      if (fPunch && fPunch->WhereAmI(x, y, "below")) {
         const_cast<KVIDGChIoSi*>(this)->fICode = k_BelowPunchThrough;
         idr->SetComment("warning: point below punch-through line");
      }
      idr->IDquality = fICode;
   } else if (fICode == KVIDZAGrid::kICODE7) {
      // for particles above last line in grid, check if we are in fact in the Bragg zone
      if (fBragg && fBragg->WhereAmI(x, y, "left")) {
         const_cast<KVIDGChIoSi*>(this)->fICode = k_LeftOfBragg;
         idr->SetComment("Point to identify below Bragg curve. Z given is a Zmin");
         idr->IDOK = kTRUE;
         idr->IDquality = fICode;
      }

   }
}
