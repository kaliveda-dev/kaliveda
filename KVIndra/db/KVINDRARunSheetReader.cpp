/*******************************************************************************
$Id: KVINDRARunSheetReader.cpp,v 1.13 2008/12/08 14:07:37 franklan Exp $
$Revision: 1.13 $
$Date: 2008/12/08 14:07:37 $
$Author: franklan $
*******************************************************************************/

#include "KVINDRARunSheetReader.h"
#include "KVINDRADBRun.h"
#include "KVINDRADB.h"
#include "Riostream.h"
#include "KVDatime.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVINDRARunSheetReader)

KVINDRARunSheetReader::KVINDRARunSheetReader(Bool_t make_tree)
{
   //Default ctor. If make_tree = kTRUE, a TTree will be created and filled with
   //data from runsheets.
   SetMakeTree(make_tree);
   fTree = 0;
   init_vars();
   fTIME_START = 0;
}

void KVINDRARunSheetReader::init_vars()
{
   //Set values of all runsheet/TTree variables to 0
   len_run = size = data_rate = acq_rate = trait_rate = ctrl_rate =
         rempli_dlt_pc = temps_mort = 0.;
   run_num = buf_sav = eve_sav = eve_lus = buf_ctrl = eve_ctrl =
         rempli_dlt_blocs = 0;
   for (int i = 0; i < 100; i++) fScalers[i] = 0;
}

const Char_t* KVINDRARunSheetReader::GetRunSheetFileName(Int_t run)
{
   //Uses directory path and file format to create full path to runsheet
   //file for run number 'run'.

   TString filename;
   filename.Form(fFileFormat.Data(), run);
   char* dirname =
      gSystem->ConcatFileName(fRunSheetDir.Data(), filename.Data());
   fFilePath = dirname;
   delete[]dirname;
   return fFilePath.Data();
}

void KVINDRARunSheetReader::CreateTree()
{
   if (fTree) delete fTree;
   fTree =
      new TTree("RunInfoScalerTree",
                "Data from INDRA runsheets. Created by KVINDRARunSheetReader.");
   fTree->SetDirectory(gROOT);
   fTree->Branch("run", &run_num, "run/I");
   fTree->Branch("start", &istart, "start/I");
   fTree->Branch("end", &iend, "end/I");
   fTree->Branch("buf_sav", &buf_sav, "buf_sav/I");
   fTree->Branch("eve_sav", &eve_sav, "eve_sav/I");
   fTree->Branch("eve_lus", &eve_lus, "eve_lus/I");
   fTree->Branch("buf_ctrl", &buf_ctrl, "buf_ctrl/I");
   fTree->Branch("eve_ctrl", &eve_ctrl, "eve_ctrl/I");
   fTree->Branch("rempli_dlt_blocs", &rempli_dlt_blocs,
                 "rempli_dlt_blocs/I");
   fTree->Branch("len_run", &len_run, "len_run/D");
   fTree->Branch("size", &size, "size/D");
   fTree->Branch("data_rate", &data_rate, "data_rate/D");
   fTree->Branch("acq_rate", &acq_rate, "acq_rate/D");
   fTree->Branch("trait_rate", &trait_rate, "trait_rate/D");
   fTree->Branch("ctrl_rate", &ctrl_rate, "ctrl_rate/D");
   fTree->Branch("rempli_dlt_pc", &rempli_dlt_pc, "rempli_dlt_pc/D");
   fTree->Branch("temps_mort", &temps_mort, "temps_mort/D");
}

