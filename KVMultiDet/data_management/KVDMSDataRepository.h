//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#ifndef __KVDMSDATAREPOSITORY_H
#define __KVDMSDATAREPOSITORY_H

#include "KVDataRepository.h"
class KVDMS;

/**
\class KVDMSDataRepository
\brief Manage remote data repository using a Data Management System
\ingroup DM
*/

class KVDMSDataRepository : public KVDataRepository {
protected:
   KVDMS* fDMS;//! connection to Data Management System

   virtual int             Chmod(const char* file, UInt_t mode);
public:
   KVDMSDataRepository();
   virtual ~KVDMSDataRepository();

   virtual KVUniqueNameList* GetDirectoryListing(const KVDataSet*,
         const Char_t* datatype = "", const Char_t* subdir = "");
   virtual Bool_t CheckSubdirExists(const Char_t* dir,
                                    const Char_t* subdir = 0);
   virtual void CopyFileFromRepository(const KVDataSet*,
                                       const Char_t* datatype,
                                       const Char_t* filename,
                                       const Char_t* destination);
   virtual int CopyFileToRepository(const Char_t* source,
                                    const KVDataSet*,
                                    const Char_t* datatype,
                                    const Char_t* filename);
   virtual Bool_t CheckFileStatus(const KVDataSet*,
                                  const Char_t* datatype,
                                  const Char_t* runfile);
   virtual void MakeSubdirectory(const KVDataSet*,
                                 const Char_t* datatype = "");
   virtual void DeleteFile(const KVDataSet*,
                           const Char_t* datatype,
                           const Char_t* filename, Bool_t confirm =
                              kTRUE);
   virtual Bool_t GetFileInfo(const KVDataSet*,
                              const Char_t* datatype,
                              const Char_t* runfile, FileStat_t& fs);

   virtual const Char_t* GetFullPathToOpenFile(const KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);

   TObject* OpenDataSetRunFile(const KVDataSet* ds, const Char_t* type, Int_t run, Option_t* opt = "");

   ClassDef(KVDMSDataRepository, 1) //Remote data repository using Data Management Systems
};

#endif
