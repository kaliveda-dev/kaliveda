/*
$Id: KVTGIDFitter.h,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Apr 21 09:26:24 2008
//Author: franklan

#ifndef __KVTGIDFITTER_H
#define __KVTGIDFITTER_H

#include "KVVirtualIDFitter.h"
#include "KVTGID.h"

class KVTGIDFitter : public KVVirtualIDFitter {
protected:
   Int_t       fType;         //type of functional
   Int_t       fLight;        //handles CsI total light-energy conversion
   Int_t       fZorA;         //is grid for Z&A or only Z identification ?
   Float_t*     fPar;         //array of fit parameters
   Float_t*     zd;           //array of Z's
   Float_t*     ad;           //array of A's
   Float_t*     xd;           //array of X coordinates
   Float_t*     yd;           //array of Y coordinates
   Int_t       irc;           //status code
   Int_t*       istate;       //status of parameters
   KVTGID*      fTGID;        //KVTGID object representing fit result
   Double_t    fXmin;         //limit for fit
   Double_t    fXmax;         //limit for fit

   void MakeTGID();

public:
   KVTGIDFitter();
   virtual ~KVTGIDFitter();

   virtual void Fit(KVIDGraph*);

   void FitPanel(Int_t functional_type = 1, Bool_t with_csi_light_energy = kTRUE,
                 Int_t first_Z = -1, Int_t last_Z = -1, Double_t xmin = 0.0, Double_t xmax = 0.0); //  *MENU*

   void SetType(Int_t t)
   {
      // Set type of functional to use:
      // t=0->basic functional       t<>0->extended functional
      // * For the basic formula :
      //   yy = ((g*E)**(mu+1)+lambda**(mu+1)*Z**2*A**mu)**(1/(mu+1))-g*E + pdy
      // * For the extended formula :
      //   yy = ((g*E)**(mu+nu+1)+(lambda*Z**alpha*A**beta)**(mu+nu+1)+
      //            xi*A**mu*(g*E)**nu)**(1/(mu+mu+1))-g*E + pdy
      fType = t;
   };
   Int_t GetType() const
   {
      return fType;
   };
   void SetLight(Int_t ih)
   {
      // Set whether to calculate non-linear light response or not
      // (if X-coordinate is CsI total light response).
      // *  If ih=0  no non-linear light response : E=xx-pdx
      // *  If ih<>0 non-linear light response included :
      //     E = sqrt(h**2+2*rho*h*(1+log(1+h/rho)))
      //    rho=eta*Z**2*A    and   h=xx-pdx
      fLight = ih;
   };
   Int_t GetLight() const
   {
      return fLight;
   };
   Int_t GetFitStatus() const;
   const Char_t* GetFitStatusString() const;

   Float_t* GetParameters() const
   {
      return fPar;
   };
   Int_t* GetStatusOfParameters() const;

   KVTGID* GetTGID() const
   {
      return fTGID;
   };

   ClassDef(KVTGIDFitter, 0) //For fitting ID grids with LTG functionals
};

#endif
