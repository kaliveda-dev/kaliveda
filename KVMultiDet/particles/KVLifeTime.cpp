//Created by KVClassFactory on Thu Jan 27 17:23:54 2011
//Author: Eric Bonnet,

#include "KVLifeTime.h"
#include "Riostream.h"
ClassImp(KVLifeTime)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVLifeTime</h2>
<h4>Simple class for store life time information of nucleus</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//_________________________________
KVLifeTime::KVLifeTime(const Char_t* name): KVNuclData(name, "s")
{
   // Default constructor
   init();

}

KVLifeTime::KVLifeTime(const KVLifeTime& o) : KVNuclData()
{
   // Copy ctor
   o.Copy(*this);
}

void KVLifeTime::Copy(TObject& object) const
{
   KVNuclData::Copy(object);
   KVLifeTime& lt = (KVLifeTime&)object;
   lt.SetResonance(IsAResonance());
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
