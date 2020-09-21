//Created by KVClassFactory on Thu Nov 22 15:26:52 2012
//Author: dgruyer

#include "KVNuclearChart.h"
#include "KVNucleus.h"
#include "KVNucleusBox.h"
#include "KVCanvas.h"
#include "TStyle.h"
#include "TBox.h"
#include "TLine.h"
#include "TString.h"
#include "TLatex.h"
#include "KVLevelScheme.h"

ClassImp(KVNuclearChart)

KVNuclearChart::KVNuclearChart(Int_t nMin, Int_t nMax, Int_t zMin, Int_t zMax, Double_t life): fNucleusBoxList(kTRUE), fMagicList(kTRUE), fSymbolList(kTRUE)
{
   fNmin = nMin;
   fNmax = nMax;
   fZmin = zMin;
   fZmax = zMax;

   if ((fNmax < 0) || (fZmax < 0)) {
      fNmax = 177;
      fZmax = 115;
   }

   fOwnHisto = 0;
   fHisto = 0;
   fShownNucleus = 0;
   fCanvas = nullptr;
   fPad = nullptr;
   fShowMagicNumbers = kTRUE;
   fShowSymbol = kFALSE;

   fSymbol = new TPaveText(0.65, 0.16, 0.75, 0.01, "NDC");
   fSymbol->SetBorderSize(1);
   fInfo = new TPaveText(0.76, 0.16, 0.99, 0.01, "NDC");
   fInfo->SetTextAlign(12);
   fInfo->SetFillColor(kWhite);
   fInfo->SetLineColor(kBlack);
   fInfo->SetTextColor(kBlack);
   fInfo->SetBorderSize(1);

   KVNucleus nuc;
   for (int zz = fZmin; zz <= fZmax; zz++) {
      KVNumberList ll = nuc.GetKnownARange(zz);
      ll.Begin();
      while (!ll.End()) {
         Int_t aa = ll.Next();
         nuc.SetZandA(zz, aa);
         Int_t nn = aa - zz;
         if ((nn >= fNmin) && (nn <= fNmax)) {
            if (nuc.IsStable() || nuc.GetLifeTime() > life) {
               KVNucleusBox* nb = new KVNucleusBox(zz, nn);
               nb->SetNuclearChart(this);
               fNucleusBoxList.Add(nb);
            }
         }
      }
   }

   fNm[0] = 2;
   fNm[1] = 8;
   fNm[2] = 20;
   fNm[3] = 28;
   fNm[4] = 50;
   fNm[5] = 82;
   fNm[6] = 126;

   fNmMin[0] = 0.;
   fNmMin[1] = 1.;
   fNmMin[2] = 12.;
   fNmMin[3] = 18.;
   fNmMin[4] = 46.;
   fNmMin[5] = 93.;
   fNmMin[6] = 98.;
   fNmMax[0] = 10.;
   fNmMax[1] = 22.;
   fNmMax[2] = 40.;
   fNmMax[3] = 52.;
   fNmMax[4] = 90.;
   fNmMax[5] = 136.;
   fNmMax[6] = 126.*3;

   fZmMin[0] = 0.2;
   fZmMin[1] = 0.2;
   fZmMin[2] = 6.;
   fZmMin[3] = 10.;
   fZmMin[4] = 26.;
   fZmMin[5] = 44.;
   fZmMin[6] = 76.;
   fZmMax[0] = 9.;
   fZmMax[1] = 17.;
   fZmMax[2] = 30.;
   fZmMax[3] = 35.;
   fZmMax[4] = 53.;
   fZmMax[5] = 76.;
   fZmMax[6] = 95.;

   return;
}

//________________________________________________________________

KVNuclearChart::KVNuclearChart(const KVNuclearChart& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVNuclearChart::~KVNuclearChart()
{
   // Destructor
//   if(fCanvas)
//     {
//     fCanvas->ResetDisabledClass();
//     fCanvas->ResetDisabledObject();
//     }
   if ((fHisto) && (fOwnHisto)) delete fHisto;
   if (fSymbol) delete fSymbol;
   if (fInfo) delete fInfo;
}

//________________________________________________________________

void KVNuclearChart::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVNuclearChart::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVNuclearChart& CastedObj = (KVNuclearChart&)obj;
}

