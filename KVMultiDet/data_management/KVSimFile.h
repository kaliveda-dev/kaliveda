//Created by KVClassFactory on Tue Jul 17 09:05:17 2012
//Author: John Frankland,,,

#ifndef __KVSIMFILE_H
#define __KVSIMFILE_H

#include "KVBase.h"
#include "TROOT.h"

class KVSimDir;

class KVSimFile : public KVBase {
protected:
   KVSimDir* fSimDir;//parent directory
   Bool_t fFiltered;//=kTRUE for filtered (reconstructed) simulated events
   Long64_t fEvents;//=number of events in TTree
   TString fDataSet;//name of dataset used to filter events
   TString fSystem;//name of system used to filter events
   Int_t fRunNumber;//run number used to filter events
   TString fGeoType;//type of geometry used for filtering
   TString fTreeName;//name of TTree
   TString fBranchName;//name of branch containing events
   TString fOrigFile;//name of simulated events file filtered to generate this file
   TString fFiltType;//type of filtering used (Geo, Geo+Thresh, or Full)
   Bool_t fGemini;//true if statistical decay calculated with Gemini++ before filter

public:
   KVSimFile();
   KVSimFile(KVSimDir* parent, const Char_t* filename, const Char_t* treeinfo, Long64_t treeEntries, const Char_t* treename, const Char_t* branchname);
   KVSimFile(KVSimDir* parent, const Char_t* filename, const Char_t* treeinfo, Long64_t treeEntries, const Char_t* treename, const Char_t* branchname,
             const Char_t* dataset, const Char_t* system, Int_t run_number, const Char_t* geo_type, const Char_t* orig_file, const Char_t* filt_type);
   KVSimFile(const KVSimFile&) ;
   virtual ~KVSimFile();
   void Copy(TObject&) const;

   Bool_t IsFiltered() const
   {
      return fFiltered;
   }
   Bool_t IsGemini() const
   {
      return fGemini;
   }
   void SetGemini(Bool_t yes = kTRUE)
   {
      fGemini = yes;
   }

   KVSimDir* GetSimDir() const
   {
      // return pointer to KVSimDir dataset to which this file belongs
      return fSimDir;
   };
   Long64_t GetEvents() const
   {
      // return number of events in file
      return fEvents;
   };
   const Char_t* GetDataSet() const
   {
      // return name of dataset used to filter data
      return fDataSet;
   };
   const Char_t* GetSystem() const
   {
      // return name of experimental system used to filter data
      return fSystem;
   };
   const Char_t* GetFilterType() const
   {
      // return type of filter used to filter data
      return fFiltType;
   };
   Int_t GetRun() const
   {
      // return run number used to define experimental conditions for filtering data
      return fRunNumber;
   };
   const Char_t* GetGeometry() const
   {
      // return type of geometry used to filter simulation:
      // 'KV' = KaliVeda geometry and particle transport
      // 'ROOT' = TGeometry description of detector geometry and particle transport
      return fGeoType;
   };
   const Char_t* GetTreeName() const
   {
      // name of TTree containing data
      return fTreeName;
   };
   const Char_t* GetBranchName() const
   {
      // name of TBranch containing data
      return fBranchName;
   };
   const Char_t* GetOriginalFile() const
   {
      // name of simulation file which was filtered to produce this filtered data file
      return fOrigFile;
   };

   void ls(Option_t* opt = "") const;

   ClassDef(KVSimFile, 1) //Handle file containing simulated or filtered simulated data
};

#endif
