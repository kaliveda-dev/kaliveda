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

   virtual Bool_t IsIdentifiable(Double_t x, Double_t y) const;
   virtual void Identify(Double_t x, Double_t y, KVIdentificationResult*) const;

   virtual void Initialize();

   ClassDef(KVIDGChIoSi, 6)     //ChIo-Si dE-E Z-identification grid
};

#endif
