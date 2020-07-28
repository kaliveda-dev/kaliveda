//Created by KVClassFactory on Mon Mar 29 14:58:00 2010
//Author: bonnet

#include "KVIntegerList.h"
#include "TClass.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TBuffer.h"

ClassImp(KVIntegerList)


void KVIntegerList::init()
{
//Initialisation
// La population est mise à 1
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
//Classe dérivée de TNamed, Reinitialisation de l'object
   TNamed::Clear();
   ResetRegle();
   ResetPopulation();
   fMult = 0;

}

//___________________________________________________________________________________________
void KVIntegerList::Copy(TObject& obj) const
{
//Classe dérivée de TNamed, fait une copie vers l'objet obj
   TNamed::Copy(obj);
   ((KVIntegerList&)obj).DeducePartitionFromTNamed();
   ((KVIntegerList&)obj).SetPopulation(this->GetPopulation());

}

//___________________________________________________________________________________________
void KVIntegerList::Print(Option_t*) const
{
//Classe dérivée de TNamed, Imprime la liste formattée et la population associée
   Info("Print", "%s : population %d", GetName(), GetPopulation());
}

//___________________________________________________________________________________________
void KVIntegerList::SetPartition(const Char_t* par)
{
//protected method, utilisée par le Streamer qui utilise le champ fName de la classe TNamed
//voir également KVIntegerList::DeducePartitionFromTNamed
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
         }
         else {
            toks = tamp.Tokenize("(");
            val = ((TObjString*)toks->At(0))->GetString().Atoi();
            freq = ((TObjString*)toks->At(1))->GetString().Atoi();
            delete toks;
         }
      }
      else {
         val = tamp.Atoi();
         freq = 1;
      }
      Add(val, freq);
   }

}
//___________________________________________________________________________________________
Bool_t KVIntegerList::ToBeUpdated()
{
//protected method, test si les grandeurs associées à la liste doivent etre mises a jour
   return TestBit(kHastobeComputed);

}

//___________________________________________________________________________________________
void KVIntegerList::CheckForUpdate()
{
//Methode permettant de mettre à jour la liste, elle est a appeler si l'utilisateur a appelé précédemment
//une méthode Add ou Remove
   if (ToBeUpdated())
      Update();
}

//___________________________________________________________________________________________
void KVIntegerList::Update()
{
//protected method, Mise a jour du nom de la partition (via SetName()), de sa longueur (fLength)
//Le bit kHastobeComputed es mis à 0 pour indiquer que la mise à jour a été faite
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
// // La liste est re initialisée via KVIntegerList::Clear()
// //remplie, puis mise à jour KVIntegerList::Update()
//    Clear();
//    Add(tab);
//    SetPopulation(1);
//    Update();
//
// }

//___________________________________________________________________________________________
void KVIntegerList::Fill(Int_t* tab, Int_t mult)
{
// La liste est re initialisée via KVIntegerList::Clear()
//remplie, puis mise à jour KVIntegerList::Update()

   Clear();
   Add(tab, mult);
   SetPopulation(1);
   Update();

}

//___________________________________________________________________________________________
// void KVIntegerList::Fill(Double_t* tab,Int_t mult)
// {
// // La liste est re initialisée via KVIntegerList::Clear()
// //remplie, puis mise à jour KVIntegerList::Update()
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
// // La liste est re initialisée via KVIntegerList::Clear()
// //remplie, puis mise à jour KVIntegerList::Update()
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
//Routine appelée par toutes les autres routines Add(...), c'est celle-ci qui doit etre derivée
//dans les classes filles
//Le bit kHastobeComputed es mis à 1 pour indiquer la nécéssité de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//
//Cette methode incremente la multiplicité fMult
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
//Si freq > la frequence initiale ( KVIntegerList::GetFrequency(Int_t ) )de la valeur, on la retire complétement
//Le bit kHastobeComputed es mis à 1 pour indiquer la nécéssité de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//Cette methode est appelée par les autres routines de type Remove...(...), c'est celle-ci qui doit etre derivée
//dans les classes filles
//
//Cette methode décremente la multiplicité fMult
//

   if (val > fLimiteRegle)   return kFALSE;
   else if (!Contains(val)) {
      return kFALSE;
   }
   else {
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
//Si freq > la frequence initiale ( KVIntegerList::GetFrequency(Int_t ) )de la valeur, on la retire complétement
//
   return remove_values(val, freq);

}

//___________________________________________________________________________________________
Bool_t KVIntegerList::Remove(Int_t val)
{
//On retire 1 fois la valeur val
//la methode retourne kTRUE si cette valeur etait effectivement presente, kFALSE sinon
//Le bit kHastobeComputed es mis à 1 pour indiquer la nécéssité de mettre a jour la partition
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
//Le bit kHastobeComputed es mis à 1 pour indiquer la nécéssité de mettre a jour la partition
//voir KVIntegerList::CheckForUpdate()
//

   return Remove(val, GetFrequency(val));
}

//___________________________________________________________________________________________
void KVIntegerList::ResetPopulation()
{
//Remet à 1 la population (comme dans le ctor)
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
//Incrémente la population de "pop" fois
   fPop += pop;
}

//___________________________________________________________________________________________
void KVIntegerList::SetPopulation(Int_t pop)
{
//Initialise la population à "pop"
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
//l'argument mom_max, correspond à l'ordre maximal pour lequel, les moments
//de la partition sont calculés automatiquement voir KVPartition
//L'objet cree doit etre effacé après utilisation par l'utilisateur
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
//L'objet cree doit etre effacé après utilisation par l'utilisateur
   TNamed* nm = new TNamed(GetName(), Form("%d", GetPopulation()));
   return nm;
}


//___________________________________________________________________________________________
TArrayI* KVIntegerList::CreateTArrayI()
{
//Cree un objet TArrayI de dimension fMult ( KVIntegerList::GetNbre() ) et rempli par ordre decroissant toutes les valeurs de la partition
//L'objet cree doit etre effacé après utilisation par l'utilisateur

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
//L'objet cree doit etre effacé après utilisation par l'utilisateur

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
//Classe dérivée de TNamed
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
//Retourne l'occurence de la valeur "val", si elle n'est pas présente
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
//Retourne la longueur du nom de la partition formatée GetName()
// utilisée dans la methode Compare
   return fLength;

}

//___________________________________________________________________________________________
void KVIntegerList::Streamer(TBuffer& R__b)
{
//Streamer specifique
//l'écriture dans un fichier root se fait par l'intermédiaire de la classe TNamed
//seul, le nom et la partition et sa population sont enregistrées dans le fichier
//la lecture utilise aussi le streamer de TNamed, puis il y a un appel a la routine
// protected DeducePartitionFromTNamed() qui permet de recréer complètement l'objet KVIntegerList
   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TNamed::Class(), this);
      DeducePartitionFromTNamed();
   }
   else {
      SetTitle(Form("%d", GetPopulation()));
      R__b.WriteClassBuffer(TNamed::Class(), this);
   }
}

//___________________________________________________________________________________________
void KVIntegerList::DeducePartitionFromTNamed()
{
//protected method, Methode utilisee par le Streamer
//Récupère les champs de TNamed:fName et TNamed:fTitle
//et met à jour
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

