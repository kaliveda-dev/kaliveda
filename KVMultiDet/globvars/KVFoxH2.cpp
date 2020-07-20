/*
$Id: KVFoxH2.cpp,v 1.2 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.2 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Tue Nov 27 17:47:49 2007
//Author: John Frankland

#include "KVFoxH2.h"

ClassImp(KVFoxH2)

////////////////////////////////////////////////////////////////////////////////
/* BEGIN_HTML
<h2>KVFoxH2</h2>
<h4>Event shape global variable : second Fox-Wolfram moment, H(2)</h4>
We use the following formula:
END_HTML
BEGIN_LATEX
H_{2} = \frac{\sum_{i,j} |p_{i}| |p_{j}| (3cos^{2}\theta_{rel} - 1) }{2\sum_{i,j} |p_{i}||p_{j}|}
END_LATEX
BEGIN_HTML
to calculate H(2) for particles with Z >= Zmin (set with SetZmin(); default Zmin=1).
By default, momenta in "CM" frame are used. (change with SetFrame()).
END_HTML
*/
////////////////////////////////////////////////////////////////////////////////

//_________________________________________________________________
void KVFoxH2::init_KVFoxH2(void)
{
   // Initialisation of the fields of the KVFoxH2 class
   // Set reference frame to "CM" by default

   den = 0;
   num = 0;
   SetFrame("CM");
   SetParameter("Zmin", 1);
   fType = kTwoBody;
}

//_________________________________________________________________
KVFoxH2::KVFoxH2(void): KVVarGlob1()
{
//
// Default constructor (may not be modified)
//
   init_KVFoxH2();
   SetName("KVFoxH2");
   SetTitle("Second Fox-Wolfram moment, H(2)");
}

//_________________________________________________________________
KVFoxH2::KVFoxH2(const Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom (may not be modified)
//
   init_KVFoxH2();
}

//_________________________________________________________________
KVFoxH2::KVFoxH2(const KVFoxH2& a): KVVarGlob1()
{
//
// Copy constructor (may not be modified)
//
   init_KVFoxH2();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVFoxH2&)a).Copy(*this);
#endif
}

//_________________________________________________________________
KVFoxH2::~KVFoxH2(void)
{
//
// Destructeur
//
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVFoxH2::Copy(TObject& a) const
#else
void KVFoxH2::Copy(TObject& a)
#endif
{
// Copy method
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.

   KVVarGlob1::Copy(a);
   // To copy a specific field, do as follows:
   //
   //((KVFoxH2 &)a).field=field;
   //
   // If setters and getters are available, proceed as follows
   //
   //((KVFoxH2 &)a).SetField(GetField());
   //
   ((KVFoxH2&)a).den = den;
   ((KVFoxH2&)a).num = num;

}

//_________________________________________________________________
KVFoxH2& KVFoxH2::operator = (const KVFoxH2& a)
{
//
// Operator = (may not be modified)
//
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVFoxH2&)a).Copy(*this);
#endif
   return *this;
}


//_________________________________________________________________

void KVFoxH2::Fill2(KVNucleus* n1, KVNucleus* n2)
{
   // Calculate contribution to H(2) for the given pair of nuclei, if both of them
   // have Z > Zmin (default Zmin=1; use SetParameter("Zmin", val) to change).
   // By default, momenta in "CM" frame are used. (change with SetFrame()).
   // NOTE: if one of the nuclei has zero momentum (strange, but could happen)
   // it is excluded from the sum.

   Double_t p1, p2, cos_th_rel;

   // check Z of both particles
   Int_t zmin = (Int_t)GetParameter("Zmin");
   if (n1->GetZ() < zmin) return;
   if (n2->GetZ() < zmin) return;

   if (n1 == n2) {
      p1 = p2 = n1->GetFrame(GetFrame(), kFALSE)->GetMomentum().Mag();
      cos_th_rel = 1.;
   }
   else {
      p1 = n1->GetFrame(GetFrame(), kFALSE)->GetMomentum().Mag();
      p2 = n2->GetFrame(GetFrame(), kFALSE)->GetMomentum().Mag();
      if ((p1 * p2) > 0.0) cos_th_rel = n1->GetFrame(GetFrame(), kFALSE)->GetMomentum().Dot(n2->GetFrame(GetFrame(), kFALSE)->GetMomentum()) / (p1 * p2);
      else cos_th_rel = 1.;
   }
   if ((p1 * p2) > 0.) {
      num += p1 * p2 * (3.*cos_th_rel * cos_th_rel - 1.);
      den += 2.*p1 * p2;
   }
}

//_________________________________________________________________
void KVFoxH2::Init(void)
{
   // methode d'initialisation des
   // variables Internes
   KVVarGlob1::Init();
   den = 0;
   num = 0;
}

//_________________________________________________________________
void KVFoxH2::Reset(void)
{
   // Remise a zero avant le
   // traitement d'un evenement
   KVVarGlob1::Init();
   den = 0;
   num = 0;
}


//_________________________________________________________________
Double_t KVFoxH2::getvalue_void() const
{
   // Return value of H(2) for the event.
   // NOTE: if the denominator i.e. the sum of all p_i*p_j = 0,
   // we return -99.

   if (den > 0.) return num / den;
   return -99;
}

