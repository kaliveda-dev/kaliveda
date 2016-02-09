//Created by KVClassFactory on Tue Jul 13 11:52:58 2010
//Author: Eric Bonnet

#ifndef __KVFILEREADER_H
#define __KVFILEREADER_H

#include "KVBase.h"
#include "Riostream.h"
#include "TObjArray.h"
#include "KVString.h"
#include "TString.h"
#include "TObjString.h"

class KVFileReader : public KVBase {
protected:
   TObjArray* toks;//->
   KVString reading_line, file_name;
   Int_t nline;
   Bool_t status;

public:
   std::ifstream f_in;

   KVFileReader();
   virtual ~KVFileReader();
   virtual void init()
   {
      //Info("init","rentre");
      reading_line = "";
      nline = 0;
      toks = new TObjArray();
   }

   KVString GetFileName()
   {
      return file_name;
   }

   void Clear(Option_t* /*opt*/ = "")
   {
      if (toks) delete toks;
      init();
   }

   Bool_t PreparForReadingAgain()
   {
      CloseFile();
      Clear();
      return OpenFileToRead(GetFileName());
   }

   Bool_t OpenFileToRead(KVString filename)
   {

      file_name = filename;

      f_in.open(filename.Data());
      status = f_in.good();

      if (!status)
         Error("OpenFileToRead", "Echec dans l ouverture du fichier %s", filename.Data());

      return status;

   }

   Bool_t IsOK()
   {
      return f_in.good();
   }

   void CloseFile()
   {
      if (f_in.is_open()) f_in.close();
   }

   void ReadLine(const Char_t* pattern)
   {
      reading_line.ReadLine(f_in);
      nline++;
      if (pattern)
         StoreParameters(pattern);
   }

   void ReadLineAndAdd(const Char_t* pattern)
   {
      reading_line.ReadLine(f_in);
      nline++;
      if (pattern)
         AddParameters(pattern);
   }

   Int_t ReadLineAndCheck(Int_t nexpect, const Char_t* pattern)
   {

      ReadLine(0);
      if (GetCurrentLine().IsNull()) {
         return 0;
      }
      StoreParameters(pattern);
      if (GetNparRead() == nexpect) {
         return 1;
      } else {
         return 2;
      }

   }

   KVString GetCurrentLine()
   {
      return reading_line;
   }

   void StoreParameters(const Char_t* pattern)
   {
      delete toks;
      toks = GetCurrentLine().Tokenize(pattern);
   }

   void AddParameters(const Char_t* pattern)
   {
      //delete toks;
      TObjArray* tamp = GetCurrentLine().Tokenize(pattern);
      Int_t ne = tamp->GetEntries();
      for (Int_t kk = 0; kk < ne; kk += 1)
         toks->Add(tamp->RemoveAt(kk));
      delete tamp;
   }

   Int_t GetNparRead()
   {
      return toks->GetEntries();
   }
   Int_t GetNlineRead()
   {
      return nline;
   }

   Double_t GetDoubleReadPar(Int_t pos)
   {
      return GetReadPar(pos).Atof();
   }
   Int_t GetIntReadPar(Int_t pos)
   {
      return GetReadPar(pos).Atoi();
   }
   TString GetReadPar(Int_t pos)
   {
      return ((TObjString*)toks->At(pos))->GetString();
   }


   ClassDef(KVFileReader, 1) //Manage the reading of file
};

#endif
