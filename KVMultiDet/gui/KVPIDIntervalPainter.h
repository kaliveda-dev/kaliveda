//Created by KVClassFactory on Mon Jan 23 14:29:32 2017
//Author: Diego Gruyer

#ifndef __KVPIDINTERVALPAINTER_H
#define __KVPIDINTERVALPAINTER_H


#include "RQ_OBJECT.h"

#include "TH1.h"
#include "TLine.h"
#include "TMarker.h"
#include "TLatex.h"
#include "KVCanvas.h"
#include "KVList.h"
#include "KVIDZAFromZGrid.h"

#include "TNamed.h"

/**
   \class KVPIDIntervalPainter
\brief Used for mass identification GUI
\ingroup GUI
*/

class KVPIDIntervalPainter : public TNamed {
   RQ_OBJECT("KVPIDIntervalPainter")

public:
   TMarker* fMarker;
   TLine*   fLine1;
   TLine*   fLine2;
   TLatex*  fLabel;

   TH1* fLinearHisto;
   interval* fInterval;
   TAxis* fXaxis, *fYaxis;
   Bool_t fHighlight;

   double pid, min, max;
   Int_t fA, fZ;

   KVCanvas* fCanvas;

   bool fDisplayLabel;


public:
   KVPIDIntervalPainter(interval* itv, TH1* hh);
   KVPIDIntervalPainter(const char* name, const char* title);
   KVPIDIntervalPainter(const TString& name, const TString& title);

   virtual void Draw(Option_t*     option = "");
   void HighLight(bool hi = true);
   void IntMod()
   {
      Emit("IntMod()");
   }
   void SetCanvas(KVCanvas* cc)
   {
      fCanvas = cc;
   }

   void Update();
   Int_t GetZ()
   {
      return fZ;
   }
   Int_t GetA()
   {
      return fA;
   }

   void SetDisplayLabel(bool dis = true);
   bool GetDisplayLabel()
   {
      return fDisplayLabel;
   }


   virtual void  Paint(Option_t* chopt = "");

   virtual ~KVPIDIntervalPainter();

   ClassDef(KVPIDIntervalPainter, 1) //toot
};

#endif
