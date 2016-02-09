//Created by KVClassFactory on Wed Jan 30 12:13:12 2013
//Author: bonnet

#include "KVChargeRadius.h"

ClassImp(KVChargeRadius)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVChargeRadius</h2>
<h4>//Simple class for store charge radius information of nucleus</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVChargeRadius::KVChargeRadius()
{
   // Default constructor
   init();
   fUnits = "fm";
}

//________________________________________________________________
KVChargeRadius::KVChargeRadius(const Char_t* name) : KVNuclData(name, "fm")
{
   // Write your code here
   init();
}

KVChargeRadius::KVChargeRadius(const KVChargeRadius& o) : KVNuclData()
{
   // Copy constructor
   o.Copy(*this);
}

void KVChargeRadius::Copy(TObject& object) const
{
   KVNuclData::Copy(object);
   KVChargeRadius& cr = (KVChargeRadius&)object;
   cr.fError = fError;
}

//________________________________________________________________
KVChargeRadius::~KVChargeRadius()
{
   // Destructor
}

//_________________________________
void KVChargeRadius::init()
{

   SetError(0);


}
