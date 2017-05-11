/*
$Id: KVZBoundMean.cpp,v 1.1 2009/04/07 12:04:11 franklan Exp $
$Revision: 1.1 $
$Date: 2009/04/07 12:04:11 $
*/

//Created by KVClassFactory on Mon Apr  6 11:15:08 2009
//Author: Francis Gagnon-Moisan

#include "KVZBoundMean.h"

ClassImp(KVZBoundMean)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVZBoundMean</h2>
<h4>Global variable returning the mean value and the standard deviation of the charge
of fragments in a selected range of Z</h4>
Example of how to use in your analysis class derived from <a href="KVSelector.html">KVSelector</a>:<br>
<code>
MyAnalysis::InitAnalysis()
{
   ...
   KVVarGlob *vg = AddGV("KVZBoundMean", "zbm_lcp");
   vg->SetParameter("Zmax", 3); // calculate mean charge of nuclei with Z<=3

   vg = AddGV("KVZBoundMean", "zbm_frag");
   vg->SetParameter("Zmin", 5); // calculate mean charge of nuclei with Z>=5
   ...
}

MyAnalysis::Analysis()
{
   ...
   Double_t zbm_lcp = GetGV("zbm_lcp")->GetValue();
   Double_t zbm_frag = GetGV("zbm_frag")->GetValue();
   ...
}
</code>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Int_t KVZBoundMean::nb = 0;
Int_t KVZBoundMean::nb_crea = 0;
Int_t KVZBoundMean::nb_dest = 0;


KVZBoundMean::KVZBoundMean(void): KVVarGlobMean()
{
//
// Default constructor (may not be modified)
//
   init_KVZBoundMean();
   SetName("KVZBoundMean");
   SetTitle("A KVZBoundMean");
}

//_________________________________________________________________
KVZBoundMean::KVZBoundMean(const Char_t* nom): KVVarGlobMean(nom)
{
//
// Constructeur avec un nom (may not be modified)
//
   init_KVZBoundMean();
}

//_________________________________________________________________
KVZBoundMean::KVZBoundMean(const KVZBoundMean& a): KVVarGlobMean()
{
//
// Copy constructor (may not be modified)
//
   init_KVZBoundMean();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZBoundMean&)a).Copy(*this);
#endif
}

//_________________________________________________________________
KVZBoundMean::~KVZBoundMean(void)
{
//
// Destructeur
//
   nb--;

   nb_dest++;

}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVZBoundMean::Copy(TObject& a) const
#else
void KVZBoundMean::Copy(TObject& a)
#endif
{
// Copy method
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.

   KVVarGlobMean::Copy(a);
   // To copy a specific field, do as follows:
   //
   //((KVZBoundMean &)a).field=field;
   //
   // If setters and getters are available, proceed as follows
   //
   //((KVZBoundMean &)a).SetField(GetField());
   //

}

//_________________________________________________________________
KVZBoundMean& KVZBoundMean::operator = (const KVZBoundMean& a)
{
//
// Operator = (may not be modified)
//
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZBoundMean&)a).Copy(*this);
#endif
   return *this;
}


//________________________________________________________________
void KVZBoundMean::init_KVZBoundMean()
{
   // PROTECTED method
   // Private initialisation method called by all constructors.
   // All member initialisations should be done here.

   fType = KVVarGlob::kOneBody; // this is a 1-body variable

   nb++;
   nb_crea++;

}

//________________________________________________________________
void KVZBoundMean::Fill(KVNucleus* n)
{
   // Calculation of contribution to 1-body global variable of nucleus n1
   //
   // Use the FillVar(v,w) method to increment the quantity of the global variable.
   // The value, v, and the weight, w, are to be evaluated from the properties of the
   // KVNucleus passed as argument. For example, to evaluate the mean parallel velocity
   // weighted by the chargeof the nucleus, you may proceed as follows:
   //
   // FillVar(n1->GetV().Z(), n1->GetZ());

   if (IsParameterSet("Zmax")) {
      if (n->GetZ() <= GetParameter("Zmax")) FillVar(n->GetZ());
   } else if (IsParameterSet("Zmin")) {
      if (n->GetZ() >= GetParameter("Zmin")) FillVar(n->GetZ());
   } else FillVar(n->GetZ());

}
