/*
$Id: KVDP2toIDGridConvertor.h,v 1.1 2008/10/13 14:25:08 franklan Exp $
$Revision: 1.1 $
$Date: 2008/10/13 14:25:08 $
*/

//Created by KVClassFactory on Thu Oct  9 17:21:55 2008
//Author: franklan

#ifndef __KVDP2TOIDGRIDCONVERTOR_H
#define __KVDP2TOIDGRIDCONVERTOR_H

#include "Riostream.h"
#include "KVString.h"
#include "TClass.h"

class KVIDGrid;
class TList;

/**
   \class KVDP2toIDGridConvertor
\brief Converts dp2-format grids to KVIDGrids
\ingroup Idenification

Set the name of the class (derived from KVIDGrid) using SetGridClass(),
then convert file with Convert("dp2_grids.dat"). The new grids are added
to gIDGridManager.
*/


class KVDP2toIDGridConvertor {
protected:
   TClass*      fGridClass;   // class of grid to create from dp2 file
   TList*       fGrids;       // list of new grids
   virtual void ReadLineCoords(KVIDGrid*, std::ifstream&);
   virtual void ReadFile(const Char_t* filename);

public:
   KVDP2toIDGridConvertor();
   virtual ~KVDP2toIDGridConvertor();

   virtual void SetGridClass(const Char_t* cl);
   void Convert(const Char_t* filename);

   ClassDef(KVDP2toIDGridConvertor, 1) //Converts dp2-format grids to KVIDGrids
};

#endif
