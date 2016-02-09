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
#include "KVMultIMFAv.h"

ClassImp(KVMultIMFAv)
//////////////////////////////////////////////////////////////////////////////////
//  Similar to KVMultIMF but the multiplicity is computed for fragments with
// Vz > 0 in the reference frame chosen by the user with KVVarGlob::SetFrame().
//  The default frame is the "CM" frame.
//
// Look at KVVarGlob class to have an example of use.
//
//_________________________________________________________________
KVMultIMFAv::KVMultIMFAv(void): KVMultIMF()
{
//
// Createur par default
//

   SetFrame("CM");
}

//_________________________________________________________________
KVMultIMFAv::KVMultIMFAv(char* nom): KVMultIMF(nom)
{
//
// Constructeur avec un nom
//
}

//_________________________________________________________________
KVMultIMFAv::~KVMultIMFAv(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVMultIMFAv
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
}


//_________________________________________________________________
void KVMultIMFAv::Fill(KVNucleus* c)
{
   //The multiplicity is incremented if the projection of the particle's velocity
   //along the 'z'-direction in the currently defined frame is >0 and if the Z of
   //nucleus *c is >=zmin
   if (c->GetFrame(fFrame.Data())->GetVpar() > 0) KVMultIMF::Fill(c);
}
