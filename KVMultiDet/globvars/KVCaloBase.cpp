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

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCaloBase</h2>
<h4>Bilan �nerg�tique d un ensemble de noyaux</h4>
<!-- */
// --> END_HTML
// Classe h�rit�e de KVVarGlob avec d'ajouter � ces membres, deux KVNameValueList permettant
// g�rer les ingr�dients/variables et les param�tres de mani�re compl�tement progressive.
// Ces deux listes sont accessibles via GetList(Option_t* option = "ing" ou "par")
//
// PRINCIPE
// KVCaloBase fait la somme des Z (Zsum), A (Asum), Ek (Eksum) et Q (Qsum) des noyaux consid�r�s (m�thode Fill(KVNucleus* ))
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
//KVCaloBase ca;
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
//    IL EST INDISPENSABLE D APPELER LA METHODE Calculate() avant d'utiliser les variables calcul�es dans KVCaloBase
//    Cette m�thode renvoie un bool�en indiquant si tout c'est bien pass� (kTRUE)

//
////////////////////////////////////////////////////////////////////////////////

KVCaloBase::KVCaloBase(void): KVVarGlob()
{
// Createur par default

   init_KVCaloBase();
   SetName("KVCaloBase");
   SetTitle("A KVCaloBase");

}

//_________________________________________________________________
KVCaloBase::KVCaloBase(const Char_t* nom): KVVarGlob(nom)
{
// Constructeur avec un nom

   init_KVCaloBase();
}

//_________________________________________________________________
KVCaloBase::KVCaloBase(const KVCaloBase& a): KVVarGlob()
{
// Contructeur par Copy

   init_KVCaloBase();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVCaloBase&)a).Copy(*this);
#endif
}

//_________________________________________________________________
KVCaloBase::~KVCaloBase(void)
{
// Destructeur

   delete nvl_ing;
   delete nvl_par;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVCaloBase::Copy(TObject& a) const
#else
void KVCaloBase::Copy(TObject& a)
#endif
{
// Methode de Copy
   KVVarGlob::Copy(a);
   nvl_ing->Copy(*((KVCaloBase&)a).GetList("ing"));
   nvl_par->Copy(*((KVCaloBase&)a).GetList("par"));

}

//_________________________________________________________________
KVCaloBase& KVCaloBase::operator = (const KVCaloBase& a)
{
// Operateur =
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVCaloBase&)a).Copy(*this);
#endif
   return *this;
}

//_________________________________________________________________
void KVCaloBase::Reset(void)
{
   // Remise a zero avant le
   // traitement d'un evenement

   for (Int_t nn = 0; nn < nvl_ing->GetNpar(); nn += 1)
      nvl_ing->SetValue(nvl_ing->GetNameAt(nn), 0.);
   kIsModified = kTRUE;

}

//_________________________________________________________________
void KVCaloBase::Print(Option_t* option) const
{
   //printf information on the object
   //opt==ing, print the list of ingredients computed
   //opt==par, print the list of parameters

   if (!strcmp(option, "ing"))
      nvl_ing->Print();
   else if (!strcmp(option, "par"))
      nvl_par->Print();
   else
      KVVarGlob::Print();

}

//_________________________________________________________________
KVNameValueList* KVCaloBase::GetList(Option_t* option) const
{
   //retourne la KVNameValueList ou sont enregistr�s les ingr�dients (option=="ing")
   //ou les param�tres (option=="par")
   //
   if (!strcmp(option, "ing"))
      return nvl_ing;
   if (!strcmp(option, "par"))
      return nvl_par;
   else {
      Info("GetList", "type has to be equal to \"ing\" or \"par\", return NULL pointer");
      return 0;
   }

}

//_________________________________________________________________
Double_t KVCaloBase::getvalue_int(Int_t i)
{
   // derived method
   // protected method
   // On retourne la ieme valeur du tableau
   // si i est superieur au nbre de variables definies dans ingredient_list
   // retourne la valeur par defaut (ie 0)
   // appel a la methode Calculate pour mettre a jour
   // les variables avant d effectuer le retour

   if (i < nvl_ing->GetNpar()) {
      Calculate();
      return GetIngValue(i);
   } else return 0;
}

//_________________________________________________________________
Int_t KVCaloBase::GetNameIndex(const Char_t* name)
{
   // derived method
   // protected method
   //return the index (position in the list ) of a given name
   return nvl_ing->GetNameIndex(name);
}

//_________________________________________________________________
const Char_t* KVCaloBase::GetValueName(Int_t ii) const
{
   // Returns name of value associated with index 'i',
   return nvl_ing->GetNameAt(ii);
};

//_________________________________________________________________
Int_t KVCaloBase::GetNumberOfValues() const
{
   //derived method
   return nvl_ing->GetNpar();

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
};

//_________________________________________________________________
Double_t* KVCaloBase::GetValuePtr(void)
{
   // On retourne un tableau rassemblant l'ensemble des ingr�dients
   // retourne 0 si aucun ingredient
   // Note, c'est � l'utilisateur d'effacer ensuite ce tableau

   if (GetNumberOfValues())
      return 0;

   Double_t* tab = new Double_t[GetNumberOfValues()];
   for (Int_t ii = 0; ii < GetNumberOfValues(); ii += 1)
      tab[ii] = getvalue_int(ii);

   return tab;
};

