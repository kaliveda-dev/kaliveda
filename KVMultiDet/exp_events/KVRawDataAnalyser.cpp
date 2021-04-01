//Created by KVClassFactory on Thu Sep 24 11:07:45 2009
//Author: John Frankland,,,

#include "KVRawDataAnalyser.h"
#include "KVMultiDetArray.h"
#include "KVClassFactory.h"
#include "KVDataSet.h"
#include "TSystem.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVRawDataAnalyser)



KVRawDataAnalyser::KVRawDataAnalyser()
{
   // Default constructor
   fRunFile = 0;
   TotalEntriesToRead = 0;
   fTreeFile = nullptr;
}

KVRawDataAnalyser::~KVRawDataAnalyser()
{
   // Destructor
}

void KVRawDataAnalyser::ProcessRun()
{
   // Perform treatment of a given run
   // Before processing each run, after opening the associated file, user's InitRun() method is called.
   // After each run, user's EndRun() is called.
   // For each event of each run, user's Analysis() method is called just after calling
   // gMultiDetArray->HandleRawDataEvent(fRunfile). In the Analysis() method the user can
   // test gMultiDetArray->HandledRawData() to know if some pertinent data was found in
   // the event or not.
   //
   // For further customisation, the pre/post-methods are called just before and just after
   // each of these methods (preInitRun(), postAnalysis(), etc. etc.)

   //Open data file
   KVString raw_file = gDataSet->GetFullPathToRunfile(GetDataType().Data(), fRunNumber);
   fRunFile = gDataSet->OpenRunfile<KVRawDataReader>(GetDataType().Data(), fRunNumber);
   if ((!fRunFile) || fRunFile->IsZombie()) {
      //skip run if file cannot be opened
      if (fRunFile) delete fRunFile;
      return;
   }

   //warning! real number of run may be different from that deduced from file name
   //we get the real run number from the data and use it to name any new files
   // Not possible for INDRAFAZIA MFM data (we use 'fake' run numbers)
   // Is this still necessary? (which dataset was concerned? camp5?)
//   Int_t newrun = fRunFile->GetRunNumberReadFromFile();
//   if (newrun && newrun != fRunNumber) {
//      cout << " *** WARNING *** run number read from file = " << newrun << endl;
//      fRunNumber = newrun;
//   }

   KVMultiDetArray::MakeMultiDetector(gDataSet->GetName(), fRunNumber);
   fCurrentRun = gExpDB->GetDBRun(fRunNumber);

   // perform any initialisations which may be required, dependent on the
   // format of the data, the multidetector, etc.
   gMultiDetArray->InitialiseRawDataReading(fRunFile);

   fEventNumber = 1; //event number

   Long64_t nevents = GetNbEventToRead();
   if (nevents <= 0) {
      nevents = 1000000000;
      cout << endl << "Reading all events from file " << raw_file.Data() << endl;
   }
   else {
      cout << endl << "Reading " << nevents << " events from file " << raw_file.Data() << endl;
   }

   cout << "Starting analysis of run " << fRunNumber << " on : ";
   TDatime now;
   cout <<  now.AsString() << endl << endl;

   fCurrentRun->Print();

   preInitRun();
   //call user's beginning of run
   InitRun();
   postInitRun();

   //loop over events in file
   while ((nevents-- ? fRunFile->GetNextEvent() : kFALSE) && !AbortProcessingLoop()) {

      gMultiDetArray->HandleRawDataEvent(fRunFile);
      preAnalysis();
      //call user's analysis. stop if returns kFALSE.
      if (!Analysis()) break;
      postAnalysis();

      if (CheckStatusUpdateInterval(fEventNumber)) DoStatusUpdate(fEventNumber);

      fEventNumber += 1;
   }

   cout << "Ending analysis of run " << fRunNumber << " on : ";
   TDatime now2;
   cout <<  now2.AsString() << endl << endl;
   cout << endl << "Finished reading " << fEventNumber - 1 << " events from file " << raw_file.Data() << endl << endl;

   preEndRun();
   //call user's end of run function
   EndRun();
   postEndRun();

   delete fRunFile;
}


