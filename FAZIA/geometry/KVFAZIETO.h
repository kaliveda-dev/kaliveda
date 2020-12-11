//Created by KVClassFactory on Fri Feb 26 14:22:01 2016
//Author: bonnet,,,

#ifndef __KVFAZIETO_H
#define __KVFAZIETO_H

#include "KVFAZIA.h"

/**
   \class KVFAZIETO
\brief FAZIA 12-block demonstrator
\ingroup FAZIAGeo
*/

class KVFAZIETO : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();
   std::map<int, double>  fBlocCentreTheta; // polar angle of block centres
   std::map<int, double>  fBlocCentrePhi;  // phi angle of block centres

public:
   KVFAZIETO();
   virtual ~KVFAZIETO();

   double GetBlockCentreTheta(int i)
   {
      return fBlocCentreTheta[i];
   }
   double GetBlockCentrePhi(int i)
   {
      return fBlocCentrePhi[i];
   }

   ClassDef(KVFAZIETO, 1) //description of the FAZIA-12B demonstrator
};

#endif
