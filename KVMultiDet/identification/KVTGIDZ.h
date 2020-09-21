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

/**
   \class KVTGIDZ
\brief Abstract base class for charged particle Z identfication using functionals developed by L. Tassan-Got (IPN Orsay)
\ingroup Identfication

A KVIDZGrid identification grid can be generated from the functional in order
to visualise the corresponding identification lines.

The following methods are defined in this class:
~~~~{.cpp}
      SetIdent() -- sets the Z for each KVIDZLine
      AddLine()  -- adds a KVIDZLine to the grid
      NewGrid()  -- generates a new KVIDZGrid
~~~~
*/

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
