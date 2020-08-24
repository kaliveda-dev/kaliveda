//Created by KVClassFactory on Wed Jun 19 16:07:04 2013
//Author: dgruyer

#ifndef __KVGAXIS_H
#define __KVGAXIS_H

#include "TGaxis.h"
#include "TLine.h"
#include "TAttText.h"

/**
\class KVGaxis
\brief Extension of TGaxis class optimised for KVGraph visualisation
\ingroup GUI
*/

class KVGaxis : public TGaxis {
protected:
   Int_t fNbins;
   Double_t* fBins;
   const char* fFormat;
   char** fLabels;

public:
   KVGaxis();
   KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, Double_t wmin, Double_t wmax, Int_t ndiv = 510, Option_t* chopt = "", Double_t gridlength = 0);

   KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, Int_t nbins, Double_t* bins, Double_t wmin = 0, Double_t wmax = 0,
           Int_t ndiv = 510, Option_t* chopt = "", Double_t gridlength = 0, char** labels = 0);

//    KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, Int_t nbins, Int_t* bins, Int_t ndiv = 510, Option_t* chopt = "", Double_t gridlength = 0);

   KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, const char* funcname, Int_t ndiv = 510, Option_t* chopt = "", Double_t gridlength = 0);
   virtual ~KVGaxis();
   virtual void PaintAxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax,
                          Double_t& wmin, Double_t& wmax, Int_t& ndiv, Option_t* chopt = "",
                          Double_t gridlength = 0, Bool_t drawGridOnly = kFALSE);
   //   void Copy(TObject& obj) const;

   void SetFormat(const char* form = "%lf")
   {
      fFormat = form;
   }

   ClassDef(KVGaxis, 1) //TAxis optimised for KVGraph visualisation
};

#endif
