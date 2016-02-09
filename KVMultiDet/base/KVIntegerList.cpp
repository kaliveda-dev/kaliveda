//Created by KVClassFactory on Mon Mar 29 14:58:00 2010
//Author: bonnet

#include "KVIntegerList.h"
#include "TClass.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TObjString.h"

ClassImp(KVIntegerList)

////////////////////////////////////////////////////////////////////////////////
/*
BEGIN_HTML
<h2>KVIntegerList</h2>
<h4>Permet de gerer une liste de nombres entiers positifs</h4>
La classe d�rive de TNamed, le champ TNamed::fName contient le nom format� de la partition et TNamed::fTitle <br>
permet de stocker la population <br>
L'�criture/lecture dans un fichier .root se fait en utilisant le streamer de TNamed <br>
Les valeurs sont tri�es par ordre d�croissant avec gestion de l'occurence <br>
Les deux exemples suivants montrent les deux facons de remplir une liste: <br>
<h5>Exemple 1:</h5>
Les methodes de type Fill(), remplissent en une fois la liste et mets � jour les grandeurs reli�es<br>
<br>
<code>
KVIntegerList* ilist = new KVIntegerList();<br>
Int_t tab[10]={1,3,6,4,9,7,3,59,8,160};<br>
ilist->Fill(tab,10);<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 160 59 9 8 7 6 4 3(2) 1 : population 1<br>
</code>
<h5>Exemple 2:</h5>
Les methodes de type Add(), remplit successivement la liste, avant d'utiliser les grandeurs reli�es<br>
l'utilisateur doit appeler la m�thode CheckForUpdate(), pour que la mise a jour du nom soit faite<br>
par contre le nombre d'elements est mis a jour automatiquement<br>
<br>
<code>
KVIntegerList* ilist = new KVIntegerList();<br>
ilist->Add(23,6);<br>
ilist->Add(12);<br>
Int_t tab[3]={1,3,6};<br>
ilist->Add(tab,3);<br>
ilist->CheckForUpdate();<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 23(6) 12 6 3 1 : population 1<br>
ilist->Remove(23,3);<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 23(6) 12 6 3 1 : population 1       &nbsp  &nbsp    !!! pour le nom  RIEN A CHANGER<br>
<code>ilist->GetNbre();<br>
(const Int_t)7                &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp le nombre, lui est mis � jour<br>
ilist->CheckForUpdate();<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 23(3) 12 6 3 1 : population 1       &nbsp  &nbsp    OK : les modifs ont �t� prises en compte<br>
ilist->GetNbre()<br>
(const Int_t)7
</code>
END_HTML

Les methodes de type Remove(), retirent des valeurs de la liste, l� encore l'utilisateur doit appeler
la m�thode CheckForUpdate(), pour que la mise a jour soit faite
Les grandeurs disponibles sont:
- le nombre d'entiers, ie la multiplicit� GetNbre()
- le nom de la partition definie comme suit : ent1(occ) ent2 ent3(occ3), o� entre parenth�ses
sont not�es les occurences aup�rieures � 1 d'une valeur
- la population GetPopulation(), permet de g�rer un ensemble de partitions (KVPartitionManager)

*/
////////////////////////////////////////////////////////////////////////////////

//___________________________________________________________________________________________
void KVIntegerList::init()
{
//Initialisation
// La population est mise � 1
   SetPopulation(1);
   fRegle = new TArrayI(100);
   fLimiteRegle = fRegle->fN - 1;
   fMult = 0;
   fLength = 0;
}

//___________________________________________________________________________________________
KVIntegerList::KVIntegerList()
{
// Default constructor
   init();
}

//___________________________________________________________________________________________
KVIntegerList::~KVIntegerList()
{
//Destructor

   if (fRegle) delete fRegle;
   fRegle = 0;

}

//___________________________________________________________________________________________
void KVIntegerList::ResetRegle()
{
//protected method, Mise a zero de l'ensemble des valeurs
   for (Int_t ii = 0; ii < fRegle->fN; ii += 1)
      fRegle->AddAt(0, ii);

}

