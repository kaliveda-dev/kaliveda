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

/**
\class KVDataRepository
\brief Base class for managing repositories of experimental data
\ingroup DM

<p>
Data repositories are defined in the user's $HOME/.kvrootrc; some typical examples are given
in $KVROOT/KVFiles/.kvrootrc:
</p>
<pre>
DataRepository: home
home.DataRepository.RootDir: $(HOME)/Data
</pre>
<p>
This is the minimum requirement to define a data repository: give a name ("home") and the
full path to the top level directory ("...RootDir: $(HOME)/Data"). In this case the top level
directory must be directly accessible to the user's machine as a locally-mounted disk.
Note that environment variables can be used, if enclosed between "$(...)".
</p>
<pre>
DataRepository: ccali
ccali.DataRepository.Type: remote
ccali.DataRepository.RootDir:       cchpssindra:/hpss/in2p3.fr/group/indra
ccali.DataRepository.ReadProtocol:     root
ccali.DataRepository.XRDServer:      ccxroot:1999
ccali.DataRepository.XRDRootDir:       /hpss/in2p3.fr/group/indra
ccali.DataRepository.XRDTunnel.host:       ccali.in2p3.fr
ccali.DataRepository.XRDTunnel.port:          10000
ccali.DataRepository.XRDTunnel.user:
ccali.DataRepository.RemoteAvailableRuns.protocol:  curl
ccali.DataRepository.RemoteAvailableRuns.url:   http://indra.in2p3.fr/KaliVedaDoc
ccali.DataRepository.FileTransfer.type:    bbftp
ccali.DataRepository.FileTransfer.server:    ccbbftp.in2p3.fr
ccali.DataRepository.FileTransfer.user:
</pre>
<p>
This is the definition of a "remote" data repository. It will be handled by an object of the class
<a href="KVRemoteDataRepository.html">KVRemoteDataRepository</a>.
</p>
<p>
A remote data repository is principally characterised by the fact that access to the data files is
via some non-local protocol: in this case it is xrootd ("...ReadProtocol: root"). The host name and port of
the xrootd server are given ("...XRDServer: ccxroot:1999"), as well as the root directory
to be used ("...XRDRootDir:  /hpss/in2p3.fr/group/indra").
</p>
<p>
In order to use a remote repository, some way to access the database of available
data files for each run type must be defined. Here it is the curl programme which is used to read
them via a website ("...RemoteAvailableRuns.protocol: curl"; "...RemoteAvailableRuns.url: http://indra.in2p3.fr/KaliVedaDoc").
</p>
<p>
Another particularity of remote data repositories is that some additional stuff may be needed
in order to be able to access the data.
This is handled by <a href="KVRemoteDataRepository.html#KVRemoteDataRepository:IsConnected">KVRemoteDataRepository::IsConnected()</a>.
In the present example, an SSH tunnel is set up in order to allow secure connection to the
xrootd server ("...XRDTunnel.*:").
</p>
<p>
Finally, transfer of data between data repositories is handled by <a href="KVDataTransfer.html">KVDataTransfer</a> and child classes.
In the present example, this is configured to use bbftp ("...FileTransfer.type: bbftp"), for which the required details are given.
</p>
<h4>Default data repository</h4>
<p>If more than one repository is defined, which one will be "active" (gDataRepository)
after initialisation of the data repository manager ? You can define the default repository
by setting the variable
</p>
<pre>
DataRepository.Default:   [name]
</pre>
<p>in your .kvrootrc file. If you don't, the default repository "by default" will be either:
the data repository named "default" if there is one; or the last one defined in your .kvrootrc.
</p>
*/

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
   const KVDataSet* fCommitDataSet;   //!
   TString fCommitDataType;     //!
   TString fCommitFileName;     //!
   KVDataSetManager* fDSM;      //handles datasets in repository
   void SetFullPath(TString& path, const Char_t* protocol);
   virtual const Char_t* GetReadProtocol(const Char_t* dataset, const Char_t* datatype);
   virtual KVDataSetManager* NewDataSetManager();

   virtual int             Chmod(const char* file, UInt_t mode);
   virtual void PrepareXRDTunnel();

   TSeqCollection*  fHelpers;          //List of helper classes for alternative file/directory access
   TObject* OpenDataSetFile(const KVDataSet* ds, const Char_t* type, const TString& fname, Option_t* opt = "");
public:
   virtual int  CopyFile(const char* f, const char* t, Bool_t overwrite = kFALSE);
   TSystem*               FindHelper(const char* path, void* dirptr = 0);
   Bool_t            HelperIsConsistentWith(TSystem* helper, const char* path, void* dirptr = 0);

   KVDataRepository();
   virtual ~ KVDataRepository();

   virtual Bool_t Init();

   virtual Bool_t CheckSubdirExists(const Char_t* dir,
                                    const Char_t* subdir = 0);
   virtual Bool_t GetFileInfo(const KVDataSet* dataset,
                              const Char_t* datatype,
                              const Char_t* runfile, FileStat_t& fs);
   virtual Bool_t CheckFileStatus(const KVDataSet* dataset,
                                  const Char_t* datatype,
                                  const Char_t* runfile);

   virtual const Char_t* GetFullPathToOpenFile(const KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);

   virtual const Char_t* GetFullPathToTransferFile(const KVDataSet* dataset,
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
   virtual KVUniqueNameList* GetDirectoryListing(const KVDataSet* dataset,
         const Char_t* datatype = "", const Char_t* subdir = "");

   virtual void CopyFileFromRepository(const KVDataSet* dataset,
                                       const Char_t* datatype,
                                       const Char_t* filename,
                                       const Char_t* destination);
   virtual int CopyFileToRepository(const Char_t* source,
                                    const KVDataSet* dataset,
                                    const Char_t* datatype,
                                    const Char_t* filename);

   virtual TFile* CreateNewFile(const KVDataSet* dataset,
                                const Char_t* datatype,
                                const Char_t* filename);
   virtual void CommitFile(TFile* file, const Char_t* datatype, const KVDataSet* dataset);

   virtual void MakeSubdirectory(const KVDataSet* dataset,
                                 const Char_t* datatype = "");

   virtual void DeleteFile(const KVDataSet* dataset,
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
   virtual KVAvailableRunsFile* NewAvailableRunsFile(const Char_t*, const KVDataSet*);
   virtual TObject* OpenDataSetRunFile(const KVDataSet* ds, const Char_t* type, Int_t run, Option_t* opt = "");
   void CreateAllNeededSubdirectories(const KVDataSet* DataSet, const Char_t* DataType);

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
