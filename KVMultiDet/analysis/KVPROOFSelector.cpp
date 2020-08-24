#include "KVPROOFSelector.h"
#include <KVClassFactory.h>
#include <TDirectory.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TTree.h>
#include "TProof.h"
#include <iostream>
#include "KVBase.h"
#include "TROOT.h"

ClassImp(KVPROOFSelector)



void KVPROOFSelector::Begin(TTree* /*tree*/)
{
   // Need to parse options here for use in Terminate
   // Also, on PROOF, any KVDataAnalyser instance has to be passed to the workers
   // via the TSelector input list.

   ParseOptions();

   if (IsOptGiven("CombinedOutputFile")) {
      fCombinedOutputFile = GetOpt("CombinedOutputFile");
   }
   else if (gProof) {
      // when running with PROOF, if the user calls SetCombinedOutputFile()
      // in InitAnalysis(), it will only be executed on the workers (in SlaveBegin()).
      // therefore we call InitAnalysis() here, but deactivate CreateTreeFile(),
      // AddTree() and AddHisto() in order to avoid interference with workers
      fDisableCreateTreeFile = kTRUE;
      InitAnalysis();              //user initialisations for analysis
      fDisableCreateTreeFile = kFALSE;
   }
}

void KVPROOFSelector::SlaveBegin(TTree* /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).
   //
   // ParseOptions : Manage options passed as arguments
   //
   // Called user method InitAnalysis where users can create trees or histos
   // using the appropiate methods :
   // CreateTrees and CreateMethods
   //
   // Test the presence or not of such histo or tree
   // to manage it properly

   ParseOptions();

   if (IsOptGiven("CombinedOutputFile")) {
      fCombinedOutputFile = GetOpt("CombinedOutputFile");
      Info("SlaveBegin", "Output file name = %s", fCombinedOutputFile.Data());
   }

   fEventsRead = 0;

   InitAnalysis();              //user initialisations for analysis

   if (ltree->GetEntries() > 0)
      for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
         TTree* tt = (TTree*)ltree->At(ii);
         tt->SetDirectory(writeFile);
         tt->AutoSave();
      }
}

Bool_t KVPROOFSelector::CreateTreeFile(const Char_t* filename)
{
   // For PROOF:
   // This method must be called before creating any user TTree in InitAnalysis().
   // If no filename is given, default name="TreeFileFrom[name of selector class].root"

   if (fDisableCreateTreeFile) return kTRUE;

   if (!strcmp(filename, ""))
      tree_file_name.Form("TreeFileFrom%s.root", ClassName());
   else
      tree_file_name = filename;

   mergeFile = new TProofOutputFile(tree_file_name.Data(), "M");
   mergeFile->SetOutputFileName(tree_file_name.Data());

   TDirectory* savedir = gDirectory;
   writeFile = mergeFile->OpenFile("RECREATE");
   if (writeFile && writeFile->IsZombie()) SafeDelete(writeFile);
   savedir->cd();

   // Cannot continue
   if (!writeFile) {
      Info("CreateTreeFile", "could not create '%s': instance is invalid!", filename);
      return kFALSE;
   }
   return kTRUE;

}

Bool_t KVPROOFSelector::Process(Long64_t entry)
{
   // Processing will abort cleanly if static flag fCleanAbort has been set
   // by some external controlling process.
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   fEventsRead = entry;
   if (!(fEventsRead % fEventsReadInterval) && fEventsRead) {
      Info("Process", " +++ %lld events processed +++ ", fEventsRead);
      ProcInfo_t pid;
      if (gSystem->GetProcInfo(&pid) == 0) {
         std::cout << "     ------------- Process infos -------------" << std::endl;
         printf(" CpuSys = %f  s.    CpuUser = %f s.    ResMem = %f MB   VirtMem = %f MB\n",
                pid.fCpuSys, pid.fCpuUser, pid.fMemResident / 1024., pid.fMemVirtual / 1024.);
      }
   }

   Bool_t ok_anal = kTRUE;
   ok_anal = Analysis();     //user analysis

   return ok_anal;
}

