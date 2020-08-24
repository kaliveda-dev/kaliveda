//Created by KVClassFactory on Tue Jul 17 09:05:17 2012
//Author: John Frankland,,,

#include "KVSimFile.h"
#include "KVSimDir.h"
#include <iostream>

ClassImp(KVSimFile)

using namespace std;

KVSimFile::KVSimFile()
{
   // Default constructor
   fFiltered = fGemini = kFALSE;
   fGemDecayPerEvent = 1;
}

KVSimFile::KVSimFile(KVSimDir* parent, const Char_t* filename, const Char_t* treeinfo, Long64_t treeEntries, const Char_t* treename, const Char_t* branchname)
   : KVBase(filename, treeinfo), fSimDir(parent), fFiltered(kFALSE), fEvents(treeEntries), fTreeName(treename), fBranchName(branchname)
{
   // Default constructor for simulated events file
   fGemini = kFALSE;
   fGemDecayPerEvent = 1;
}

KVSimFile::KVSimFile(KVSimDir* parent, const Char_t* filename, const Char_t* treeinfo, Long64_t treeEntries, const Char_t* treename, const Char_t* branchname,
                     const Char_t* dataset, const Char_t* system, Int_t run_number, const Char_t* geo_type, const Char_t* orig_file, const Char_t* filt_type)
   : KVBase(filename, treeinfo), fSimDir(parent), fFiltered(kTRUE), fEvents(treeEntries), fDataSet(dataset), fSystem(system),
     fRunNumber(run_number), fGeoType(geo_type), fTreeName(treename), fBranchName(branchname), fOrigFile(orig_file), fFiltType(filt_type)
{
   // Default constructor for filtered (reconstructed) simulated events file
   fGemini = kFALSE;
   fGemDecayPerEvent = 1;
}
//________________________________________________________________

KVSimFile::KVSimFile(const KVSimFile& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVSimFile::~KVSimFile()
{
   // Destructor
}

//________________________________________________________________

void KVSimFile::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSimFile::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   KVSimFile& CastedObj = (KVSimFile&)obj;
   CastedObj.fSimDir = fSimDir;
   CastedObj.fFiltered = fFiltered;
   CastedObj.fEvents = fEvents;
   CastedObj.fDataSet = fDataSet;
   CastedObj.fSystem = fSystem;
   CastedObj.fRunNumber = fRunNumber;
   CastedObj.fGeoType = fGeoType;
   CastedObj.fTreeName = fTreeName;
   CastedObj.fBranchName = fBranchName;
   CastedObj.fOrigFile = fOrigFile;
   CastedObj.fFiltType = fFiltType;
   CastedObj.fGemini = fGemini;
   CastedObj.fGemDecayPerEvent = fGemDecayPerEvent;
}

void KVSimFile::ls(Option_t*) const
{
   TROOT::IndentLevel();
   if (!fFiltered) cout << "SIMULATED EVENTS FILE : " << GetName() << endl;
   else cout << "FILTERED EVENTS FILE : " << GetName() << endl;
   cout << "--treename: " << GetTreeName() << endl;
   cout << "--branchname: " << GetBranchName() << endl;
   cout << "--infos: " << GetTitle() << endl;
   cout << "--events: " << fEvents << endl;
   if (fFiltered) {
      cout << "--Filtering conditions:" << endl;
      cout << "----dataset:" << fDataSet << endl;
      cout << "----system:" << fSystem << endl;
      cout << "----run:" << fRunNumber << endl;
      cout << "----geometry:" << fGeoType << endl;
      cout << "----filter:" << fFiltType << endl;
      cout << "----gemini:";
      if (fGemini) cout << " " << fGemDecayPerEvent << " decays per event";
      else cout << " no";
      cout << endl;
   }
}
