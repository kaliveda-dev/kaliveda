//Created by KVClassFactory on Sat Nov 24 11:07:04 2012
//Author: gruyer,,,

#ifndef __KVNUCLEUSBOX_H
#define __KVNUCLEUSBOX_H

#include "TBox.h"
#include "KVNucleus.h"
#include "KVNuclearChart.h"
#include "TLatex.h"

/**
\class KVNucleusBox
\brief A TBox representing a nucleus in a KVNuclearChart
\ingroup NucEvents
*/

class KVNucleusBox : public TBox {
protected:
   Int_t fZ, fN, fA;
   Bool_t fDrawSame;
   Bool_t fOwnNucleus;
   Bool_t fShowSymbol;
   KVNucleus* fNucleus;
   KVNuclearChart* fNuclearChart;
   TLatex fSymbol;

public:
   KVNucleusBox(Int_t Z, Int_t N, Double_t size = 0.4, Int_t colStable = kBlack, Int_t colRadio = kGray + 1);
   KVNucleusBox(KVNucleus* nuc, Double_t size = 0.4, Bool_t owner = kTRUE);

   KVNucleusBox(const KVNucleusBox&) ;
   virtual ~KVNucleusBox();
   void Copy(TObject&) const;

   void ExecuteEvent(Int_t event, Int_t px, Int_t py);
   void SetDrawMode(Bool_t DrawSame);
   void SetNuclearChart(KVNuclearChart* nuclearchart)
   {
      fNuclearChart = nuclearchart;
   }
   void EnableToolTip();

   Int_t GetShowSymbol()
   {
      return fShowSymbol;
   }
   void SetShowSymbol(Bool_t value = kTRUE); // *TOGGLE*
   void SetSymbolSize(Float_t size = 0.02)
   {
      fSymbol.SetTextSize(size);
   }

   virtual void        Delete(Option_t* option = "")
   {
      TBox::Delete(option);
   }
   virtual void        DrawClass() const
   {
      TBox::DrawClass();
   }
   virtual TObject*    DrawClone(Option_t* option = "") const
   {
      return TBox::DrawClone(option);
   }
   virtual void        Dump() const
   {
      TBox::Dump();
   }

   virtual void        Inspect() const
   {
      TBox::Inspect();
   }
   virtual void        SaveAs(const char* filename = "", Option_t* option = "") const
   {
      TBox::SaveAs(filename, option);
   }
   virtual void        SetDrawOption(Option_t* option = "")
   {
      TBox::SetDrawOption(option);
   }

   virtual void        SetLineAttributes()
   {
      TBox::SetLineAttributes();
   }
   virtual void        SetFillAttributes()
   {
      TBox::SetFillAttributes();
   }
   void Paint(Option_t* option = "");


   ClassDef(KVNucleusBox, 1) //A TBox representing a nucleus in a KVNuclearChart
};

#endif
