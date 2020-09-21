/*
$Id: KVIDMap.h,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Mon Apr 14 16:54:49 2008
//Author: franklan

#ifndef __KVIDMAP_H
#define __KVIDMAP_H

#include "KVIDGraph.h"

/**
\class KVIDMap
\brief Collection of graphical contours used for particle identification
\ingroup Identification
*/

class KVIDMap : public KVIDGraph {

public:
   KVIDMap();
   virtual ~KVIDMap();


   ClassDef(KVIDMap, 1) //Collection of graphical contours used for particle identification
};

#endif
