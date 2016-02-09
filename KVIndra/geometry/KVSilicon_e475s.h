/*
$Id: KVSilicon_e475s.h,v 1.10 2009/05/22 14:54:47 ebonnet Exp $
$Revision: 1.10 $
$Date: 2009/05/22 14:54:47 $
*/

//Created by KVClassFactory on Wed Sep 19 13:46:35 2007
//Author: bonnet

#ifndef __KVSILICON_E475S_H
#define __KVSILICON_E475S_H

#include "KVSilicon.h"
#include "KVFunctionCal.h"
#include "KVString.h"

class KVDBParameterSet;

class KVSilicon_e475s : public KVSilicon {

protected:

   KVFunctionCal* fcalibPG;//!channel-energy function conversion (PG)
   KVFunctionCal* fcalibGG;//!channel-energy function conversion (GG)

   void init();

public:

   KVSilicon_e475s();
   KVSilicon_e475s(Float_t thick);
   virtual ~KVSilicon_e475s() {};

   void SetCalibrators() {};
   void SetCalibrator(KVDBParameterSet* kvdbps);
   void ChangeCalibParameters(KVDBParameterSet* kvdbps);

   Double_t GetOriginalValue(Float_t to, TString signal);
   Double_t GetCalibratedValue(Float_t from, TString signal);

   KVFunctionCal* GetCalibratorBySignal(TString signal) const;
   Bool_t   IsCalibrated() const;
   Bool_t   IsCalibratedBySignal(TString signal) const;

   Double_t GetCalibratedEnergy();
   Double_t GetEnergy();

   Double_t GetCorrectedEnergy(KVNucleus*, Double_t e = -1., Bool_t transmission = kTRUE);

   Short_t GetCalcACQParam(KVACQParam*, Double_t) const;

   ClassDef(KVSilicon_e475s, 1) //derivation of KVSilicon class for E475s experiment

};


#endif
