//Created by KVClassFactory on Tue Apr 30 09:30:00 2013
//Author: dgruyer

#ifndef __KVLAUNCHERGUI_H
#define __KVLAUNCHERGUI_H

#include "TGFrame.h"
#include "TGButton.h"

#include "TList.h"
#include "TClass.h"

#include "KVNameValueList.h"

/**
\class KVLauncherGUI
\brief Generic GUI launcher
\ingroup GUI
*/

class KVLauncherGUI : public TGMainFrame {

public:
   TList fButtonList;

   virtual void SetDefault();
   virtual void MapAll();
   virtual void Build();

public:
   KVLauncherGUI();
   virtual ~KVLauncherGUI();

   virtual void AddButtons();
   virtual TGTextButton* AddButton(const char* name, const char* method, TObject* obj = 0);
   TGTextButton* GetButton(const char* method);
   void SetEnabled(const char* method, Bool_t e = kTRUE);

   Bool_t IsEnabled(const char* method);
   void DummyMethod();
   virtual void Close();

   ClassDef(KVLauncherGUI, 1) //Generic GUI launcher
};

#endif
