//Created by KVClassFactory on Mon Feb  3 11:02:18 2014
//Author: John Frankland,,,

#include "KVHistogram.h"
#include <TROOT.h>

ClassImp(KVHistogram)



void KVHistogram::ParseExpressionAndSelection()
{
   if (!fHisto) return;
   KVString exp, sel, weight, x, y, z, meanrms;
   ParseHistoTitle(fHisto->GetTitle(), exp, sel, weight);
   ParseExpressionString(exp, x, y, z);
   fParams.SetValue("VARX", x);
   fParams.SetValue("VARY", y);
   fParams.SetValue("VARZ", z);
   fParams.SetValue("SELECTION", sel);
   fParams.SetValue("EXPRESSION", exp);
   fParams.SetValue("WEIGHT", weight);
   meanrms.Form("%f (%f)", fHisto->GetMean(), fHisto->GetRMS());
   fParams.SetValue("MEANRMSX", meanrms);
   meanrms.Form("%f (%f)", fHisto->GetMean(2), fHisto->GetRMS(2));
   fParams.SetValue("MEANRMSY", meanrms);
}

Double_t KVHistogram::GetMean(Int_t axis) const
{
   if (fHisto) return fHisto->GetMean(axis);
   return fCut->GetMean(axis);
}

KVHistogram::KVHistogram(TH1* h)
{
   // Default constructor
   fHisto = h;
   fCut = nullptr;
   fParams.SetValue("WEIGHT", "1");
   if (h) {
      ParseExpressionAndSelection();
      SetType("Histo");
      SetName(h->GetName());
      SetLabel(h->ClassName());
   }
}

KVHistogram::KVHistogram(TCutG* cut)
{
   // Ctor for TCutG object
   fHisto = nullptr;
   fCut = cut;
   SetType("Cut");
   fParams.SetValue("VARX", cut->GetVarX());
   fParams.SetValue("VARY", cut->GetVarY());
   fParams.SetValue("VARZ", "");
   fParams.SetValue("SELECTION", "");
   fParams.SetValue("EXPRESSION", "");
   fParams.SetValue("WEIGHT", "1");
   fParams.SetValue("MEANRMSX", -1);
   fParams.SetValue("MEANRMSY", -1);
   SetName(cut->GetName());
   SetLabel(cut->ClassName());
}

KVHistogram::~KVHistogram()
{
   // Destructor
   SafeDelete(fHisto);
}

void KVHistogram::ParseHistoTitle(const Char_t* title, KVString& exp, KVString& sel, KVString& weight)
{
   // Take histo title "VAREXP [WEIGHT] { SELECTION }"
   // and separate the three components

   exp = "";
   sel = "";
   weight = "1";
   TString tmp(title);
   Int_t ss = tmp.Index("{");
   if (ss > 0) {
      Int_t se = tmp.Index("}");
      sel = tmp(ss + 1, se - ss - 1);
      sel.Remove(TString::kBoth, ' ');
      exp = tmp(0, ss);
      exp.Remove(TString::kBoth, ' ');
   }
   else {
      exp = tmp;
      exp.Remove(TString::kBoth, ' ');
   }
   // get weight from expression string if any
   KVString tmp1(exp);
   tmp1.Begin(" ");
   exp = tmp1.Next();
   if (!tmp1.End()) {
      weight = tmp1.Next();
      weight.Remove(TString::kBoth, '[');
      weight.Remove(TString::kBoth, ']');
   }
}

void KVHistogram::ParseExpressionString(const Char_t* exp, KVString& varX, KVString& varY,
                                        KVString& varZ)
{
   // Parse expression strings "VARZ:VARY:VARX" or "VARY:VARX" or "VARX"

   KVString tmp(exp);
   tmp.Remove(TString::kBoth, ' ');
   // hide any "::" scope resolution operators (e.g. TMath::DegToRad)
   tmp.ReplaceAll("::", "_scope_");
   Int_t nvar = tmp.CountChar(':');
   tmp.Begin(":");
   varX = varY = varZ = "";
   if (nvar == 2) {
      varZ = tmp.Next();
      varZ.ReplaceAll("_scope_", "::");
   }
   if (nvar >= 1) {
      varY = tmp.Next();
      varY.ReplaceAll("_scope_", "::");
   }
   varX = tmp.Next();
   varX.ReplaceAll("_scope_", "::");
}

const Char_t* KVHistogram::GetVarX() const
{
   return fParams.GetStringValue("VARX");
}
const Char_t* KVHistogram::GetVarY() const
{
   return fParams.GetStringValue("VARY");
}
const Char_t* KVHistogram::GetVarZ() const
{
   return fParams.GetStringValue("VARZ");
}
const Char_t* KVHistogram::GetSelection() const
{
   return fParams.GetStringValue("SELECTION");
}

const Char_t* KVHistogram::GetWeight() const
{
   // Return weighting used for filling histogram
   return fParams.GetStringValue("WEIGHT");
}

const Char_t* KVHistogram::GetMeanRMSX() const
{
   return fParams.GetStringValue("MEANRMSX");
}

const Char_t* KVHistogram::GetMeanRMSY() const
{
   return fParams.GetStringValue("MEANRMSY");
}

void KVHistogram::SetWeight(const Char_t* weight)
{
   // Set weighting factor used to fill histogram
   fParams.SetValue("WEIGHT", weight);
}

void KVHistogram::ls(Option_t*) const
{
   TROOT::IndentLevel();
   TROOT::IncreaseDirLevel();
   std::cout << "KVHistogram::" << GetName() << " " << GetLabel();
//    cout << " X:" << GetVarX();
//    if(IsType("Cut")){
//        cout << " Y:" << GetVarY();
//    }
//    else
//    {
//        if(strcmp(GetVarY(),"")) cout << " Y:" << GetVarY();
//        if(strcmp(GetVarZ(),"")) cout << " Z:" << GetVarZ();
//        if(strcmp(GetSelection(),"")) cout << " [" << GetSelection() << "]";
//    }
   if (GetHisto()) std::cout << " : " << GetHisto()->GetTitle();
   else std::cout << " : " << GetCut()->GetName();
   std::cout << std::endl;
   TROOT::DecreaseDirLevel();
}

TObject* KVHistogram::GetObject() const
{
   // In order to have context menu access to the contained histogram or cut
   if (fHisto) return (TObject*)fHisto;
   return fCut;
}

const Char_t* KVHistogram::GetExpression() const
{
   return fParams.GetStringValue("EXPRESSION");
}

const Char_t* KVHistogram::GetHistoTitle() const
{
   if (fHisto) return fHisto->GetTitle();
   if (fCut) return fCut->GetName();
   return "";
}
