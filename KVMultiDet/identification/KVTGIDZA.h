/***************************************************************************
                          KVTGIDZA.h  -  description
                             -------------------
    begin                : 5 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGIDZA.h,v 1.8 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/

#ifndef KVTGIDZA__H
#define KVTGIDZA__H

#include "KVTGIDZ.h"

class KVTGIDZA: public KVTGIDZ {
protected:
   virtual void SetIdent(KVIDLine*, Double_t ID);
   virtual KVIDLine* AddLine(KVIDGrid*);

public:
   KVTGIDZA()
   {
   };
   KVTGIDZA(const Char_t* name, const Char_t* function,
            Double_t xmin, Double_t xmax, Int_t npar, Int_t x_par,
            Int_t y_par);
   KVTGIDZA(const Char_t* name, Int_t npar, Int_t type, Int_t light);
   virtual ~ KVTGIDZA()
   {
   };

   void AddLineToGrid(KVIDGrid* g, Int_t ID, Int_t npoints, Double_t xmin,
                      Double_t xmax, Bool_t logscale = kFALSE);

   ClassDef(KVTGIDZA, 1)        //Base class for Z & A identification functionals by L. Tassan-Got
};

#endif
