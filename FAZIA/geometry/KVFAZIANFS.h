//Created by KVClassFactory on Fri Feb 26 17:11:15 2016
//Author: bonnet,,,

#ifndef __KVFAZIANFS_H
#define __KVFAZIANFS_H

#include "KVFAZIA.h"

/**
   \class KVFAZIANFS
\brief configuration of FAZIA telescopes for NFS experiments
\ingroup FAZIAGeo
*/

class KVFAZIANFS : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();
   void BuildTarget();

public:
   KVFAZIANFS();
   virtual ~KVFAZIANFS();
   void SetNameOfDetectors(KVEnv& env);

   ClassDef(KVFAZIANFS, 1) //configuration of FAZIA telescopes for NFS experiments
};

#endif
