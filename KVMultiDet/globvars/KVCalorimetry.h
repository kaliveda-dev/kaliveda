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

/**
   \class KVCalorimetry
\brief Improved calorimetry of hot nuclei
\ingroup GlobalVariables

 This is an extension of the KVCaloBase basic calorimetry, with several operating modes.

 ## Two filling modes

 1. Normal mode (default) - as for KVCaloBase
 2. Mode distinguishing light and heavy particles depending on Z.
    Activated by calling UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor)
     FragmentMinimumCharge is discriminating parameter :
       - KVNucleus::GetZ()<FragmentMinimumCharge -> "light particles"
       - KVNucleus::GetZ()>=FragmentMinimumCharge -> "fragments"
     the ParticleFactor parameter is used in the calculation of Zsum, Asum, Eksum, Qsum.

    In method SumUp we have

~~~~~~~~~~~~~~~~~~~~
 Eksum = \Sigma Ek(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*\Sigma Ek(Z<[FragmentMinimumCharge])
~~~~~~~~~~~~~~~~~~~~

    and the different contributions are stored in the ingredients list:

~~~~~~~~~~~~~~~~~~~~
       root [12] ca.Print("ing");    //Example output
          Ingredients, 17 stored:
          0 | Zpart | 3.000000000
          1 | Apart | 7.000000000
          2 | Ekpart | 10.0000000
          3 | Qpart | 17.37470000
          4 | Mpart | 2.000000000
          5 | Zfrag | 7.000000000
          6 | Afrag | 16.00000000
          7 | Ekfrag | 40.0000000
          8 | Qfrag | 5.683700000
          9 | Mfrag | 1.000000000
          10 | Zsum | 13.00000000
          11 | Asum | 30.00000000
          12 | Eksum | 60.0000000
          13 | Qsum | 40.43310000
          14 | Msum | 5.000000000
          15 | Qini | -15.8724000
          16 | Exci | 116.3055000
       root [13] ca.Print("par");
          Parameters, 2 stored:
          0 | FragmentMinimumCharge | 5.000
          1 | ParticleFactor | 2.0000
~~~~~~~~~~~~~~~~~~~~


 ## Two calculation modes

 1. Normal mode (default) - as KVCaloBase
 2. Inclusion of free neutrons
    Activated by calling

~~~~~~~~~~~~~~~~~~~~{.cpp}
   IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);

    Mn =  [AsurZ]*Zsum - Asum  (methode SumUp)
    Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0   (methode Calculate)
    Exci = Asum/[LevelDensityParameter] * T*T
~~~~~~~~~~~~~~~~~~~~

 __N.B.__ : If calculated neutron multiplicity Mneu<0, we set Mneu=0 and add a parameter 'Aexcess' = TMath::Abs(Mneu)
  In this case calorimetry is performed without considering free neutrons.

    All informations are stored in ingredients list:

~~~~~~~~~~~~~~~~~~~~
       root [31] ca.Print("ing");       //Example
          Ingredients, 13 stored:
          0 | Zsum | 10.0000000000
          1 | Asum | 25.0000000000
          2 | Eksum | 62.913404145
          3 | Qsum | 39.2010000000
          4 | Msum | 5.00000000000
          5 | Aneu | 2.00000000000
          6 | Qneu | 16.1426000000
          7 | Mneu | 2.00000000000
          8 | Qini | -2.1081000000
          9 | Temp | 6.45670207291 (MeV)
          10 | Exci | 104.22250414 (MeV)
          11 | Ekneu | 12.91340414 (MeV)
       root [32] ca.Print("par");
          Parameters, 3 stored:
          0 | AsurZ | 2.50000000
          1 | NeutronMeanEnergyFactor | 1.00
          2 | LevelDensityParameter | 10.000
~~~~~~~~~~~~~~~~~~~~

 ### Temperature
 When free neutrons are considered, the temperature is calculated as part of the method.
 If not, you can call method DeduceTemperature(Double_t LevelDensityParameter) which gives the temperature
 according to the Fermi gas formula:

~~~~~~~~~~~~~~~~~~~~{.cpp}
 T = TMath::Sqrt(Exci * [LevelDensityParameter]/Asum)
~~~~~~~~~~~~~~~~~~~~

 ## Summary
    You __must__ call Calculate() before using any variables calculated by this class
    This method returns kTRUE if all went well.
    Methods:

~~~~~~~~~~~~~~~~~~~~{.cpp}
       void UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor);
       void DeduceTemperature(Double_t LevelDensityParameter);
       void IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);
~~~~~~~~~~~~~~~~~~~~

    __must__ be called before Fill()-ing the variable

*/

class KVCalorimetry: public KVCaloBase {

protected:

   Bool_t kfree_neutrons_included;  // = kTRUE -> Estimation des neutrons libre est faite
   Bool_t kchargediff;  // = kTRUE -> distinction entre particule et fragment
   Bool_t ktempdeduced; // = kTRUE -> calcul de la temperature

   void SumUp();

   void SetFragmentMinimumCharge(Double_t value);
   void SetParticleFactor(Double_t value);
   void SetLevelDensityParameter(Double_t value);
   void SetNeutronMeanEnergyFactor(Double_t value);
   void ComputeTemperature() const;

   void init_KVCalorimetry();
   void  fill(const KVNucleus*);

public:

   KVCalorimetry(void);
   KVCalorimetry(const Char_t* nom);

   virtual ~KVCalorimetry(void);

   void UseChargeDiff(Int_t FragmentMinimumCharge, Double_t ParticleFactor);
   void DeduceTemperature(Double_t LevelDensityParameter);
   void IncludeFreeNeutrons(Double_t AsurZ, Double_t NeutronMeanEnergyFactor, Double_t LevelDensityParameter);
   void SetAsurZ(Double_t value);

   void Init();
   void Calculate();

   ClassDef(KVCalorimetry, 1) //compute calorimetry

};


#endif
