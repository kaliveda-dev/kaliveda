/***************************************************************************
                          KVTGIDZ.h  -  description
                             -------------------
    begin                : 5 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGIDZ.h,v 1.5 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/

#ifndef KVTGIDZ__H
#define KVTGIDZ__H

#include "KVTGID.h"

class KVTGIDZ: public KVTGID {
protected:
   virtual void SetIdent(KVIDLine*, Double_t ID);
   virtual KVIDLine* AddLine(KVIDGrid*);

public:
   KVTGIDZ()
   {
   };
   KVTGIDZ(const Char_t* name, const Char_t* function,
           Double_t xmin, Double_t xmax, Int_t npar, Int_t x_par,
           Int_t y_par);
   KVTGIDZ(const Char_t* name, Int_t npar, Int_t type, Int_t light, Int_t mass);
   KVTGIDZ(const KVTGID&);
   virtual ~ KVTGIDZ()
   {
   };

   ClassDef(KVTGIDZ, 1)         //Abstract base class for Z identification functionals by L. Tassan-Got
};

#endif