void KVRawDataAnalyser::SubmitTask()
{
   // Perform analysis of chosen runs
   // Before beginning the loop over the runs, the user's InitAnalysis() method is called.
   // After completing the analysis of all runs, the user's EndAnalysis() method is called.
   //
   // Further customisation of the event loop is possible by overriding the methods
   //   preInitAnalysis()
   //   postInitAnalysis()
   //   preEndAnalysis()
   //   postEndAnalysis()
   // which are executed respectively just before and just after the methods.

   if (gDataSet != GetDataSet()) GetDataSet()->cd();

   preInitAnalysis();
   //call user's initialisation
   InitAnalysis();
   postInitAnalysis();

   CalculateTotalEventsToRead();

   //loop over runs
   GetRunList().Begin();
   while (!GetRunList().End() && !AbortProcessingLoop()) {
      fRunNumber = GetRunList().Next();
      ProcessRun();
   }

   if (fCombinedOutputFile != "") {
      Info("Terminate", "combine = %s", fCombinedOutputFile.Data());
      // combine histograms and trees from analysis into one file
      TString file1, file2;
      file1.Form("HistoFileFrom%s.root", ClassName());
      file2.Form("TreeFileFrom%s.root", ClassName());
      if (fHistoList.GetEntries()) {
         if (fTreeFile) {
            Info("Terminate", "both");
            SaveHistos();
            fTreeFile->Write();
            delete fTreeFile;
            KVBase::CombineFiles(file1, file2, fCombinedOutputFile, kFALSE);
         }
         else {
            // no trees - just rename histo file
            Info("Terminate", "histo");
            SaveHistos(fCombinedOutputFile);
         }
      }
      else if (fTreeFile) {
         // no histos - just rename tree file
         Info("Terminate", "tree");
         fTreeFile->Write();
         delete fTreeFile;
         gSystem->Rename(file2, fCombinedOutputFile);
      }
      else  Info("Terminate", "none");
   }
   else {
      SaveHistos();
      if (fTreeFile) {
         fTreeFile->Write();
         delete fTreeFile;
      }
   }

   preEndAnalysis();
   //call user's end of analysis
   EndAnalysis();
   postEndAnalysis();
}


void KVRawDataAnalyser::CalculateTotalEventsToRead()
{
   //loop over runs and calculate total events
   TotalEntriesToRead = 0;
   GetRunList().Begin();
   while (!GetRunList().End()) {
      Int_t r = GetRunList().Next();
      TotalEntriesToRead += gExpDB->GetDBRun(r)->GetEvents();
   }
}

void KVRawDataAnalyser::AddTree(TTree* tree)
{
   // Add a user analysis results TTree to list which will be automatically
   // written to disk at end of analysis.
   // User must call CreateTreeFile(const Char_t*) before calling this method

   if (!fTreeFile) {
      Error("AddTree", "You must call CreateTreeFile(const Char_t*) before using this method!");
      return;
   }
   tree->SetDirectory(fTreeFile);
   tree->AutoSave();
   fTreeList.Add(tree);
}

void KVRawDataAnalyser::FillHisto(const Char_t* histo_name, Double_t one, Double_t two, Double_t three, Double_t four)
{
   //Find in the list, if there is an histogram named "histo_name"
   //If not print an error message
   //If yes redirect to the right method according to its closest mother class
   //to fill it

   TH1* h1 = 0;
   if ((h1 = GetHisto(histo_name))) {
      if (h1->InheritsFrom("TH3"))
         FillTH3((TH3*)h1, one, two, three, four);
      else if (h1->InheritsFrom("TProfile2D"))
         FillTProfile2D((TProfile2D*)h1, one, two, three, four);
      else if (h1->InheritsFrom("TH2"))
         FillTH2((TH2*)h1, one, two, three);
      else if (h1->InheritsFrom("TProfile"))
         FillTProfile((TProfile*)h1, one, two, three);
      else if (h1->InheritsFrom("TH1"))
         FillTH1(h1, one, two);
      else
         Warning("FillHisto", "%s -> Classe non prevue ...", fHistoList.FindObject(histo_name)->ClassName());
   }
   else {
      Warning("FillHisto", "%s introuvable", histo_name);
   }
}

