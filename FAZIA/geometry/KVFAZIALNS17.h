//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#ifndef __KVFAZIALNS17_H
#define __KVFAZIALNS17_H

#include "KVFAZIA.h"

/**
\class KVFAZIALNS17
\brief FAZIA set-up for LNS 2017
\ingroup FAZIAGeo
*/

class KVFAZIALNS17 : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

   void RutherfordTelescope();
   void DefineStructureFormats(KVGeoImport& g)
   {
      g.SetStructureNameFormat("RUTH", "$type");
   }
   virtual void SetNameOfDetectors(KVEnv& env);

public:
   KVFAZIALNS17();
   virtual ~KVFAZIALNS17();

   ClassDef(KVFAZIALNS17, 1) //Description of the FAZIA set up
};

#endif
