/*
$Id: KVCaloBase.cpp,v 1.4 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.4 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Mon Apr 14 15:01:51 2008
//Author: eric bonnet,,,

#include "KVCaloBase.h"
#include "KVNDTManager.h"

ClassImp(KVCaloBase)




void KVCaloBase::Copy(TObject& a) const
{
// Methode de Copy
   KVVarGlob::Copy(a);
   nvl_ing.Copy(dynamic_cast<KVCaloBase&>(a).nvl_ing);
}

void KVCaloBase::Init()
{
   // Init() is called by KVGVList::MakeBranches(), so this is the latest they
   // can be set up.
   SetNameIndex("Zsum", 0);
   SetNameIndex("Asum", 1);
   SetNameIndex("Eksum", 2);
   SetNameIndex("Qsum", 3);
   SetNameIndex("Msum", 4);
   SetNameIndex("Qini", 5);
   SetNameIndex("Exci", 6);
}

//_________________________________________________________________
void KVCaloBase::Reset(void)
{
   // Remise a zero avant le
   // traitement d'un evenement

#ifdef WITH_CPP11
   for (auto& par : nvl_ing) {
      par.Set(0.0);
   }
#else
   for (KVNameValueList::Iterator it = nvl_ing.begin(); it != nvl_ing.end(); ++it)(*it).Set(0.0);
#endif
   kIsModified = kTRUE;
}

//_________________________________________________________________
void KVCaloBase::Print(Option_t* option) const
{
   //printf information on the object
   //opt==ing, print the list of ingredients computed
   //opt==par, print the list of parameters

   if (!strcmp(option, "ing"))
      nvl_ing.Print();
   else if (!strcmp(option, "par"))
      GetParameters().Print();
   else
      KVVarGlob::Print();

}

//_________________________________________________________________
const KVNameValueList& KVCaloBase::GetList(Option_t* option) const
{
   //retourne la KVNameValueList ou sont enregistres les ingredients (option=="ing")
   //ou les parametres (option=="par")
   //
   if (!strcmp(option, "ing"))
      return nvl_ing;
   if (!strcmp(option, "par"))
      return GetParameters();
   else {
      Info("GetList", "type has to be equal to \"ing\" or \"par\"");
      return GetParameters();
   }

}

//_________________________________________________________________
Double_t KVCaloBase::getvalue_int(Int_t i) const
{
   // can't assume all ingredients declared to SetNameIndex in list nvl_ing exist
   //  - as they are dynamically created as data is filled
   // therefore use NameIndex to retrieve name, then look for value in list by name

   return GetIngValue(GetValueName(i));
}

//_________________________________________________________________
Char_t KVCaloBase::GetValueType(Int_t i) const
{
   // Returns type of value depending on name:
   //   Zsum I
   //   Asum I
   //   Eksum D
   //   Qsum D
   //   Msum I
   //   Aneu I
   //   Qneu D
   //   Mneu I
   //   Qini D
   //   Temp D
   //   Exci D
   //   Ekneu D
   //   Zpart I
   //   Apart I
   //   Ekpart D
   //   Qpart D
   //   Mpart I
   //   Zfrag I
   //   Afrag I
   //   Ekfrag D
   //   Qfrag D
   //   Mfrag I

   TString name = GetValueName(i);
   if (name.BeginsWith("E") || name.BeginsWith("Q") || name.BeginsWith("T")) return 'D';
   else return 'I';
}

//_________________________________________________________________
std::vector<Double_t> KVCaloBase::GetValueVector(void) const
{
   // On retourne un tableau rassemblant l'ensemble des ingredients

   std::vector<Double_t> tab;
   for (Int_t ii = 0; ii < GetNumberOfValues(); ++ii)
      tab.push_back(getvalue_int(ii));
   return tab;
}

//________________________________________________________________
void KVCaloBase::init_KVCaloBase()
{
   // protected method
   // Private initialisation method called by all constructors.
   // All member initialisations should be done here.

   fType = KVVarGlob::kOneBody; // this is a 1-body variable
   //KVNameValueList contentant les ingredients et les parametres
   //de la variable globale
   //Elles sont remplies au fur et a mesure des
   //methodes, pas besoin de definition a priori des
   //noms des ingredients / parametres
   nvl_ing.SetName("Ingredients");
   kIsModified = kTRUE;
}

//________________________________________________________________
Double_t KVCaloBase::GetIngValue(const KVString& name) const
{
   //return the value of a name given ingredient
   //if it is not defined return 0
   if (!nvl_ing.HasParameter(name.Data())) return 0;
   return nvl_ing.GetDoubleValue(name.Data());
}
//________________________________________________________________
Double_t KVCaloBase::GetIngValue(Int_t idx) const
{
   // protected method,
   //return the value of a index given ingredient
   return nvl_ing.GetDoubleValue(idx);
}
//________________________________________________________________
void KVCaloBase::SetIngValue(KVString name, Double_t value)
{
   // protected method,
   //set the value a name given ingredient
   nvl_ing.SetValue(name.Data(), value);
}
//________________________________________________________________
void KVCaloBase::AddIngValue(KVString name, Double_t value)
{
   // protected method,
   //increment the value of a name given ingredient
   //if it is not defined, it's created
   Double_t before = GetIngValue(name);
   before += value;
   SetIngValue(name, before);
}

//________________________________________________________________
void KVCaloBase::fill(const KVNucleus* n)
{
   // Remplissage des energies, masse, charge et defaut de masse
   // Pour l'energie cinetique, si l'utilisateur a utilise en amont
   // la methode KVVarGlob::SetFrame(const Char_t*), c'est dans ce repere que les energies sont sommees
   // (a condition que chaque KVNucleus possede le repere avec un nom identique)
   //
   // somme simple sur les A, Z, Ek, Q sans distinction du type de particules

   kIsModified = kTRUE;
   AddIngValue("Zsum", n->GetZ());
   AddIngValue("Asum", n->GetA());
   AddIngValue("Eksum", n->GetKE());
   AddIngValue("Qsum", n->GetMassExcess());
   AddIngValue("Msum", 1);
}

//________________________________________________________________
void KVCaloBase::SumUp()
{
   // protected method
   // Appele par Calculate pour mettre a jour les differents ingredients
   // de la calorimetrie :
   //
   // Trois modes de sommes:
   //------------------
   //
   // determination de l exces de masse de la source recontruite, dernier ingredient de l'equation :
   // Exci + Qini  = \Sigma Ek + \Sigma Q -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   // defaut de masse de la source reconstruite

   SetIngValue("Qini", nn.GetMassExcess(TMath::Nint(GetIngValue("Zsum")), TMath::Nint(GetIngValue("Asum"))));

}

//________________________________________________________________
void KVCaloBase::ComputeExcitationEnergy()
{
   Double_t exci = GetIngValue("Qsum") + GetIngValue("Eksum") - GetIngValue("Qini");
   SetIngValue("Exci", exci);
}

//________________________________________________________________
void KVCaloBase::AddNeutrons(Int_t mult, Double_t mke)
{
   //Add extra neutrons
   // multiplicity (number) and mean kinetic energy

   kIsModified = kTRUE;
   AddIngValue("Asum", mult);
   AddIngValue("Eksum", mult * mke);
   AddIngValue("Qsum", mult * nn.GetMassExcess(0, 1));
   AddIngValue("Msum", mult);

}
//________________________________________________________________
void   KVCaloBase::Calculate()
{
   //Realisation de la calorimetrie
   //Calcul de l'energie d'excitation
   //appel de SumUp()
   //
   // Resolution de l'equation
   // Exci + Qini  = \Sigma Ek + \Sigma Q
   //    -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   //

   if (!kIsModified) return;
   kIsModified = kFALSE;
   // premier calcul depuis le dernier remplissage par Fill
   SumUp();

   ComputeExcitationEnergy();
}

//________________________________________________________________
Bool_t   KVCaloBase::RootSquare(Double_t aa, Double_t bb, Double_t cc)
{
   // protected method
   //
   // calcul les racines du polynome d'ordre 2 : aa*x*x + bb*xx + cc = 0
   // les racines sont accessibles par les variables kracine_min et kracine_max
   //
   // kroot_status>=0 -> tout c'est bien passe   la fonction retourne kTRUE
   //    =0 2 racines reelles distinctes
   //    =1 2 racines reelles egales (aa==0)
   //
   // kroot_status<0 les deux racines sont mises a zero la fonction retourne kFALSE
   //    =-1 2 racines imaginaires (Delta<0)
   //    =-2 aa=bb=0
   // le calcul n'est alors pas poursuivi, la methode Calculate() retournera kFALSE
   // la cause peut etre discriminee en appelant la methode GetValue("RootStatus")
   //
   kracine_max = 0, kracine_min = 0;
   Double_t x1, x2;
   kroot_status = 0;
   if (aa != 0) {
      Double_t Delta = TMath::Power(bb, 2.) - 4.*aa * cc;
      if (Delta < 0) {
         //Warning("RootSquare","Delta<0 - Solutions imaginaires");
         kroot_status = -1;
         SetIngValue("RootStatus", kroot_status);
      }
      else {
         Double_t racDelta = TMath::Sqrt(Delta);
         x1 = (-1.*bb + racDelta) / (2.*aa);
         x2 = (-1.*(bb + racDelta)) / (2.*aa);
         kroot_status = 0;
         if (x1 > x2)  {
            kracine_max = x1;
            kracine_min = x2;
         }
         else        {
            kracine_max = x2;
            kracine_min = x1;
         }
      }
   }
   else {
      if (bb != 0) {
         kroot_status = 1;
         kracine_max = kracine_min = -1.*cc / bb;
      }
      else {
         kroot_status = -2;
         kracine_max = kracine_min = 0;
         SetIngValue("RootStatus", kroot_status);
      }
   }
   if (kroot_status < 0) {
      SetIngValue("RootStatus", kroot_status);
      return kFALSE;
   }
   else {
      return kTRUE;
   }

}