//___________________________________________________________________________________________
void KVIntegerList::Clear(Option_t*)
{
//Classe d�riv�e de TNamed, Reinitialisation de l'object
   TNamed::Clear();
   ResetRegle();
   ResetPopulation();
   fMult = 0;

}

//___________________________________________________________________________________________
void KVIntegerList::Copy(TObject& obj) const
{
//Classe d�riv�e de TNamed, fait une copie vers l'objet obj
   TNamed::Copy(obj);
   ((KVIntegerList&)obj).DeducePartitionFromTNamed();
   ((KVIntegerList&)obj).SetPopulation(this->GetPopulation());

}

//___________________________________________________________________________________________
void KVIntegerList::Print(Option_t*) const
{
//Classe d�riv�e de TNamed, Imprime la liste formatt�e et la population associ�e
   Info("Print", "%s : population %d", GetName(), GetPopulation());
}

//___________________________________________________________________________________________
void KVIntegerList::SetPartition(const Char_t* par)
{
//protected method, utilis�e par le Streamer qui utilise le champ fName de la classe TNamed
//voir �galement KVIntegerList::DeducePartitionFromTNamed
   KVString st(par);
   st.Begin(" ");
   TObjArray* toks = 0;
   while (!st.End()) {
      KVString tamp = st.Next();
      Int_t val;
      Int_t freq;
      if (tamp.Contains("(")) {
         if (!tamp.Contains(")")) {
            Warning("SetPartition", "%s ->pb de coherence dans les parentheses", tamp.Data());
            return;
         } else {
            toks = tamp.Tokenize("(");
            val = ((TObjString*)toks->At(0))->GetString().Atoi();
            freq = ((TObjString*)toks->At(1))->GetString().Atoi();
            delete toks;
         }
      } else {
         val = tamp.Atoi();
         freq = 1;
      }
      Add(val, freq);
   }

}
//___________________________________________________________________________________________
Bool_t KVIntegerList::ToBeUpdated()
{
//protected method, test si les grandeurs associ�es � la liste doivent etre mises a jour
   return TestBit(kHastobeComputed);

}

//___________________________________________________________________________________________
void KVIntegerList::CheckForUpdate()
{
//Methode permettant de mettre � jour la liste, elle est a appeler si l'utilisateur a appel� pr�c�demment
//une m�thode Add ou Remove
   if (ToBeUpdated())
      Update();
}

//___________________________________________________________________________________________
void KVIntegerList::Update()
{
//protected method, Mise a jour du nom de la partition (via SetName()), de sa longueur (fLength)
//Le bit kHastobeComputed es mis � 0 pour indiquer que la mise � jour a �t� faite
//
   KVString snom = "", stamp = "";
   for (Int_t ii = fLimiteRegle; ii >= 0; ii -= 1) {
      Int_t contenu = fRegle->At(ii);
      if (contenu > 0) {
         //Formattage du nom de la partition
         stamp.Form("%d", ii);
         snom += stamp;
         if (contenu > 1) {
            stamp.Form("(%d)", contenu);
            snom += stamp;
         }
         snom += " ";
      }
   }
   if (snom != "") snom.Remove(snom.Length() - 1);
   SetName(snom.Data());
   fLength = snom.Length();

   SetBit(kHastobeComputed, kFALSE);

}

//___________________________________________________________________________________________
// void KVIntegerList::Fill(TArrayI* tab)
// {
// // La liste est re initialis�e via KVIntegerList::Clear()
// //remplie, puis mise � jour KVIntegerList::Update()
//    Clear();
//    Add(tab);
//    SetPopulation(1);
//    Update();
//
// }

//___________________________________________________________________________________________
void KVIntegerList::Fill(Int_t* tab, Int_t mult)
{
// La liste est re initialis�e via KVIntegerList::Clear()
//remplie, puis mise � jour KVIntegerList::Update()

   Clear();
   Add(tab, mult);
   SetPopulation(1);
   Update();

}

