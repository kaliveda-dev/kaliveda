/*
$Id: KVTextEntry.cpp,v 1.1 2007/12/11 16:03:45 franklan Exp $
$Revision: 1.1 $
$Date: 2007/12/11 16:03:45 $
*/

//Created by KVClassFactory on Tue Dec 11 14:36:47 2007
//Author: John Frankland

#include "KVTextEntry.h"

ClassImp(KVTextEntry)

//______________________________________________________________________________
void KVTextEntry::SetText(const char* text, Bool_t emit)
{
   // Sets text entry to text, clears the selection and moves
   // the cursor to the end of the line.
   // Whatever the value of fMaxLen (set with SetMaxLength()),
   // no truncation takes place, i.e. the text is of unlimited length.

   TString oldText(GetText());

   fText->Clear();
   fText->AddText(0, text); // new text

   End(kFALSE);
   if (oldText != GetText()) {
      if (emit) TextChanged();           // emit signal
      fClient->NeedRedraw(this);
   }
}

//______________________________________________________________________________
void KVTextEntry::SetMaxLength(Int_t maxlen)
{
   // Set the maximum length of the text in the editor.
   // Any marked text will be unmarked.
   // The cursor position is set to 0 and the first part of the
   // string is shown, but no truncation ever takes place.

   fMaxLen = maxlen;

   SetCursorPosition(0);
   Deselect();
}

//______________________________________________________________________________
void KVTextEntry::Insert(const char* newText)
{
   // Removes any currently selected text, inserts newText,
   // sets it as the new contents of the text entry.

   TString old(GetText());
   TString t(newText);

   if (t.IsNull()) return;

   for (int i = 0; i < t.Length(); i++) {
      if (t[i] < ' ') t[i] = ' '; // unprintable/linefeed becomes space
   }

   Int_t minP = MinMark();
   Int_t maxP = MaxMark();
   Int_t cp = fCursorIX;

   if (HasMarkedText()) {
      fText->RemoveText(minP, maxP - minP);
      cp = minP;
   }

   if (fInsertMode == kReplace) fText->RemoveText(cp, t.Length());
   Int_t ncp = TMath::Min(cp + t.Length(), GetMaxLength());
   fText->AddText(cp, t.Data());

   SetCursorPosition(ncp);
   if (old != GetText()) TextChanged();
}

