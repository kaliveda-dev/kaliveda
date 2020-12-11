/***************************************************************************
$Id: KVINDRADB1.cpp,v 1.19 2007/04/26 16:40:57 franklan Exp $
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "Riostream.h"
#include "KVINDRA.h"
#include "KVINDRADB1.h"
#include "KVINDRADBRun.h"
#include "KVDBParameterSet.h"
#include "KVRunListLine.h"
#include "TString.h"

using namespace std;

ClassImp(KVINDRADB1)

KVINDRADB1::KVINDRADB1(const Char_t* name): KVINDRADB(name)
{
   //default ctor
}

//___________________________________________________________________________

void KVINDRADB1::Build()
{

   //Use KVINDRARunListReader utility subclass to read complete runlist
   TString runlist_fullpath;
   KVBase::SearchKVFile(GetDBEnv("Runlist"), runlist_fullpath, fDataSet);
   SetRLCommentChar(GetDBEnv("Runlist.Comment")[0]);
   if (!strcmp(GetDBEnv("Runlist.Separator"), "<TAB>"))
      SetRLSeparatorChar('\t');
   else
      SetRLSeparatorChar(GetDBEnv("Runlist.Separator")[0]);
   GetLineReader()->SetFieldKeys(3, "tape", "run", "events");
   GetLineReader()->SetRunKeys(2, "run", "events");
   ReadRunList(runlist_fullpath.Data());

   ReadSystemList();
   ReadChIoPressures();
}

//____________________________________________________________________________

void KVINDRADB1::GoodRunLine()
{
   //For each "good run line" in the run list file, we:
   //      add a KVINDRADBRun to the database if it doesn't already exist
   //      add a KVDBTape to the database if the "tape" field is active and if it doesn't already exist
   //      set properties of run and tape objects
   //kFirstRun & kLastRun are set

   KVRunListLine* csv_line = GetLineReader();

   //run number
   Int_t run_n = csv_line->GetIntField("run");

   if (!run_n) {
      cout << "run_n = 0 ?????????  line number =" << GetRLLineNumber() <<
           endl;
      GetLineReader()->Print();
      return;
   }

   /*********************************************
   IF LINE HAS A TAPE NUMBER WE
    LOOK FOR THE TAPE IN THE DATA
    BASE. IF IT DOESN'T EXIST WE
    CREATE IT.
   *********************************************/
   KVDBTape* tape = 0;
   //tape number (if tape field is filled)
   if (csv_line->HasFieldValue("tape")) {
      Int_t tape_n = csv_line->GetIntField("tape");
      //already exists ?
      tape = GetTape(tape_n);
      if (!tape) {
         tape = new KVDBTape(tape_n);
         AddTape(tape);
      }
   }
   else {
      Error("GoodRunLine", "No tape field ? run=%d", run_n);
   }

   /*********************************************
   WE CREATE A NEW RUN AND ADD
    IT TO THE DATABASE. WE SET ALL
    AVAILABLE INFORMATIONS ON
    RUN FROM THE FILE. ERROR IF
    DBASE RUN ALREADY EXISTS =>
    SAME RUN APPEARS TWICE
   *********************************************/
   KVINDRADBRun* run = GetRun(run_n);
   if (!run) {

      run = new KVINDRADBRun(run_n);
      AddRun(run);

      //add run to tape ?
      if (tape)
         tape->AddRun(run);

      if (csv_line->HasFieldValue("events"))
         run->SetEvents(csv_line->GetIntField("events"));
      else
         Error("GoodRunLine", "No events field ? run=%d", run_n);
      if (csv_line->HasFieldValue("FAR"))
         run->SetScaler("Faraday 1", csv_line->GetIntField("FAR"));
      else
         Error("GoodRunLine", "No FAR field ? run=%d", run_n);
      if (csv_line->HasFieldValue("MFI"))
         run->SetScaler("MFI", csv_line->GetIntField("MFI"));
      else
         Error("GoodRunLine", "No MFI field ? run=%d", run_n);
      if (csv_line->HasFieldValue("blocs"))
         run->SetScaler("Buffers", csv_line->GetIntField("blocs"));
      else
         Error("GoodRunLine", "No blocs field ? run=%d", run_n);
      if (csv_line->HasFieldValue("G. dir"))
         run->SetScaler("Gene DIRECT", csv_line->GetIntField("G. dir"));
      else
         Error("GoodRunLine", "No G. dir field ? run=%d", run_n);
      if (csv_line->HasFieldValue("G. marq"))
         run->SetScaler("Gene MARQ", csv_line->GetIntField("G. marq"));
      else
         Error("GoodRunLine", "No G. marq field ? run=%d", run_n);
      if (csv_line->HasFieldValue("G. TM"))
         run->SetScaler("Gene TM", csv_line->GetIntField("G. TM"));
      else
         Error("GoodRunLine", "No G. TM field ? run=%d", run_n);
      if (csv_line->HasFieldValue("TM %"))
         run->SetTMpercent(csv_line->GetFloatField("TM %"));
      else
         Error("GoodRunLine", "No TM %% field ? run=%d", run_n);
      if (csv_line->HasField("Decl"))
         run->SetTrigger(GetRunListTrigger("Decl", "M>=%d"));
      else
         Error("GoodRunLine", "No Decl field ? run=%d", run_n);
      if (csv_line->HasFieldValue("Remarque"))
         run->SetComments(csv_line->GetField("Remarque"));

   }
   else {
      Error("GoodRunLine", "Run %d already exists", run_n);
   }
}