//___________________________________________________________________________________________
// void KVIntegerList::Fill(Double_t* tab,Int_t mult)
// {
// // La liste est re initialis�e via KVIntegerList::Clear()
// //remplie, puis mise � jour KVIntegerList::Update()
//
//    Clear();
//    Add(tab,mult);
//    SetPopulation(1);
//    Update();
//
// }

//___________________________________________________________________________________________
// void KVIntegerList::Fill(KVEvent* evt,Option_t* opt)
// {
// // La liste est re initialis�e via KVIntegerList::Clear()
// //remplie, puis mise � jour KVIntegerList::Update()
//
//    if (!evt) return;
//    Clear();
//
//    KVNucleus* nuc=0;
//    while ( (nuc = (KVNucleus* )evt->GetNextParticle(opt)) )
//       Add(nuc->GetZ());
//    SetPopulation(1);
//    Update();
//
// }

//___________________________________________________________________________________________
void KVIntegerList::add_values(Int_t val, Int_t freq)
{
//protected method, Ajout de "freq" fois la valeur val
//Verification de la taille du tableau (fLimiteRegle) et extension si besoin
//
//Routine appel�e par toutes les autres routines Add(...), c'est celle-ci qui doit etre deriv�e
//dans les classes filles
//Le bit kHastobeComputed es mis � 1 pour indiquer la n�c�ssit� de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//
//Cette methode incremente la multiplicit� fMult
//

   if (val > fLimiteRegle) {
      fRegle->Set(val + 1);
      fLimiteRegle = val;
   }
   fMult += freq;
   fRegle->AddAt(fRegle->At(val) + freq, val);
   SetBit(kHastobeComputed, kTRUE);

}

//___________________________________________________________________________________________
void KVIntegerList::Add(Int_t val, Int_t freq)
{
//Ajout de "freq" fois la valeur val
   add_values(val, freq);

}

//___________________________________________________________________________________________
void KVIntegerList::Add(Int_t* tab, Int_t mult)
{
//Ajout d'un tableau d'entiers de "mult" valeurs
   for (Int_t ii = 0; ii < mult; ii += 1)
      Add(tab[ii], 1);
}

//___________________________________________________________________________________________
void KVIntegerList::Add(TArrayI* tab)
{
//Ajout de valeurs via un TArrayI
   Int_t mult = tab->GetSize();
   for (Int_t ii = 0; ii < mult; ii += 1)
      Add(tab->At(ii), 1);
}

//___________________________________________________________________________________________
void KVIntegerList::Add(Double_t* tab, Int_t mult)
{
//Ajout d'un tableau de "mult" valeurs, version Double_t (Conversion Double_t -> Int_t (arrondi, TMath::Nint()))
   for (Int_t ii = 0; ii < mult; ii += 1)
      Add(tab[ii], 1);
}

//___________________________________________________________________________________________
void KVIntegerList::Add(Int_t val)
{
//Ajout de la valeur val (Int_t)
   Add(val, 1);
}

//___________________________________________________________________________________________
void KVIntegerList::Add(Double_t val)
{
//Ajout de la valeur val version Double_t (Conversion Double_t -> Int_t (arrondi, TMath::Nint()))
   Add(TMath::Nint(val), 1);
}

//___________________________________________________________________________________________
void KVIntegerList::Add(Double_t val, Int_t freq)
{
//Ajout de "freq" fois la valeur val

   Add(TMath::Nint(val), freq);
}


