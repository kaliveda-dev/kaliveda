/*
$Id: KVCalorimetry.cpp,v 1.4 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.4 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Mon Apr 14 15:01:51 2008
//Author: eric bonnet,,,

#include "KVCalorimetry.h"
#include "KVNDTManager.h"

ClassImp(KVCalorimetry)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCalorimetry</h2>
<h4>Improved calorimetry of hot nuclei</h4>
<!-- */
// --> END_HTML
//
// This is an extension of the KVCaloBase basic calorimetry, with several operating modes.
//
// ## Two filling modes
//
// 1. Normal mode (default) - as for KVCaloBase
// 2. Mode distinguishing light and heavy particles depending on Z.
//    Activated by calling UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor)
//     FragmentMinimumCharge is discriminating parameter :
//       - KVNucleus::GetZ()<FragmentMinimumCharge -> "light particles"
//       - KVNucleus::GetZ()>=FragmentMinimumCharge -> "fragments"
//     the ParticleFactor parameter is used in the calculation of Zsum, Asum, Eksum, Qsum.
//
//    In method SumUp we have
//
//~~~~~~~~~~~~~~~~~~~~
// Eksum = \Sigma Ek(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*\Sigma Ek(Z<[FragmentMinimumCharge])
//~~~~~~~~~~~~~~~~~~~~
//
//    and the different contributions are stored in the ingredients list:
//
//~~~~~~~~~~~~~~~~~~~~
//       root [12] ca.Print("ing");    //Example output
//          Ingredients, 17 stored:
//          0 | Zpart | 3.000000000
//          1 | Apart | 7.000000000
//          2 | Ekpart | 10.0000000
//          3 | Qpart | 17.37470000
//          4 | Mpart | 2.000000000
//          5 | Zfrag | 7.000000000
//          6 | Afrag | 16.00000000
//          7 | Ekfrag | 40.0000000
//          8 | Qfrag | 5.683700000
//          9 | Mfrag | 1.000000000
//          10 | Zsum | 13.00000000
//          11 | Asum | 30.00000000
//          12 | Eksum | 60.0000000
//          13 | Qsum | 40.43310000
//          14 | Msum | 5.000000000
//          15 | Qini | -15.8724000
//          16 | Exci | 116.3055000
//       root [13] ca.Print("par");
//          Parameters, 2 stored:
//          0 | FragmentMinimumCharge | 5.000
//          1 | ParticleFactor | 2.0000
//~~~~~~~~~~~~~~~~~~~~
//
//
// ## Two calculation modes
//
// 1. Normal mode (default) - as KVCaloBase
// 2. Inclusion of free neutrons
//    Activated by calling
//
//~~~~~~~~~~~~~~~~~~~~
//   IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);
//
//    Mn =  [AsurZ]*Zsum - Asum  (methode SumUp)
//    Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0   (methode Calculate)
//    Exci = Asum/[LevelDensityParameter] * T*T
//~~~~~~~~~~~~~~~~~~~~
//
// __N.B.__ : If calculated neutron multiplicity Mneu<0, we set Mneu=0 and add a parameter 'Aexcess' = TMath::Abs(Mneu)
//  In this case calorimetry is performed without considering free neutrons.
//
//    All informations are stored in ingredients list:
//
//~~~~~~~~~~~~~~~~~~~~
//       root [31] ca.Print("ing");       //Example
//          Ingredients, 13 stored:
//          0 | Zsum | 10.0000000000
//          1 | Asum | 25.0000000000
//          2 | Eksum | 62.913404145
//          3 | Qsum | 39.2010000000
//          4 | Msum | 5.00000000000
//          5 | Aneu | 2.00000000000
//          6 | Qneu | 16.1426000000
//          7 | Mneu | 2.00000000000
//          8 | Qini | -2.1081000000
//          9 | Temp | 6.45670207291 (MeV)
//          10 | Exci | 104.22250414 (MeV)
//          11 | Ekneu | 12.91340414 (MeV)
//       root [32] ca.Print("par");
//          Parameters, 3 stored:
//          0 | AsurZ | 2.50000000
//          1 | NeutronMeanEnergyFactor | 1.00
//          2 | LevelDensityParameter | 10.000
//~~~~~~~~~~~~~~~~~~~~
//
// ### Temperature
// When free neutrons are considered, the temperature is calculated as part of the method.
// If not, you can call method DeduceTemperature(Double_t LevelDensityParameter) which gives the temperature
// according to the Fermi gas formula:
//
//~~~~~~~~~~~~~~~~~~~~
// T = TMath::Sqrt(Exci * [LevelDensityParameter]/Asum)
//~~~~~~~~~~~~~~~~~~~~
//
// ## Summary
//    You __must__ call Calculate() before using any variables calculated by this class
//    This method returns kTRUE if all went well.
//    Methods:
//
//~~~~~~~~~~~~~~~~~~~~
//       void UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor);
//       void DeduceTemperature(Double_t LevelDensityParameter);
//       void IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);
//~~~~~~~~~~~~~~~~~~~~
//
//    __must__ be called before Fill()-ing the variable
//
////////////////////////////////////////////////////////////////////////////////

