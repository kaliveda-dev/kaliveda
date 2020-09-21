//Created by KVClassFactory on Tue Dec  4 09:49:15 2012
//Author: dgruyer

#ifndef __KVZALINEFINDER_H
#define __KVZALINEFINDER_H

#include "KVBase.h"

#include <TH2F.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TFile.h>
#include <TMath.h>
#include <TNamed.h>
#include <TRandom3.h>
#include <TSpectrum.h>
#include <TROOT.h>

#include <Riostream.h>
#include <RQ_OBJECT.h>

#include "KVIDZAGrid.h"

#include <vector>

/**
\class KVZALineFinder
\brief (try to) find mass lines from charge lines
\ingroup GUI
 */

class KVZALineFinder : public KVBase {
   RQ_OBJECT("KVZALineFinder")

protected:
   KVIDZAGrid* fGrid;
   TH2*        fHisto;

   KVIDZAGrid* fGeneratedGrid;
   TH2*        fLinearHisto;
   TH2* fConvertHisto;

   TSpectrum fSpectrum;
   TGraph*   fPoints;
   Int_t     fNPoints;

   TList*    fLines;
   Int_t     fBinsByZ;

   std::vector<int> fAList;

public:
   KVZALineFinder(KVIDZAGrid* gg, TH2* hh);
   KVZALineFinder(const KVZALineFinder&) ;
   virtual ~KVZALineFinder();
   void Copy(TObject&) const;

protected:
   TH2* LinearizeHisto(Int_t nZbin = 40);
   void FindALine(Int_t zz, Int_t width = 10);
   void FindZLine(Int_t zz);
   void SortLines(TList* Lines);
   void MakeGrid();
   void DrawGrid();

public:
   KVIDZAGrid* GetGrid()
   {
      return fGeneratedGrid;
   }
   TH2* GetHisto()
   {
      return fHisto;
   }
   void SetNbinsByZ(Int_t binByZ)
   {
      fBinsByZ = binByZ;
   }

   void SetAList(const char* Alist);
   void Draw(Option_t* /*opt_*/ = "")
   {
      if (fLines)fLines->Execute("Draw", "\"PN\"");
   }
   void ProcessIdentification(Int_t zmin = -1, Int_t zmax = -1);

   void Increment(Float_t x)  // *SIGNAL*
   {
      Emit("Increment(Float_t)", x);
   }
   void IncrementLinear(Float_t x)  // *SIGNAL*
   {
      Emit("IncrementLinear(Float_t)", x);
   }


   ClassDef(KVZALineFinder, 1) //(try to) find masse lines from charge lines
};

#endif
