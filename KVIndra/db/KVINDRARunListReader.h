#ifndef __RUNLISTREADER_H__
#define __RUNLISTREADER_H__

#include <TString.h>
#include <KVRunListLine.h>
#include <Riostream.h>

/**
   \class KVINDRARunListReader
\brief Utitlity base class for reading INDRA runlist files.
\ingroup INDRADB

Use multiple inheritance with this class as one base class, then method "GoodRunLine"
can be overridden to perform implementation-specific tasks.
*/

class KVINDRARunListReader {

private:

   TString fRunsFile;           //!name of file to read
   std::ifstream fRunsStream;        //!input file stream
   KVRunListLine* fLineReader;  //!object used to analyse lines in file
   TString fLine;               //!last line read from file
   Char_t fComment;             //!character placed at beginning of comment lines
   Char_t fSeparator;           //!character used to separate fields on each line of file
   Int_t fCurrentLine;          //!number of line in runlist file we are currently analysing
   Int_t fTrigger;              //!last multiplicity trigger read from file
   Bool_t fNewRunList; //!flag set when reading new ("Version=10") runlist
   Int_t fVersion;//!version number of new runlist

public:

   Bool_t IsNewRunList() const
   {
      return fNewRunList;
   };

   KVINDRARunListReader();
   virtual ~ KVINDRARunListReader();

   Bool_t OpenRLFile(const Char_t*);
   void CloseRLFile();
   Bool_t ReadRLLine();

   inline void SetRLCommentChar(Char_t c)
   {
      fComment = c;
   };
   inline void SetRLSeparatorChar(Char_t c)
   {
      fSeparator = c;
   };

   void ReadRunList(const Char_t* name = "");

   virtual void GoodRunLine();

   KVRunListLine* GetLineReader() const
   {
      return fLineReader;
   };

   Int_t GetRunListTrigger(const Char_t* field, const Char_t* fmt);

   Int_t GetRLLineNumber() const
   {
      return fCurrentLine;
   };

   ClassDef(KVINDRARunListReader, 1)    //Utility base class for reading INDRA runlists
};

#endif
