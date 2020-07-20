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
<h4>Calorimetry of hot nuclei</h4>
<!-- */
// --> END_HTML
// Inherits from KVVarGlob adding two KVNameValueList to handle the ingredients/parameters.
// These lists are accessible via GetList(Option_t* option = "ing" or "par")
//
// ## Principle
// KVCaloBase sums the Z (Zsum), A (Asum), Ek (Eksum) and Q (Qsum) of the considered nuclei (method Fill(KVNucleus* ))
// These ingredients allow to calculate the excitation energy using the following relation:
//
//~~~~~~~~~~~~~~~~~~~~~~~
// Exci + Qini  = Eksum + Qsum -> Exci = Eksum + Qsum - Qini
//~~~~~~~~~~~~~~~~~~~~~~~
//
// __N.B.__ If method KVVarGlob::SetFrame(const Char_t* ) is called, kinetic energies of nuclei
//  are taken in that frame
//
//
// ### Example of use
//
//~~~~~~~~~~~~~~~~~~~~~
//KVNucleus alpha(2,4,10); //definition of nuclei
//KVNucleus triton(1,3);
//KVNucleus azote(7,16,40);
//
//KVCaloBase ca;
//
//ca.Fill(&alpha);   //filling the variable
//ca.Fill(&triton);
//ca.Fill(&azote);
//
//ca.Calculate();
//ca.Print("ing");   // print ingredients
//
//Ingredients, 7 stored:
//0 | Zsum | 10.00000   Sum of charges
//1 | Asum | 23.00000   Sum of masses
//2 | Eksum | 50.0000   Sum of kinetic energies (MeV)
//3 | Qsum | 23.05840   Sum of mass excess (MeV)
//4 | Msum | 3.000000   Multiplicity
//5 | Qini | -5.15400   Mass Excess of the initial state (reconstructed source)
//6 | Exci | 78.21240   Excitation energy (MeV)
//
//ca.GetValue(0);  // return value "Zsum"
//10.0000
//ca.GetValue("Exci")
//78.21240
//~~~~~~~~~~~~~~~~~~~~~
//
// __N.B.__  You *must* call method Calculate() before trying to use any of the results of KVCaloBase
//   This method returns kTRUE if the calculation succeeded
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
   //retourne la KVNameValueList ou sont enregistres les ingredients (option=="ing")
   //ou les parametres (option=="par")
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
Double_t KVCaloBase::getvalue_int(Int_t i) const
{
   // derived method
   // protected method
   // On retourne la ieme valeur du tableau
   // si i est superieur au nbre de variables definies dans ingredient_list
   // retourne la valeur par defaut (ie 0)
   // appel a la methode Calculate pour mettre a jour
   // les variables avant d effectuer le retour

   if (i < nvl_ing->GetNpar()) {
      return GetIngValue(i);
   }
   return 0;
}

//_________________________________________________________________
Int_t KVCaloBase::GetNameIndex(const Char_t* name) const
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
}

//_________________________________________________________________
std::vector<Double_t> KVCaloBase::GetValuePtr(void) const
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
   nvl_ing = new KVNameValueList();
   nvl_ing->SetName("Ingredients");
   nvl_par = new KVNameValueList();
   nvl_par->SetName("Parameters");

   kIsModified = kTRUE;

}

//________________________________________________________________
Double_t KVCaloBase::GetIngValue(KVString name) const
{
   //return the value of a name given ingredient
   //if it is not defined return 0
   if (!nvl_ing->HasParameter(name.Data())) return 0;
   return nvl_ing->GetDoubleValue(name.Data());
}
//________________________________________________________________
Double_t KVCaloBase::GetIngValue(Int_t idx) const
{
   // protected method,
   //return the value of a index given ingredient
   return nvl_ing->GetDoubleValue(idx);
}
//________________________________________________________________
void KVCaloBase::SetIngValue(KVString name, Double_t value) const
{
   // protected method,
   //set the value a name given ingredient
   nvl_ing->SetValue(name.Data(), value);
}
//________________________________________________________________
void KVCaloBase::AddIngValue(KVString name, Double_t value) const
{
   // protected method,
   //increment the value of a name given ingredient
   //if it is not defined, it's created
   Double_t before = GetIngValue(name);
   before += value;
   SetIngValue(name, before);
}
//________________________________________________________________
Bool_t KVCaloBase::HasParameter(KVString name) const
{
   // protected method,
   //Check if a given parameter is defined
   return nvl_par->HasParameter(name.Data());
}
//________________________________________________________________
Double_t KVCaloBase::GetParValue(KVString name) const
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
   // Pour l'energie cinetique, si l'utilisateur a utilise en amont
   // la methode KVVarGlob::SetFrame(const Char_t*), c'est dans ce repere que les energies sont sommees
   // (a condition que chaque KVNucleus possede le repere avec un nom identique)
   //
   // somme simple sur les A, Z, Ek, Q sans distinction du type de particules

   kIsModified = kTRUE;
   AddIngValue("Zsum", n->GetZ());
   AddIngValue("Asum", n->GetA());
   AddIngValue("Eksum", n->GetFrame(GetFrame(), kFALSE)->GetKE());
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
   //AddIngValue("Zsum",n->GetZ());
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
