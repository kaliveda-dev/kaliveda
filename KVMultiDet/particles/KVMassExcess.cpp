//Created by KVClassFactory on Thu Jan 27 17:23:54 2011
//Author: Eric Bonnet,,,

#include "KVMassExcess.h"
#include "Riostream.h"

ClassImp(KVMassExcess)

//_________________________________
KVMassExcess::KVMassExcess(const Char_t* name): KVNuclData(name, "MeV")
{
   // Default constructor
   init();

}

//_________________________________
KVMassExcess::KVMassExcess(): KVNuclData()
{
   // Default constructor
   init();
   fUnits = "MeV";

}


//_________________________________
KVMassExcess::~KVMassExcess()
{
   // Destructor
}



