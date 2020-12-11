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
#include "TObjString.h"
#include "TObjArray.h"
#include "TROOT.h"
#include "KVIdentificationResult.h"
#include "KVIDTelescope.h"

ClassImp(KVIDGChIoSi)
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

Bool_t KVIDGChIoSi::IsIdentifiable(Double_t x, Double_t y, TString* rejected_by) const
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
         if (rejected_by) *rejected_by = fSeuil->GetName();
         return kFALSE;
      };
   }
   //must be left of Emax_Si line
   if (fEmaxSi) {
      can_id =  fEmaxSi->WhereAmI(x, y, "left");
      if (!can_id) {
         const_cast < KVIDGChIoSi* >(this)->fICode = k_RightOfEmaxSi;
         if (rejected_by) *rejected_by = fEmaxSi->GetName();
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
   }
   else if (fICode == KVIDZAGrid::kICODE7) {
      // for particles above last line in grid, check if we are in fact in the Bragg zone
      if (fBragg && fBragg->WhereAmI(x, y, "left")) {
         const_cast<KVIDGChIoSi*>(this)->fICode = k_LeftOfBragg;
         idr->SetComment("Point to identify below Bragg curve. Z given is a Zmin");
         idr->IDOK = kTRUE;
         idr->IDquality = fICode;
      }

   }
}
