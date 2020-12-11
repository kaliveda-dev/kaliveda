//Created by KVClassFactory on Thu Mar 19 12:53:22 2020
//Author: John Frankland,,,

#ifndef __KVFAZIAMIDRAPEXPLORER_H
#define __KVFAZIAMIDRAPEXPLORER_H

#include "KVFAZIA.h"

/**
   \class KVFAZIAMidRapExplorer
\brief FAZIA setup with two belts of 6 blocks
\ingroup FAZIAGeo
*/

class KVFAZIAMidRapExplorer : public KVFAZIA {

   struct {
      Int_t fNblocsX; // number of blocks in X (vertical) axis
      Int_t fNblocsY; // number of blocks in Y (horizontal) axis
      Double_t fBeltDistanceTarget; // nominal distance belt-target (cm)
      Double_t fThetaBelt; // polar angle of centre belt
      Double_t fPhiBelt; // polar angle of centre belt
      Double_t fInterBlockSpacing; // spacing between blocks in belt
      Double_t fDistanceOffset; // distance offset as function of theta to keep same phi coverage
   } fBlockParams[2];

   void BuildFAZIA();
   void GetGeometryParameters();

public:
   KVFAZIAMidRapExplorer()
   {
      SetTitle(ClassName());
   }
   virtual ~KVFAZIAMidRapExplorer() {}

   void SetNameOfDetectors(KVEnv&);

   ClassDef(KVFAZIAMidRapExplorer, 1) //FAZIA setup with two belts of 6 blocks
};

#endif
