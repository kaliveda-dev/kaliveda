/*
$Id: KVPulseHeightDefect.h,v 1.2 2008/01/24 10:09:38 franklan Exp $
$Revision: 1.2 $
$Date: 2008/01/24 10:09:38 $
*/

//Created by KVClassFactory on Mon Feb 19 17:32:55 2007
//Author: franklan

#ifndef __KVPULSEHEIGHTDEFECT_H
#define __KVPULSEHEIGHTDEFECT_H

#include <KVCalibrator.h>


class KVPulseHeightDefect : public KVCalibrator {
   TF1* fMoulton;           //!Moulton formula for PHD =  f(E,Z)
   TF1* fDeltaEphd;           //!deltaE calculated including PHD
   Int_t fZ;                   //!Z of nucleus to be calibrated
   Bool_t wrong;//!
public:

   void init();

   KVPulseHeightDefect();
   KVPulseHeightDefect(KVDetector*);
   virtual ~KVPulseHeightDefect();

   virtual Double_t Compute(Double_t, const KVNameValueList& = "") const;
   virtual Double_t Invert(Double_t, const KVNameValueList& = "") const;

   void SetZ(Int_t z)
   {
      fZ = z;
   };
   Int_t GetZ() const
   {
      return fZ;
   };
   Double_t PHDMoulton(Double_t* x, Double_t* par);
   TF1* GetMoultonPHDFunction(Int_t Z);
   Double_t ELossActive(Double_t* x, Double_t* par);
   TF1* GetELossFunction(Int_t Z, Int_t A, Bool_t Wrong = kFALSE);

   ClassDef(KVPulseHeightDefect, 1) //Silicon PHD described by Moulton formula
};

#endif
