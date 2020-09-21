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

/**
\class KVTGIDFitter
\brief Fit of E-DE functional
\author Laurent TASSAN-GOT  17/03/2001</h3>

This class interfaces the Fortran package written by Laurent Tassan-Got
to fit KVIDGrid objects using his functionals (see <a href="http://arxiv.org/abs/nucl-ex/0103004">Nucl.Instrum.Meth. B194 (2002) 503-512</a>).
The following is the documentation from Laurent's code.
<br>
<pre>
                    ******************
                    *  Automatic fit *
                    ******************

  globede  : user-friendly routine in which only the data have to be supplied.
             The routine estimates good initial values, reasonnable limits,
             automatically from the data and manages the whole fit procedure

      call globede(npts,zd,ad,xd,yd,ixt,ih,par,istate,irc)

 INPUT :
   npts : [I] number of data points really used in zd, ad, xd, yd
    zd  : [R] array of Z's (npts values)
    ad  : [R] array of A's (npts values)
    xd  : [R] array of E's (npts values)
    yd  : [R] array of DE's (npts values)
   ixt  : [I] =0->basic functional       <>0->extended functional
       * For the basic formula :
         yy = ((g*E)**(mu+1)+lambda**(mu+1)*Z**2*A**mu)**(1/(mu+1))-g*E + pdy
       * For the extended formula :
         yy = ((g*E)**(mu+nu+1)+(lambda*Z**alpha*A**beta)**(mu+nu+1)+
                  xi*Z**2*A**mu*(g*E)**nu)**(1/(mu+nu+1))-g*E + pdy

       *  If ih=0  no non-linear light response : E=xx-pdx
       *  If ih<>0 non-linear light response included :
           E = sqrt(h**2+2*rho*h*(1+log(1+h/rho)))
          rho=eta*Z**2*A    and   h=xx-pdx
    ih  : [I]  =0->no non-linear light response
              <>0->non-linear light response included
 OUTPUT :
    par : [R] array of parameters resulting from the fit, according to the
              ixt, ih combination the numbers and orders of parameters are :
              ixt=0  ih=0   5 parameters: lambda, mu, g, pdx, pdy
              ixt=0  ih<>0  6 parameters: lambda, mu, g, pdx, pdy, eta
              ixt<>0 ih=0   9 parameters: lambda, alpha, beta, mu, nu,
                                         xi, g, pdx, pdy
              ixt<>0 ih<>0 10 parameters: lambda, alpha, beta, mu, nu,
                                         xi, g, pdx, pdy, eta
  istate : [I] array of status of parameters
                 =0 -> free parameter
                 =1 -> parameter constrained by the lower limit
                 =2 -> parameter constrained by the upper limit
                 =3 -> constant parameter (bl(i)=bu(i)=par(i))
    irc  : [I] return code of the fit :
              0 -> convergence reached
              1 -> convergence reached, but not well marked minimum
              2 -> too many iterations, convergence not reached
             -1 -> no identification line with at least 2 points
             -2 -> too few data points
             -3 -> addressing problem between Fortran and C
</pre>
*/

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

   void SetFunctionalType(Int_t t)
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
   Int_t GetFunctionalType() const
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