void KVRawDataAnalyser::FillHisto(const Char_t* histo_name, const Char_t* label, Double_t weight)
{
   // Fill 1D histogram with named bins
   TH1* h1 = 0;
   if ((h1 = GetHisto(histo_name))) {
      h1->Fill(label, weight);
   }
   else {
      Warning("FillHisto", "%s introuvable", histo_name);
   }
}

void KVRawDataAnalyser::FillTree(const Char_t* tree_name)
{
   //Filltree method, the tree named tree_name
   //has to be declared with AddTTree(TTree*) method
   //
   //if no sname="", all trees in the list is filled
   //
   if (!strcmp(tree_name, "")) {
      fTreeList.Execute("Fill", "");
   }
   else {
      TTree* tt = 0;
      if ((tt = GetTree(tree_name))) {
         tt->Fill();
      }
      else {
         Warning("FillTree", "%s introuvable", tree_name);
      }
   }
}

void KVRawDataAnalyser::SaveHistos(const Char_t* filename, Option_t* option, Bool_t onlyfilled)
{
   // Write in file all histograms declared with AddHisto(TH1*)
   //
   // If no filename is specified, set default name : HistoFileFrom[name_of_class].root
   //
   // If a filename is specified, search in gROOT->GetListOfFiles() if
   // this file has been already opened
   //  - if yes write in it
   //  - if not, create it with the corresponding option, write in it
   // and close it just after
   //
   // onlyfilled flag allow to write all (onlyfilled=kFALSE, default)
   // or only histograms (onlyfilled=kTRUE) those have been filled

   if (!fHistoList.GetEntries()) return;

   TString histo_file_name = "";
   if (!strcmp(filename, ""))
      histo_file_name.Form("HistoFileFrom%s.root", ClassName());
   else
      histo_file_name = filename;

   Bool_t justopened = kFALSE;

   TFile* file = 0;
   TDirectory* pwd = gDirectory;
   //if filename correspond to an already opened file, write in it
   //if not open/create it, depending on the option ("recreate" by default)
   //and write in it
   if (!(file = (TFile*)gROOT->GetListOfFiles()->FindObject(histo_file_name.Data()))) {
      file = new TFile(histo_file_name.Data(), option);
      justopened = kTRUE;
   }
   file->cd();
   TIter next(GetHistoList());
   TObject* obj = 0;
   while ((obj = next())) {
      if (obj->InheritsFrom("TH1")) {
         if (onlyfilled) {
            if (((TH1*)obj)->GetEntries() > 0) {
               obj->Write();
            }
         }
         else {
            obj->Write();
         }
      }
   }
   if (justopened)
      file->Close();
   pwd->cd();
}

Bool_t KVRawDataAnalyser::CreateTreeFile(const Char_t* filename)
{
   // This method must be called before creating any user TTree in InitAnalysis().
   // If no filename is given, default name="TreeFileFrom[name of analysis class].root"

   TString tree_file_name;
   if (!strcmp(filename, ""))
      tree_file_name.Form("TreeFileFrom%s.root", ClassName());
   else
      tree_file_name = filename;

   TDirectory* savedir = gDirectory;
   fTreeFile = new TFile(tree_file_name, "RECREATE");
   savedir->cd();

   return kTRUE;
}

//_______________________________________________________________________//

void KVRawDataAnalyser::Make(const Char_t* kvsname)
{
   //Automatic generation of derived class for raw data analysis

   KVClassFactory cf(kvsname, "Analysis of raw data", "",
                     kTRUE, "RawAnalysisTemplate");

   cf.AddImplIncludeFile("KVMultiDetArray.h");
   cf.GenerateCode();
}

void KVRawDataAnalyser::AbortDuringRunProcessing()
{
   // Method called to abort analysis during processing of a run
}

/** \example ExampleRawAnalysis.cpp
# Example of an analysis class for raw (i.e. unreconstructed) data

This is the analysis class generated by default by KaliVedaGUI for raw data analysis.

\include ExampleRawAnalysis.h
*/

