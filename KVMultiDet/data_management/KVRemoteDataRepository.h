/*
$Id: KVRemoteDataRepository.h,v 1.4 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.4 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Thu Jul 13 23:44:40 2006
//Author: John Frankland

#ifndef __KVREMOTEDATAREPOSITORY_H
#define __KVREMOTEDATAREPOSITORY_H

#include <KVDataRepository.h>

class KVUniqueNameList;

class KVRemoteDataRepository: public KVDataRepository {
protected:
   virtual KVDataSetManager* NewDataSetManager();

public:

   KVRemoteDataRepository();
   virtual ~ KVRemoteDataRepository();

   virtual Bool_t IsRemote() const
   {
      return kTRUE;
   };
   virtual Bool_t IsConnected();
   virtual Bool_t CheckSubdirExists(const Char_t* dir,
                                    const Char_t* subdir = 0);
   virtual Bool_t GetFileInfo(KVDataSet* ds,
                              const Char_t* datatype,
                              const Char_t* runfile, FileStat_t& fs);
   virtual Bool_t CheckFileStatus(KVDataSet* ds,
                                  const Char_t* datatype,
                                  const Char_t* runfile);

   virtual void CopyFileFromRepository(KVDataSet* ds,
                                       const Char_t* datatype,
                                       const Char_t* filename,
                                       const Char_t* destination);
   virtual void CopyFileToRepository(const Char_t* source,
                                     KVDataSet* ds,
                                     const Char_t* datatype,
                                     const Char_t* filename);
   virtual void MakeSubdirectory(KVDataSet* ds,
                                 const Char_t* datatype = "");
   virtual KVUniqueNameList* GetDirectoryListing(KVDataSet* ds,
         const Char_t* datatype = "");
   virtual void DeleteFile(KVDataSet* ds,
                           const Char_t* datatype,
                           const Char_t* filename, Bool_t confirm =
                              kTRUE);
   virtual const Char_t* GetFullPathToTransferFile(KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);
   virtual const Char_t* GetFullPathToOpenFile(KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);

   ClassDef(KVRemoteDataRepository, 0) //Class handling data repositories on distant machines
};

#endif
