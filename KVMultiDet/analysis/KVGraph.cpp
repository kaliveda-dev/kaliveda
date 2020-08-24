//Created by KVClassFactory on Wed Jun 19 09:06:08 2013
//Author: dgruyer

#include "KVGraph.h"
#include "TPad.h"
#include "TAxis.h"
#include "TMath.h"
#include "Riostream.h"
#include "TH2F.h"
#include "KVGaxis.h"
#include "KVNumberList.h"

ClassImp(KVGraph)

KVGraph::KVGraph()
{
   // Default constructor
   init();
}

//________________________________________________________________

//KVGraph::KVGraph(const KVGraph& obj) : TGraph()
//{
//    // Copy constructor
//    // This ctor is used to make a copy of an existing object (for example
//    // when a method returns an object), and it is always a good idea to
//    // implement it.
//    // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

//    obj.Copy(*this);
//}

//________________________________________________________________

KVGraph::KVGraph(Int_t n) : TGraphErrors(n)
{
   // Write your code here
   init();
}

//________________________________________________________________

//KVGraph::KVGraph(Int_t n, const Int_t* x, const Int_t* y) : TGraphErrors(n, x, y)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(Int_t n, const Float_t* x, const Float_t* y) : TGraphErrors(n, x, y)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(Int_t n, const Double_t* x, const Double_t* y) : TGraphErrors(n, x, y)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(const TVectorF& vx, const TVectorF& vy) : TGraphErrors(vx, vy)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(const TVectorD& vx, const TVectorD& vy) : TGraphErrors(vx, vy)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(const TH1* h) : TGraphErrors(h)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(const TF1* f, Option_t* option) : TGraphErrors(f, option)
//{
//    // Write your code here
//    init();
//}

////________________________________________________________________

//KVGraph::KVGraph(const char* filename, const char* format, Option_t* option) : TGraphErrors(filename, format, option)
//{
//    // Write your code here
//    init();
//}

KVGraph::~KVGraph()
{
   // Destructor
}

void KVGraph::Draw(Option_t* chopt)
{
   TGraphErrors::Draw(chopt);

   gPad->SetFillColor(0);
   gPad->SetBorderMode(0);
   gPad->SetBorderSize(2);
   gPad->SetFrameBorderMode(0);
   gPad->SetFrameLineColor(0);
   gPad->SetFrameBorderMode(0);

   gPad->Modified();
   gPad->Update();

   TString opt(chopt);
   opt.ToLower();
   if (!opt.Contains("t")) return;

   Double_t xmin = GetXaxis()->GetXmin();
   Double_t xmax = GetXaxis()->GetXmax();
   Double_t ymin = GetYaxis()->GetXmin();
   Double_t ymax = GetYaxis()->GetXmax();

   if ((!fNDivX) && fUseX) fDisplayDivX = GetBinArray(fNDivX, kTRUE);
   if ((!fNDivY) && fUseY) fDisplayDivY = GetBinArray(fNDivY, kFALSE);

   Double_t xstr, xstp, ystr, ystp;
   if (fUseX) {
      xstr = TMath::MinElement(fNDivX, fDisplayDivX);
      xstp = TMath::MaxElement(fNDivX, fDisplayDivX);
   }
   else {
      xstr = xmin;
      xstp = xmax;
   }

   if (fUseY) {
      ystr = TMath::MinElement(fNDivY, fDisplayDivY);
      ystp = TMath::MaxElement(fNDivY, fDisplayDivY);
   }
   else {
      ystr = ymin;
      ystp = ymax;
   }

   switch (fCropMode) {
      case kNoCrop:
         xstr = TMath::Min(xmin, xstr);
         xstp = TMath::Max(xmax, xstp);
         ystr = TMath::Min(ymin, ystr);
         ystp = TMath::Max(ymax, ystp);
         break;
      case kCropOnDiv:
         break;
      case kCropOnGraph:
         xstr = TMath::Min(TMath::MinElement(GetN(), GetX()), xstr);
         xstp = TMath::Max(TMath::MaxElement(GetN(), GetX()), xstp);
         ystr = TMath::Min(TMath::MinElement(GetN(), GetY()), ystr);
         ystp = TMath::Max(TMath::MaxElement(GetN(), GetY()), ystp);
         break;
      case  kCropMin:
         xstr = TMath::Min(TMath::MinElement(GetN(), GetX()), xstr);
         xstp = TMath::Max(xmax, xstp);
         ystr = TMath::Min(TMath::MinElement(GetN(), GetY()), ystr);
         ystp = TMath::Max(ymax, ystp);
         break;
   }

   TGaxis* axis = 0;
   // draw new x axis and hide the old one but keeping its option
   if (fUseX) axis = new KVGaxis(xstr, ymin, xstp, ymin, fNDivX, fDisplayDivX);
   else      axis = new TGaxis(xstr, ymin, xstp, ymin, xstr, xstp, fNDivX);
   axis->ImportAxisAttributes(GetXaxis());
   HideAxis(GetXaxis());
   axis->Draw();

   // draw new y axis and hide the old one but keeping its option
   if (fUseY) axis = new KVGaxis(xmin, ystr, xmin, ystp, fNDivY, fDisplayDivY);
   else      axis = new TGaxis(xmin, ystr, xmin, ystp, ystr, ystp, fNDivY);
   axis->ImportAxisAttributes(GetYaxis());
   HideAxis(GetYaxis());
   axis->Draw();

   gPad->Modified();
   gPad->Update();
}

void KVGraph::HideAxis(TAxis* ax)
{
   ax->SetTitleSize(0.0);
   ax->SetTickLength(0);
   ax->SetAxisColor(0);
   ax->SetLabelColor(0);
}

Double_t* KVGraph::GetBinArray(Int_t& nbins, Bool_t xAxis)
{
   Double_t* x;
   if (xAxis) x = GetX();
   else      x = GetY();

   KVNumberList nb;
   for (Int_t i = 0; i < GetN(); i++) nb.Add((x[i]) * 100);

   nb.Begin();

   Double_t* bins = new Double_t[nbins];
   Int_t i = 0;
   while (!nb.End()) bins[i++] = (nb.Next()) / 100.;

   return bins;
}

void KVGraph::init()
{
   fDisplayDivX = 0;
   fDisplayDivY = 0;
   fNDivX = fNDivY = 0;
   fCropMode = kCropOnGraph;
   fUseX = fUseY = kTRUE;
}











