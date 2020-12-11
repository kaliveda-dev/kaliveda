/*
$Id: KVINDRAUpDater_e416a.h,v 1.1 2007/02/14 14:12:00 franklan Exp $
$Revision: 1.1 $
$Date: 2007/02/14 14:12:00 $
*/

//Created by KVClassFactory on Wed Feb 14 12:54:25 2007
//Author: franklan

#ifndef __KVINDRAUPDATER_E416A_H
#define __KVINDRAUPDATER_E416A_H

#include <KVINDRAUpDater.h>

/**
\class KVINDRAUpDater_e416a
\brief Sets run parameters for INDRA_e416a dataset
\ingroup INDRADB
*/

class KVINDRAUpDater_e416a : public KVINDRAUpDater {
public:

   KVINDRAUpDater_e416a();
   virtual ~KVINDRAUpDater_e416a();

   virtual void SetChIoPressures(KVDBRun*);

   ClassDef(KVINDRAUpDater_e416a, 1) //Sets run parameters for INDRA_e416a dataset
};

#endif
