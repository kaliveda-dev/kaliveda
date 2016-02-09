//Created by KVClassFactory on Wed Jun 19 09:06:08 2013
//Author: dgruyer

#ifndef __KVGRAPH_H
#define __KVGRAPH_H

#include "TGraphErrors.h"

class KVGraph : public TGraphErrors {
protected:
   Double_t* fDisplayDivX;
   Double_t* fDisplayDivY;
   Int_t fNDivX, fNDivY;
   Bool_t fUseX, fUseY;

   Int_t fCropMode;

   virtual void init();
   void HideAxis(TAxis* ax);
   Double_t* GetBinArray(Int_t& nbins, Bool_t xAxis = kTRUE);

public:

   enum CropOption {
      kNoCrop,
      kCropOnDiv,
      kCropOnGraph,
      kCropMin
   };

   KVGraph();
   KVGraph(Int_t n);
//    KVGraph(Int_t n, const Int_t* x, const Int_t* y);
//    KVGraph(Int_t n, const Float_t* x, const Float_t* y);
//    KVGraph(Int_t n, const Double_t* x, const Double_t* y);
//    KVGraph(const TVectorF& vx, const TVectorF& vy);
//    KVGraph(const TVectorD& vx, const TVectorD& vy);
//    KVGraph(const TH1* h);
//    KVGraph(const TF1* f, Option_t* option = "");
//    KVGraph(const char* filename, const char* format = "%lg %lg", Option_t* option = "");

   virtual ~KVGraph();
   void Draw(Option_t* chopt = "");

   void SetDisplayDivX(Int_t ndiv, Double_t* div = 0)
   {
      fNDivX = ndiv;
      fDisplayDivX = div;
   }
   void SetDisplayDivY(Int_t ndiv, Double_t* div = 0)
   {
      fNDivY = ndiv;
      fDisplayDivY = div;
   }

   void SetCropMode(Int_t crop = kCropOnGraph)
   {
      fCropMode = crop;
   }
   void SetUseX(Bool_t usex = kTRUE)
   {
      fUseX = usex;
   }
   void SetUseY(Bool_t usey = kTRUE)
   {
      fUseY = usey;
   }


   ClassDef(KVGraph, 1) //TGraph with new draw option
};

#endif
