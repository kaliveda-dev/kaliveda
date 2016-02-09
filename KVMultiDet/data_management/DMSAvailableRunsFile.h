//Created by KVClassFactory on Mon Jul 20 11:33:43 2009
//Author: John Frankland

#ifndef __DMSAVAILABLERUNSFILE_H
#define __DMSAVAILABLERUNSFILE_H

#include "KVAvailableRunsFile.h"

class DMSAvailableRunsFile : public KVAvailableRunsFile {

public:
   DMSAvailableRunsFile();
   DMSAvailableRunsFile(const Char_t*, KVDataSet*);
   virtual ~DMSAvailableRunsFile();

   virtual void Update(Bool_t no_existing_file = kFALSE);

   ClassDef(DMSAvailableRunsFile, 1) //Handles list of available runs using Data Management Systems
};

#endif
