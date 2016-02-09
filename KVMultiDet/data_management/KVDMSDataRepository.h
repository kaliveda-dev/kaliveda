//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#ifndef __KVDMSDATAREPOSITORY_H
#define __KVDMSDATAREPOSITORY_H

#include "KVDataRepository.h"
class KVDMS;

class KVDMSDataRepository : public KVDataRepository {
protected:
   KVDMS* fDMS;//! connection to Data Management System

   virtual int             Chmod(const char* file, UInt_t mode);
public:
   KVDMSDataRepository();
   virtual ~KVDMSDataRepository();

   virtual KVUniqueNameList* GetDirectoryListing(KVDataSet*,
         const Char_t* datatype = "");
   virtual Bool_t CheckSubdirExists(const Char_t* dir,
                                    const Char_t* subdir = 0);
   virtual void CopyFileFromRepository(KVDataSet*,
                                       const Char_t* datatype,
                                       const Char_t* filename,
                                       const Char_t* destination);
   virtual void CopyFileToRepository(const Char_t* source,
                                     KVDataSet*,
                                     const Char_t* datatype,
                                     const Char_t* filename);
   virtual Bool_t CheckFileStatus(KVDataSet*,
                                  const Char_t* datatype,
                                  const Char_t* runfile);
   virtual void MakeSubdirectory(KVDataSet*,
                                 const Char_t* datatype = "");
   virtual void DeleteFile(KVDataSet*,
                           const Char_t* datatype,
                           const Char_t* filename, Bool_t confirm =
                              kTRUE);
   virtual Bool_t GetFileInfo(KVDataSet*,
                              const Char_t* datatype,
                              const Char_t* runfile, FileStat_t& fs);

   ClassDef(KVDMSDataRepository, 1) //Remote data repository using Data Management Systems
};

#endif
