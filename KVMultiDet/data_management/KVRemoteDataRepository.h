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

/**
\class KVRemoteDataRepository
\brief Manage a collection of data files on a remote machine
\ingroup DM

<p>
Data repositories are defined in the user's $HOME/.kvrootrc; some typical examples are given
in $KVROOT/KVFiles/.kvrootrc:
</p>
<pre>
DataRepository: ccali
ccali.DataRepository.Type: remote
ccali.DataRepository.RootDir:       cchpssindra:/hpss/in2p3.fr/group/indra
ccali.DataRepository.ReadProtocol:     root
ccali.DataRepository.XRDServer:      ccxrdsn012:1999
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
This is the definition of a "remote" data repository.
A remote data repository is principally characterised by the fact that access to the data files is
via some non-local protocol: in this case it is xrootd ("...ReadProtocol: root"). The host name and port of
the xrootd server are given ("...XRDServer: ccxrdsn012:1999"), as well as the root directory
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
*/

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
   virtual Bool_t GetFileInfo(const KVDataSet* ds,
                              const Char_t* datatype,
                              const Char_t* runfile, FileStat_t& fs);
   virtual Bool_t CheckFileStatus(const KVDataSet* ds,
                                  const Char_t* datatype,
                                  const Char_t* runfile);

   virtual void CopyFileFromRepository(const KVDataSet* ds,
                                       const Char_t* datatype,
                                       const Char_t* filename,
                                       const Char_t* destination);
   virtual int CopyFileToRepository(const Char_t* source,
                                    const KVDataSet* ds,
                                    const Char_t* datatype,
                                    const Char_t* filename);
   virtual void MakeSubdirectory(const KVDataSet* ds,
                                 const Char_t* datatype = "");
   virtual KVUniqueNameList* GetDirectoryListing(const KVDataSet* ds,
         const Char_t* datatype = "", const Char_t* subdir = "");
   virtual void DeleteFile(const KVDataSet* ds,
                           const Char_t* datatype,
                           const Char_t* filename, Bool_t confirm =
                              kTRUE);
   virtual const Char_t* GetFullPathToTransferFile(const KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);
   virtual const Char_t* GetFullPathToOpenFile(const KVDataSet* dataset,
         const Char_t* datatype,
         const Char_t* runfile);

   ClassDef(KVRemoteDataRepository, 0) //Class handling data repositories on distant machines
};

#endif
