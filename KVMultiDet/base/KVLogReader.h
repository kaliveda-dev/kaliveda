/*******************************************************************************
$Id: KVLogReader.h,v 1.10 2008/04/07 15:34:27 franklan Exp $
*******************************************************************************/
#ifndef __KVLOGREADER_H
#define __KVLOGREADER_H

#include "Rtypes.h"
#include "KVString.h"

/**
  \class KVLogReader
  \brief Base class for reading batch log files at CC-IN2P3
  \ingroup Core
 */
class KVLogReader {
protected:

   KVString fFMT;                //format string used to extract run number from job name
   KVString fJobname;            //name of job
   Double_t fCPUused;           //normalized used CPU time in seconds
   Double_t fCPUreq;            //requested CPU time in seconds
   Double_t fMEMreq;            //requested memory (VIRTUAL STORAGE) in KB
   Double_t fSCRreq;            //requested scratch disk space in KB
   Double_t fScratchKB;         //used disk space in KB
   Double_t fMemKB;             //used memory in KB
   KVString fStatus;             //status string
   Bool_t fOK;                  //job OK or not ?
   Bool_t fGotRequests;   //set true when disk & memory request has been read
   Bool_t fGotStatus;   //set true when end of job infos have been read

protected:
   virtual Int_t GetByteMultiplier(const KVString& unit) = 0;
   virtual void ReadLine(const KVString& line, Bool_t&);
   virtual void ReadCPU(const KVString& line) = 0;
   void ReadStorageReq(const KVString& line);
   virtual void ReadScratchUsed(const KVString& line) = 0;
   virtual void ReadMemUsed(const KVString& line) = 0;
   virtual void ReadStatus(const KVString& line) = 0;
   void ReadJobname(const KVString& line);
   virtual Double_t ReadStorage(const KVString& stor) = 0;

public:

   KVLogReader();
   virtual ~ KVLogReader()
   {
   };

   void ReadFile(const Char_t* fname);
   virtual void Reset();

   void SetNameFormat(const Char_t* fmt)
   {
      fFMT = fmt;
   };

   Double_t GetCPUused() const
   {
      return fCPUused;
   };
   Double_t GetCPUrequest() const
   {
      return fCPUreq;
   };
   Double_t GetSCRATCHrequest() const
   {
      return fSCRreq;
   };
   Double_t GetMEMrequest() const
   {
      return fMEMreq;
   };
   Double_t GetCPUratio() const;
   Double_t GetSCRATCHused() const
   {
      return fScratchKB;
   };
   Double_t GetMEMused() const
   {
      return fMemKB;
   };
   const Char_t* GetStatus() const
   {
      return fStatus.Data();
   };
   const Char_t* GetJobname() const
   {
      return fJobname.Data();
   };
   Int_t GetRunNumber() const;

   Bool_t JobOK() const
   {
      return (fOK && fGotRequests);
   };

   Bool_t Killed() const
   {
      return (fStatus == "KILLED");
   };
   Bool_t SegFault() const
   {
      return (fStatus.Contains("egmentation"));
   };
   virtual Bool_t Incomplete() const;

   ClassDef(KVLogReader, 0)//Tool for reading CCIN2P3 batch logs
};

#endif
