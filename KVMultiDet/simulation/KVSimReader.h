//Created by KVClassFactory on Wed Jun 30 17:45:01 2010
//Author: bonnet

#ifndef __KVSIMREADER_H
#define __KVSIMREADER_H

#include "KVBase.h"
#include "Riostream.h"
#include "KVSimEvent.h"
#include "KVSimNucleus.h"
#include "KVString.h"
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "KVList.h"
#include "TObject.h"
#include "TDatime.h"
#include "TStopwatch.h"
#include "KVFileReader.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TDirectory.h"


class KVSimReader : public KVFileReader {

   void init();

protected:

   TTree* tree;//!
   TFile* file;//!
   KVSimEvent* evt;//!
   KVSimNucleus* nuc;//!

   Int_t nevt;
   KVString tree_name, tree_title, branch_name, root_file_name;
   Bool_t kmode;
   Bool_t fMultiFiles;
   Int_t fFileIndex;

   //infos ou objets specifiques a une simulation
   //
   KVList* linked_info;    //Liste d'info (TNamed) enregistree dans l arbre
   KVList* linked_objects; //liste d'objets a enregistree avec l'arbre

   KVNameValueList* nv;

public:

   KVSimReader();
   virtual void ConvertEventsInFile(KVString filename);
   virtual ~KVSimReader();

   void CleanAll();

   void SetMultiFiles(Bool_t on = kTRUE) {
      fMultiFiles = on;
   }
   void SetFileIndex(Int_t i) {
      fFileIndex = i;
   }

   virtual KVString GetDate() {
      TDatime now;
      KVString stime;
      stime.Form("%d_%02d_%02d_%02d:%02d:%02d",
                 now.GetYear(),
                 now.GetMonth(),
                 now.GetDay(),
                 now.GetHour(),
                 now.GetMinute(),
                 now.GetSecond()
                );
      return stime;

   }

   void SetFillingMode(Bool_t mode = kTRUE) {
      kmode = mode;
   }
   virtual void DeclareTree(Option_t* option);

   TTree* GetTree() {
      return tree;
   }
   virtual void FillTree() {
      GetTree()->Fill();
   }
   virtual Bool_t HasToFill() {
      return kmode;
   }
   virtual void SaveTree();
   void Run(Option_t* option = "recreate");

   //------------------
   KVList* GetLinkedObjects();
   void AddObject(TObject* obj);
   void WriteObjects();
   void CreateObjectList();

   //------------------
   KVList* GetSimuInfo();
   void AddInfo(const Char_t* name, const Char_t* val);
   void AddInfo(TNamed* named);
   void WriteInfo();
   void CreateInfoList();

   //------------------
   virtual void ReadFile();
   virtual Bool_t ReadHeader();
   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();

   Int_t GetNumberOfEvents() {
      return nevt;
   }

   void SetTreeName(const Char_t* n) {
      tree_name = n;
   }
   void SetTreeTitle(const Char_t* n) {
      tree_title = n;
   }
   void SetROOTFileName(const Char_t* n) {
      root_file_name = n;
   }
   void SetBranchName(const Char_t* n) {
      branch_name = n;
   }

   virtual void ConvertAndSaveEventsInFile(KVString filename);

   static KVSimReader* MakeSimReader(const char* model_uri);

   ClassDef(KVSimReader, 1) //Base class to read output files for simulation and create tree using KVEvent type class
};

#endif
