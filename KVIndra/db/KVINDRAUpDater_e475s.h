/*
$Id: KVINDRAUpDater_e475s.h,v 1.2 2007/09/21 07:44:09 franklan Exp $
$Revision: 1.2 $
$Date: 2007/09/21 07:44:09 $
*/

//Created by KVClassFactory on Tue Sep 18 12:14:51 2007
//Author: bonnet

#ifndef __KVINDRAUPDATER_E475S_H
#define __KVINDRAUPDATER_E475S_H

#include "KVINDRAUpDater.h"
#include "KVDBRun.h"

class KVINDRAUpDater_e475s : public KVINDRAUpDater {
public:

   KVINDRAUpDater_e475s();
   virtual ~KVINDRAUpDater_e475s();


   void SetCalibrationParameters(UInt_t);
   void SetCalibParameters(KVDBRun* run);

   void SetPedestals(KVDBRun*);


   ClassDef(KVINDRAUpDater_e475s, 1) //Sets run parameters for INDRA_e475s dataset
};

#endif
