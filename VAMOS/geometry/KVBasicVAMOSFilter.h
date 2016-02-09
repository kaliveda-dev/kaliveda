//Created by KVClassFactory on Fri Jun 19 10:59:24 2015
//Author: Dijon Aurore

#ifndef __KVBASICVAMOSFILTER_H
#define __KVBASICVAMOSFILTER_H

#include "KVBase.h"
#include "KVList.h"
#include "KVMacros.h" // 'UNUSED' macro

class TCutG;
class KVBasicVAMOSFilter;
class KVCutList;

class KVCutList : public KVList {
public :
   Float_t fThetaVamos;
   KVCutList() {};
   virtual ~KVCutList()
   {
      fThetaVamos = 0.;
   };
   const Char_t* GetName() const
   {
      return GetCutListName(fThetaVamos);
   };
   static const Char_t* GetCutListName(Float_t th_vamos)
   {
      return Form("%.1f", th_vamos);
   }
   void SetThetaVamos(Float_t th_vamos)
   {
      fThetaVamos = th_vamos;
   }
   Float_t GetThetaVamos() const
   {
      return fThetaVamos;
   }

   ClassDef(KVCutList, 1) //
};

class KVBasicVAMOSFilter : public KVBase {

   KVList fMainList;
   KVCutList* fCutList; //!
   Int_t   fNcuts;

   KVCutList* FindCutListForThetaVamos(Float_t th_vamos);
   static const Char_t* GetCutListName(Float_t th_vamos);
   KVCutList* NewCutListForThetaVamos(Float_t th_vamos);
   Bool_t IsInThetaVrange(Double_t thetav)
   {
      return -2 <= thetav && thetav <= 2;
   }
   Bool_t IsInPhiVrange(Double_t phiv)
   {
      return -2 <= phiv && phiv <= 2;
   }
   Bool_t IsInDeltaRange(Double_t delta)
   {
      return 0.945 <= delta && delta <= 1.2;
   }


public:

   KVBasicVAMOSFilter();
   virtual ~KVBasicVAMOSFilter();
   void Copy(TObject& obj) const;

   void AddCutForThetaVamos(Float_t th_vamos, TCutG* cut);

   Bool_t IsTrajectoryAccepted(Float_t theta_vamos, Double_t delta, Double_t thetav, Double_t phiv);


   using KVBase::Print;
   void Print(Option_t* opt = "")
   {
      UNUSED(opt);
      fMainList.ls();
   }

   ClassDef(KVBasicVAMOSFilter, 1) //Basic filter for VAMOS using simple TCuts on delta vs ThetaV matrices
};

#endif