KVCalorimetry::KVCalorimetry(void): KVCaloBase()
{
// Createur par default

   init_KVCalorimetry();
   SetName("KVCalorimetry");
   SetTitle("A KVCalorimetry");

}

//_________________________________________________________________
KVCalorimetry::KVCalorimetry(const Char_t* nom): KVCaloBase(nom)
{
// Constructeur avec un nom

   init_KVCalorimetry();
}

//_________________________________________________________________
KVCalorimetry::~KVCalorimetry(void)
{
// Destructeur

}

//________________________________________________________________
void KVCalorimetry::init_KVCalorimetry()
{
   // protected method
   // Private initialisation method called by all constructors.
   // All member initialisations should be done here.

   kfree_neutrons_included = kFALSE;
   kchargediff = kFALSE;
   ktempdeduced = kFALSE;

   kIsModified = kTRUE;

}

//________________________________________________________________
void KVCalorimetry::SetFragmentMinimumCharge(Double_t value)
{
   // protected method, set the value of FragmentMinimumCharge parameter
   SetParameter("FragmentMinimumCharge", value);
}
//________________________________________________________________
void KVCalorimetry::SetParticleFactor(Double_t value)
{
   // protected method, set the value of ParticleFactor parameter
   SetParameter("ParticleFactor", value);
}
//________________________________________________________________
void KVCalorimetry::SetLevelDensityParameter(Double_t value)
{
   // protected method, set the value of LevelDensityParameter parameter
   SetParameter("LevelDensityParameter", value);
}
//________________________________________________________________
void KVCalorimetry::SetAsurZ(Double_t value)
{
   // protected method, set the value of AsurZ parameter
   SetParameter("AsurZ", value);
}
//________________________________________________________________
void KVCalorimetry::SetNeutronMeanEnergyFactor(Double_t value)
{
   // protected method, set the value of NeutronMeanEnergyFactor parameter
   // value = 1.0 : surface emission
   // value = 1.5 : volume emission
   SetParameter("NeutronMeanEnergyFactor", value);
}

//________________________________________________________________
void KVCalorimetry::UseChargeDiff(Int_t FragmentMinimumCharge, Double_t ParticleFactor)
{
   //Make a difference between particle with a charge (GetZ) greater (fragments)
   //or smaller (particles) than FragmentMinimumCharge.
   //
   //When sum on charge (Zsum), mass (Asum), energy (Eksum), are performed, two partial sums are done,
   //respect to the previous distinction and particle ones will be multiply by ParticleFactor
   //
   //for example, for the kinetic energy, it gives at the end :
   //Eksum = \Sigma Ek(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*\Sigma Ek(Z<[FragmentMinimumCharge])
   //this operation is done in the SumUp() method
   //
   //NOTE : when this method is called, Reset of the object are called also
   //it has to be called before the first Fill
   SetFragmentMinimumCharge(FragmentMinimumCharge);
   SetParticleFactor(ParticleFactor);
   kchargediff = kTRUE;
   kIsModified = kTRUE;
   Reset();

}

//________________________________________________________________
void KVCalorimetry::DeduceTemperature(Double_t LevelDensityParameter)
{
   //The temperature will be computed, the parameter LevelDensityParameter
   //is needed in the formula : Exci = Asum/[LevelDensityParameter] * T*T (resolved in Calculate() method)
   //
   //this method is automaticaly called by the IncludeFreeNeutrons method
   //
   //NOTE : when this method is called, Reset of the object are called also
   //it has to be called before the first Fill
   SetLevelDensityParameter(LevelDensityParameter);
   ktempdeduced = kTRUE;
   kIsModified = kTRUE;
   Reset();

}

