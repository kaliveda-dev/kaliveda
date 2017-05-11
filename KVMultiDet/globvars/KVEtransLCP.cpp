#include "KVEtransLCP.h"
//Author: John Frankland

ClassImp(KVEtransLCP)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the sum of the tranverse energies of LCP
// (particles with Z < 3)
//   in the reference frame chosen by the user with KVVarGlob::SetFrame().
//   Default frame is laboratory.

//_________________________________________________________________
KVEtransLCP::KVEtransLCP()
{
   //Default ctor
}

//_________________________________________________________________
KVEtransLCP::KVEtransLCP(const Char_t* name): KVEtrans(name)
{
   //ctor with name
}



//_________________________________________________________________
KVEtransLCP::~KVEtransLCP(void)
{
//
// Destructeur
//
}

//_________________________________________________________________
void KVEtransLCP::Fill(KVNucleus* c)
{
   //Add particle's transverse kinetic energy in frame chosen with SetFrame()
   //to the sum of transverse energies if Z<3
   if (c->GetZ() < 3) KVEtrans::Fill(c);
}
