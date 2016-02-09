/*
$Id: KVChIo_e475s.h,v 1.9 2009/05/22 14:54:47 ebonnet Exp $
$Revision: 1.9 $
$Date: 2009/05/22 14:54:47 $
*/

//Created by KVClassFactory on Thu Sep 20 09:46:32 2007
//Author: bonnet

#ifndef __KVCHIO_E475S_H
#define __KVCHIO_E475S_H

#include "KVChIo.h"
#include "KVFunctionCal.h"
#include "KVDBParameterSet.h"
#include "KVString.h"

class KVChIo_e475s : public KVChIo {
protected:

   KVFunctionCal* fcalibPG;//!channel-energy function conversion (PG)
   KVFunctionCal* fcalibGG;//!channel-energy function conversion (GG)

public:

   KVChIo_e475s();
   KVChIo_e475s(Float_t pressure, Float_t thick = 50.0 * KVUnits::mm);
   virtual ~KVChIo_e475s() {};

   virtual void SetCalibrators() {};
   void SetCalibrator(KVDBParameterSet* kvdbps);
   void ChangeCalibParameters(KVDBParameterSet* kvdbps);

   Double_t GetOriginalValue(Float_t to, TString signal);
   Double_t GetCalibratedValue(Float_t from, TString signal);

   KVFunctionCal* GetCalibratorBySignal(TString signal) const;
   virtual Bool_t IsCalibrated() const;
   Bool_t   IsCalibratedBySignal(TString signal) const;

   Double_t GetCalibratedEnergy();
   virtual Double_t  GetEnergy();
   virtual Short_t GetCalcACQParam(KVACQParam*, Double_t) const;

   ClassDef(KVChIo_e475s, 1) //derivation of KVChIo class for E475s experiment
};


#endif
