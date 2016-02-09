//Created by KVClassFactory on Tue Jan 17 15:10:46 2012
//Author: bonnet

#include "KVDalitzPlot.h"
#include "TMath.h"
#include "TPad.h"
#include "TCanvas.h"
#include "KVCanvas.h"

ClassImp(KVDalitzPlot)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDalitzPlot</h2>
<h4>Fill 3D observables {a1,a2,a3} in a dalitz plot</h4>
, a1+a2+a3 = 1
and fill in a equilateral triangle where the distance from each side of the triangle represent the value of each componment</h4>

<!-- */
// --> END_HTML
/*
- Normalize first the sum to 1 => a1+a2+a3 = 1
- Deduce the position in an equilateral triangle as following
a1n -> distance with the bottom border   /\
a2n -> distance with the left border    /  \
a3n -> distance with the right border  /____\
*/
////////////////////////////////////////////////////////////////////////////////
KVDalitzPlot::KVDalitzPlot(const char* name, const char* title, Bool_t ordered, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup) :
   TH2F(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup)
{
   fOrdered = ordered;
   fShowBorder = 1;
   fShowCenter = 1;
   lb1 = lb2 = lb3 = 0;
   lc1 = lc2 = lc3 = 0;
}


KVDalitzPlot::KVDalitzPlot()
{
   // Default constructor
}

//________________________________________________________________

KVDalitzPlot::KVDalitzPlot(const KVDalitzPlot& obj) : TH2F()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVDalitzPlot::~KVDalitzPlot()
{
   // Destructor
}

//________________________________________________________________

void KVDalitzPlot::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVDalitzPlot::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TH2F::Copy(obj);
   //KVDalitzPlot& CastedObj = (KVDalitzPlot&)obj;
}

Int_t KVDalitzPlot::FillAsDalitz(Double_t a1, Double_t a2, Double_t a3)
{
   //Place the 3 observable in an equilateral triangle
   //after normalized the sum to 1
   //Info : the permutation is not done

   if (!fOrdered) return FillMe(a1, a2, a3);

   Int_t result = 0;
   result += FillMe(a1, a2, a3);
   result += FillMe(a2, a3, a1);
   result += FillMe(a3, a1, a2);
   result += FillMe(a1, a3, a2);
   result += FillMe(a2, a1, a3);
   result += FillMe(a3, a2, a1);
   return result;
}

Int_t KVDalitzPlot::FillMe(Double_t a1, Double_t a2, Double_t a3)
{
   Double_t sum = a1 + a2 + a3;
   if (sum > 0) {
      Double_t a1n = a1 / sum;
      Double_t a2n = a2 / sum;

      Double_t xI = 1. / TMath::Sqrt(3) * (a1n + 2 * a2n);
      Double_t yI = a1n;

      return TH2F::Fill(xI, yI);
   }
   return -1;
}

void  KVDalitzPlot::Draw(Option_t*)
{
   GetXaxis()->SetNdivisions(506);
   GetXaxis()->SetLabelFont(42);
   GetXaxis()->SetLabelSize(0.0);
   GetXaxis()->SetTitleSize(0.035);
   GetXaxis()->SetTickLength(0);
   GetXaxis()->SetTitleFont(42);
   GetXaxis()->SetAxisColor(0);
   GetXaxis()->SetLabelColor(0);

   GetYaxis()->SetNdivisions(506);
   GetYaxis()->SetLabelFont(42);
   GetYaxis()->SetLabelSize(0.0);
   GetYaxis()->SetTitleSize(0.035);
   GetYaxis()->SetTickLength(0);
   GetYaxis()->SetTitleFont(42);
   GetYaxis()->SetAxisColor(0);
   GetYaxis()->SetLabelColor(0);

   TH2F::Draw("col");

   SetShowBorder(1);
   SetShowCenter(1);

   gPad->GetCanvas()->SetRightMargin(0.001);
   gPad->GetCanvas()->SetTopMargin(0.001);

   TCanvas* cc = gPad->GetCanvas();
   if (cc->InheritsFrom("KVCanvas"))((KVCanvas*)cc)->DisableClass("TLine");

   //   gPad->GetCanvas()->SetLeftMargin(0.001);
   //   gPad->GetCanvas()->SetBottomMargin(0.001);

   gPad->SetBorderMode(0);
   gPad->SetBorderSize(2);
   gPad->SetLogz();
   gPad->SetFrameBorderMode(0);
   gPad->SetFrameLineColor(0);
   gPad->Modified();
   gPad->Update();
}

void KVDalitzPlot::SetShowBorder(Int_t value)
{
   fShowBorder = value;

   SafeDelete(lb1);
   SafeDelete(lb2);
   SafeDelete(lb3);

   if (fShowBorder) {
      lb1 = new TLine(0.5758319, 1., 0., 0.);
      lb1->SetLineWidth(2);
      lb1->Draw("same");
      lb2 = new TLine(0.5758319, 1., 1.15, 0.);
      lb2->SetLineWidth(2);
      lb2->Draw("same");
      lb3 = new TLine(0., 0., 1.15, 0.);
      lb3->SetLineWidth(2);
      lb3->Draw("same");
   }

   gPad->SetFrameLineColor(0);
   gPad->Modified();
   gPad->Update();
}

void KVDalitzPlot::SetShowCenter(Int_t value)
{
   fShowCenter = value;

   SafeDelete(lc1);
   SafeDelete(lc2);
   SafeDelete(lc3);

   if (fShowCenter) {
      lc1 = new TLine(0.575, 1, 0.575, 0.333);
      lc1->SetLineWidth(2);
      lc1->SetLineStyle(9);
      lc1->Draw();
      lc2 = new TLine(0, 0, 0.575, 0.333);
      lc2->SetLineWidth(2);
      lc2->SetLineStyle(9);
      lc2->Draw();
      lc3 = new TLine(0.575, 0.333, 1.15, 0);
      lc3->SetLineWidth(2);
      lc3->SetLineStyle(9);
      lc3->Draw();
   }
   gPad->SetFrameLineColor(0);
   gPad->Modified();
   gPad->Update();

}



