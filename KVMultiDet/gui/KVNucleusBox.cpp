//Created by KVClassFactory on Sat Nov 24 11:07:04 2012
//Author: gruyer,,,

#include "KVNucleusBox.h"
#include "TROOT.h"
#include "TVirtualPad.h"
#include "TVirtualX.h"
#include "TClass.h"
#include "TMath.h"
#include "TContextMenu.h"


ClassImp(KVNucleusBox)

KVNucleusBox::KVNucleusBox(Int_t Z, Int_t N, Double_t size, Int_t colStable, Int_t colRadio): TBox(N - size, Z - size, N + size, Z + size)
{
   // Constructor with Z,N of a Nucleus

   fZ = Z;
   fN = N;
   fA = fN + fZ;

   fNucleus = new KVNucleus;
   fNucleus->SetZandA(fZ, fA);
   fOwnNucleus = kTRUE;
   fSymbol.SetText(fN, fZ, fNucleus->GetLatexSymbol());
   fSymbol.SetTextAlign(22);
   fSymbol.SetTextSize(.02);

   if (fNucleus->IsStable()) {
      SetLineColor(kBlack);
      SetLineWidth(2);
      SetFillColor(colStable);
   }
   else if (fNucleus->GetLifeTime() > 1.e-06) {
      SetLineColor(kBlack);
      SetFillColor(colRadio);
   }
   else {
      SetLineColor(kBlack);
      SetFillColor(colRadio);
   }
   //  SetToolTipText(Form("%s (Z=%d,N=%d)",fNucleus->GetSymbol(),fZ,fN),250);

   fShowSymbol = kFALSE;

}

void KVNucleusBox::EnableToolTip()
{
   SetToolTipText(Form("%s (Z=%d,N=%d)", fNucleus->GetSymbol(), fZ, fN), 250);
   return;
}


KVNucleusBox::KVNucleusBox(KVNucleus* nuc, Double_t size, Bool_t owner): TBox(nuc->GetN() - size, nuc->GetZ() - size, nuc->GetN() + size, nuc->GetZ() + size)
{
   // Constructor with Z,N of a Nucleus

   fNucleus = nuc;
   fOwnNucleus = owner;
   fDrawSame = kFALSE;

   fZ = fNucleus->GetZ();
   fN = fNucleus->GetN();
   fA = fN + fZ;
   fSymbol.SetText(fN, fZ, fNucleus->GetLatexSymbol());
   fSymbol.SetTextAlign(22);
   fSymbol.SetTextSize(.02);

   if (fNucleus->IsStable()) {
      SetLineColor(kBlack);
      SetLineWidth(2);
      SetFillColor(kBlack);
   }
   else if (fNucleus->GetLifeTime() > 1.e-06) {
      SetLineColor(kBlack);
      SetFillColor(kGray + 1);
   }
   else {
      SetLineColor(kGray);
      SetFillColor(kGray);
   }
   SetToolTipText(Form("%s (Z=%d,N=%d)", fNucleus->GetSymbol(), fZ, fN), 250);
   fShowSymbol = kFALSE;

}

//________________________________________________________________
void KVNucleusBox::SetDrawMode(Bool_t DrawSame)
{
   fDrawSame = DrawSame;
   if (fDrawSame) SetFillStyle(0);
   else SetFillStyle(1);
}


//________________________________________________________________

KVNucleusBox::KVNucleusBox(const KVNucleusBox& obj)  : TBox()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVNucleusBox::~KVNucleusBox()
{
   // Destructor
   if (fOwnNucleus) delete fNucleus;
}

//________________________________________________________________

void KVNucleusBox::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVNucleusBox::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TBox::Copy(obj);
   //KVNucleusBox& CastedObj = (KVNucleusBox&)obj;
}

void KVNucleusBox::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
   if (event == kButton1Double) {
      fNuclearChart->ShowNucleusInfo(fNucleus);
   }
   else if (event == kButton3Down) {
      fNuclearChart->SetCurrentNuc(fNucleus);
      KVCanvas* cc = fNuclearChart->GetCanvas();
      cc->GetContextMenu()->Popup(px, py, fNuclearChart, cc, cc->cd());
      return;
   }
   TBox::ExecuteEvent(event, px, py);
}

void KVNucleusBox::SetShowSymbol(Bool_t i)
{
   fShowSymbol = i;
}

void KVNucleusBox::Paint(Option_t* option)
{
   TBox::Paint(option);
   if (fShowSymbol) fSymbol.Paint(option);
}


