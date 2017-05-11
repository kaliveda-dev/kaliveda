//
//Author: Daniel Cussol
//
// 17/02/2004
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Elle retoune la somme des charges de l'evenement
//

#include "KVZboundAv.h"

ClassImp(KVZboundAv)
//////////////////////////////////////////////////////////////////////////////////
//  Similar to KVZbound but the sum is computed for fragments and particles with a
// positive parallel  velocity, Vz>0, in the reference frame defined by a call to SetFrame()
// ("CM" frame by default).
//
// ====> Example: compute the sum of the particles and fragment with a charge
// greater than or equal to 5 and Vz > Vcm.
//
// ...
// KVZboundAv *zb5av=new KVZboundAv("zbound5Av");
// zb5av->SetFrame("CM");
// zb5av->SetZmin(5);
//
// KVGVList *gvlist=new KVGVList(); // (or KVGVList *gvlist=GetGVList(); if in a KVSelector)
// gvlist->Add(zb5av);
// ...
//
// Look at KVVarGlob class to have an example of use.
//
//_________________________________________________________________
KVZboundAv::KVZboundAv(void): KVZbound()
{
//
// Createur par default
//

   SetFrame("CM");
}

//_________________________________________________________________
KVZboundAv::KVZboundAv(const char* nom): KVZbound(nom)
{
//
// Constructeur avec un nom
//
}

//_________________________________________________________________
KVZboundAv::~KVZboundAv(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZboundAv
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
}


//_________________________________________________________________
void KVZboundAv::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   if (c->GetFrame(fFrame.Data(), kFALSE)->GetVpar() > 0 && c->GetZ() >= zmin)
      FillVar(c->GetZ());
}
