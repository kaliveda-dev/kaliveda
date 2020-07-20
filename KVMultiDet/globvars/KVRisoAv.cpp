//
//Author: Daniel Cussol
//
// 17/02/2004
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Elle retoune la somme des charges de l'evenement
//

#include "KVRisoAv.h"

ClassImp(KVRisoAv)
//////////////////////////////////////////////////////////////////////////////////
//  Similar to KVRiso but the ratio is computed for particles and fragments with
// the parallel component of their velocity Vz > 0 in the reference frame chosen
// by the user with KVVarGlob::SetFrame().
//  The default frame is the "CM" frame.
//

KVRisoAv::KVRisoAv(void): KVRiso()
{
//
// Createur par default
//

   SetFrame("CM");
}

//_________________________________________________________________
KVRisoAv::KVRisoAv(const char* nom): KVRiso(nom)
{
//
// Constructeur avec un nom
//
}

//_________________________________________________________________
KVRisoAv::~KVRisoAv(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVRisoAv
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
}


//_________________________________________________________________
void KVRisoAv::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   if (c->GetFrame(GetFrame(), kFALSE)->GetVpar() > 0) KVRiso::Fill(c);
}
