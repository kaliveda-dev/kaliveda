/*
$Id: KVINDRADB_e475s.cpp,v 1.6 2009/04/09 09:23:20 ebonnet Exp $
$Revision: 1.6 $
$Date: 2009/04/09 09:23:20 $
*/

//Created by KVClassFactory on Tue Sep 18 11:17:53 2007
//Author: Eric Bonnet

#include "KVINDRADB_e475s.h"
#include "Riostream.h"
#include "TF1.h"
#include "TString.h"
#include "TObjString.h"
#include "TList.h"

#include "KVDBParameterSet.h"
#include "KVINDRA.h"
#include "KVMultiDetArray.h"
#include "KVDataSet.h"
#include "KVINDRADB.h"
#include "KVNumberList.h"

using namespace std;

ClassImp(KVINDRADB_e475s)



//------------------------------
void KVINDRADB_e475s::init()
//------------------------------
{
   //Define duplicate run, create the tables of the database.
   cout << "init de KVINDRADB_e475s" << endl;

   fCalibrations = AddTable("Calibrations", "List of available calibrations");

}


//------------------------------
KVINDRADB_e475s::KVINDRADB_e475s()
//------------------------------
{
   //Default constructor
   init();
}

//------------------------------
KVINDRADB_e475s::~KVINDRADB_e475s()
//------------------------------
{
   //Destructor
}

//------------------------------
KVINDRADB_e475s::KVINDRADB_e475s(const Char_t* name): KVINDRADB(name)
//------------------------------
{
   //ctor.
   init();
}

//------------------------------
void KVINDRADB_e475s::Build()
{
//------------------------------
   KVINDRADB::Build();

   ReadCalibrations();

}

//------------------------------
void KVINDRADB_e475s::ReadPedestalList()
//------------------------------
{

   ifstream finput;
   if (!OpenCalibFile("Pedestals", finput)) {
      Error("ReadPedestalList()", "Could not open file %s",
            GetCalibFileName("Pedestals"));
      return;
   }

   Info("ReadPedestalList()",
        "Reading calibration parameters...");

   TString sline;
   TString stit;
   TObjArray* toks = NULL;
   while (finput.good()) {
      TString calib_det = "", calib_acq = "", calib_file = "";
      sline.ReadLine(finput);
      if (!sline.IsNull()) {
         cout << sline << endl;
         toks = sline.Tokenize(" ");
         if (toks->GetEntries() >= 1) {
            calib_file = ((TObjString*)(*toks)[0])->GetString();
            if (toks->GetEntries() >= 2) {
               calib_det = ((TObjString*)(*toks)[1])->GetString();
               if (toks->GetEntries() >= 3) {
                  calib_acq = ((TObjString*)(*toks)[2])->GetString();
               }
            }
         }

         stit.Form("%s/%s", gDataSet->GetDataSetDir(), calib_file.Data());
         ifstream fin(stit.Data());
         ReadPedestalFile(fin, calib_det, calib_acq);
         fin.close();
      }
   }
   finput.close();


}

//------------------------------
void KVINDRADB_e475s::ReadCalibrations()
//------------------------------
{

   ifstream finput;
   if (!OpenCalibFile("CalibFile", finput)) {
      Error("ReadCalibrations()", "Could not open file %s",
            GetCalibFileName("CalibFile"));
      return;
   }

   Info("ReadCalibrations()",
        "Reading calibration parameters...");

   TString sline;
   TString stit;
   TString calib_det, calib_gain, calib_file;
   TObjArray* toks = NULL;
   while (finput.good()) {
      sline.ReadLine(finput);
      if (!sline.IsNull()) {
         cout << sline << endl;
         toks = sline.Tokenize(" ");
         calib_det = ((TObjString*)(*toks)[0])->GetString();
         calib_gain = ((TObjString*)(*toks)[1])->GetString();
         calib_file = ((TObjString*)(*toks)[2])->GetString();
         stit.Form("%s/%s", gDataSet->GetDataSetDir(), calib_file.Data());
         ifstream fin(stit.Data());
         ReadCalibFile(fin, calib_det, calib_gain);
         fin.close();
      }
   }
   finput.close();

}