void KVPROOFSelector::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.
   // if tree have been defined in the CreateTrees method
   // manage the merge of them in ProofLite session
   //

   if (ltree->GetEntries() > 0) {

      if (writeFile) {
         TDirectory* savedir = gDirectory;
         TTree* tt = 0;
         for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
            tt = (TTree*)ltree->At(ii);
            writeFile->cd();
            tt->Write();
         }
         mergeFile->Print();
         fOutput->Add(mergeFile);

         for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
            tt = (TTree*)ltree->At(ii);
            tt->SetDirectory(0);
         }

         gDirectory = savedir;
         writeFile->Close();
      }

   }

}

void KVPROOFSelector::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
   //
   // This method call the user defined EndAnalysis
   // where user can do what she wants
   //

   TDatime now;
   Info("Terminate", "Analysis ends at %s", now.AsString());

   if (fCombinedOutputFile != "") {
      Info("Terminate", "combine = %s", fCombinedOutputFile.Data());
      // combine histograms and trees from analysis into one file
      TString file1, file2;
      file1.Form("HistoFileFrom%s.root", ClassName());
      file2.Form("TreeFileFrom%s.root", ClassName());
      if (GetOutputList()->FindObject("ThereAreHistos")) {
         if (GetOutputList()->FindObject(file2)) {
            Info("Terminate", "both");
            SaveHistos();
            KVBase::CombineFiles(file1, file2, fCombinedOutputFile, kFALSE);
         }
         else {
            // no trees - just rename histo file
            Info("Terminate", "histo");
            SaveHistos(fCombinedOutputFile);
         }
      }
      else if (GetOutputList()->FindObject(file2)) {
         // no histos - just rename tree file
         Info("Terminate", "tree");
         gSystem->Rename(file2, fCombinedOutputFile);
      }
      else  Info("Terminate", "none");
   }

   EndAnalysis();               //user end of analysis routine
}

//____________________________________________________________________________

KVHashList* KVPROOFSelector::GetHistoList() const
{

   //return the list of created trees
   return lhisto;

}

//____________________________________________________________________________

TH1* KVPROOFSelector::GetHisto(const Char_t* histo_name) const
{

   return lhisto->get_object<TH1>(histo_name);

}

//____________________________________________________________________________

void KVPROOFSelector::AddHisto(TH1* histo)
{
   // Declare a histogram to be used in analysis.
   // This method must be called when using PROOF.

   if (fDisableCreateTreeFile) return;

   lhisto->Add(histo);
   fOutput->Add(histo);
   if (!fOutput->FindObject("ThereAreHistos")) fOutput->Add(new TNamed("ThereAreHistos", "...so save them!"));
}

void KVPROOFSelector::AddTree(TTree* tree)
{
   // Declare a TTree to be used in analysis.
   // This method must be called when using PROOF.

   if (fDisableCreateTreeFile) return;

   ltree->Add(tree);
}

//____________________________________________________________________________

void KVPROOFSelector::FillHisto(const Char_t* histo_name, Double_t one, Double_t two, Double_t three, Double_t four)
{

   //Find in the list, if there is an histogram named "sname"
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
         Warning("FillHisto", "%s -> Classe non prevue ...", lhisto->FindObject(histo_name)->ClassName());
   }
   else {
      Warning("FillHisto", "%s introuvable", histo_name);
   }

}

//____________________________________________________________________________

void KVPROOFSelector::FillTH1(TH1* h1, Double_t one, Double_t two)
{

   h1->Fill(one, two);

}

//____________________________________________________________________________

void KVPROOFSelector::FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three)
{

   h1->Fill(one, two, three);

}

//____________________________________________________________________________

void KVPROOFSelector::FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three)
{

   h2->Fill(one, two, three);

}

//____________________________________________________________________________

void KVPROOFSelector::FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four)
{

   h2->Fill(one, two, three, four);
}

