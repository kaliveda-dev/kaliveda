//Created by KVClassFactory on Tue Jul 17 08:46:41 2012
//Author: John Frankland,,,

#include "KVSimDir.h"
#include "KVSimFile.h"
#include "TSystemDirectory.h"
#include "TSystem.h"
#include "KVString.h"
#include "TTree.h"
#include "TKey.h"
#include "TFile.h"
#include "TBranchElement.h"
#include <iostream>
using namespace std;

ClassImp(KVSimDir)



KVSimDir::KVSimDir()
{
   // Default constructor
   init();
}

KVSimDir::KVSimDir(const Char_t* name, const Char_t* path)
   : KVBase(name, path)
{
   // Ctor with name and directory to analyse
   init();
   // make sure we have absolute path
   KVString _path(path);
   gSystem->ExpandPathName(_path);
   if (!gSystem->IsAbsoluteFileName(_path)) {
      if (_path.CountChar('.') == 1) {
         _path.ReplaceAll(".", "$(PWD)");
         gSystem->ExpandPathName(_path);
      }
   }
   SetTitle(_path);
}

void KVSimDir::init()
{
   // Default initialisations
   fSimData.SetName("Simulated Data");
   fFiltData.SetName("Filtered Simulated Data");
}

//________________________________________________________________

KVSimDir::KVSimDir(const KVSimDir& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVSimDir::~KVSimDir()
{
   // Destructor
}

//________________________________________________________________

void KVSimDir::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSimDir::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVSimDir& CastedObj = (KVSimDir&)obj;
}

//________________________________________________________________

void KVSimDir::AnalyseDirectory()
{
   // Read contents of directory given to ctor.
   // Each ROOT file will be analysed by AnalyseFile().

   Info("AnalyseDirectory", "Analysing %s...", GetDirectory());
   fSimData.Clear();
   fFiltData.Clear();
   //loop over files in current directory
   TSystemDirectory thisDir(".", GetDirectory());
   unique_ptr<TList> fileList(thisDir.GetListOfFiles());
   TIter nextFile(fileList.get());
   TSystemFile* aFile = 0;
   while ((aFile = (TSystemFile*)nextFile())) {

      if (aFile->IsDirectory()) continue;

      KVString fileName = aFile->GetName();
      if (!fileName.EndsWith(".root") && !fileName.Contains(".root."))  continue; /* skip non-ROOT files */

      AnalyseFile(fileName);

   }
}

//________________________________________________________________

void KVSimDir::AnalyseFile(const Char_t* filename)
{
   // Analyse ROOT file given as argument.
   // If there is a TTree in the file, then we look at all of its branches until we find one
   // containing objects which derive from KVEvent:
   //
   //   -- if they inherit from KVSimEvent, we add the file to the list of simulated data:
   //            * a KVSimFile is created. The title of the TTree where data were found will
   //               be used as 'Information' on the nature of the simulation.
   //   -- if they inherit from KVReconstructedEvent, we add the file to the list of filtered data.
   //            * a KVSimFile is created. Informations on the filtered data are extracted from
   //              TNamed objects in the file with names 'Dataset', 'System', 'Run', 'Geometry'
   //              (type of geometry used, 'ROOT' or 'KV'), 'Origin' (i.e. the name of the simulation
   //              file which was filtered), 'Filter' (type of filter: Geo, GeoThresh or Full).
   //              These objects are automatically created when data is filtered using KVEventFiltering.
   //
   // Analysis of the file stops after the first TTree with a branch satisfying one of the
   // two criteria is found (it is assumed that in each file there is only one TTree containing
   // either simulated or filtered data).

   //Info("AnalyseFile", "Analysing file %s...", filename);
   TString fullpath;
   AssignAndDelete(fullpath, gSystem->ConcatFileName(GetDirectory(), filename));
   unique_ptr<TFile> file(TFile::Open(fullpath));
   if (!file.get() || file->IsZombie()) return;
   // look for TTrees in file
   TIter next(file->GetListOfKeys());
   TKey* key;
   while ((key = (TKey*)next())) {
      TString cn = key->GetClassName();
      if (cn == "TTree") {
         // look for branch with KVEvent objects
         TTree* tree = (TTree*)file->Get(key->GetName());
         TSeqCollection* branches = tree->GetListOfBranches();
         TIter nextB(branches);
         TBranchElement* branch;
         while ((branch = (TBranchElement*)nextB())) {
            TString branch_classname = branch->GetClassName();
            TClass* branch_class = TClass::GetClass(branch_classname, kFALSE, kTRUE);
            if (branch_class && branch_class->InheritsFrom("KVEvent")) {
               if (branch_class->InheritsFrom("KVReconstructedEvent")) {
                  // filtered data. there must be TNamed called 'Dataset', 'System', & 'Run' in the file.
                  unique_ptr<TNamed> ds((TNamed*)file->Get("Dataset"));
                  unique_ptr<TNamed> orig((TNamed*)file->Get("Origin"));
                  unique_ptr<TNamed> sys((TNamed*)file->Get("System"));
                  unique_ptr<TNamed> r((TNamed*)file->Get("Run"));
                  unique_ptr<TNamed> g((TNamed*)file->Get("Geometry"));
                  unique_ptr<TNamed> f((TNamed*)file->Get("Filter"));
                  TString dataset;
                  if (ds.get()) dataset = ds->GetTitle();
                  TString system;
                  if (sys.get()) system = sys->GetTitle();
                  TString run;
                  if (r.get()) run = r->GetTitle();
                  TString origin;
                  if (orig.get()) origin = orig->GetTitle();
                  TString geometry;
                  if (g.get()) geometry = g->GetTitle();
                  TString filter;
                  if (f.get()) filter = f->GetTitle();
                  Int_t run_number = run.Atoi();
                  KVSimFile* fff = new KVSimFile(this, filename, tree->GetTitle(), tree->GetEntries(), tree->GetName(), branch->GetName(),
                                                 dataset, system, run_number, geometry, origin, filter);
                  fFiltData.Add(fff);
                  unique_ptr<TNamed> gem((TNamed*)file->Get("Gemini++"));
                  if (gem.get()) {
                     if (!strcmp(gem->GetTitle(), "yes")) fff->SetGemini();
                     unique_ptr<TNamed> gemdec((TNamed*)file->Get("GemDecayPerEvent"));
                     if (gemdec.get()) {
                        TString gemdecperev = gemdec->GetTitle();
                        fff->SetGemDecayPerEvent(gemdecperev.Atoi());
                     }
                  }
                  return;
               }
               else {
                  fSimData.Add(new KVSimFile(this, filename, tree->GetTitle(), tree->GetEntries(), tree->GetName(), branch->GetName()));
                  return;
               }
            }
         }
      }
   }
}

void KVSimDir::AddSimData(KVSimFile* f)
{
   fSimData.Add(f);
}

void KVSimDir::AddFiltData(KVSimFile* f)
{
   fFiltData.Add(f);
}

//________________________________________________________________

void KVSimDir::ls(Option_t*) const
{
   TROOT::IndentLevel();
   cout << "SIMULATION SET: " << GetName() << endl;
   cout << "DIRECTORY: " << GetDirectory() << endl;
   cout << "CONTENTS:" << endl;
   cout << "--simulated data:" << endl;
   fSimData.ls();
   cout << "--filtered data:" << endl;
   fFiltData.ls();
}