//------------------------------
void KVINDRADB_e475s::ReadCalibFile(ifstream& fin, TString dettype, TString detgain)
{
//------------------------------
   Info("ReadCalibFile()",
        "Reading calibration parameters of file %s%s", dettype.Data(), detgain.Data());

   KVDBParameterSet* parset = nullptr;
   TList list_record;

   TString detname;
   TString sline;
   TString calib_type, calib_signal, calib_runs, calib_ring, calib_mods;
   unique_ptr<TObjArray> toks;
   unique_ptr<TObjArray> tokspara;
   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      while (sline.BeginsWith("+KVCalibrator")) {
         sline.ReadLine(fin);
         while (sline.BeginsWith("<LIST> Type=")) {
            sline.ReplaceAll("<LIST> Type=", "");
            if (sline.Contains("(")) {
               toks.reset(sline.Tokenize("("));
               sline = ((TObjString*)toks->At(0))->GetString();
            }
            calib_type = sline;
            sline.ReadLine(fin);
            while (sline.BeginsWith("<LIST> Signal=")) {
               sline.ReplaceAll("<LIST> Signal=", "");
               calib_signal = sline;
               cout << calib_signal << endl;
               if (calib_signal != dettype + detgain) Error("ReadCalibFile()", "Erreur de synchro ...");
               sline.ReadLine(fin);
               while (sline.BeginsWith("++<LIST> Runs=")) {
                  sline.ReplaceAll("++<LIST> Runs=", "");
                  calib_runs = sline;
                  cout << calib_runs << endl;
                  sline.ReadLine(fin);
                  list_record.RemoveAll();
                  while (sline.BeginsWith("<LIST> Rings=")) {
                     sline.ReplaceAll("<LIST> Rings=", "");
                     calib_ring = sline;
                     cout << calib_ring << endl;
                     sline.ReadLine(fin);
                     while (sline.BeginsWith("<LIST> Modules=") || sline.BeginsWith("<PARAMETER> Function=")) {
                        if (sline.BeginsWith("<LIST> Modules=")) {
                           sline.ReplaceAll("<LIST> Modules=", "");
                           calib_mods = sline;
                           cout << calib_mods << endl;
                           //sprintf(detname,"%s_%02d%02d_%s",dettype.Data(),calib_ring.Atoi(),calib_mods.Atoi(),detgain.Data());
                           detname.Form("%s_%02d%02d", dettype.Data(), calib_ring.Atoi(), calib_mods.Atoi());
                        }
                        else {
                           sline.ReplaceAll("<PARAMETER> Function=", "");
                           cout << sline << endl;
                           toks.reset(sline.Tokenize(":"));
                           TF1 ff(Form("f%s", detname.Data()), ((TObjString*)toks->At(0))->GetString().Data());

                           TString sparaline = ((TObjString*)toks->At(1))->GetString();

                           tokspara.reset(sparaline.Tokenize(" "));
                           if (tokspara->GetEntries() != ff.GetNpar()) cout << tokspara->GetEntries() << " " << ff.GetNpar() << endl;
                           for (Int_t pp = 0; pp < ff.GetNpar(); pp += 1) {
                              ff.SetParameter(pp, ((TObjString*)tokspara->At(pp))->GetString().Atof());
                              printf("%d %g\n", pp, ff.GetParameter(pp));
                           }

                           parset = new KVDBParameterSet(detname, Form("%s(%s)", calib_type.Data(), detgain.Data()), ff.GetNpar() + 3);
                           parset->SetParamName(0, ff.GetExpFormula().Data());
                           for (Int_t pp = 0; pp < ff.GetNpar(); pp += 1) {
                              parset->SetParameter(pp + 1, ff.GetParameter(pp));
                              parset->SetParamName(pp + 1, ff.GetParName(pp));
                           }
                           parset->SetParameter(ff.GetNpar() + 1, 0.);
                           parset->SetParamName(ff.GetNpar() + 1, "xmin");
                           parset->SetParameter(ff.GetNpar() + 2, 4096.);
                           parset->SetParamName(ff.GetNpar() + 2, "xmax");

                           fCalibrations->AddRecord(parset);
                           list_record.Add(parset);

                        }
                        sline.ReadLine(fin);
                     }
                  }
                  KVNumberList nl(calib_runs);
                  nl.Begin();
                  while (!nl.End()) {
                     Int_t run = nl.Next();
                     for (Int_t ll = 0; ll < list_record.GetEntries(); ll += 1)
                        if (!(((KVDBRecord*)list_record.At(ll))->AddLink("Runs", GetRun(run))))
                           Error("ReadCalibFile()", "Pb de chargements ...");
                  }
               }
            }
         }
      }
   }

   fin.close();
}

