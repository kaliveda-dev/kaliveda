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
#include "KVZtotAv.h"

ClassImp(KVZtotAv)
//////////////////////////////////////////////////////////////////////////////////
//  Global variable returning the sum of the fragments charges
//  Ztot for fragments with a parallel component of the velocity Vz > 0
//  in the reference frame chosen by the user with KVVarGlob::SetFrame().
//  The default frame is the "CM" frame.
//
// Look at KVVarGlob class to have an example of use.
//
//_________________________________________________________________
KVZtotAv::KVZtotAv(void): KVZtot()
{
//
// Createur par default
//
   SetFrame("CM");

}

//_________________________________________________________________
KVZtotAv::KVZtotAv(const char* nom): KVZtot(nom)
{
//
// Constructeur avec un nom
//
   SetFrame("CM");
}

//_________________________________________________________________
KVZtotAv::~KVZtotAv(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZtotAv
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
}


//_________________________________________________________________
void KVZtotAv::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   if (c->GetFrame(fFrame.Data(), kFALSE)->GetVpar() > 0) KVZtot::Fill(c);
}