//________________________________________________________________
void KVCaloBase::init_KVCaloBase()
{
   // protected method
   // Private initialisation method called by all constructors.
   // All member initialisations should be done here.

   fType = KVVarGlob::kOneBody; // this is a 1-body variable
   //KVNameValueList contentant les ingr�dients et les param�tres
   //de la variable globale
   //Elles sont remplies au fur et � mesure des
   //methodes, pas besoin de d�finition a priori des
   //noms des ingr�dients / param�tres
   nvl_ing = new KVNameValueList();
   nvl_ing->SetName("Ingredients");
   nvl_par = new KVNameValueList();
   nvl_par->SetName("Parameters");

   kIsModified = kTRUE;

}

//________________________________________________________________
Double_t KVCaloBase::GetIngValue(KVString name)
{
   //return the value of a name given ingredient
   //if it is not defined return 0
   if (!nvl_ing->HasParameter(name.Data())) return 0;
   return nvl_ing->GetDoubleValue(name.Data());
}
//________________________________________________________________
Double_t KVCaloBase::GetIngValue(Int_t idx)
{
   // protected method,
   //return the value of a index given ingredient
   return nvl_ing->GetDoubleValue(idx);
}
//________________________________________________________________
void KVCaloBase::SetIngValue(KVString name, Double_t value)
{
   // protected method,
   //set the value a name given ingredient
   nvl_ing->SetValue(name.Data(), value);
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
Bool_t KVCaloBase::HasParameter(KVString name)
{
   // protected method,
   //Check if a given parameter is defined
   return nvl_par->HasParameter(name.Data());
}
//________________________________________________________________
Double_t KVCaloBase::GetParValue(KVString name)
{
   //return the value of a name given parameter
   return nvl_par->GetDoubleValue(name.Data());
}

//________________________________________________________________
void KVCaloBase::SetParameter(const Char_t* par, Double_t value)
{
   //protected method
   //Set the vamlue of a given name parameter
   nvl_par->SetValue(par, value);
   KVVarGlob::SetParameter(par, value);

}

//________________________________________________________________
void KVCaloBase::Fill(KVNucleus* n)
{
   // Remplissage des energies, masse, charge et defaut de masse
   // Pour l'�nergie cin�tique, si l'utilisateur a utilis� en amont
   // la m�thode KVVarGlob::SetFrame(const Char_t*), c'est dans ce rep�re que les �nergies sont somm�es
   // (� condition que chaque KVNucleus possede le repere avec un nom identique)
   //
   // somme simple sur les A, Z, Ek, Q sans distinction du type de particules

   kIsModified = kTRUE;
   AddIngValue("Zsum", n->GetZ());
   AddIngValue("Asum", n->GetA());
   AddIngValue("Eksum", n->GetFrame(fFrame.Data(), kFALSE)->GetKE());
   AddIngValue("Qsum", n->GetMassExcess());
   AddIngValue("Msum", 1);

}

//________________________________________________________________
void KVCaloBase::SumUp()
{
   // protected method
   // Appel� par Calculate pour mettre � jour les diff�rents ingr�dients
   // de la calorim�trie :
   //
   // Trois modes de sommes:
   //------------------
   //
   // d�termination de l exc�s de masse de la source recontruite, dernier ingr�dient de l'�quation :
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
   //AddIngValue("Zsum",n->GetZ());
   AddIngValue("Asum", mult);
   AddIngValue("Eksum", mult * mke);
   AddIngValue("Qsum", mult * nn.GetMassExcess(0, 1));
   AddIngValue("Msum", mult);

}
//________________________________________________________________
Bool_t   KVCaloBase::Calculate(void)
{
   //R�alisation de la calorim�trie
   //Calcul de l'�nergie d'excitation
   //appel de SumUp()
   //
   // R�solution de l'�quation
   // Exci + Qini  = \Sigma Ek + \Sigma Q
   //    -> Exci = \Sigma Ek + \Sigma Q - Qini
   //
   //

   if (!kIsModified) return kTRUE;
   kIsModified = kFALSE;
   // premier calcul depuis le dernier remplissage par Fill
   SumUp();

   ComputeExcitationEnergy();
   return kTRUE;
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
   // le calcul n'est alors pas poursuivi, la m�thode Calculate() retournera kFALSE
   // la cause peut �tre discrimin�e en appelant la m�thode GetValue("RootStatus")
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
      } else {
         Double_t racDelta = TMath::Sqrt(Delta);
         x1 = (-1.*bb + racDelta) / (2.*aa);
         x2 = (-1.*(bb + racDelta)) / (2.*aa);
         kroot_status = 0;
         if (x1 > x2)  {
            kracine_max = x1;
            kracine_min = x2;
         } else        {
            kracine_max = x2;
            kracine_min = x1;
         }
      }
   } else {
      if (bb != 0) {
         kroot_status = 1;
         kracine_max = kracine_min = -1.*cc / bb;
      } else {
         kroot_status = -2;
         kracine_max = kracine_min = 0;
         SetIngValue("RootStatus", kroot_status);
      }
   }
   if (kroot_status < 0) {
      SetIngValue("RootStatus", kroot_status);
      return kFALSE;
   } else {
      return kTRUE;
   }

}
