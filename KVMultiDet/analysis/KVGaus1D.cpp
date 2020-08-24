//Created by KVClassFactory on Tue Feb  7 16:09:40 2012
//Author: bonnet

#include "KVGaus1D.h"
#include "TFrame.h"

ClassImp(KVGaus1D)

KVGaus1D::KVGaus1D(Bool_t batch) : KVAutoFit(batch)
{
   // Default constructor
   SetName("Gaus1D");
}

//________________________________________________________________
KVGaus1D::KVGaus1D(const KVGaus1D&)  : KVAutoFit()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

}

//________________________________________________________________
KVGaus1D::~KVGaus1D()
{
   // Destructor
}


//________________________________________________________________
Bool_t KVGaus1D::NewFunction_1D()
{

   Int_t nf = lfunc->GetEntries();

   f1Dfit = new TF1(Form("func_%d", ++nf), "gaus", Xmin, Xmax);
   lfunc->Add(f1Dfit);

   hclone->GetXaxis()->SetRangeUser(Xmin, Xmax);
   f1Dfit->SetParameters(
      hclone->Integral(),
      hclone->GetMean(),
      hclone->GetRMS()
   );
   return kTRUE;

}

//________________________________________________________________
void KVGaus1D::Gather()
{

   Int_t event = gPad->GetEvent();
   TObject* select = gPad->GetSelected();
   if (!select) return;

   if (event == kButton1Down) {
      if (!strcmp("Gather", select->GetTitle())) {
         Info("", "On fit tous les pics gaussiens");

         Double_t xdeb = c1->GetFrame()->GetX1();
         Double_t xfin = c1->GetFrame()->GetX2();
         Int_t nfin = 0;
         TF1* f1;
         TIter it(lfunc);
         while ((f1 = (TF1*)it.Next())) {
            if (xdeb <= f1->GetParameter(1) && f1->GetParameter(1) <= xfin) {
               nfin += 1;
            }
         }
         TString expr = "";
         TString ff = "";
         for (Int_t ii = 0; ii < nfin; ii += 1) {
            ff.Form("gaus(%d)", ii * 3);
            expr += ff;
            if (ii < nfin - 1)
               expr += "+";
         }

         TF1* fsum = new TF1("sum", expr.Data(), xdeb, xfin);
         Int_t np = 0;
         it.Reset();
         while ((f1 = (TF1*)it.Next())) {
            for (Int_t ii = 0; ii < f1->GetNpar(); ii += 1) {
               fsum->SetParameter(np++, f1->GetParameter(ii));
            }
         }
         hfit->Fit(fsum, "0N", "", xdeb, xfin);
         fsum->SetLineColor(4);
         fsum->SetLineWidth(2);
         fsum->Draw("same");

         lfunc->Add(fsum);
         c1->Update();
      }
   }

}
