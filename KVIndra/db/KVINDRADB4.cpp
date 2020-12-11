/***************************************************************************
$Id: KVINDRADB4.cpp,v 1.18 2007/04/26 16:40:58 franklan Exp $
                          KVINDRADB4.cpp  -  description
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
#include "KVINDRADB4.h"
#include "KVINDRADBRun.h"
#include "KVDBParameterSet.h"
#include "KVRunListLine.h"
#include "TString.h"

using namespace std;

ClassImp(KVINDRADB4)

KVINDRADB4::KVINDRADB4(const Char_t* name): KVINDRADB(name)
{
   //default ctor
}

//___________________________________________________________________________

KVINDRADB4::~KVINDRADB4()
{

}

//___________________________________________________________________________

void KVINDRADB4::Build()
{
   //Use KVINDRARunListReader utility subclass to read complete runlist

   TString runlist_fullpath;
   KVBase::SearchKVFile(GetCalibFileName("Runlist"), runlist_fullpath,
                        fDataSet);
   SetRLCommentChar('#');
   SetRLSeparatorChar('|');
   GetLineReader()->SetFieldKeys(3, "RUN", "BEAM", "TARGET");
   GetLineReader()->SetRunKeys(2, "RUN", "EVENTS");
   ReadRunList(runlist_fullpath.Data());
   ReadSystemList();
   ReadChIoPressures();
}

//____________________________________________________________________________

void KVINDRADB4::GoodRunLine()
{
   //For each "good run line" in the run list file, we:
   //      add a KVINDRADBRun to the database if it doesn't already exist
   //      set properties of run objects
   //kFirstRun & kLastRun are set

   KVRunListLine* csv_line = GetLineReader();

   //run number
   Int_t run_n = csv_line->GetIntField("RUN");

   if (!run_n) {
      cout << "run_n = 0 ?????????  line number =" << GetRLLineNumber() <<
           endl;
      GetLineReader()->Print();
      return;
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

      if (csv_line->HasFieldValue("EVENTS"))
         run->SetEvents(csv_line->GetIntField("EVENTS"));
      else
         Error("GoodRunLine", "No events field ? run=%d", run_n);
      if (csv_line->HasField("TRIGGER"))
         run->SetTrigger(GetRunListTrigger("TRIGGER", "M>=%d"));
      else
         Error("GoodRunLine", "No TRIGGER field ? run=%d", run_n);
      if (csv_line->HasFieldValue("RUNCOMMENT"))
         run->SetComments(csv_line->GetField("RUNCOMMENT"));
      if (csv_line->HasFieldValue("TSTART"))
         run->SetStartDate(csv_line->GetField("TSTART"));

   }
   else {
      Error("GoodRunLine", "Run %d already exists", run_n);
   }
}
