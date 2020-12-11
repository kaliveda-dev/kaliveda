/*
$Id: KVINDRADB_e416a.cpp,v 1.2 2007/02/14 14:12:31 franklan Exp $
$Revision: 1.2 $
$Date: 2007/02/14 14:12:31 $
*/

//Created by KVClassFactory on Tue Feb 13 19:00:55 2007
//Author: franklan

#include "KVINDRADB_e416a.h"
#include "KVDB_BIC_Pressures.h"

using namespace std;

ClassImp(KVINDRADB_e416a)


KVINDRADB_e416a::KVINDRADB_e416a(const Char_t* name): KVINDRADB(name)
{
   //Default constructor
   fBICPressures = AddTable("BIC Pressures", "Pressures of BIC");
}

//____________________________________________________________________________

KVINDRADB_e416a::~KVINDRADB_e416a()
{
   //Destructor
}

//____________________________________________________________________________

void KVINDRADB_e416a::Build()
{
   KVINDRADB::Build();
   ReadBICPressures();
}

//____________________________________________________________________________

void KVINDRADB_e416a::ReadBICPressures()
{
   //Read BIC pressures for different run ranges and enter into database.
   //Format of file is:
   //
   //# some comments
   //#which start with '#'
// Run Range : 289 385
// BIC_1 30.0
// BIC_2 30.0
// BIC_3 30.0
   //
   //Pressures (of CF4) are given in Torr.

   ifstream fin;
   if (!OpenCalibFile("BICPressures", fin)) {
      Error("ReadBICPressures()", "Could not open file %s",
            GetCalibFileName("BICPressures"));
      return;
   }
   Info("ReadBICPressures()", "Reading BIC pressures parameters...");

   TString sline;
   UInt_t frun = 0, lrun = 0;
   UInt_t run_ranges[MAX_NUM_RUN_RANGES][2];
   UInt_t rr_number = 0;
   Bool_t prev_rr = kFALSE;     //was the previous line a run range indication ?
   Bool_t read_pressure = kFALSE; // have we read any pressures recently ?

   KVDB_BIC_Pressures* parset = 0;
   TList* par_list = new TList();

   //any ChIo not in list is assumed absent (pressure = 0)
   Float_t pressure[3] = { 0, 0, 0 };

   while (fin.good()) {         // parcours du fichier

      sline.ReadLine(fin);
      if (sline.BeginsWith("Run Range :")) {    // run range found
         if (!prev_rr) {        // New set of run ranges to read

            //have we just finished reading some pressures ?
            if (read_pressure) {
               parset = new KVDB_BIC_Pressures(pressure);
               GetTable("BIC Pressures")->AddRecord(parset);
               par_list->Add(parset);
               LinkListToRunRanges(par_list, rr_number, run_ranges);
               par_list->Clear();
               for (int zz = 0; zz < 3; zz++) pressure[zz] = 0.;
               read_pressure = kFALSE;
            }
            rr_number = 0;

         }
         if (sscanf(sline.Data(), "Run Range : %u %u", &frun, &lrun) != 2) {
            Warning("ReadBICPressures()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
         }
         else {
            prev_rr = kTRUE;
            run_ranges[rr_number][0] = frun;
            run_ranges[rr_number][1] = lrun;
            rr_number++;
            if (rr_number == MAX_NUM_RUN_RANGES) {
               Error("ReadBICPressures", "Too many run ranges (>%d)",
                     rr_number);
               rr_number--;
            }
         }
      }                         // Run Range found
      if (fin.eof()) {          //fin du fichier
         //have we just finished reading some pressures ?
         if (read_pressure) {
            parset = new KVDB_BIC_Pressures(pressure);
            GetTable("BIC Pressures")->AddRecord(parset);
            par_list->Add(parset);
            LinkListToRunRanges(par_list, rr_number, run_ranges);
            par_list->Clear();
            for (int zz = 0; zz < 3; zz++) pressure[zz] = 0.;
            read_pressure = kFALSE;
         }
      }
      if (sline.BeginsWith("BIC")) {  //line with BIC pressure data

         prev_rr = kFALSE;

         //split up BIC name and pressure
         TObjArray* toks = sline.Tokenize(' ');
         TString chio = ((TObjString*)(*toks)[0])->String();
         KVString press = ((TObjString*)(*toks)[1])->String();
         delete toks;

         read_pressure = kTRUE;

         if (chio == "BIC_1") pressure[0] = press.Atof();
         else if (chio == "BIC_2") pressure[1] = press.Atof();
         else if (chio == "BIC_3") pressure[2] = press.Atof();
         else read_pressure = kFALSE;

      }                         //line with BIC pressure data
   }                            //parcours du fichier
   delete par_list;
   fin.close();
}

