//Created by KVClassFactory on Wed Apr  3 15:57:14 2013
//Author: John Frankland,,,

#ifndef __KVTestListView_H
#define __KVTestListView_H
#include "RQ_OBJECT.h"
#include <KVBase.h>
#include <TGFrame.h>
#include <TTimer.h>
#include <TGButton.h>
#include "KVListView.h"

class KVTestListView : public KVBase {
   RQ_OBJECT("KVTestListView")
   TGMainFrame* MainFrame;
   KVListView* fLV;

public:
   KVTestListView(TClass* obclass, Int_t ncols, TString* columns);
   virtual ~KVTestListView();
   void DoClose();
   void CloseWindow();
   void DisplayList(TCollection* l)
   {
      fLV->Display(l);
   }

   ClassDef(KVTestListView, 1) //simple GUI with KVListView
};

#endif
