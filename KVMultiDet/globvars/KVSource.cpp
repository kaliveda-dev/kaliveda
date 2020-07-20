/*
$Id: KVSource.cpp,v 1.3 2007/06/27 14:53:12 franklan Exp $
$Revision: 1.3 $
$Date: 2007/06/27 14:53:12 $
*/

//Created by KVClassFactory on Wed Jul  5 17:37:01 2006
//Author: Daniel CUSSOL

#include "KVSource.h"

ClassImp(KVSource)

////////////////////////////////////////////////////////////////////////////////
// Simple source reconstruction global variable
//
// It takes all nuclei selected with SetSelection(KVParticleCondition&) and adds them
// together (charge, mass, momentum, energy) in order to calculate
// a 'source' nucleus, for which we can obtain various quantities (see below).
// The reference frame from which particle momenta are taken can be set with SetFrame("..").
// Each nucleus which is included in the source definition is added
// to a group with the same name as this variable. This can be used for further analysis
// of the particles (by testing KVParticle::BelongsToGroup("...") method).
//
// Index     Name       Meaning
//-----------------------------------------------------
//   0        Ex        Excitation Energy
//   1        Z         Z source
//   2        A         A source
//   3        Vz        z component of velocity
//   4        Vx        x component of velocity
//   5        Vy        y component of velocity
//   6        Theta     polar angle relative to the beam
//   7        Mult      number of nuclei added to source
//
// These quantities can be obtained either by name:
//
//      Double_t estar = GetValue("Ex");
//
// or by index:
//
//      Double_t estar = GetValue(0);
//
// or in an array:
//
//     Double_t *vals = GetValuePtr();
//     Double_t estar = vals[0]
//
////////////////////////////////////////////////////////////////////////////////

//_________________________________________________________________
void KVSource::init_KVSource(void)
{
   // Initialisation called by ctor.
   // Set up correspondance between variable names and index.

   SetNameIndex("Ex", 0);
   SetNameIndex("Z", 1);
   SetNameIndex("A", 2);
   SetNameIndex("Vz", 3);
   SetNameIndex("Vx", 4);
   SetNameIndex("Vy", 5);
   SetNameIndex("Theta", 6);
   SetNameIndex("Mult", 7);
   fMult = 0;
}


//_________________________________________________________________
KVSource::KVSource(void): KVVarGlob()
{
//
// Createur par default
//
   init_KVSource();
   SetName("KVSource");
   SetTitle("A KVSource");
}

//_________________________________________________________________
KVSource::KVSource(const Char_t* nom): KVVarGlob(nom)
{
//
// Constructeur avec un nom
//
   init_KVSource();
}

//_________________________________________________________________
KVSource::KVSource(const KVSource& a): KVVarGlob()
{
//
// Contructeur par Copy
//
   init_KVSource();
   a.Copy(*this);
}

//_________________________________________________________________
KVSource::~KVSource(void)
{
//
// Destructeur
//
}

//_________________________________________________________________
void KVSource::Copy(TObject& a) const
{
//
// Methode de Copy
//
   KVVarGlob::Copy(a);
}


//_________________________________________________________________
void KVSource::Init(void)
{
// methode d'initialisation des
// variables internes
   fSource.Clear();
   fMult = 0;
}

//_________________________________________________________________
void KVSource::Reset(void)
{
// Remise a zero avant le
// traitement d'un evenement
   fSource.Clear();
   fMult = 0;
}

//_________________________________________________________________

void KVSource::Fill(KVNucleus* c)
{
   //Add the particle's contribution to global variable.

   fSource += (*((KVNucleus*)c->GetFrame(GetFrame(), kFALSE)));
   c->AddGroup(GetName());
   fMult++;
}

//_________________________________________________________________
Double_t KVSource::getvalue_void() const
{
   // returns the excitation energy of the source
   return fSource.GetExcitEnergy();
}

//_________________________________________________________________
Double_t KVSource::getvalue_int(Int_t i) const
{
// returns the i-th value
//
// Index  Meaning
//--------------------------------------
// 0    E*
// 1    Zsource
// 2    Asource
// 3    Vz
// 4    Vx
// 5    Vy
// 6    Theta
// 7    Mult
//
//
   Double_t rval = 0;
   switch (i) {
      case 0:
         rval = fSource.GetExcitEnergy();
         break;

      case 1:
         rval = fSource.GetZ();
         break;

      case 2:
         rval = fSource.GetA();
         break;

      case 3:
         rval = fSource.GetV().Z();
         break;

      case 4:
         rval = fSource.GetV().X();
         break;

      case 5:
         rval = fSource.GetV().Y();
         break;

      case 6:
         rval = fSource.GetTheta();
         break;

      case 7:
         rval = fMult;
         break;

      default :
         rval = 0.;
         break;
   }
   return rval;
}

Char_t KVSource::GetValueType(Int_t i) const
{
   // Returns 'D' for all floating-point values,
   // returns 'I' for source Z and A and Mult
   static Char_t val_type = 'I';
   if ((i > 0 && i < 3) || (i == 7)) return val_type;
   return KVVarGlob::GetValueType(i);
}

//_________________________________________________________________
TObject* KVSource::GetObject(void) const
{
//
// Returns address of the KVNucleus used to calculate source properties
//
   TObject* obj = (TObject*)&fSource;


   return obj;
}
