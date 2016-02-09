/*
$Id: KVDataRepository.h,v 1.12 2007/12/11 12:45:47 franklan Exp $
$Revision: 1.12 $
$Date: 2007/12/11 12:45:47 $
*/

//Created by KVClassFactory on Wed Apr 26 16:17:37 2006
//Author: franklan

#ifndef __KVDATAREPOSITORY_H
#define __KVDATAREPOSITORY_H

#include "KVBase.h"
#include "TString.h"
#include "TSystem.h"
#include "KVAvailableRunsFile.h"
#include "TEnv.h"

class KVList;
class KVUniqueNameList;
class TFile;
class KVDataSet;
class KVDataSetManager;

class KVDataRepository: public KVBase {

protected:

   TString fLocalrootdir;
   TString fAccessprotocol;
   TString fReadprotocol;
   TString fXrootdserver;
   TString fXrootdrootdir;
   Bool_t fXRDtunnel;
   TString fXRDtunHost;
   TString fXRDtunUser;
   TString fXRDtunSpec;
   Int_t fXRDtunRetry;
   Int_t fXRDtunPort;
   TString fRfioserver;
   TString fRfiorootdir;
   TString fAccessroot;
   TString fReadroot;
   TString fTransfertype;
   TString fTransferExec;
   TString fTransferserver;
   TString fTransferuser;
   Bool_t fCanWrite; //kTRUE if new files can be created and written directly in the repository; kFALSE if they have to be written locally then copied to repository
   //used by CreateNewFile and CommitFile
   KVDataSet* fCommitDataSet;   //!
   TString fCommitDataType;     //!
   TString fCommitFileName;     //!
   KVDataSetManager* fDSM;      //handles datasets in repository
   void SetFullPath(TString& path, const Char_t* protocol);
   virtual const Char_t* GetReadProtocol(const Char_t* dataset, const Char_t* datatype);
   virtual KVDataSetManager* NewDataSetManager();

   virtual int             Chmod(const char* file, UInt_t mode);
   virtual void PrepareXRDTunnel();

   TSeqCollection*  fHelpers;          //List of helper classes for alternative file/directory access
public:
   virtual int  CopyFile(const char* f, const char* t, Bool_t overwrite = kFALSE);
   TSystem*               FindHelper(const char* path, void* dirptr = 0);
   Bool_t            HelperIsConsistentWith(TSystem* helper, const char* path, void* dirptr = 0);

   KVDataRepository();
   virtual ~ KVDataRepository();

   virtual Bool_t Init();

   virtual Bool_t CheckSubdirExists(const Char_t* dir,
                                    const Char_t* subdir = 0);
   virtual Bool_t GetFileInfo(KVDataSet* dataset,
                              const Char_t* datatype,
                              const Char_t* runfile, FileStat_t& fs);
   virtual Bool_t CheckFileStatus(KVDataSet* dataset,
                                  const Char_t* datatype,
                                  const Char_t* runfile);

   virtual const Char_t* GetFullPathToOpenFile(KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);

   virtual const Char_t* GetFullPathToTransferFile(KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);

   //Returns kTRUE for remote repositories, kFALSE for local repositories
   virtual Bool_t IsRemote() const
   {
      return kFALSE;
   };
   //Always returns kTRUE for local repositories
   virtual Bool_t IsConnected()
   {
      return kTRUE;
   };
   //Returns kTRUE if new files can be created and written directly in the repository,
   //returns kFALSE if they have to be written locally then copied to repository
   //This is set by the property [repository name].DataRepository.CanWrite
   //in the repository configuration (see $KVROOT/KVFiles/.kvrootrc)
   virtual Bool_t CanWrite() const
   {
      return fCanWrite;
   }
   virtual KVUniqueNameList* GetDirectoryListing(KVDataSet* dataset,
         const Char_t* datatype = "");

   virtual void CopyFileFromRepository(KVDataSet* dataset,
                                       const Char_t* datatype,
                                       const Char_t* filename,
                                       const Char_t* destination);
   virtual void CopyFileToRepository(const Char_t* source,
                                     KVDataSet* dataset,
                                     const Char_t* datatype,
                                     const Char_t* filename);

   virtual TFile* CreateNewFile(KVDataSet* dataset,
                                const Char_t* datatype,
                                const Char_t* filename);
   virtual void CommitFile(TFile* file);

   virtual void MakeSubdirectory(KVDataSet* dataset,
                                 const Char_t* datatype = "");

   virtual void DeleteFile(KVDataSet* dataset,
                           const Char_t* datatype,
                           const Char_t* filename, Bool_t confirm =
                              kTRUE);

   virtual void Print(Option_t* opt = "") const;
//returns protocol used for remote file transfer
   virtual const Char_t* GetFileTransferType() const
   {
      return fTransfertype;
   };
//returns full path to executable used for remote file transfer
   virtual const Char_t* GetFileTransferExec() const
   {
      return fTransferExec;
   };
//returns server url used for remote file transfer
   virtual const Char_t* GetFileTransferServer() const
   {
      return fTransferserver;
   };
//returns user name used for remote file transfer
   virtual const Char_t* GetFileTransferUser() const
   {
      return fTransferuser;
   };
//returns root directory of data repository (fLocalrootdir)
   virtual const Char_t* GetRootDirectory() const
   {
      return fLocalrootdir;
   };

   virtual KVDataSetManager* GetDataSetManager() const;

   void cd();
   static KVDataRepository* NewRepository(const Char_t* type);
   virtual KVAvailableRunsFile* NewAvailableRunsFile(const Char_t*, KVDataSet*);

   virtual void PrintAvailableDatasetsUpdateWarning() const
   {
      ;
   }

   ClassDef(KVDataRepository, 0)       //Base class handling files in data repository
};

//........ global variable
R__EXTERN KVDataRepository* gDataRepository;

#ifdef __CCIN2P3_RFIO
#include "TRFIOFile.h"
class KVRFIOSystem : public TRFIOSystem {
public:
   KVRFIOSystem(): TRFIOSystem() { };
   virtual ~KVRFIOSystem() { };

   Int_t       Unlink(const char* path);
   int             Chmod(const char* file, UInt_t mode);

   ClassDef(KVRFIOSystem, 0) // TRFIOSystem with fully-functioning Unlink and Chmod methods
};
#endif
#endif
