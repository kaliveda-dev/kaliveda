//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#ifndef __KVFAZIASYM_H
#define __KVFAZIASYM_H

#include "KVFAZIA.h"

class KVFAZIASYM : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

   void RutherfordTelescope();
   void DefineStructureFormats(KVGeoImport& g)
   {
      g.SetStructureNameFormat("RUTH", "$type");
   }
   virtual void SetNameOfDetectors(KVEnv& env);

public:
   KVFAZIASYM();
   virtual ~KVFAZIASYM();

   ClassDef(KVFAZIASYM, 1) //Description of the FAZIA set up
};

#endif
