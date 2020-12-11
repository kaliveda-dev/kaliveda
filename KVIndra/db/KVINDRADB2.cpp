/***************************************************************************
$Id: KVINDRADB2.cpp,v 1.16 2007/04/26 16:40:58 franklan Exp $
                          KVINDRADB2.cpp  -  description
                             -------------------
    begin                : mars 2004
    copyright            : (C) 2004 by daniel cussol
    email                : cussol@in2p3.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "Riostream.h"
#include "KVINDRA.h"
#include "KVINDRADB2.h"
#include "KVINDRADBRun.h"
#include "KVDBParameterSet.h"
#include "TString.h"

using namespace std;

ClassImp(KVINDRADB2)

KVINDRADB2::KVINDRADB2(const Char_t* name): KVINDRADB(name)
{
   //default ctor
}

//___________________________________________________________________________

KVINDRADB2::~KVINDRADB2()
{

}

//___________________________________________________________________________

void KVINDRADB2::Build()
{
   //Use KVINDRARunListReader utility subclass to read complete runlist
   TString runlist_fullpath;
   KVBase::SearchKVFile(GetCalibFileName("Runlist"), runlist_fullpath,
                        fDataSet);
   SetRLCommentChar('#');
   SetRLSeparatorChar('\t');
   GetLineReader()->SetFieldKeys(3, "Tape", "Run", "Events");
   GetLineReader()->SetRunKeys(2, "Run", "Events");
   ReadRunList(runlist_fullpath.Data());
   ReadSystemList();
   ReadChIoPressures();
}


//____________________________________________________________________________

void KVINDRADB2::GoodRunLine()
{
   //For each "good run line" in the run list file, we:
   //      add a KVINDRADBRun to the database if it doesn't already exist
   //      add a KVDBTape to the database if the "tape" field is active and if it doesn't already exist
   //      set properties of run and tape objects
   //kFirstRun & kLastRun are set

   KVRunListLine* csv_line = GetLineReader();

   //run number
   Int_t run_n = csv_line->GetIntField("Run");

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
   if (csv_line->HasFieldValue("Tape")) {
      Int_t tape_n = csv_line->GetIntField("Tape");
      //already exists ?
      tape = GetTape(tape_n);
      if (!tape) {
         tape = new KVDBTape(tape_n);
         AddTape(tape);
      }
   }

   /*********************************************
   WE CREATE A NEW RUN AND ADD
    IT TO THE DATABASE. WE SET ALL
    AVAILABLE INFORMATIONS ON
    RUN FROM THE FILE. ERROR IF
    DBASE RUN ALREADY EXISTS =>
    SAME RUN APPEARS TWICE
   *********************************************/
   KVINDRADBRun* run = (KVINDRADBRun*) GetRun(run_n);
   if (!run) {

      run = new KVINDRADBRun(run_n);
      AddRun(run);

      //add run to tape ?
      if (tape)
         tape->AddRun(run);

      if (csv_line->HasFieldValue("Events"))
         run->SetEvents(csv_line->GetIntField("Events"));
      else
         Error("GoodRunLine", "No Events field ? run=%d", run_n);
      if (csv_line->HasFieldValue("FAR"))
         run->SetScaler("Faraday 1", csv_line->GetIntField("FAR"));
      else
         Error("GoodRunLine", "No FAR field ? run=%d", run_n);
      if (csv_line->HasFieldValue("MFI"))
         run->SetScaler("MFI", csv_line->GetIntField("MFI"));
      if (csv_line->HasFieldValue("Blocs"))
         run->SetScaler("Buffers", csv_line->GetIntField("Blocs"));
      else
         Error("GoodRunLine", "No Blocs field ? run=%d", run_n);
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
      if (csv_line->HasField("Trigger"))
         run->SetTrigger(GetRunListTrigger("Trigger", "M>=%d"));
      else
         Error("GoodRunLine", "No Trigger field ? run=%d", run_n);
      if (csv_line->HasFieldValue("NB"))
         run->SetComments(csv_line->GetField("NB"));

   }
   else {
      Error("GoodRunLine", "Run %d already exists", run_n);
   }
}
