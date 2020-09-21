//Created by KVClassFactory on Thu Jan 27 17:23:54 2011
//Author: Eric Bonnet,

#include "KVSpinParity.h"
#include "Riostream.h"
ClassImp(KVSpinParity)


//_________________________________
KVSpinParity::KVSpinParity(const Char_t* name): KVNuclData(name, "s")
{
   // Default constructor
   init();

}

//_________________________________
KVSpinParity::KVSpinParity(): KVNuclData()
{
   // Default constructor
   init();

}

//_________________________________
KVSpinParity::~KVSpinParity()
{
   // Destructor
}

//_________________________________
void KVSpinParity::init()
{

   SetMultiple(kFALSE);

}
