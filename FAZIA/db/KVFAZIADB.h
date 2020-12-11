//Created by KVClassFactory on Tue Jan 20 11:52:49 2015
//Author: E. Bonnet,,,

#ifndef __KVFAZIADB_H
#define __KVFAZIADB_H

#include "KVExpDB.h"
#include "KVDBTable.h"
#include "KVFAZIADBRun.h"
#include "KVDBRecord.h"
#include "KVFileReader.h"

class KVNumberList;
class KVDBSystem;

/**
  \class KVFAZIADB
  \brief Experiment database for FAZIA
  \ingroup FAZIADB
 */
class KVFAZIADB : public KVExpDB {

protected:
   //specific table associated to fazia experiment
   //
   KVDBTable* fExceptions;       //-> table of exceptions
   KVDBTable* fCalibrations;       //-> table for calibrations
   KVDBTable* fOoODets; //(optional) Liste les detecteurs hors service

   virtual void ReadOoODetectors();

   void init();

   void ReadCalibFile(const Char_t* filename);
   TString fDONEfile;   //!
   TString fFAILEDfile; //!

public:

   KVFAZIADB();
   KVFAZIADB(const Char_t* name);
   virtual ~KVFAZIADB();

   virtual void Build();

   void ReadNewRunList();
   KVFAZIADBRun* GetRun(Int_t run) const
   {
      return (KVFAZIADBRun*) GetDBRun(run);
   }

   const Char_t* GetFileName(const Char_t* meth, const Char_t* keyw);
   KVFileReader* GetKVFileReader(const Char_t* meth, const Char_t* keyw);
   TEnv* GetFileTEnv(const Char_t* meth, const Char_t* keyw);

   void ReadExceptions();
   void ReadRutherfordCounting();
   void ReadRutherfordCrossSection();
   void ReadComments();
   void ReadCalibrationFiles();

   virtual void cd();

   void BuildQuickAndDirtyDataBase(TString);
   Bool_t TransferAcquisitionFileToCcali(TString file, TString ccali_rep = "/fazia", TString option = "frv");
   Bool_t TransferRunToCcali(Int_t run, TString path, TString ccali_rep = "/fazia", TString option = "frv");
   void TransferRunListToCcali(KVNumberList lrun, TString path, TString ccali_rep = "/fazia", TString option = "frv");
   void StartTransfer(TString filename = "runlist.dat", TString ccali_rep = "/fazia", TString option = "frv");


   ClassDef(KVFAZIADB, 3) //database for FAZIA detector
};
//........ global variable
R__EXTERN KVFAZIADB* gFaziaDB;

#endif
