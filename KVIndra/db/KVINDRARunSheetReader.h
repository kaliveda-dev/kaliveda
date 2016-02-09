/*******************************************************************************
$Id: KVINDRARunSheetReader.h,v 1.10 2007/10/24 14:10:26 franklan Exp $
$Revision: 1.10 $
$Date: 2007/10/24 14:10:26 $
$Author: franklan $
*******************************************************************************/

#ifndef __KVINDRARUNSHEETREADER_H
#define __KVINDRARUNSHEETREADER_H

#include "KVString.h"
#include "KVNumberList.h"
#include "TTree.h"
#include "KVDatime.h"
#include "Riostream.h"

class KVINDRARunSheetReader {

   TString fRunSheetDir;        //full path to directory holding run sheets
   TString fFileFormat;         //format string for run sheet file names
   TString fFilePath;           //temporary string holding full filename of current runsheet

   Bool_t fMakeTree;            //set to kTRUE if TTree is to be filled
   TTree* fTree;                //TTree can be made and filled from info

   /***** temp variables used to read runsheet and (if required) fill TTree ******/
   KVDatime start_run, endrun;
   Double_t len_run, size, data_rate, acq_rate, trait_rate, ctrl_rate,
            rempli_dlt_pc, temps_mort;
   Int_t run_num, buf_sav, eve_sav, eve_lus, buf_ctrl, eve_ctrl,
         rempli_dlt_blocs;
   Int_t istart, iend;
   Int_t fScalers[1024];//scalers for tree
   TString stat_eve;

   UInt_t fTIME_START;          //zero-time for all run start/stop times

   void init_vars();

public:

   KVINDRARunSheetReader(Bool_t make_tree = kFALSE);
   virtual ~ KVINDRARunSheetReader()
   {
   };

   const Char_t* GetRunSheetDir()
   {
      return fRunSheetDir;
   };
   void SetRunSheetDir(const Char_t* dir)
   {
      fRunSheetDir = dir;
   };
   const Char_t* GetFileFormat()
   {
      return fFileFormat;
   };
   void SetFileFormat(const Char_t* fmt)
   {
      fFileFormat = fmt;
   };
   const Char_t* GetRunSheetFileName(Int_t run);
   Double_t GetNumberField(TString&, const Char_t* delim =
                              ":", int index = 1);
   TString GetStringField(TString&, const Char_t* delim =
                             ":", int index = 1);
   TString GetDateField(TString&, const Char_t* delim = ":");

   Bool_t IsMakeTree() const
   {
      return fMakeTree;
   };
   void SetMakeTree(Bool_t yes = kTRUE)
   {
      std::cout << "SetMakeTree called with yes=" << yes << std::endl;
      fMakeTree = yes;
   };
   void CreateTree();

   TTree* GetTree() const
   {
      return fTree;
   };
   void StartTreeViewer() const
   {
      if (fTree) fTree->StartViewer();
   };

   Bool_t ReadRunSheet(Int_t run);
   Int_t ReadAllRunSheets(Int_t first = 1, Int_t last = 9999);

   ClassDef(KVINDRARunSheetReader, 0)   //For reading run sheets of INDRA experiments
};

#endif
