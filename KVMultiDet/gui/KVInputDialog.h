/*
$Id: KVInputDialog.h,v 1.4 2007/12/11 16:04:28 franklan Exp $
$Revision: 1.4 $
$Date: 2007/12/11 16:04:28 $
$Author: franklan $
*/

#ifndef __KVINPUTDIALOG_H
#define __KVINPUTDIALOG_H

#include "RQ_OBJECT.h"
class TGTransientFrame;
class TString;
class KVTextEntry;
class TGTextButton;
class TGWindow;

/**
 \class  KVInputDialog
 \brief General purpose dialog box asking for some input in the form of a string
 \ingroup GUI
 */
class KVInputDialog {

   RQ_OBJECT("KVInputDialog")

protected:
   TGTransientFrame* fMain;
   TString* fAnswer;            //the answer to the question
   KVTextEntry* fTextEntry;     //text entry for answer
   TGTextButton* fOKBut;        //OK button
   TGTextButton* fCancelBut;    //Cancel button
   Bool_t* fOK;                 //set to kTRUE if OK button is pressed

public:

   KVInputDialog(const TGWindow* main, const Char_t* question,
                 TString* answer, Bool_t* ok, const Char_t* tooltip =
                    "");
   virtual ~ KVInputDialog();

   void ReadAnswer();

   void DoClose();
   void CloseWindow();

   ClassDef(KVInputDialog, 0)  //General purpose dialog asking for a string value
};

#endif
