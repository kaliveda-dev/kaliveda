//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#ifndef __IRODSDATAREPOSITORY_H
#define __IRODSDATAREPOSITORY_H

#include "KVDMSDataRepository.h"

/**
\class IRODSDataRepository
\brief Remote data repository using IRODS (Integrated Rule-Oriented Data System) DMS
\ingroup DM
*/

class IRODSDataRepository : public KVDMSDataRepository {
public:
   IRODSDataRepository();
   virtual ~IRODSDataRepository();
   virtual void PrintAvailableDatasetsUpdateWarning() const;

   ClassDef(IRODSDataRepository, 1) //Remote data repository using IRODS
};

#endif
