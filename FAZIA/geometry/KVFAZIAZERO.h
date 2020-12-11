//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#ifndef __KVFAZIAZERO_H
#define __KVFAZIAZERO_H

#include "KVFAZIA.h"

/**
   \class KVFAZIAZERO
\brief FAZIA setup, FAZIAZERO experiment
\ingroup FAZIAGeo
*/

class KVFAZIAZERO : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

   void PlasticDetectors();
   void DefineStructureFormats(KVGeoImport& g)
   {
      g.SetStructureNameFormat("RUTH", "$type");
   }
   virtual void SetNameOfDetectors(KVEnv& env);

public:
   KVFAZIAZERO();
   virtual ~KVFAZIAZERO();

   ClassDef(KVFAZIAZERO, 1) //Description of the FAZIA set up
};

#endif