void KVNuclearChart::Draw(Option_t* option)
{
   gStyle->SetOptStat(0);

   TString opt(option);
   Bool_t DrawSame = opt.Contains("same");

   if (DrawSame && gPad) {
      fPad = gPad;
      TObject* obj = 0;
      TIter next(gPad->GetListOfPrimitives());
      while ((obj = next())) {
         if (obj->InheritsFrom("TH2")) {
            fHisto = (TH2*) obj;
            break;
         }
      }
   }
   else {
      KVCanvas* cc = new KVCanvas;
      TPad* pp = (TPad*) cc->cd();
      fPad = pp;

      Double_t marging = 0.001;
      pp->SetTopMargin(marging);
      pp->SetRightMargin(marging);
      pp->SetBottomMargin(marging);
      pp->SetLeftMargin(marging);

      fHisto = new TH2F("NuclearChart", "", fNmax - fNmin + 2, fNmin - 1, fNmax + 1, fZmax - fZmin + 2, fZmin - 1, fZmax + 1);
      fOwnHisto = kTRUE;
      TAxis* ax = 0;
      ax = fHisto->GetXaxis();
      ax->SetNdivisions(000);
      ax = fHisto->GetYaxis();
      ax->SetNdivisions(000);

      fHisto->Draw();
   }

   if (gPad->GetCanvas()->InheritsFrom("KVCanvas")) {
      KVCanvas* cc = (KVCanvas*) gPad->GetCanvas();
      cc->DisableClass("TLine");
      cc->DisableClass("KVNucleusBox");
      cc->DisableClass("TLatex");
      cc->DisableObject(fSymbol);
      cc->DisableObject(fInfo);
      fCanvas = cc;
   }

   fNucleusBoxList.Execute("SetDrawMode", Form("%d", DrawSame));
   fNucleusBoxList.Execute("EnableToolTip", "");
   fNucleusBoxList.Execute("Draw", "");

   ShowMagicNumbers();

   return;
}

void KVNuclearChart::ShowNucleusInfo(KVNucleus* nuc)
{
   if ((fShownNucleus) && (fShownNucleus == nuc)) {
      fSymbol->Clear();
      fInfo->Clear();
      fPad->GetListOfPrimitives()->Remove(fSymbol);
      fPad->GetListOfPrimitives()->Remove(fInfo);
      update_pad();
      fShownNucleus = 0;
      return;
   }

   fShownNucleus = nuc;
   fInfo->Clear();
   fInfo->AddText(Form("Z = %d  N = %d", nuc->GetZ(), nuc->GetN()));
   if (nuc->GetLifeTime() > 1.e+30) fInfo->AddText("T_{1/2} > 1.00e+30 s");
   else fInfo->AddText(Form("T_{1/2} = %3.2e s", nuc->GetLifeTime()));
   if (!fOwnHisto && fHisto) {
      TAxis* ax = fHisto->GetXaxis();
      Int_t xmin = ax->FindBin(nuc->GetN() - 0.5);
      Int_t xmax = ax->FindBin(nuc->GetN() + 0.5);
      ax = fHisto->GetYaxis();
      Int_t ymin = ax->FindBin(nuc->GetZ() - 0.5);
      Int_t ymax = ax->FindBin(nuc->GetZ() + 0.5);
      Int_t M = fHisto->Integral(xmin, xmax, ymin, ymax);
      fInfo->AddText(Form("M = %d", M));
   }
   fPad->GetListOfPrimitives()->Remove(fInfo);
   fInfo->Draw("same");

   TString symbText = nuc->GetSymbol();
   symbText.ReplaceAll(Form("%d", nuc->GetA()), Form("^{%d}", nuc->GetA()));
//  symbText += Form("_{%d}", nuc->GetN());

   if ((!fSymbol) || (!fInfo)) return;

   fSymbol->Clear();
   fSymbol->AddText(symbText.Data());
   fPad->GetListOfPrimitives()->Remove(fSymbol);
   fSymbol->SetFillColor(kBlack);
   fSymbol->SetLineColor(kBlack);
   fSymbol->SetTextColor(kWhite);
   fSymbol->Draw("same");
   update_pad();

}

