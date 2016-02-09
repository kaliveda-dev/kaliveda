/*
$Id: KVVirtualIDFitter.h,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Apr 21 09:25:57 2008
//Author: franklan

#ifndef __KVVIRTUALIDFITTER_H
#define __KVVIRTUALIDFITTER_H

#include "KVBase.h"
#include "KVIDGrid.h"
#include "TPad.h"

class KVVirtualIDFitter : public KVBase {
   static KVVirtualIDFitter* gDefaultFitter;

protected:
   KVIDGraph*   fGrid;        //the grid to fit
   TVirtualPad* fPad;         //the pad in which grid is drawn (=0 if not drawn)

public:
   KVVirtualIDFitter();
   virtual ~KVVirtualIDFitter();

   virtual void Fit(KVIDGraph*) = 0;

   void SetGrid(KVIDGraph* g)
   {
      fGrid = g;
   };
   KVIDGraph* GetGrid() const
   {
      return fGrid;
   };
   void SetPad(TVirtualPad* p)
   {
      fPad = p;
   };
   TVirtualPad* GetPad() const
   {
      return fPad;
   };

   static KVVirtualIDFitter* GetDefaultFitter();

   ClassDef(KVVirtualIDFitter, 0) //ABC for fitting ID grids with functionals
};

#endif
