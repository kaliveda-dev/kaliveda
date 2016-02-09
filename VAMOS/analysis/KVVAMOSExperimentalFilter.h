//Created by KVClassFactory on Wed Jun 17 17:04:27 2015
//Author: Dijon Aurore

#ifndef __KVVAMOSEXPERIMENTALFILTER_H
#define __KVVAMOSEXPERIMENTALFILTER_H

#include "TProfile2D.h"
#include "KVNumberList.h"

class KVVAMOSExperimentalFilter : public TProfile2D {

   Int_t fCurBin;
   Int_t fCurBinDelta;
   Int_t fCurBinThetaV;

   KVNumberList fRunList;

public:

   KVVAMOSExperimentalFilter();

   KVVAMOSExperimentalFilter(Int_t nbins_delta, Double_t delta_low, Double_t delta_up, Int_t nbins_thetav, Double_t thetav_low, Double_t thetav_up, Double_t phiv_low, Double_t phiv_up);

   KVVAMOSExperimentalFilter(Int_t nbins_delta, Double_t delta_low, Double_t delta_up, Int_t nbins_thetav, Double_t thetav_low, Double_t thetav_up);

   virtual ~KVVAMOSExperimentalFilter();
   void Copy(TObject& obj) const;

   Bool_t IsTrajectoryAccepted(Double_t delta, Double_t thetav, Double_t phiv);


   Double_t Delta()   const
   {
      return GetXaxis()->GetBinCenter(fCurBinDelta);
   }
   Double_t DDelta()  const
   {
      return GetXaxis()->GetBinWidth(fCurBinDelta);
   }
   Double_t ThetaV()  const
   {
      return GetYaxis()->GetBinCenter(fCurBinThetaV);
   }
   Double_t DThetaV() const
   {
      return GetYaxis()->GetBinWidth(fCurBinThetaV);
   }
   Double_t PhiV()    const
   {
      return GetBinContent(fCurBin);
   }
   Double_t DPhiV()   const
   {
      return 2 * GetBinError(fCurBin);
   }

   void SetRunList(KVNumberList& runlist)
   {
      fRunList = runlist;
   }
   const Char_t* GetRunList()
   {
      return fRunList.GetList();
   }


   ClassDef(KVVAMOSExperimentalFilter, 1) //VAMOS filter created from experimental data
};

#endif
