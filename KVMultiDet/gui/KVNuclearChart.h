//Created by KVClassFactory on Thu Nov 22 15:26:52 2012
//Author: dgruyer

#ifndef __KVNUCLEARCHART_H
#define __KVNUCLEARCHART_H

#include "KVBase.h"
#include "TH2.h"
#include "KVList.h"
#include "KVNucleus.h"
#include "TPaveText.h"
#include "KVCanvas.h"
#include "KVNumberList.h"

#include <TVirtualPad.h>

/**
   \class KVNuclearChart
\brief Used to draw nuclear chart
\ingroup NucEvents
*/

class KVNuclearChart : public KVBase {

protected:
   Int_t fNmin;
   Int_t fNmax;
   Int_t fZmin;
   Int_t fZmax;

   Bool_t fOwnHisto;

   Int_t fNm[7];
   Double_t fZmMin[7], fZmMax[7], fNmMin[7], fNmMax[7];

   Int_t fShowSymbol;
   Int_t fShowMagicNumbers;

   TH2* fHisto;
   KVList fNucleusBoxList;
   KVList fMagicList;
   KVList fSymbolList;
   KVNucleus* fShownNucleus;
   TPaveText* fSymbol;
   TPaveText* fInfo;
   KVCanvas* fCanvas;
   TVirtualPad* fPad;
   KVNucleus* fCurrentNuc;

   void update_pad()
   {
      if (fPad) {
         fPad->Modified();
         fPad->Update();
      }
   }

public:
//    KVNuclearChart();
   KVNuclearChart(Int_t nMin = 0, Int_t nMax = -1, Int_t zMin = 0, Int_t zMax = -1, Double_t life = 1.e-06);
   KVNuclearChart(const KVNuclearChart&) ;
   virtual ~KVNuclearChart();
   void Copy(TObject&) const;

   void Draw(Option_t* option = "");
   void ShowNucleusInfo(KVNucleus* nuc);
   void ShowLevelScheme(const char* decays); // *MENU*
   void SetCurrentNuc(KVNucleus* nuc)
   {
      fCurrentNuc = nuc;
   }

   Int_t GetShowSymbol()
   {
      return fShowSymbol;
   }
   void SetShowSymbol(Int_t value = 1); // *TOGGLE*
   void ShowSymbol();
   void ShowBoxSymbols(Bool_t on = kTRUE);
   void SetBoxSymbolSize(Float_t size = 0.02);

   Int_t GetShowMagicNumbers()
   {
      return fShowMagicNumbers;
   }
   void SetShowMagicNumbers(Int_t value = 1); // *TOGGLE*
   void ShowMagicNumbers();

   KVCanvas* GetCanvas()
   {
      return fCanvas;
   }

   virtual void        Delete(Option_t* option = "")
   {
      KVBase::Delete(option);
   }
   virtual void        DrawClass() const
   {
      KVBase::DrawClass();
   }
   virtual TObject*    DrawClone(Option_t* option = "") const
   {
      return KVBase::DrawClone(option);
   }
   virtual void        Dump() const
   {
      KVBase::Dump();
   }

   virtual void        Inspect() const
   {
      KVBase::Inspect();
   }
   virtual void        SaveAs(const char* filename = "", Option_t* option = "") const
   {
      KVBase::SaveAs(filename, option);
   }
   virtual void        SetDrawOption(Option_t* option = "")
   {
      KVBase::SetDrawOption(option);
   }

   virtual void        SetTitle(const char* title)
   {
      KVBase::SetTitle(title);
   }
   virtual void        SetName(const char* name)
   {
      KVBase::SetName(name);
   }


   ClassDef(KVNuclearChart, 1) //Used to draw nuclear chart
};

#endif