//________________________________________________________________
void KVCalorimetry::IncludeFreeNeutrons(Double_t AsurZ, Double_t NeutronMeanEnergyFactor, Double_t LevelDensityParameter)
{

   //Free neutrons are taken into account
   //AsurZ parameter, allow to evaluate the number of free neutrons
   //Mn =  [AsurZ]*Zsum - Asum (done by the method SumUp)
   //
   //then the parameters NeutronMeanEnergyFactor, LevelDensityParameter are used
   //in the formula :
   //Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0
   //which is resolved in Calculate() method
   //
   //NOTE : when this method is called, Reset of the object are called also
   //it has to be called before the first Fill

   SetAsurZ(AsurZ);
   SetNeutronMeanEnergyFactor(NeutronMeanEnergyFactor);
   DeduceTemperature(LevelDensityParameter);
   kfree_neutrons_included = kTRUE;
   kIsModified = kTRUE;
   Reset();

}

//________________________________________________________________
void KVCalorimetry::Fill(KVNucleus* n)
{
   // Remplissage des energies, masse, charge et defaut de masse
   // Pour l'energie cinetique, si l'utilisateur a utilise en amont
   // la methode KVVarGlob::SetFrame(const Char_t*), c'est dans ce repere que les energies sont sommees
   // (a condition que chaque KVNucleus possede le repere avec un nom identique)
   //
   // Deux modes de remplissages :
   //----------------------------
   // - mode par default, somme simple sur les A, Z, Ek, Q sans distinction du type de particules
   //
   // - mode avec distinction particules / fragments, actif si la methode
   // UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor) a ete appelee :
   // ->Une distinction entre produits avec une
   // charge strictement inferieur à FragmentMinimumCharge (particules) et superieur ou egale (fragments)
   // est appliquee

   kIsModified = kTRUE;

   if (kchargediff) {

      if (n->GetZ() >= GetParValue("FragmentMinimumCharge")) {
         AddIngValue("Zfrag", n->GetZ());
         AddIngValue("Afrag", n->GetA());
         AddIngValue("Ekfrag", n->GetFrame(GetFrame(), kFALSE)->GetKE());
         AddIngValue("Qfrag", n->GetMassExcess());
         AddIngValue("Mfrag", 1);
      }
      else {
         AddIngValue("Zpart", n->GetZ());
         AddIngValue("Apart", n->GetA());
         AddIngValue("Ekpart", n->GetFrame(GetFrame(), kFALSE)->GetKE());
         AddIngValue("Qpart", n->GetMassExcess());
         AddIngValue("Mpart", 1);
      }

      return;

   }
   KVCaloBase::Fill(n);

}

//________________________________________________________________
void KVCalorimetry::SumUp()
{
   // protected method
   // Appele par Calculate pour mettre a jour les differents ingredients
   // de la calorimetrie :
   //
   // Trois modes de sommes:
   //------------------
   // - mode normal (par defaut)
   // determination de l exces de masse de la source recontruite, dernier ingredient de l'equation :
   // Exci + Qini  = \Sigma Ek + \Sigma Q -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // - mode avec distinction particules / fragments, actif si la methode
   // UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor) a ete appelee :
   // -> une distinction entre produits avec une charge strictement inferieur a FragmentMinimumCharge (particules)
   // et superieur ou egale (fragments) est appliquee
   // Ainsi dans la methode SumUp() pour les energies cinetiques, par exemple
   // l'energie cinetique de la source reconstruite sera
   // Eksum = Ekfrag(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*Ekpart(Z<[FragmentMinimumCharge])
   // Determination ensuite de l exces de masse de la source
   //
   // - mode avec prise en compte des neutrons libres, actif si la methode
   // IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter)
   // L'estimation du nombre neutrons, est fait en utilisant un AsurZ (paramètre de la calorimétrie)
   // suppose de la source reconstruite :
   // le nombre de neutrons libres est alors egal :
   // Mn =  [AsurZ]*Zsum - Asum
   // Pour un Zsou reconstruit, on rajoute des neutrons pour que le Asou corresponde a un AsurZ predefini
   // On en deduit ensuite l'exces de masse asscoie a ces neutrons
   // Determination ensuite de l exces de masse de la source

   // Les proprietes de la source sont calculees

   if (kchargediff) {
      // somme des contributions fragments et particules
      AddIngValue("Zsum", GetIngValue("Zfrag") + GetParValue("ParticleFactor")*GetIngValue("Zpart"));
      AddIngValue("Asum", GetIngValue("Afrag") + GetParValue("ParticleFactor")*GetIngValue("Apart"));
      AddIngValue("Eksum", GetIngValue("Ekfrag") + GetParValue("ParticleFactor")*GetIngValue("Ekpart"));
      AddIngValue("Qsum", GetIngValue("Qfrag") + GetParValue("ParticleFactor")*GetIngValue("Qpart"));
      AddIngValue("Msum", GetIngValue("Mfrag") + GetParValue("ParticleFactor")*GetIngValue("Mpart"));
   }

   //printf("Eksum=%lf avant neutrons \n",GetIngValue("Eksum"));

   if (kfree_neutrons_included) {
      // conservation du AsurZ du systeme --> multiplicite moyenne des neutrons
      Double_t Mneutron = Double_t(TMath::Nint(GetParValue("AsurZ") * GetIngValue("Zsum") - GetIngValue("Asum")));
      if (Mneutron < 0) {
         //Warning("SumUp","Nombre de neutrons déduits négatif : %1.0lf -> on le met à zéro",Mneutron);
         SetIngValue("Aexcess", TMath::Abs(Mneutron));
         Mneutron = 0;
      }
      SetIngValue("Aneu", Mneutron);
      SetIngValue("Qneu", Mneutron * nn.GetMassExcess(0, 1));
      SetIngValue("Mneu", Mneutron);

      // prise en compte des neutrons dans la source
      AddIngValue("Asum", GetIngValue("Mneu"));
      AddIngValue("Qsum", GetIngValue("Qneu"));
      AddIngValue("Msum", GetIngValue("Mneu"));

   }
   //printf("Eksum=%lf apres neutrons \n",GetIngValue("Eksum"));
   // defaut de masse de la source reconstruite
   KVCaloBase::SumUp();

}

