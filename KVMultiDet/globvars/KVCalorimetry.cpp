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
<h4>Bilan énergétique d un ensemble de noyaux</h4>
<!-- */
// --> END_HTML
// Classe héritée de KVCaloBase avec d'ajouter à ces membres, deux KVNameValueList permettant
// gérer les ingrédients/variables et les paramètres de manière complètement progressive.
// Ces deux listes sont accessibles via GetList(Option_t* option = "ing" ou "par")
//
// PRINCIPE
// KVCalorimetry fait la somme des Z (Zsum), A (Asum), Ek (Eksum) et Q (Qsum) des noyaux considérés (méthode Fill(KVNucleus* ))
// Ces ingrédients permettent ensuite de calculer l'énergie d'excitation en utilisant
// le bilan énergétique suivant:
// Exci + Qini  = Eksum + Qsum -> Exci = Eksum + Qsum - Qini
// A noter : Si l'utilisateur définit un repère via KVVarGlob::SetFrame(const Char_t* ) les énergies cinétiques
// des noyaux sont prises dans ce référentiel ( KVNucleus::GetFrame("nom_du_referentiel)"->GetKE() )
//------------------
// Exemple d utilisation :
//------------------
//
//KVNucleus alpha(2,4,10); //definition des noyaux
//KVNucleus triton(1,3);
//KVNucleus azote(7,16,40);
//
//KVCalorimetry ca;
//
//ca.Fill(&alpha);   //Remplissage de la variable
//ca.Fill(&triton);
//ca.Fill(&azote);
//
//ca.Calculate();    //Calcul
//ca.Print("ing");   //Print
//Ingredients, 7 stored:
//0 | Zsum | 10.00000   Sum of charges
//1 | Asum | 23.00000   Sum of masses
//2 | Eksum | 50.0000   Sum of kinetic energiex (MeV)
//3 | Qsum | 23.05840   Sum of mass excess (MeV)
//4 | Msum | 3.000000   Multiplicity
//5 | Qini | -5.15400   Mass Excess of the initial state (reconstructed sourceeeeeee)
//6 | Exci | 78.21240   Excitation energy (MeV)
//
//ca.GetValue(0)
//10.0000
//ca.GetValue("Exci")
//78.21240
//
//------------------------ //Fin de l'exemple
//
// ACCES AUX VARIABLES
//
//L'accès aux variables se fait via les deux méthodes :
// - GetValue(Int_t) choix par index
// - GetValue(const Char_t* ) choix par nom
// - GetValuePtr(), renvoie un tableau de Double_t*, de la dimension correspondant au nombre de valeurs (méthode GetNumberOfValues())
// C'est à l' utilisateur ensuite d'effacer ce tableau via "delete [] tableau"
// - Pour connaître l'index d une variable : GetNameIndex(const Char_t* )
// - Pour connaître une variable à un index donné : GetValueName(Int_t )
//
// PLUSIEURS MODES DE FONCTIONNEMENT
//
// Deux modes de remplissage :
//------------------------
// 1. Mode normal (par défaut) comme explicité plus haut
// 2. Mode distinguant les noyaux suivant leur charge, ie séparation entre particules et fragments
//    Actif avec l appel de la méthode void UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor)
//    le paramètre FragmentMinimumCharge est le critère discriminant :
//       - KVNucleus::GetZ()<FragmentMinimumCharge -> particules
//       - KVNucleus::GetZ()>=FragmentMinimumCharge -> fragments
//    le paramètre ParticleFactor correspond au facteur lorsque les grandeurs Zsum, Asum, Eksum, Qsum
//    sont calculées.
//    Dans la méthode SumUp on a  : Eksum = \Sigma Ek(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*\Sigma Ek(Z<[FragmentMinimumCharge])
//    et dans la liste des ingrédients, sont ajoutés les deux contributions particules et fragments et les paramètres choisis
//    sont également enregistrés
//
//       root [12] ca.Print("ing");    //Exemple d'output avec cette methode
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
//
//----------------------------
//
// Deux modes de calcul :
//------------------------
// 1. Mode normal (par défaut) comme explicité plus haut
// 2. Mode incluant les neutrons libres
//    Actif avec l appel de la méthode IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);
//
//    Mn =  [AsurZ]*Zsum - Asum  (methode SumUp)
//    Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0   (methode Calculate)
//    Exci = Asum/[LevelDensityParameter] * T*T
//
//       A NOTER : Dans le cas ou le calcul de la multiplicité de neutrons retourne un nombre négatif (Mneu<0), le
//       multiplicité de neutrons est mise à zéro (Mneu=0) et on rajoute un paramètre Aexcess = TMath::Abs(Mneu)
//       La calorimétrie se fait en considérant aucun neutrons libres
//
//    Dans la liste des ingrédients, sont ajoutés les contributions relatives aux neutrons et les paramètres choisis
//    sont également enregistrés
//       root [31] ca.Print("ing");       //Exemple d'output avec cette methode
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
// Température (MeV)
// Dans le cas où les neutrons libres sont pris en compte, la détermination de la température
// fait partie du calcul, dans les autres cas, l'utilisateur peut appeler en début de traitement
// la méthode DeduceTemperature(Double_t LevelDensityParameter) qui donne la température suivant la formule :
// T = TMath::Sqrt(Exci * [LevelDensityParameter]/Asum)
//
// Pour Resume,
//    IL EST INDISPENSABLE D APPELER LA METHODE Calculate() avant d'utiliser les variables calculées dans KVCalorimetry
//    Cette méthode renvoie un booléen indiquant si tout c'est bien passé (kTRUE)
//    les methodes :
//       void UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor);
//       void DeduceTemperature(Double_t LevelDensityParameter);
//       void IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);
//    DOIVENT ETRE APPELEES AVANT LES OPERATIONS DE FILL de l'objet KVCalorimetry

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
   // Pour l'énergie cinétique, si l'utilisateur a utilisé en amont
   // la méthode KVVarGlob::SetFrame(const Char_t*), c'est dans ce repère que les énergies sont sommées
   // (à condition que chaque KVNucleus possede le repere avec un nom identique)
   //
   // Deux modes de remplissages :
   //----------------------------
   // - mode par défault, somme simple sur les A, Z, Ek, Q sans distinction du type de particules
   //
   // - mode avec distinction particules / fragments, actif si la méthode
   // UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor) a été appelée :
   // ->Uune distinction entre produits avec une
   // charge strictement inférieur à FragmentMinimumCharge (particules) et supérieur ou égale (fragments)
   // est appliquée

   kIsModified = kTRUE;

   if (kchargediff) {

      if (n->GetZ() >= GetParValue("FragmentMinimumCharge")) {
         AddIngValue("Zfrag", n->GetZ());
         AddIngValue("Afrag", n->GetA());
         AddIngValue("Ekfrag", n->GetFrame(fFrame.Data(), kFALSE)->GetKE());
         AddIngValue("Qfrag", n->GetMassExcess());
         AddIngValue("Mfrag", 1);
      } else {
         AddIngValue("Zpart", n->GetZ());
         AddIngValue("Apart", n->GetA());
         AddIngValue("Ekpart", n->GetFrame(fFrame.Data(), kFALSE)->GetKE());
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
   // Appelé par Calculate pour mettre à jour les différents ingrédients
   // de la calorimétrie :
   //
   // Trois modes de sommes:
   //------------------
   // - mode normal (par defaut)
   // détermination de l excès de masse de la source recontruite, dernier ingrédient de l'équation :
   // Exci + Qini  = \Sigma Ek + \Sigma Q -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // - mode avec distinction particules / fragments, actif si la méthode
   // UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor) a été appelée :
   // -> une distinction entre produits avec une charge strictement inférieur à FragmentMinimumCharge (particules)
   // et supérieur ou égale (fragments) est appliquée
   // Ainsi dans la méthode SumUp() pour les énergies cinétiques, par exemple
   // l'énergie cinétique de la source reconstruite sera
   // Eksum = Ekfrag(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*Ekpart(Z<[FragmentMinimumCharge])
   // Détermination ensuite de l excès de masse de la source
   //
   // - mode avec prise en compte des neutrons libres, actif si la métode
   // IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter)
   // L'estimation du nombre neutrons, est fait en utilisant un AsurZ (paramètre de la calorimétrie)
   // supposé de la source reconstruite :
   // le nombre de neutrons libres est alors égal :
   // Mn =  [AsurZ]*Zsum - Asum
   // Pour un Zsou reconstruit, on rajoute des neutrons pour que le Asou corresponde à un AsurZ prédéfini
   // On en déduit ensuite l'exces de masse asscoié à ces neutrons
   // Détermination ensuite de l excès de masse de la source

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
Bool_t   KVCalorimetry::Calculate(void)
{
   //Réalisation de la calorimétrie
   //Calcul de l'énergie d'excitation, température (optionnel), de l'énergie moyenne des neutrons (optionnel)
   //appel de SumUp()
   //Cette méthore retourne kTRUE si tout s'est bien passée, kFALSE si il y a un problème dans la résolution
   //du polynome d'ordre 2
   //
   // Deux modes de calcul:
   //------------------
   // - mode normal (par defaut)
   // Résolution de l'équation
   // Exci + Qini  = \Sigma Ek + \Sigma Q
   //    -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // Optionnel :
   // le calcul de la température peut être également fait si la méthode DeduceTemperature(Double_t LevelDensityParameter) a été appelée
   // elle est obtenue via la formule : Exci = Asum/[LevelDensityParameter] * T*T
   //
   // - mode avec prise en compte des neutrons libres, actif si la métode
   // IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter)
   // Résolution de l'équation (polynome deuxième degrée en T (température) )
   // Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0
   // on y obtient directement la température
   //

   //Info("Calculate","Debut");

   if (!kIsModified) return kTRUE;
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
      } else {
         return kFALSE;
      }

   } else {

      ComputeExcitationEnergy();
      if (ktempdeduced) {
         ComputeTemperature();
      }

   }
   return kTRUE;
}

//________________________________________________________________
void KVCalorimetry::ComputeTemperature()
{

   Double_t exci = GetIngValue("Exci");
   Double_t temp = TMath::Sqrt(GetParValue("LevelDensityParameter") * exci / GetIngValue("Asum"));
   SetIngValue("Temp", temp);

}