//___________________________________________________________________________________________
Bool_t KVIntegerList::remove_values(Int_t val, Int_t freq)
{
//protected method, On retire "freq" fois la valeur val
//la methode retourne kTRUE si cette valeur etait effectivement presente, kFALSE sinon
//Si freq > la frequence initiale ( KVIntegerList::GetFrequency(Int_t ) )de la valeur, on la retire compl�tement
//Le bit kHastobeComputed es mis � 1 pour indiquer la n�c�ssit� de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//Cette methode est appel�e par les autres routines de type Remove...(...), c'est celle-ci qui doit etre deriv�e
//dans les classes filles
//
//Cette methode d�cremente la multiplicit� fMult
//

   if (val > fLimiteRegle)   return kFALSE;
   else if (!Contains(val)) {
      return kFALSE;
   } else {
      Int_t freq_rel = TMath::Min(fRegle->At(val), freq);
      fRegle->AddAt(TMath::Max(fRegle->At(val) - freq, 0), val);
      fMult -= freq_rel;
      SetBit(kHastobeComputed, kTRUE);
      return kTRUE;
   }

}

//___________________________________________________________________________________________
Bool_t KVIntegerList::Remove(Int_t val, Int_t freq)
{
//On retire "freq" fois la valeur val
//la methode retourne kTRUE si cette valeur etait effectivement presente, kFALSE sinon
//Si freq > la frequence initiale ( KVIntegerList::GetFrequency(Int_t ) )de la valeur, on la retire compl�tement
//
   return remove_values(val, freq);

}

//___________________________________________________________________________________________
Bool_t KVIntegerList::Remove(Int_t val)
{
//On retire 1 fois la valeur val
//la methode retourne kTRUE si cette valeur etait effectivement presente, kFALSE sinon
//Le bit kHastobeComputed es mis � 1 pour indiquer la n�c�ssit� de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//

   return Remove(val, 1);

}

//___________________________________________________________________________________________
Bool_t KVIntegerList::RemoveAll(Int_t val)
{
//Reture completement la valeur val
//la methode retourne kTRUE si cette valeur etait effectivement presente, kFALSE sinon
//
//Le bit kHastobeComputed es mis � 1 pour indiquer la n�c�ssit� de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//

   return Remove(val, GetFrequency(val));
}

//___________________________________________________________________________________________
void KVIntegerList::ResetPopulation()
{
//Remet � 1 la population (comme dans le ctor)
   SetPopulation(1);
}

//___________________________________________________________________________________________
Int_t KVIntegerList::GetPopulation() const
{
   return fPop;
}

//___________________________________________________________________________________________
void KVIntegerList::AddPopulation(Int_t pop)
{
//Incr�mente la population de "pop" fois
   fPop += pop;
}

//___________________________________________________________________________________________
void KVIntegerList::SetPopulation(Int_t pop)
{
//Initialise la population � "pop"
   fPop = pop;
}

//___________________________________________________________________________________________
Int_t KVIntegerList::GetNbre() const
{

   return fMult;

}
//___________________________________________________________________________________________
/*
KVPartition* KVIntegerList::CreateKVPartition(Int_t mom_max)
{
//Cree un objet KVPartition
//l'argument mom_max, correspond � l'ordre maximal pour lequel, les moments
//de la partition sont calcul�s automatiquement voir KVPartition
//L'objet cree doit etre effac� apr�s utilisation par l'utilisateur
   KVPartition* par = new KVPartition(fLimiteRegle,mom_max);
   TArrayI* tab = CreateTArrayI();
   par->Fill(tab);
   delete tab;
   return par;
}
*/
//___________________________________________________________________________________________
TNamed* KVIntegerList::CreateTNamed()
{
//Cree un objet TNamed
//le nom de la partition est le champ TNamed::fName et la population de la partition dans le champ TNamed::fTitle
//voir TNamed
//L'objet cree doit etre effac� apr�s utilisation par l'utilisateur
   TNamed* nm = new TNamed(GetName(), Form("%d", GetPopulation()));
   return nm;
}


//___________________________________________________________________________________________
TArrayI* KVIntegerList::CreateTArrayI()
{
//Cree un objet TArrayI de dimension fMult ( KVIntegerList::GetNbre() ) et rempli par ordre decroissant toutes les valeurs de la partition
//L'objet cree doit etre effac� apr�s utilisation par l'utilisateur

   TArrayI* tab = new TArrayI(GetNbre());
   Int_t mm = 0;
   for (Int_t ii = fLimiteRegle; ii >= 0; ii -= 1) {
      Int_t contenu = fRegle->At(ii);
      for (Int_t cc = 0; cc < contenu; cc += 1)
         tab->AddAt(ii, mm++);
   }
   return tab;

}

