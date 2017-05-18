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
<h4>Bilan �nerg�tique d un ensemble de noyaux</h4>
<!-- */
// --> END_HTML
// Classe h�rit�e de KVCaloBase avec d'ajouter � ces membres, deux KVNameValueList permettant
// g�rer les ingr�dients/variables et les param�tres de mani�re compl�tement progressive.
// Ces deux listes sont accessibles via GetList(Option_t* option = "ing" ou "par")
//
// PRINCIPE
// KVCalorimetry fait la somme des Z (Zsum), A (Asum), Ek (Eksum) et Q (Qsum) des noyaux consid�r�s (m�thode Fill(KVNucleus* ))
// Ces ingr�dients permettent ensuite de calculer l'�nergie d'excitation en utilisant
// le bilan �nerg�tique suivant:
// Exci + Qini  = Eksum + Qsum -> Exci = Eksum + Qsum - Qini
// A noter : Si l'utilisateur d�finit un rep�re via KVVarGlob::SetFrame(const Char_t* ) les �nergies cin�tiques
// des noyaux sont prises dans ce r�f�rentiel ( KVNucleus::GetFrame("nom_du_referentiel)"->GetKE() )
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
//L'acc�s aux variables se fait via les deux m�thodes :
// - GetValue(Int_t) choix par index
// - GetValue(const Char_t* ) choix par nom
// - GetValuePtr(), renvoie un tableau de Double_t*, de la dimension correspondant au nombre de valeurs (m�thode GetNumberOfValues())
// C'est � l' utilisateur ensuite d'effacer ce tableau via "delete [] tableau"
// - Pour conna�tre l'index d une variable : GetNameIndex(const Char_t* )
// - Pour conna�tre une variable � un index donn� : GetValueName(Int_t )
//
// PLUSIEURS MODES DE FONCTIONNEMENT
//
// Deux modes de remplissage :
//------------------------
// 1. Mode normal (par d�faut) comme explicit� plus haut
// 2. Mode distinguant les noyaux suivant leur charge, ie s�paration entre particules et fragments
//    Actif avec l appel de la m�thode void UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor)
//    le param�tre FragmentMinimumCharge est le crit�re discriminant :
//       - KVNucleus::GetZ()<FragmentMinimumCharge -> particules
//       - KVNucleus::GetZ()>=FragmentMinimumCharge -> fragments
//    le param�tre ParticleFactor correspond au facteur lorsque les grandeurs Zsum, Asum, Eksum, Qsum
//    sont calcul�es.
//    Dans la m�thode SumUp on a  : Eksum = \Sigma Ek(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*\Sigma Ek(Z<[FragmentMinimumCharge])
//    et dans la liste des ingr�dients, sont ajout�s les deux contributions particules et fragments et les param�tres choisis
//    sont �galement enregistr�s
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
// 1. Mode normal (par d�faut) comme explicit� plus haut
// 2. Mode incluant les neutrons libres
//    Actif avec l appel de la m�thode IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter);
//
//    Mn =  [AsurZ]*Zsum - Asum  (methode SumUp)
//    Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0   (methode Calculate)
//    Exci = Asum/[LevelDensityParameter] * T*T
//
//       A NOTER : Dans le cas ou le calcul de la multiplicit� de neutrons retourne un nombre n�gatif (Mneu<0), le
//       multiplicit� de neutrons est mise � z�ro (Mneu=0) et on rajoute un param�tre Aexcess = TMath::Abs(Mneu)
//       La calorim�trie se fait en consid�rant aucun neutrons libres
//
//    Dans la liste des ingr�dients, sont ajout�s les contributions relatives aux neutrons et les param�tres choisis
//    sont �galement enregistr�s
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
// Temp�rature (MeV)
// Dans le cas o� les neutrons libres sont pris en compte, la d�termination de la temp�rature
// fait partie du calcul, dans les autres cas, l'utilisateur peut appeler en d�but de traitement
// la m�thode DeduceTemperature(Double_t LevelDensityParameter) qui donne la temp�rature suivant la formule :
// T = TMath::Sqrt(Exci * [LevelDensityParameter]/Asum)
//
// Pour Resume,
//    IL EST INDISPENSABLE D APPELER LA METHODE Calculate() avant d'utiliser les variables calcul�es dans KVCalorimetry
//    Cette m�thode renvoie un bool�en indiquant si tout c'est bien pass� (kTRUE)
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
   // Pour l'�nergie cin�tique, si l'utilisateur a utilis� en amont
   // la m�thode KVVarGlob::SetFrame(const Char_t*), c'est dans ce rep�re que les �nergies sont somm�es
   // (� condition que chaque KVNucleus possede le repere avec un nom identique)
   //
   // Deux modes de remplissages :
   //----------------------------
   // - mode par d�fault, somme simple sur les A, Z, Ek, Q sans distinction du type de particules
   //
   // - mode avec distinction particules / fragments, actif si la m�thode
   // UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor) a �t� appel�e :
   // ->Uune distinction entre produits avec une
   // charge strictement inf�rieur � FragmentMinimumCharge (particules) et sup�rieur ou �gale (fragments)
   // est appliqu�e

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
   // Appel� par Calculate pour mettre � jour les diff�rents ingr�dients
   // de la calorim�trie :
   //
   // Trois modes de sommes:
   //------------------
   // - mode normal (par defaut)
   // d�termination de l exc�s de masse de la source recontruite, dernier ingr�dient de l'�quation :
   // Exci + Qini  = \Sigma Ek + \Sigma Q -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // - mode avec distinction particules / fragments, actif si la m�thode
   // UseChargeDiff(Int_t FragmentMinimumCharge,Double_t ParticleFactor) a �t� appel�e :
   // -> une distinction entre produits avec une charge strictement inf�rieur � FragmentMinimumCharge (particules)
   // et sup�rieur ou �gale (fragments) est appliqu�e
   // Ainsi dans la m�thode SumUp() pour les �nergies cin�tiques, par exemple
   // l'�nergie cin�tique de la source reconstruite sera
   // Eksum = Ekfrag(Z>=[FragmentMinimumCharge]) + [ParticleFactor]*Ekpart(Z<[FragmentMinimumCharge])
   // D�termination ensuite de l exc�s de masse de la source
   //
   // - mode avec prise en compte des neutrons libres, actif si la m�tode
   // IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter)
   // L'estimation du nombre neutrons, est fait en utilisant un AsurZ (param�tre de la calorim�trie)
   // suppos� de la source reconstruite :
   // le nombre de neutrons libres est alors �gal :
   // Mn =  [AsurZ]*Zsum - Asum
   // Pour un Zsou reconstruit, on rajoute des neutrons pour que le Asou corresponde � un AsurZ pr�d�fini
   // On en d�duit ensuite l'exces de masse asscoi� � ces neutrons
   // D�termination ensuite de l exc�s de masse de la source

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
         //Warning("SumUp","Nombre de neutrons d�duits n�gatif : %1.0lf -> on le met � z�ro",Mneutron);
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
   //R�alisation de la calorim�trie
   //Calcul de l'�nergie d'excitation, temp�rature (optionnel), de l'�nergie moyenne des neutrons (optionnel)
   //appel de SumUp()
   //Cette m�thore retourne kTRUE si tout s'est bien pass�e, kFALSE si il y a un probl�me dans la r�solution
   //du polynome d'ordre 2
   //
   // Deux modes de calcul:
   //------------------
   // - mode normal (par defaut)
   // R�solution de l'�quation
   // Exci + Qini  = \Sigma Ek + \Sigma Q
   //    -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // Optionnel :
   // le calcul de la temp�rature peut �tre �galement fait si la m�thode DeduceTemperature(Double_t LevelDensityParameter) a �t� appel�e
   // elle est obtenue via la formule : Exci = Asum/[LevelDensityParameter] * T*T
   //
   // - mode avec prise en compte des neutrons libres, actif si la m�tode
   // IncludeFreeNeutrons(Double_t AsurZ,Double_t NeutronMeanEnergyFactor,Double_t LevelDensityParameter)
   // R�solution de l'�quation (polynome deuxi�me degr�e en T (temp�rature) )
   // Asum/[LevelDensityParameter] * T*T + Qi - \Sigma Ek - [NeutronMeanEnergyFactor]*Mn*T - \Sigma Q = 0
   // on y obtient directement la temp�rature
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
