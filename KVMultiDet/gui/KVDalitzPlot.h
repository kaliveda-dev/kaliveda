//Created by KVClassFactory on Tue Jan 17 15:10:46 2012
//Author: bonnet

#ifndef __KVDALITZPLOT_H
#define __KVDALITZPLOT_H

#include "TH2F.h"
#include "TLine.h"
#include "Riostream.h"

class KVDalitzPlot : public TH2F {

protected:
   Bool_t fOrdered;
   Int_t fShowBorder;
   Int_t fShowCenter;

   TLine* lb1, *lb2, *lb3; //!
   TLine* lc1, *lc2, *lc3; //!

   Int_t FillMe(Double_t a1, Double_t a2, Double_t a3);

public:
   KVDalitzPlot();
   KVDalitzPlot(const char* name, const char* title, Bool_t ordered = kFALSE, Int_t nbinsx = 120, Double_t xlow = 0., Double_t xup = 1.2, Int_t nbinsy = 120, Double_t ylow = 0., Double_t yup = 1.2);
   KVDalitzPlot(const KVDalitzPlot&) ;
   virtual ~KVDalitzPlot();
   void Copy(TObject&) const;

   Int_t FillAsDalitz(Double_t a1, Double_t a2, Double_t a3);
   void  Draw(Option_t* opt = "");
   void  SetOrdered(Bool_t ordered = kTRUE)
   {
      fOrdered = ordered;
   }
   void  SetShowBorder(Int_t value = 1); // *TOGGLE*
   void  SetShowCenter(Int_t value = 1); // *TOGGLE*
   Int_t  GetShowBorder()
   {
      return fShowBorder;
   }
   Int_t  GetShowCenter()
   {
      return fShowCenter;
   }

   ClassDef(KVDalitzPlot, 1) //Fill 3D observables in a dalitz plot
};

#endif
