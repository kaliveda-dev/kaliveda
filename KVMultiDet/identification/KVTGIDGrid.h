/*
$Id: KVTGIDGrid.h,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Fri Apr 25 15:55:08 2008
//Author: franklan

#ifndef __KVTGIDGRID_H
#define __KVTGIDGRID_H

#include "KVIDZAGrid.h"

class KVTGID;

class KVTGIDGrid : public KVIDZAGrid {
protected:
   KVTGID*      fTGID;     //KVTGID used to generate grid

public:
   KVTGIDGrid();
   KVTGIDGrid(KVTGID*, KVIDZAGrid* = 0);
   virtual ~KVTGIDGrid();
   void WriteToAsciiFile(std::ofstream&);
   void ReadFromAsciiFile(std::ifstream&);
   virtual void Generate(Double_t xmax, Double_t xmin,
                         Int_t ID_min = 0, Int_t ID_max = 0,
                         Int_t npoints = 50, Bool_t logscale = kTRUE);//  *MENU* *ARGS={xmax=>fXmax,xmin=>fXmin}

   const KVTGID* GetTGID() const;
   virtual void AddIdentifier(KVIDentifier* id);

   ClassDef(KVTGIDGrid, 1) //Grid representing result of fit
};

#endif