Bool_t KVINDRARunSheetReader::ReadRunSheet(Int_t run)
{
   //Read runsheet corresponding to run number 'run'
   //Store the informations we read in the runsheet in the run database.
   //Create TTree if required and if not already done
   //Return kTRUE if all goes well.

   cout <<
        "<KVINDRARunSheetReader::ReadRunSheet> : Reading runsheet for new run, Run "
        << run << endl;
   KVINDRADBRun* indra_db_run = gIndraDB->GetRun(run);
   if (!indra_db_run) {
      //create new run and add it to the database
      indra_db_run = new KVINDRADBRun(run);
      gIndraDB->AddRun(indra_db_run);
   }

   //open runsheet file
   ifstream read_file(GetRunSheetFileName(run));
   if (!read_file.good()) {
      cout << "Error opening file : " << fFilePath.Data() << endl;
      return kFALSE;
   }
   //reset runsheet/TTree variables
   init_vars();

   KVString line, dumstr;

   line.ReadLine(read_file);

   //read all infos up to list of scalers (which begins with the line "*** Impression des Echelles :"
   while (read_file.good() && !line.Contains("Echelles")) {

      if (line.Contains("Run numero"))
         run_num = (Int_t) GetNumberField(line);
      if (line.Contains("Debut")) {
         start_run.SetGanacqDate(GetDateField(line));
         istart = start_run.Convert();
         if (!fTIME_START) {
            //we put the "time origin" at the start of the first run we read
            fTIME_START = start_run.Convert();
         }
         indra_db_run->SetStartDate(start_run.AsGanacqDateString());
      }
      if (line.Contains("Fin")) {
         endrun.SetGanacqDate(GetDateField(line));
         iend = endrun.Convert();
         indra_db_run->SetEndDate(endrun.AsGanacqDateString());
      }
      if (line.Contains("Length of run")) {
         sscanf(line.Data(),
                "Length of run:   %lf   seconds,  Size :  %lf  MB (  %lf  MB/sec)",
                &len_run, &size, &data_rate);
         indra_db_run->SetTime(len_run / 60.);
         indra_db_run->SetSize(size);
      }
      if (line.Contains("Stat_eve")) {
         dumstr = GetStringField(line);
         stat_eve = GetStringField(dumstr, " ", 0);
      }
      if (line.Contains("Buffers sauves")) {
         dumstr = GetStringField(line);
         buf_sav = (Int_t) GetNumberField(dumstr, " ", 0);
         eve_sav = (Int_t) GetNumberField(line, ":", 2);
         indra_db_run->SetScaler("Buffers", buf_sav);
         indra_db_run->SetEvents(eve_sav);
      }
      if (line.Contains("Evenements lus"))
         eve_lus = (Int_t) GetNumberField(line);
      if (line.Contains("Buffers ctrl")) {
         dumstr = GetStringField(line);
         buf_ctrl = (Int_t) GetNumberField(dumstr, " ", 0);
         eve_ctrl = (Int_t) GetNumberField(line, ":", 2);
      }
      if (line.Contains("Taux d'acquisition")) {
         dumstr = GetStringField(line);
         acq_rate = GetNumberField(dumstr, " ", 0);
      }
      if (line.Contains("Taux de traitement")) {
         dumstr = GetStringField(line);
         trait_rate = GetNumberField(dumstr, " ", 0);
      }
      if (line.Contains("Taux de controle")) {
         dumstr = GetStringField(line);
         ctrl_rate = GetNumberField(dumstr, " ", 0);
      }
      if (line.Contains("Remplissage")) {
         dumstr = GetStringField(line);
         rempli_dlt_pc = GetNumberField(dumstr, " ", 0);
         rempli_dlt_blocs = (Int_t) GetNumberField(dumstr, " ", 3);
      }

      //read next line
      line.ReadLine(read_file);
   }
   if (line.Contains("Echelles")) {
      //found beginning of scalers
      line.ReadLine(read_file);
      //find first scaler line
      while (read_file.good() && !line.Contains(":"))  line.ReadLine(read_file);
      //now read all scalers
      while (read_file.good() && line.Contains(":")) {
         dumstr = GetStringField(line, ":", 0);
         dumstr.Remove(TString::kBoth, ' ');
         Int_t scaler = (Int_t)GetNumberField(line);
         //cout << "SCALER: \"" << dumstr.Data() << "\" = " << scaler << endl;
         indra_db_run->SetScaler(dumstr.Data(), scaler);
         line.ReadLine(read_file);
         if (fMakeTree) {
            dumstr.ReplaceAll(" ", "_");
            dumstr.ReplaceAll("-", "_");
            TBranch* br_sca = fTree->FindBranch(dumstr.Data());
            Int_t index;
            //do we need to add a branch for this scaler ?
            if (!br_sca) {
               index = fTree->GetListOfBranches()->GetEntries();
               br_sca = fTree->Branch(dumstr.Data(), &fScalers[index], Form("%s/I", dumstr.Data()));
               //cout << "br_sca = fTree->Branch( \"" <<dumstr.Data()<<"\", &fScalers["<<index<<"], "<<Form("%s/I", dumstr.Data())<<");"<<endl;
            }
            else {
               index = fTree->GetListOfBranches()->IndexOf(br_sca);
            }
            //fill tableau with value
            fScalers[index] = scaler;
            //cout << "fScalers["<<index<<"] = "<<scaler<<";"<<endl;
         }
      }
   }
   //in case events sauves = buffers sauves = 0, we use events lus & buffers ctrl
   if (eve_sav == 0)
      indra_db_run->SetEvents(eve_lus);
   if (buf_sav == 0) {
      indra_db_run->SetScaler("Buffers", buf_ctrl);
      indra_db_run->SetSize(buf_ctrl * 16384. / 1024. / 1024.);
   }

   temps_mort = 100. * indra_db_run->GetTempsMort();
   indra_db_run->SetTMpercent(temps_mort);

   istart -= fTIME_START;
   iend -= fTIME_START;

   if (fMakeTree)
      fTree->Fill();

   return kTRUE;
}

