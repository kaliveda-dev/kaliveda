//Created by KVClassFactory on Fri Feb 17 14:52:45 2017
//Author: John Frankland,,,

#ifndef __KVBATCHSYSTEMPARAMETERSGUI_H
#define __KVBATCHSYSTEMPARAMETERSGUI_H

#include "KVNameValueListGUI.h"
#include "KVDataAnalyser.h"

#include <TGNumberEntry.h>
#include <TGTextEntry.h>

/**
  \class  KVBatchSystemParametersGUI
   \brief Utility GUI used for setting batch system parameters
   \ingroup GUI
   */
class KVBatchSystemParametersGUI : public KVNameValueListGUI {
   KVDataAnalyser* fAnalyser;
   TGCheckButton* fAJN; // auto job name widget
   TGTextEntry* fJN; // job name widget
   TGTextEntry* fJNF;// job name format widget
   TGCheckButton* fMJ; // multijob widget
   TGNumberEntry* fRPJ; // runs per job widget

public:
   KVBatchSystemParametersGUI(const TGWindow* main, KVNameValueList* params, KVDataAnalyser* dan, Bool_t* cancel);

   virtual ~KVBatchSystemParametersGUI();

   void SetAutoBatchName(Bool_t on);
   void UpdateAutoBatchName(const char* = 0);

   ClassDef(KVBatchSystemParametersGUI, 1) //KVNameValueListGUI
};

#endif
