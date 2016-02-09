//Created by KVClassFactory on Thu May 16 11:38:27 2013
//Author: gruyer,,,

#ifndef __KVTEXTDIALOG_H
#define __KVTEXTDIALOG_H

#include "RQ_OBJECT.h"
class TGTransientFrame;
class TString;
class TGTextEntry;
class TGTextButton;
class TGWindow;

class KVTextDialog {
   RQ_OBJECT("KVTextDialog")

protected:
   TGTransientFrame* fMain;
   TString* fAnswer;            //the answer to the question
   TGTextEntry* fTextEntry;     //drop down list
   TGTextButton* fOKBut;        //OK button
   TGTextButton* fCancelBut;    //Cancel button
   Bool_t* fOK;                 //set to kTRUE if OK button is pressed

public:
   KVTextDialog(const TGWindow* main, const Char_t* question,
                const Char_t* default_value,
                TString* chosen, Bool_t* ok, const char* unit = "");
   virtual ~KVTextDialog();

   void ReadAnswer();

   void DoClose();
   void CloseWindow();

   ClassDef(KVTextDialog, 1) //
};

#endif
