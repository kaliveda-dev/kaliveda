/***************************************************************************
                          KVIDGCsI.h  -  description
                             -------------------
    begin                : Nov 24 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDGCsI.h,v 1.18 2009/04/06 15:09:29 franklan Exp $
***************************************************************************/

#ifndef KVIDGCsI_H
#define KVIDGCsI_H

#include "KVIDZAGrid.h"

/**
   \class KVIDGCsI
\brief Identification grids for CsI R-L (fast-slow) matrices.
\ingroup Identification

The identification procedure is supposed to be identical to that of the FORTRAN algorithm IdnCsOr
developed by Laurent Tassan-Got and used by the INDRA collaboration since 1993.

The status codes returned by GetQualityCode() are the same as IdnCsOr, with the addition of kICODE10
for identification of gammas:
~~~~{.cpp}
 KVIDGCsI::kICODE0  ok
 KVIDGCsI::kICODE1  Z ok, mais les masses superieures a A sont possibles
 KVIDGCsI::kICODE2  Z ok, mais les masses inferieures a A sont possibles
 KVIDGCsI::kICODE3  Z ok, mais les masses superieures ou inferieures a A sont possibles
 KVIDGCsI::kICODE4  Z ok, masse hors limite superieure ou egale a A
 KVIDGCsI::kICODE5  Z ok, masse hors limite inferieure ou egale a A
 KVIDGCsI::kICODE6  au-dessus de la ligne fragment, Z est alors un Zmin
 KVIDGCsI::kICODE7  a gauche de la ligne fragment, Z est alors un Zmin et le plus probable
 KVIDGCsI::kICODE8  Z indetermine ou (x,y) hors limites
 KVIDGCsI::kICODE9  pas de lignes pour ce module
 KVIDGCsI::kICODE10 gamma
~~~~
*/

class KVIDGCsI: public KVIDZAGrid {

   KVIDLine* IMFLine;//!
   KVIDLine* GammaLine;//!

protected:
   KVIDLine* GetNearestIDLine(Double_t x, Double_t y,
                              const Char_t* position, Int_t& idx_min,
                              Int_t& idx_max);
   virtual void BackwardsCompatibilityFix();

public:

   KVIDGCsI();
   KVIDGCsI(const KVIDGCsI&);
   virtual ~ KVIDGCsI();

   inline KVIDLine* GetGammaLine() const
   {
      return GammaLine;
   }
   inline KVIDLine* GetIMFLine() const
   {
      return IMFLine;
   }


   virtual void Identify(Double_t x, Double_t y,
                         KVIdentificationResult*) const;

   virtual Bool_t IsIdentifiable(Double_t x, Double_t y, TString* = nullptr) const;

   KVIDZALine* GetZALine(Int_t z, Int_t a, Int_t&) const;
   KVIDZALine* GetZLine(Int_t z, Int_t&) const;

   void IdentZA(Double_t x, Double_t y, Int_t& Z, Double_t& A);
   virtual void Initialize();
   virtual TClass* DefaultIDLineClass()
   {
      return TClass::GetClass("KVIDCsIRLLine");
   }

   ClassDef(KVIDGCsI, 2)       //CsI Rapide-Lente grid & identification
};

#endif
