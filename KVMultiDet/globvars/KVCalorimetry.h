/*
$Id: KVCalorimetry.h,v 1.3 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.3 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Mon Apr 14 15:01:51 2008
//Author: eric bonnet,,,

#ifndef __KVCALORIMETRY_H
#define __KVCALORIMETRY_H

#include "KVCaloBase.h"

class KVCalorimetry: public KVCaloBase {

protected:

   Bool_t kfree_neutrons_included;  // = kTRUE -> Estimation des neutrons libre est faite
   Bool_t kchargediff;  // = kTRUE -> distinction entre particule et fragment
   Bool_t ktempdeduced; // = kTRUE -> calcul de la temperature

   void SumUp();

   void SetFragmentMinimumCharge(Double_t value);
   void SetParticleFactor(Double_t value);
   void SetLevelDensityParameter(Double_t value);
   void SetAsurZ(Double_t value);
   void SetNeutronMeanEnergyFactor(Double_t value);
   void ComputeTemperature();

   void init_KVCalorimetry();

public:

   KVCalorimetry(void);
   KVCalorimetry(Char_t* nom);

   virtual ~KVCalorimetry(void);

   void UseChargeDiff(Int_t FragmentMinimumCharge, Double_t ParticleFactor);
   void DeduceTemperature(Double_t LevelDensityParameter);
   void IncludeFreeNeutrons(Double_t AsurZ, Double_t NeutronMeanEnergyFactor, Double_t LevelDensityParameter);

   void  Fill(KVNucleus*);
   Bool_t Calculate(void);

   ClassDef(KVCalorimetry, 1) //compute calorimetry

};


#endif
