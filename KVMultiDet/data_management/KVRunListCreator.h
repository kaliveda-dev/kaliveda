#ifndef __KVRunListCreator_H
#define __KVRunListCreator_H

#include "KVUniqueNameList.h"
#include "TString.h"
#include "KVRawDataReader.h"

/**
  \class KVRunListCreator
  \brief Set up a list of runs from a directory containing data files
  \ingroup DM
 */
class KVRunListCreator {

   TString fRunSheetDir;        //full path to directory holding runs
   TString fFileFormat;         //format string for run file names
   TString fDataType;           //type of data (KVRawDataReader plugin name)

   KVUniqueNameList fRunInfos;
   unique_ptr<KVRawDataReader> fReader;

public:

   KVRunListCreator(const TString& dir, const TString& datatype = "", const TString& fmt = "");
   virtual ~KVRunListCreator() {}

   const Char_t* GetRunDir()
   {
      return fRunSheetDir;
   }
   void SetRunDir(const Char_t* dir)
   {
      fRunSheetDir = dir;
   }
   const Char_t* GetFileFormat()
   {
      return fFileFormat;
   }
   void SetFileFormat(const Char_t* fmt)
   {
      fFileFormat = fmt;
   }
   Int_t ScanDirectory();
   const KVUniqueNameList& GetRunInfos() const
   {
      return fRunInfos;
   }

   ClassDef(KVRunListCreator, 0)//Set up a list of runs from a directory containing data files
};

#endif