//________________________________________________________________
void KVCalorimetry::Calculate()
{
   //Realisation de la calorimétrie
   //Calcul de l'energie d'excitation, temperature (optionnel), de l'energie moyenne des neutrons (optionnel)
   //appel de SumUp()
   //Cette methode retourne kTRUE si tout s'est bien passee, kFALSE si il y a un probleme dans la resolution
   //du polynome d'ordre 2
   //
   // Deux modes de calcul:
   //------------------
   // - mode normal (par defaut)
   // Resolution de l'equation
   // Exci + Qini  = \Sigma Ek + \Sigma Q
   //    -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // Optionnel :
   // le calcul de la temperature peut etre egalement fait si la methode DeduceTemperature(Double_t LevelDensityParameter) a ete appelee
   // elle est obtenue via la formule : Exci = Asum/[LevelDensityParameter] * T*T
   //
   // - mode avec prise en compte des neutrons libres, actif si la methode
   // IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter)
   // Resolution de l'equation (polynome deuxieme degree en T (temperature) )
   // Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0
   // on y obtient directement la temperature
   //

   //Info("Calculate","Debut");

   if (!kIsModified) return;
   kIsModified = kFALSE;
   // premier calcul depuis le dernier remplissage par Fill
   SumUp();

   if (kfree_neutrons_included) {

      Double_t coefA = GetIngValue("Asum") / GetParValue("LevelDensityParameter");
      Double_t coefB = -1.*GetParValue("NeutronMeanEnergyFactor") * GetIngValue("Mneu");
      Double_t coefC = GetIngValue("Qini") - GetIngValue("Qsum") - GetIngValue("Eksum");

      // Resolution du polynome de degre 2
      // Les champs ne sont remplis que si une solution reelle est trouvee
      if (RootSquare(coefA, coefB, coefC)) {
         // la solution max donne la temperature
         SetIngValue("Temp", kracine_max);
         SetIngValue("Exci", coefA * TMath::Power(GetIngValue("Temp"), 2.));

         // ajout de l'energie des neutrons a l energie totale de la source
         SetIngValue("Ekneu", GetParValue("NeutronMeanEnergyFactor") * GetIngValue("Mneu")*GetIngValue("Temp"));
         AddIngValue("Eksum", GetIngValue("Ekneu"));

         //parametre additionnel
         //SetIngValue("Tmin",kracine_min); // la deuxieme solution de l'eq en T2
      }
      else {
         return;
      }

   }
   else {

      ComputeExcitationEnergy();
      if (ktempdeduced) {
         ComputeTemperature();
      }

   }
}

//________________________________________________________________
void KVCalorimetry::ComputeTemperature() const
{

   Double_t exci = GetIngValue("Exci");
   Double_t temp = TMath::Sqrt(GetParValue("LevelDensityParameter") * exci / GetIngValue("Asum"));
   SetIngValue("Temp", temp);

}
