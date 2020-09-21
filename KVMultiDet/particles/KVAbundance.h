//Created by KVClassFactory on Wed Feb 23 16:09:53 2011
//Author: bonnet

#ifndef __KVABUNDANCE_H
#define __KVABUNDANCE_H

#include "KVNuclData.h"

/**
\class KVAbundance
\brief Value of the relative abundance of isotopes
\ingroup NucProp
*/

class KVAbundance : public KVNuclData {

private:
   void init();
public:
   KVAbundance();
   KVAbundance(const Char_t* name);
   virtual ~KVAbundance();

   ClassDef(KVAbundance, 1) //Value of the relative abundance
};

#endif
