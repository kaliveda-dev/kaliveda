//
//Author: Daniel Cussol
//
// 17/02/2004
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Elle retoune la somme des charges de l'evenement
//

#include <math.h>
#include <stdio.h>
#include "KVMultLegAv.h"

ClassImp(KVMultLegAv)
//////////////////////////////////////////////////////////////////////////////////
//  Similar to KVMultLeg but the sum is computed for fragments with
// Vz > 0 in the reference frame chosen by the user with KVVarGlob::SetFrame().
//  The default frame is the "CM" frame.
//
// Look at KVVarGlob class to have an example of use.
//
//_________________________________________________________________
KVMultLegAv::KVMultLegAv(void): KVMultLeg()
{
//
// Createur par default
//

   SetFrame("CM");
}

//_________________________________________________________________
KVMultLegAv::KVMultLegAv(const char* nom): KVMultLeg(nom)
{
//
// Constructeur avec un nom
//
}

//_________________________________________________________________
KVMultLegAv::~KVMultLegAv(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVMultLegAv
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
}


//_________________________________________________________________
void KVMultLegAv::Fill(KVNucleus* c)
{
   //The multiplicity is incremented if the projection of the particle's velocity
   //along the 'z'-direction in the currently defined frame is >0 and if the Z of
   //nucleus *c is <=zmax
   if (c->GetFrame(fFrame.Data(), kFALSE)->GetVpar() > 0) KVMultLeg::Fill(c);
}
