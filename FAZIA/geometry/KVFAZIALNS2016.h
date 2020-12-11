//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#ifndef __KVFAZIALNS2016_H
#define __KVFAZIALNS2016_H

#include "KVFAZIA.h"

/**
\class KVFAZIALNS2016
\brief FAZIA set-up for LNS 2016
\ingroup FAZIAGeo
*/

class KVFAZIALNS2016 : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

   void RutherfordTelescope();
   void DefineStructureFormats(KVGeoImport& g)
   {
      g.SetStructureNameFormat("RUTH", "$type");
   }
   virtual void SetNameOfDetectors(KVEnv& env);

public:
   KVFAZIALNS2016();
   virtual ~KVFAZIALNS2016();

   ClassDef(KVFAZIALNS2016, 1) //Description of the FAZIA set up
};

#endif
