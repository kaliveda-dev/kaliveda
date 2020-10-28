//Created by KVClassFactory on Mon May  2 14:00:47 2016
//Author: John Frankland,,,

#ifndef __KVFISSIONKINEMATICS_H
#define __KVFISSIONKINEMATICS_H

#include "KVBase.h"
#include "KVEvent.h"

/**
\class KVFissionKinematics
\brief Simple calculation of two-body decay
\ingroup Simulation
*/

class KVFissionKinematics : public KVBase {
   KVEvent fFission;       // calculated fission event
   KVNucleus fMother;      // mother nucleus
   KVNucleus fDaughter;    // one of the fission fragments

public:
   KVFissionKinematics(const KVNucleus& fis, Int_t Z1 = 0, Int_t A1 = 0);

   virtual ~KVFissionKinematics() {}

   const KVEvent* Generate();

   ClassDef(KVFissionKinematics, 1) //Simple calculation of two-body decay
};

#endif
