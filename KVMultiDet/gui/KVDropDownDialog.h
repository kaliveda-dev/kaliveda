/*
$Id: KVDropDownDialog.h,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
$Author: franklan $
*/

#ifndef __KVDropDownDialog_H
#define __KVDropDownDialog_H

#include "RQ_OBJECT.h"
class TGTransientFrame;
class TString;
class TGComboBox;
class TGTextButton;
class TGWindow;

class KVDropDownDialog {

   RQ_OBJECT("KVDropDownDialog")

protected:
   TGTransientFrame* fMain;
   TString* fAnswer;            //the answer to the question
   TGComboBox* fDropDown;     //drop down list
   TGTextButton* fOKBut;        //OK button
   TGTextButton* fCancelBut;    //Cancel button
   Bool_t* fOK;                 //set to kTRUE if OK button is pressed

public:

   KVDropDownDialog(const TGWindow* main, const Char_t* question,
                    const Char_t* choice_list, const Char_t* default_choice,
                    TString* chosen, Bool_t* ok);
   virtual ~ KVDropDownDialog();

   void ReadAnswer();

   void DoClose();
   void CloseWindow();

   ClassDef(KVDropDownDialog, 0)  //General purpose dialog asking to choose from a drop down list
};

#endif
