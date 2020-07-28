//Created by KVClassFactory on Mon Mar 29 14:58:00 2010
//Author: bonnet

#include "KVPartition.h"
#include "TMath.h"
#include "Riostream.h"
#include "TArrayI.h"

ClassImp(KVPartition)



void KVPartition::init(Int_t mommax)
{
//Initialisation
   fValues = new KVValues("Moments", mommax);

   ftab = new TArrayI(20);
   ftab_diff = new TArrayI(20);

   fMult_diff = 0;

}

//___________________________________________________________________________________________
KVPartition::KVPartition(Int_t mommax)
{
// Constructor
   init(mommax);
}

//___________________________________________________________________________________________
KVPartition::KVPartition()
{
// Default constructor
   init();
}

//___________________________________________________________________________________________
KVPartition::~KVPartition()
{
//Destructeur
   delete fValues;
   delete ftab;
   delete ftab_diff;
}


//___________________________________________________________________________________________
void KVPartition::Clear(Option_t*)
{
//Methode dérivée de KVIntegerList, Reinitialisation de l'object

   KVIntegerList::Clear();

   fValues->Reset();
   for (Int_t ii = 0; ii < ftab->fN; ii += 1)        ftab->AddAt(ii, 0);
   for (Int_t ii = 0; ii < ftab_diff->fN; ii += 1)   ftab_diff->AddAt(ii, 0);

   fMult = 0;
   fMult_diff = 0;

}

//___________________________________________________________________________________________
void KVPartition::Copy(TObject& obj) const
{
//Methode dérivée de KVIntegerList, fait une copie dans l'objet "obj"
   KVIntegerList::Copy(obj);
   ((KVPartition&)obj).Update();

}

//___________________________________________________________________________________________
void KVPartition::Update()
{
//protected method, Methode dérivée de KVIntegerList,
//Appel de KVIntegerList::Update()
//les deux TArrayI ftab et ftab_diff sont mis à jour
//un test de dimension est fait pour etendre si besoin

   if (fMult > ftab->fN)           ftab->Set(fMult);
   if (fMult_diff > ftab_diff->fN) ftab_diff->Set(fMult_diff);
   //Info("Update","fMult=%d fMult_diff=%d",fMult,fMult_diff);
   Int_t mdiff = 0, mtot = 0;
   KVString snom = "", stamp = "";
   for (Int_t ii = fLimiteRegle; ii >= 0; ii -= 1) {
      Int_t contenu = fRegle->At(ii);
      if (contenu > 0) {
         ftab_diff->AddAt(ii, mdiff++);
         for (Int_t mm = 0; mm < contenu; mm += 1) {
            ftab->AddAt(ii, mtot++);
         }

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
void KVPartition::Print(Option_t* option) const
{
//Methode dérivée de KVIntegerList, imprime les informations
//option="" -> appel uniquement de KVIntegerList::Print
//option=="Moments", imprime les moments calculés via la classe KVValues
//option=="Partition", imprime le nombre de valeurs GetMult() et le nombre de valeurs differentes (GetMultDiff())
   KVIntegerList::Print(option);
   if (!strcmp(option, "Moments")) {
      Info("Print", "Moments #Sigma Z^{ordre}");
      Info("Print", "Nombres de moments calcules %d", GetOrdreMax());
      for (Int_t mm = 0; mm <= GetOrdreMax(); mm += 1) {
         Info("Print", "Moments d'ordre %d -> %1.0lf", mm, GetMoment(mm));
      }
   }
   else if (!strcmp(option, "Partition")) {
      Info("Print", "Multiplicite %d / Nombre de valeurs differentes %d", GetMult(), GetMultDiff());
   }
   else {

   }

}

//___________________________________________________________________________________________
void KVPartition::add_values(Int_t val, Int_t freq)
{
//protected method, dérivée de KVIntegerList
//Incrementation si besoin du nombre de valeurs differentes GetMultDiff
//Remplissage de l'objet KVIntegerList::fValues, pour le calcul des moments
   if (!Contains(val)) fMult_diff += 1;
   KVIntegerList::add_values(val, freq);
   fValues->FillVar(val, freq);

}

//___________________________________________________________________________________________
Bool_t KVPartition::remove_values(Int_t val, Int_t freq)
{
//protected method, dérivée de KVIntegerList
//Decrementation si besoin du nombre de valeurs differentes GetMultDiff
//Les valeurs considerees sont également retires dans le calcul des moments, object KVIntegerList::fValues

   if (GetFrequency(val) <= freq) fMult_diff -= 1;
   fValues->FillVar(val, -1.*freq);
   return KVIntegerList::remove_values(val, freq);

}

//___________________________________________________________________________________________
Double_t  KVPartition::GetZmax(Int_t rang) const
{

//Donne le 1er, 2eme ... plus gros de la partition
//Attention notation C
// - le premier -> 0
// - le dernier -> N-1 ou N est la multiplicite

   return ((rang < GetMoment(0)) ? Double_t(GetValeur(rang)) : -1.);

}

//___________________________________________________________________________________________
Double_t  KVPartition::GetZmin(Int_t rang) const
{

//Donne le 1er, 2eme ... plus petit de la partition
//Attention notation C
// - le premier -> 0
// - le dernier -> N-1 ou N est la multiplicite

   Int_t inverse = Int_t(GetMoment(0)) - 1 - rang;
   return ((inverse < GetMoment(0)) ? Double_t(GetValeur(inverse)) : -1.);

}

//___________________________________________________________________________________________
Bool_t KVPartition::RemoveAt(Int_t rang)
{

//return kTRUE if the indicated rank is occupied
   Int_t val = Int_t(GetZmax(rang));
   if (val == -1) return kFALSE;
   return Remove(val);

}

//___________________________________________________________________________________________
Bool_t KVPartition::RemoveAllValuesAt(Int_t rang)
{

//return kTRUE if the indicated rank is occupied
   Int_t val = Int_t(GetZmax(rang));
   if (val == -1) return kFALSE;
   return RemoveAll(val);
}
