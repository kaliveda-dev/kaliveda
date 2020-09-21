//Created by KVClassFactory on Mon Jan 23 14:29:32 2017
//Author: Diego Gruyer

#include "KVPIDIntervalPainter.h"
#include "TCanvas.h"
#include "TFrame.h"

ClassImp(KVPIDIntervalPainter)


KVPIDIntervalPainter::KVPIDIntervalPainter(interval* itv, TH1* hh): TNamed(Form("%d_%d", itv->GetZ(), itv->GetA()), Form("%d_%d", itv->GetZ(), itv->GetA()))
{

   fDisplayLabel = false;
   fInterval = itv;
   fLinearHisto = hh;
   pid = fInterval->GetPID();
   min = fInterval->GetPIDmin();
   max = fInterval->GetPIDmax();
   fHighlight = false;

   fZ = fInterval->GetZ();
   fA = fInterval->GetA();

   fCanvas = 0;

   fMarker = new TMarker(pid, fLinearHisto->GetBinContent(fLinearHisto->FindBin(pid)), 23);
   fLabel  = new TLatex(fMarker->GetX(), fMarker->GetY(), Form(" (%d,%d)", fZ, fA));
   fLabel->SetTextAlign(12);
   fLabel->SetTextFont(42);
   fLabel->SetTextSize(0.0421941);

   fLine1  = new TLine(min, 0, min, 200); //,fLinearHisto->GetBinContent(fLinearHisto->FindBin(min)));
   fLine2  = new TLine(max, 0, max, 200); //,fLinearHisto->GetBinContent(fLinearHisto->FindBin(max)));

//    fMarker->SetMarkerColor(kGray+1);
   fLine1->SetLineColor(kGray + 1);
   fLine2->SetLineColor(kGray + 1);

   fXaxis = fLinearHisto->GetXaxis();
   fYaxis = fLinearHisto->GetYaxis();
}

//____________________________________________________________________________//

KVPIDIntervalPainter::KVPIDIntervalPainter(const char* name, const char* title)
   : TNamed(name, title)
{
   // Constructor inherited from TNamed
}

//____________________________________________________________________________//

KVPIDIntervalPainter::KVPIDIntervalPainter(const TString& name, const TString& title)
   : TNamed(name, title)
{
   // Constructor inherited from TNamed
}

//____________________________________________________________________________//

KVPIDIntervalPainter::~KVPIDIntervalPainter()
{
   // Destructor

   fCanvas->GetListOfPrimitives()->Remove(fMarker);
   fCanvas->GetListOfPrimitives()->Remove(fLine1);
   fCanvas->GetListOfPrimitives()->Remove(fLine2);
   if (fDisplayLabel) fCanvas->GetListOfPrimitives()->Remove(fLabel);
   fCanvas->GetListOfPrimitives()->Remove(this);

   delete fMarker;
   delete fLine1;
   delete fLine2;
}

//____________________________________________________________________________//


void KVPIDIntervalPainter::Draw(Option_t*)
{
   if (fMarker) fMarker->Draw();
   if (fLine1)  fLine1->Draw();
   if (fLine2)  fLine2->Draw();
}

void KVPIDIntervalPainter::HighLight(bool hi)
{
   fHighlight = hi;
   if (hi) {
      fMarker->SetMarkerSize(1);
      fMarker->SetMarkerColor(kRed + 1);
      fLabel->SetTextColor(kRed + 1);
      fLine1->SetLineWidth(2);
      fLine1->SetLineColor(kRed + 1);
      fLine2->SetLineWidth(2);
      fLine2->SetLineColor(kRed + 1);
   }
   else {
      fMarker->SetMarkerSize(1);
      fMarker->SetMarkerColor(kBlack);
      fLabel->SetTextColor(kBlack);
      fLine1->SetLineWidth(1);
      fLine1->SetLineColor(kGray + 1);
      fLine2->SetLineWidth(1);
      fLine2->SetLineColor(kGray + 1);
   }
}

void KVPIDIntervalPainter::Update()
{
   SetName(Form("%d_%d", fInterval->GetZ(), fInterval->GetA()));
   SetTitle(Form("%d_%d", fInterval->GetZ(), fInterval->GetA()));
   fZ = fInterval->GetZ();
   fA = fInterval->GetA();
   fLabel->SetText(fLabel->GetX(), fLabel->GetY(), Form(" (%d,%d)", fZ, fA));
}

void KVPIDIntervalPainter::SetDisplayLabel(bool dis)
{
   if (fDisplayLabel == dis) return;
   fDisplayLabel = dis;
   if (fDisplayLabel) fLabel->Draw();
   else fCanvas->GetListOfPrimitives()->Remove(fLabel);
}

void KVPIDIntervalPainter::Paint(Option_t*)
{
   double ym = fCanvas->GetFrame()->GetY2();
   if (fCanvas->GetLogy()) ym = TMath::Exp(ym * TMath::Log(10));

   Color_t col = kBlack;
   if (!(fInterval->GetA() % 2)) col = kBlue;

   if (!fHighlight) {
      fLine1->SetLineColor(col);
      fLine2->SetLineColor(col);
      fMarker->SetMarkerColor(col);
      fLabel->SetTextColor(col);
   }

   double mi = fXaxis->GetBinCenter(fXaxis->GetFirst());
   double ma = fXaxis->GetBinCenter(fXaxis->GetLast());

   fLine1->SetY1(0);
   fLine2->SetY1(0);
   if (pid > mi && pid < ma) {
      fLine1->SetY2(ym);
      fLine2->SetY2(ym);
      fMarker->SetMarkerSize(1);
      if (fMarker->GetX() != pid) {
         fMarker->SetY(fLinearHisto->GetBinContent(fLinearHisto->FindBin(fMarker->GetX())));
         fInterval->SetPID(fMarker->GetX());
         pid = fMarker->GetX();
         if (fDisplayLabel) {
            fLabel->SetX(fMarker->GetX());
            fLabel->SetY(fMarker->GetY());
         }
         IntMod();
      }
      if (fLine1->GetX1() != min) {
         fInterval->SetPIDmin(fLine1->GetX1());
         min = fLine1->GetX1();
         IntMod();
      }
      if (fLine2->GetX1() != max) {
         fInterval->SetPIDmax(fLine2->GetX1());
         max = fLine2->GetX1();
         IntMod();
      }
   }
   else {
      fLine1->SetY2(0);
      fLine2->SetY2(0);
      fMarker->SetMarkerSize(0);
   }
}