//____________________________________________________________________________

void KVPROOFSelector::FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four)
{

   h3->Fill(one, two, three, four);
}

//____________________________________________________________________________

void KVPROOFSelector::SaveHistos(const Char_t* filename, Option_t* option, Bool_t onlyfilled)
{
   // Write in file all histograms declared with AddHisto(TH1*)
   // This method works with PROOF.
   //
   // If no filename is specified, set default name : HistoFileFrom[KVEvenSelector::GetName()].root
   //
   // If a filename is specified, search in gROOT->GetListOfFiles() if
   // this file has been already opened
   //  - if yes write in it
   //  - if not, create it with the corresponding option, write in it
   // and close it just after
   //
   // onlyfilled flag allow to write all (onlyfilled=kFALSE, default)
   // or only histograms (onlyfilled=kTRUE) those have been filled

   TString histo_file_name = "";
   if (!strcmp(filename, ""))
      histo_file_name.Form("HistoFileFrom%s.root", GetName());
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
   TIter next(GetOutputList());
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

//____________________________________________________________________________

KVHashList* KVPROOFSelector::GetTreeList() const
{
   //return the list of created trees
   return ltree;

}

//____________________________________________________________________________

TTree* KVPROOFSelector::GetTree(const Char_t* tree_name) const
{
   //return the tree named tree_name
   return ltree->get_object<TTree>(tree_name);

}

//____________________________________________________________________________

void KVPROOFSelector::FillTree(const Char_t* tree_name)
{
   //Filltree method, the tree named tree_name
   //has to be declared with AddTTree(TTree*) method
   //
   //if no sname="", all trees in the list is filled
   //
   if (!strcmp(tree_name, "")) {
      ltree->Execute("Fill", "");
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

void KVPROOFSelector::SetOpt(const Char_t* option, const Char_t* value)
{
   //Set a value for an option
   KVString tmp(value);
   fOptionList.SetValue(option, tmp);
}

//_________________________________________________________________

Bool_t KVPROOFSelector::IsOptGiven(const Char_t* opt)
{
   // Returns kTRUE if the option 'opt' has been set

   return fOptionList.HasParameter(opt);
}

//_________________________________________________________________

TString KVPROOFSelector::GetOpt(const Char_t* opt) const
{
   // Returns the value of the option
   // Only use after checking existence of option with IsOptGiven(const Char_t* opt)

   return fOptionList.GetTStringValue(opt);
}

//_________________________________________________________________

void KVPROOFSelector::UnsetOpt(const Char_t* opt)
{
   // Removes the option 'opt' from the internal lists, as if it had never been set

   fOptionList.RemoveParameter(opt);
}

void KVPROOFSelector::ParseOptions()
{
   // Analyse comma-separated list of options given to TTree::Process
   // and store all "option=value" pairs in fOptionList.
   // Options can then be accessed using IsOptGiven(), GetOptString(), etc.
   // This method is called by SlaveBegin

   fOptionList.Clear(); // clear list
   KVString option = GetOption();
   option.Begin(",");
   while (!option.End()) {

      KVString opt = option.Next();
      opt.Begin("=");
      KVString param = opt.Next();
      KVString val = opt.Next();
      while (!opt.End()) {
         val += "=";
         val += opt.Next();
      }

      SetOpt(param.Data(), val.Data());
   }

   fOptionList.Print();
}

void KVPROOFSelector::Make(const Char_t* classname)
{
   // Generate a new class derived from this one with given name

   KVClassFactory cf(classname, "User class for PROOF", "KVPROOFSelector");
   cf.SetInheritAllConstructors(kFALSE);
   cf.AddMethod("InitAnalysis", "void");
   cf.AddMethod("Analysis", "Bool_t");
   cf.AddMethodBody("Analysis", "   // Write your code here\n   return kTRUE;");
   cf.AddMethod("EndAnalysis", "void");
   cf.GenerateCode();
}
