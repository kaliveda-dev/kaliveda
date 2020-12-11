/***************************************************************************
                          KVIDGChIoSi.h  -  description
                             -------------------
    begin                : Nov 24 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDGChIoSi.h,v 1.24 2009/04/06 15:10:09 franklan Exp $
***************************************************************************/

#ifndef KVIDGChIoSi_H
#define KVIDGChIoSi_H

#include "KVIDZAGrid.h"
#include "TString.h"

/**
  \class KVIDGChIoSi
  \ingroup INDRAIdent
  \brief Identification grids for ChIo-Si telescopes of INDRA

 Z-only identification grids for INDRA Ionisation chamber-Silicon telescopes.
 To identify a particle with this grid, do:
~~~~{.cpp}
     if( grid->IsIdentifiable(x,y) ){
            grid->Identify(x,y,nuc);
     }
~~~~

 After attempting identification with method Identify(), GetQualityCode() method returns
 one of the following status codes:

~~~~{.cpp}
 KVIDZAGrid::kICODE0,                   OK
 KVIDZAGrid::kICODE1,                   slight ambiguity of Z, which could be larger
 KVIDZAGrid::kICODE2,                   slight ambiguity of Z, which could be smaller
 KVIDZAGrid::kICODE3,                   slight ambiguity of Z, which could be larger or smaller
 KVIDZAGrid::kICODE4,                   point is in between two lines of different Z, too far from either to be considered well-identified
 KVIDZAGrid::kICODE5,                   point is in between two lines of different Z, too far from either to be considered well-identified
 KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid
 KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid
 KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid
 KVIDGChIoSi::k_BelowPunchThrough   "warning: point below punch-through line",
 KVIDGChIoSi::k_BelowSeuilSi      "point to identify left of Si threshold line (bruit/arret ChIo?)",
 KVIDGChIoSi::k_LeftOfBragg       "point to identify below Bragg curve. Z given is a Zmin",
 KVIDGChIoSi::k_RightOfEmaxSi         "point to identify has E_Si > Emax_Si i.e. codeur is saturated. Unidentifiable",
~~~~

 Correctly-identified particles with no ambiguity have quality codes < KVIDZAGrid::kICODE4.

 Particles with quality code KVIDGChIoSi::k_BelowPunchThrough are also in principle correctly identified,
 but we give a warning that the point is below the line 'Punch_through' delimiting the region in which the
 identification lines can be mixed with particles punching through the silicon if no additional condition
 is placed on e.g. the CsI detector behind the silicon not firing.

Particles with quality code KVIDGChIoSi::k_LeftOfBragg are below the Bragg curve and the Z attributed
 is a minimum value.

 Particles with code KVIDZAGrid::kICODE7 are (far) above the last line of the grid, their Z is also a minimum.
 */
class KVIDGChIoSi : public KVIDZAGrid {

private:

   KVIDLine* fBragg;            //bragg line
   KVIDLine* fPunch;            //punch-through line
   KVIDLine* fSeuil;            //seuil silicium line
   KVIDLine* fEmaxSi;           //saturation codeur silicon

   void init();
   void BackwardsCompatibilityFix();

public:

   enum {
      k_BelowPunchThrough = KVIDZAGrid::kICODE10 + 1,    //point to ID was below punch-through line (bruit)
      k_BelowSeuilSi,           //point to ID was left of "Seuil Si" line (bruit/arret ChIo?)
      k_LeftOfBragg,            //point to ID is between 'Seuil Si' and 'Bragg': Z given is a Zmin.
      k_RightOfEmaxSi          //point to ID has E_Si > Emax_Si i.e. codeur is saturated. Unidentifiable.
   };

public:
   KVIDGChIoSi();
   KVIDGChIoSi(TString pattern);
   virtual ~ KVIDGChIoSi();

   KVIDLine* GetSeuilSiLine()
   {
      return (fSeuil);
   };
   KVIDLine* GetBraggLine()
   {
      return (fBragg);
   };
   KVIDLine* GetPunchThroughLine()
   {
      return (fPunch);
   };
   KVIDLine* GetEmaxSiLine()
   {
      return (fEmaxSi);
   };

   virtual Bool_t IsIdentifiable(Double_t x, Double_t y, TString* rejected_by = nullptr) const;
   virtual void Identify(Double_t x, Double_t y, KVIdentificationResult*) const;

   virtual void Initialize();

   ClassDef(KVIDGChIoSi, 6)     //ChIo-Si dE-E Z-identification grid
};

#endif