//___________________________________________________________________________________________
Int_t* KVIntegerList::CreateTableOfValues()
{
//Cree un objet tableau d'entier de dimension fMult ( KVIntegerList::GetNbre() )
//et rempli par ordre decroissant toutes les valeurs de la partition
//L'objet cree doit etre effac� apr�s utilisation par l'utilisateur

   Int_t* tab = new Int_t[GetNbre()];
   Int_t mm = 0;
   for (Int_t ii = fLimiteRegle; ii >= 0; ii -= 1) {
      Int_t contenu = fRegle->At(ii);
      for (Int_t cc = 0; cc < contenu; cc += 1)
         tab[mm++] = ii;
   }
   return tab;

}

//___________________________________________________________________________________________
Int_t KVIntegerList::Compare(const TObject* obj) const
{
//Classe d�riv�e de TNamed
//Compare deux objets de type KVIntegerList, le premier test concerne la longueur du nom (KVIntegerList::GetLengthName())
//Si elle est identique, on test caractere par caractere, les deux noms TNamed::GetName()
//Retourne 0 si les noms des deux KVIntegerList sont exactement les memes, -1 sinon;
//
   const char* s1 = this->GetName();
   const char* s2 = obj->GetName();

   Ssiz_t len1 = this->GetLengthName();

   if (len1 != ((KVIntegerList*)obj)->GetLengthName()) return -1;

   Int_t ii = 0;
   while (s1[ii] == s2[ii]) {
      if (ii == len1) return 0;
      ii += 1;
   }
   return -1;

}

//___________________________________________________________________________________________
Int_t KVIntegerList::GetFrequency(Int_t val) const
{
//Retourne l'occurence de la valeur "val", si elle n'est pas pr�sente
//dans la liste retourne -1
   return (val <= fLimiteRegle ? fRegle->At(val) : -1);

}

//___________________________________________________________________________________________
Bool_t KVIntegerList::Contains(Int_t val) const
{
//Retourne kTRUE si la valeur "val" est dans la liste, kFALSE sinon
//dans la liste retourne -1
   return (GetFrequency(val) > 0);

}

//___________________________________________________________________________________________
Ssiz_t KVIntegerList::GetLengthName() const
{
//Retourne la longueur du nom de la partition format�e GetName()
// utilis�e dans la methode Compare
   return fLength;

}

//___________________________________________________________________________________________
void KVIntegerList::Streamer(TBuffer& R__b)
{
//Streamer specifique
//l'�criture dans un fichier root se fait par l'interm�diaire de la classe TNamed
//seul, le nom et la partition et sa population sont enregistr�es dans le fichier
//la lecture utilise aussi le streamer de TNamed, puis il y a un appel a la routine
// protected DeducePartitionFromTNamed() qui permet de recr�er compl�tement l'objet KVIntegerList
   if (R__b.IsReading()) {
      TNamed::Class()->ReadBuffer(R__b, this);
      DeducePartitionFromTNamed();
   } else {
      SetTitle(Form("%d", GetPopulation()));
      TNamed::Class()->WriteBuffer(R__b, this);
   }
}

//___________________________________________________________________________________________
void KVIntegerList::DeducePartitionFromTNamed()
{
//protected method, Methode utilisee par le Streamer
//R�cup�re les champs de TNamed:fName et TNamed:fTitle
//et met � jour
   DeducePartitionFromName();
   DeducePopulationFromTitle();

}
//___________________________________________________________________________________________
void KVIntegerList::DeducePopulationFromTitle()
{
//protected method, utilisee par le Streamer

   KVString st(GetTitle());
   SetPopulation(st.Atoi());

}
//___________________________________________________________________________________________
void KVIntegerList::DeducePartitionFromName()
{
//protected method, utilisee par le Streamer

   SetPartition(GetName());

}