//------------------------------
void KVINDRADB_e475s::ReadPedestalFile(ifstream& fin, TString dettype, TString detgain)
{
//------------------------------
   Info("ReadPedestalFile()",
        "Reading pedestal value of file %s - %s", dettype.Data(), detgain.Data());

   KVDBParameterSet* parset = NULL;
   TList* list_record = new TList();

   Char_t detname[50];
   TString sline;
   TString calib_type, calib_signal, calib_det, calib_acq;
   TString calib_runs, calib_ring, calib_mods;
   TObjArray* toks = NULL;
   TObjArray* tokspara = NULL;
   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      while (sline.BeginsWith("+KVCalibrator")) {
         sline.ReadLine(fin);
         while (sline.BeginsWith("<LIST> Type=")) {
            sline.ReplaceAll("<LIST> Type=", "");
            calib_type = sline;
            cout << calib_type << endl;
            sline.ReadLine(fin);
            while (sline.BeginsWith("<LIST> KVDetector=")) {
               sline.ReplaceAll("<LIST> KVDetector=", "");
               calib_det = sline;
               cout << calib_det << endl;
               sline.ReadLine(fin);
               while (sline.BeginsWith("<LIST> KVACQParam=")) {
                  sline.ReplaceAll("<LIST> KVACQParam=", "");
                  calib_acq = sline;
                  cout << calib_acq << endl;
                  sline.ReadLine(fin);
                  while (sline.BeginsWith("++<LIST> Runs=")) {
                     sline.ReplaceAll("++<LIST> Runs=", "");
                     calib_runs = sline;
                     cout << calib_runs << endl;
                     sline.ReadLine(fin);

                     list_record->RemoveAll();
                     while (sline.BeginsWith("<LIST> Rings=")) {
                        sline.ReplaceAll("<LIST> Rings=", "");
                        calib_ring = sline;
                        cout << calib_ring << endl;
                        sline.ReadLine(fin);
                        while (sline.BeginsWith("<LIST> Modules=") || sline.BeginsWith("<PARAMETER> Function=")) {
                           if (sline.BeginsWith("<LIST> Modules=")) {
                              sline.ReplaceAll("<LIST> Modules=", "");
                              calib_mods = sline;
                              cout << calib_mods << endl;
                              //sprintf(detname,"%s_%02d%02d_%s",dettype.Data(),calib_ring.Atoi(),calib_mods.Atoi(),detgain.Data());
                              sprintf(detname, "%s_%02d%02d", calib_det.Data(), calib_ring.Atoi(), calib_mods.Atoi());
                           }
                           else {
                              sline.ReplaceAll("<PARAMETER> Function=", "");
                              cout << sline << endl;
                              toks = sline.Tokenize(":");
                              TF1 ff(Form("f%s", detname), ((TObjString*)(*toks)[0])->GetString().Data());

                              TString sparaline = ((TObjString*)(*toks)[1])->GetString();
                              delete toks;

                              tokspara = sparaline.Tokenize(" ");
                              if (tokspara->GetEntries() != ff.GetNpar()) cout << tokspara->GetEntries() << " " << ff.GetNpar() << endl;
                              for (Int_t pp = 0; pp < ff.GetNpar(); pp += 1) {
                                 ff.SetParameter(pp, ((TObjString*)(*tokspara)[pp])->GetString().Atof());
                                 printf("%d %lf\n", pp, ff.GetParameter(pp));
                              }
                              delete tokspara;

                              parset = new KVDBParameterSet(detname, Form("%s", calib_acq.Data()), ff.GetNpar() + 1);
                              parset->SetParamName(0, ff.GetExpFormula().Data());
                              for (Int_t pp = 0; pp < ff.GetNpar(); pp += 1) {
                                 parset->SetParameter(pp + 1, ff.GetParameter(pp));
                                 parset->SetParamName(pp + 1, ff.GetParName(pp));
                              }

                              fPedestals->AddRecord(parset);
                              list_record->Add(parset);

                           }
                           sline.ReadLine(fin);
                        }
                     }
                     KVNumberList nl(calib_runs);
                     nl.Begin();
                     while (!nl.End()) {
                        Int_t run = nl.Next();
                        for (Int_t ll = 0; ll < list_record->GetEntries(); ll += 1)
                           if (!(((KVDBRecord*)list_record->At(ll))->AddLink("Runs", GetRun(run))))
                              Error("ReadCalibFile()", "Pb de chargements ...");
                     }
                  }
               }
            }
         }
      }
   }
   fin.close();

   list_record->Clear();
   delete list_record;

}
