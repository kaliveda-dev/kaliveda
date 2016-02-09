//Created by KVClassFactory on Sat Jul 18 16:54:29 2009
//Author: John Frankland

#ifndef __SRBDATAREPOSITORY_H
#define __SRBDATAREPOSITORY_H

#include "KVDMSDataRepository.h"

class SRBDataRepository : public KVDMSDataRepository {
public:
   SRBDataRepository();
   virtual ~SRBDataRepository();

   ClassDef(SRBDataRepository, 1) //Remote data repository using SRB
};

#endif
