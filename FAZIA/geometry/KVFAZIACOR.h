//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#ifndef __KVFAZIACOR_H
#define __KVFAZIACOR_H

#include "KVFAZIA.h"
/**
  \class KVFAZIACOR
  \brief Description of FAZIA geometry for FAZIACOR experiment
  \ingroup FAZIAGeo
 */
class KVFAZIACOR : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

   void RutherfordTelescope();
   void DefineStructureFormats(KVGeoImport& g)
   {
      g.SetStructureNameFormat("RUTH", "$type");
   }
   virtual void SetNameOfDetectors(KVEnv& env);

public:
   KVFAZIACOR();
   virtual ~KVFAZIACOR();

   ClassDef(KVFAZIACOR, 1) //Description of the FAZIA set up
};

#endif