//_______________________________________________________________________________________________//

Double_t KVINDRARunSheetReader::GetNumberField(TString& line,
      const Char_t* delim,
      int index)
{
   //Split line according to delimiter.
   //Return the index-th element (index = 0, 1, 2, ...) as a Double_t
   TObjArray* toks = line.Tokenize(delim);
   KVString val = ((TObjString*) toks->At(index))->GetString();
   delete toks;
   return val.Atof();
}

//_______________________________________________________________________________________________//

TString KVINDRARunSheetReader::GetStringField(TString& line,
      const Char_t* delim,
      int index)
{
   //Split line according to delimiter.
   //Return the index-th element (index = 0, 1, 2, ...) as a string
   TObjArray* toks = line.Tokenize(delim);
   TString val = ((TObjString*) toks->At(index))->GetString();
   delete toks;
   return val;
}

//_______________________________________________________________________________________________//

TString KVINDRARunSheetReader::GetDateField(TString& line,
      const Char_t* delim)
{
   //Special case of GetStringField for date-strings with ':' used in between hours, minutes, seconds.
   //If ':' is also the delimiter of the string, we need to stick the time back together.
   TObjArray* toks = line.Tokenize(delim);

   TString date =
      ((TObjString*) toks->At(1))->GetString().Strip(TString::kLeading,
            ' ');
   if (!strcmp(delim, ":")) {
      for (int i = 2; i < toks->GetEntries(); i++) {
         date += ":";
         date += ((TObjString*) toks->At(i))->GetString();
      }
   }
   delete toks;
   date = date.Strip(TString::kTrailing, ' ');
   return date;
}

//_______________________________________________________________________________________________//

Int_t KVINDRARunSheetReader::ReadAllRunSheets(Int_t first, Int_t last)
{
   //Read all run sheets in the directory fRunSheetDir from run number 'first' to 'last'
   //(default values are 1 and 9999).
   //Returns the number of run sheets actually read.

   Int_t n_lus = 0;             //number of run sheets read

   if (fMakeTree) CreateTree();

   for (int run = first; run <= last; run++) {

      //Check run sheet file exists - if not, skip run
      if (gSystem->AccessPathName(GetRunSheetFileName(run)))
         continue;

      n_lus += (Int_t) ReadRunSheet(run);
   }

   if (fMakeTree) fTree->Print();
   return n_lus;
}
