//Created by KVClassFactory on Thu Jan 27 17:23:54 2011
//Author: Eric Bonnet,

#include "KVLifeTime.h"
#include "Riostream.h"
ClassImp(KVLifeTime)

//_________________________________
KVLifeTime::KVLifeTime(const Char_t* name): KVNuclData(name, "s")
{
   // Default constructor
   init();

}


//_________________________________
KVLifeTime::KVLifeTime(): KVNuclData()
{
   // Default constructor
   init();

}

//_________________________________
KVLifeTime::~KVLifeTime()
{
   // Destructor
}

//_________________________________
void KVLifeTime::init()
{

   SetResonance(kFALSE);

}
