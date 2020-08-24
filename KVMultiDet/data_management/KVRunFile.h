//Created by KVClassFactory on Thu Aug  4 12:08:39 2016
//Author: John Frankland,,,

#ifndef __KVRUNFILE_H
#define __KVRUNFILE_H

#include "KVBase.h"

#include <KVDBRun.h>
#include <KVDatime.h>

/**
\class KVRunFile
\brief Description of an individual run file in an experimental dataset
\ingroup DM
*/

class KVRunFile : public KVBase {
   KVDBRun* fRun;         // pointer to associated database run
   KVDatime fFileWritten; // modification date of file
   KVString fVersion;     // version of KaliVeda used to write file
   KVString fUser;        // name of user who wrote file

public:
   KVRunFile();
   KVRunFile(KVDBRun*, const KVString&, const KVDatime&, const KVString&, const KVString&);

   virtual ~KVRunFile();

   const KVDBRun* GetRun() const
   {
      return fRun;
   }
   Int_t GetRunNumber() const
   {
      return (fRun ? fRun->GetNumber() : 0);
   }
   Int_t GetTrigger() const
   {
      return (fRun ? fRun->GetTrigger() : 0);
   }
   Int_t GetEvents() const
   {
      return (fRun ? fRun->GetEvents() : 0);
   }
   const Char_t* GetFileWritten() const
   {
      return fFileWritten.AsString();
   }
   const Char_t* GetComments() const
   {
      return (fRun ? fRun->GetComments() : "");
   }
   const Char_t* GetVersion() const
   {
      return fVersion;
   }
   const Char_t* GetUser() const
   {
      return fUser;
   }
   Int_t Compare(const TObject* obj) const;
   Bool_t IsSortable() const
   {
      return kTRUE;
   }

   ClassDef(KVRunFile, 1) //A file containing data for a run
};

#endif
