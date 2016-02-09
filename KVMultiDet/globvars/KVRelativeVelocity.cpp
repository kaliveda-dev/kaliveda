/*
$Id: KVRelativeVelocity.cpp,v 1.3 2007/11/29 13:16:43 franklan Exp $
$Revision: 1.3 $
$Date: 2007/11/29 13:16:43 $
*/

//Created by KVClassFactory on Wed Nov 28 11:05:04 2007
//Author: Eric Bonnet

#include "KVRelativeVelocity.h"

ClassImp(KVRelativeVelocity)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVRelativeVelocity</h2>
<h4>calculate for a given group of particles the mean relative velocity</h4>
Sum i>j |Vij| / Sum i>j
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//_________________________________________________________________
void KVRelativeVelocity::init_KVRelativeVelocity(void)
{

   fType = KVVarGlob::kOneBody;
   heaviest = 0;
//
// Initialisation of the fields of the KVRelativeVelocity class
// This private method is called by the constructors only.
//
}

//_________________________________________________________________
KVRelativeVelocity::KVRelativeVelocity(void): KVVarGlobMean()
{
//
// Default constructor (may not be modified)
//
   init_KVRelativeVelocity();
   SetName("KVRelativeVelocity");
   SetTitle("A KVRelativeVelocity");
}

//_________________________________________________________________
KVRelativeVelocity::KVRelativeVelocity(Char_t* nom): KVVarGlobMean(nom)
{
//
// Constructeur avec un nom (may not be modified)
//
   init_KVRelativeVelocity();
}

//_________________________________________________________________
KVRelativeVelocity::KVRelativeVelocity(const KVRelativeVelocity& a): KVVarGlobMean()
{
//
// Copy constructor (may not be modified)
//
   init_KVRelativeVelocity();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVRelativeVelocity&)a).Copy(*this);
#endif
}

//_________________________________________________________________
KVRelativeVelocity::~KVRelativeVelocity(void)
{
//
// Destructeur
//
   if (heaviest) {
      heaviest->Clear("nodelete");
      delete heaviest;
      heaviest = 0;
   }
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVRelativeVelocity::Copy(TObject& a) const
#else
void KVRelativeVelocity::Copy(TObject& a)
#endif
{
// Copy method
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.

   KVVarGlobMean::Copy(a);
   // To copy a specific field, do as follows:
   //
   //((KVRelativeVelocity &)a).field=field;
   //
   // If setters and getters are available, proceed as follows
   //
   //((KVRelativeVelocity &)a).SetField(GetField());
   //
   //copy list of fragments if it exists
   if (heaviest) {
      KVNucleus* tmp;
      TIter next(heaviest);
      while ((tmp = (KVNucleus*) next()))
         ((KVRelativeVelocity&) a).Fill(tmp);
   }

}

//_________________________________________________________________
KVRelativeVelocity& KVRelativeVelocity::operator = (const KVRelativeVelocity& a)
{
//
// Operator = (may not be modified)
//
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVRelativeVelocity&)a).Copy(*this);
#endif
   return *this;
}


//_________________________________________________________________
void KVRelativeVelocity::Fill(KVNucleus* cc)
{

//
// Filling method
//
//
// Use the FillVar(val) or the FillVar(val,w) method to increment the quantity
// of the global variable.
// The weight w is evaluated from the properties of the KVNucleus passed as an
// argument.
//
// For example, to evaluate the mean charge of all fragments,
// you may proceed as follows:
//
// FillVar(c->GetZ());
//
// Another example: to evaluate the mean parallel velocity weighted by the charge
// of the nucleus:
//
// FillVar(c->GetV().Z(),c->GetZ());
//
   if (!heaviest) {
      heaviest = new TList;
      heaviest->SetOwner(kFALSE);
      heaviest->Add(cc);
   } else {
      for (Int_t ii = 0; ii < heaviest->GetEntries(); ii += 1) {
         TVector3 ww = cc->BoostVector() - ((KVNucleus*)heaviest->At(ii))->BoostVector();
         FillVar(ww.Mag());
      }
      heaviest->Add(cc);
   }

}
//_________________________________________________________________
void KVRelativeVelocity::Reset(void)
{
//
// Remise a Zero de la variable et du pointeur
//
   KVVarGlobMean::Reset();
   if (heaviest) {
      heaviest->Clear("nodelete");
   }

}

//_________________________________________________________________
void KVRelativeVelocity::Init(void)
{
//
// Remise a Zero de la variable et du pointeur
//
   KVVarGlobMean::Init();
   if (heaviest) {
      heaviest->SetOwner(kFALSE);
      heaviest->Clear("nodelete");
   }

}