void KVNuclearChart::ShowLevelScheme(const char* decays)
{
   KVLevelScheme* lvs = new KVLevelScheme(fCurrentNuc->GetSymbol());
   lvs->Draw();

   KVString dec = decays;
   dec.Begin(",");
   while (!dec.End()) lvs->DrawThreshold(dec.Next());
}

void KVNuclearChart::SetShowSymbol(Int_t value)
{
   fShowSymbol = value;
   ShowSymbol();
}

void KVNuclearChart::ShowSymbol()
{
   if (!fShowSymbol) {
      fSymbolList.Clear();
      update_pad();
      return;
   }

   KVNucleus nuc;
   for (int zz = fZmin; zz <= fZmax; zz++) {
      nuc.SetZ(zz);
      KVNumberList ll = nuc.GetKnownARange(zz, 1.e-6);
      if (ll.IsEmpty()) continue;
      Double_t xx = ll.First() - zz - (zz == 1 ? 1 : 2);
      Double_t yy = zz;

      TLatex* label = new TLatex(xx, yy, nuc.GetSymbol("EL"));
      label->SetTextAlign(22);
      label->SetTextSize(0.025);
      label->SetTextFont(42);

      TLatex* last = (TLatex*)fSymbolList.Last();
      if (last && (last->GetX() > label->GetX())) last->SetX(label->GetX());

      fSymbolList.AddLast(label);
   }

   TObject* obj = 0;
   TIter it(&fSymbolList);
   while ((obj = it())) obj->Draw();

   update_pad();
}

void KVNuclearChart::ShowBoxSymbols(Bool_t on)
{
   // draw name of isotope in each box

   TIter it(&fNucleusBoxList);
   KVNucleusBox* box;
   while ((box = (KVNucleusBox*)it())) box->SetShowSymbol(on);
   update_pad();
}

void KVNuclearChart::SetBoxSymbolSize(Float_t size)
{
   TIter it(&fNucleusBoxList);
   KVNucleusBox* box;
   while ((box = (KVNucleusBox*)it())) box->SetSymbolSize(size);
   update_pad();
}

void KVNuclearChart::SetShowMagicNumbers(Int_t value)
{
   fShowMagicNumbers = value;
   ShowMagicNumbers();
}

void KVNuclearChart::ShowMagicNumbers()
{
   if (!fShowMagicNumbers) {
      fMagicList.Clear();
      update_pad();
      return;
   }

   for (int i = 0; i < 7; i++) {
      Int_t num = fNm[i];
      if ((num >= fNmin + 1) && (num <= fNmax - 1) && (fZmMin[i] < fZmax)) {
         TLine* ll1 = new TLine(num - 0.5, TMath::Max(fZmin * 1., fZmMin[i]), num - 0.5, TMath::Min(fZmax * 1., fZmMax[i]));
         TLine* ll2 = new TLine(num + 0.5, TMath::Max(fZmin * 1., fZmMin[i]), num + 0.5, TMath::Min(fZmax * 1., fZmMax[i]));
         fMagicList.AddLast(ll1);
         fMagicList.AddLast(ll2);
         if (num >= 20) {
            TLatex* label = new TLatex(num * 1., fZmMin[i] - 1.5, Form("N=%d", num));
            label->SetTextAlign(22);
            label->SetTextSize(0.025);
            label->SetTextFont(42);
            fMagicList.AddLast(label);
         }
      }
   }

   for (int i = 0; i < 7; i++) {
      Int_t num = fNm[i];
      if ((num >= fZmin + 1) && (num <= fZmax - 1) && (fNmMin[i] < fNmax)) {
         TLine* ll1 = new TLine(TMath::Max(fNmin * 1., fNmMin[i]), num - 0.5, TMath::Min(fNmax * 1., fNmMax[i]), num - 0.5);
         TLine* ll2 = new TLine(TMath::Max(fNmin * 1., fNmMin[i]), num + 0.5, TMath::Min(fNmax * 1., fNmMax[i]), num + 0.5);
         fMagicList.AddLast(ll1);
         fMagicList.AddLast(ll2);
         if (num >= 20) {
            TLatex* label = new TLatex(fNmMin[i] - 1.5, num * 1., Form("Z=%d", num));
            label->SetTextAlign(32);
            label->SetTextSize(0.025);
            label->SetTextFont(42);
            fMagicList.AddLast(label);
         }
      }
   }

   fMagicList.Execute("Draw", "");

}







