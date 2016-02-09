/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Fri Jun 12 11:34:53 2009
//Author: Abdelouahao Chbihi

#include "KVIDSiCsIVamos.h"
#include "KVINDRAReconNuc.h"
#include "KVIDGridManager.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDSiCsIVamos)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSiCsIVamos</h2>
<h4>Identification map SIE_xx-CSIyy of Vamos</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSiCsIVamos::KVIDSiCsIVamos() :
#if __cplusplus < 201103L
   fgrid(NULL), fSi(NULL), fCsI(NULL)
#else
   // C++11 supported, use nullptr
   fgrid(nullptr), fSi(nullptr), fCsI(nullptr)
#endif
{
}

KVIDSiCsIVamos::~KVIDSiCsIVamos()
{
   // Destructor
}

void KVIDSiCsIVamos::Initialize()
{
   // Initialize telescope for current run.
   // Pointers to grids for run are set, and if there is at least 1 (GG) grid,
   // we set IsReadyForID = kTRUE

   fSi = (KVSiliconVamos*)GetDetector(1);
   fCsI = (KVCsIVamos*)GetDetector(2);
   fgrid = 0;
   TIter next(fIDGrids);
   KVIDGrid* grid = 0;

   while ((grid = (KVIDGrid*)next())) {
      if (!strcmp(grid->GetVarY(), "SI")) fgrid = (KVIDZAGrid*)grid;
   }
   if (fgrid) {
      SetBit(kReadyForID);
      fgrid->Initialize();
      fgrid->Print();
   } else
      ResetBit(kReadyForID);
   //fgrid->Print();
}

const Char_t* KVIDSiCsIVamos::GetName() const
{
   // don t give any name, adapted for detectors of Vamos FP
   TString nom;
   nom = GetDetector(1)->GetName();
   nom += "_";
   nom += GetDetector(2)->GetName();
   const_cast<KVIDSiCsIVamos*>(this)->SetName(nom.Data());
   return TNamed::GetName();
}


Bool_t KVIDSiCsIVamos::Identify(KVIdentificationResult* idr, Double_t csi, Double_t si)
{
   //Particle identification and code setting using identification grids.


   //perform identification in Si - CsI map, in Vamos FP

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   KVIDGrid* theIdentifyingGrid = 0;

   fgrid->Identify(csi, si, idr);
   theIdentifyingGrid = (KVIDGrid*)fgrid;

   if (theIdentifyingGrid->GetQualityCode() == KVIDZAGrid::kICODE8) {
      // only if the final quality code is kICODE8 do we consider that it is
      // worthwhile looking elsewhere. In all other cases, the particle has been
      // "identified", even if we still don't know its Z and/or A (in this case
      // we consider that we have established that they are unknowable).
      return kFALSE;
   }

   if (theIdentifyingGrid->GetQualityCode() == KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE7 (above last line in grid) then the estimated
      // Z is only a minimum value (Zmin)
      idr->IDcode = 5;
      return kTRUE;
   }


   // set general ID code SiLi-CsI
   idr->IDcode = 3;
   return kTRUE;
}


