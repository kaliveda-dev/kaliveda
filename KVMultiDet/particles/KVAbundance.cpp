//Created by KVClassFactory on Wed Feb 23 16:09:53 2011
//Author: bonnet

#include "KVAbundance.h"

ClassImp(KVAbundance)

//_________________________________
KVAbundance::KVAbundance(const Char_t* name): KVNuclData(name, "percentage")
{
   // Default constructor
   init();
}

//_________________________________
KVAbundance::KVAbundance()
{
   // Default constructor
   init();
   fUnits = "percentage";
}

//_________________________________
KVAbundance::~KVAbundance()
{
   // Destructor
}

//_________________________________
void KVAbundance::init()
{
   SetMeasured(kTRUE);
}


