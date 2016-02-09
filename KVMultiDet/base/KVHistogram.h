//Created by KVClassFactory on Mon Feb  3 11:02:18 2014
//Author: John Frankland,,,

#ifndef __KVHISTOGRAM_H
#define __KVHISTOGRAM_H

#include "KVBase.h"
#include "TH1.h"
#include "TCutG.h"
#include "KVNameValueList.h"

class KVHistogram : public KVBase {
protected:
   TH1* fHisto;//pointer to histogram
   TCutG* fCut;//pointer to cut
   KVNameValueList fParams;//histogram parameters

   Double_t GetMean(Int_t axis = 1) const;

public:
   KVHistogram(TH1* h = nullptr);
   KVHistogram(TCutG* cut);
   virtual ~KVHistogram();

   static void ParseHistoTitle(const Char_t* title, KVString& exp, KVString& sel, KVString& weight);
   static void ParseExpressionString(const Char_t* exp, KVString& varX, KVString& varY,
                                     KVString& varZ);

   TH1* GetHisto() const
   {
      return fHisto;
   }
   TCutG* GetCut() const
   {
      return fCut;
   }
   const Char_t* GetExpression() const;
   const Char_t* GetHistoTitle() const;
   const Char_t* GetVarX() const;
   const Char_t* GetVarY() const;
   const Char_t* GetVarZ() const;
   const Char_t* GetSelection() const;
   const Char_t* GetWeight() const;
   Double_t GetMeanX() const
   {
      return GetMean();
   }
   Double_t GetMeanY() const
   {
      return GetMean(2);
   }
   const Char_t* GetMeanRMSX() const;
   const Char_t* GetMeanRMSY() const;

   void SetWeight(const Char_t*);

   void ls(Option_t* option = "") const;

   TObject* GetObject() const;
   void ParseExpressionAndSelection();

   ClassDef(KVHistogram, 1) //Wrapper for histograms used by KVTreeAnalyzer
};

#endif
