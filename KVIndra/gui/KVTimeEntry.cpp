//
// Implementation de KVTimeEntry
//

#include "KVTimeEntry.h"
#include "Riostream.h"

ClassImp(KVTimeEntryLayout)

void KVTimeEntryLayout::Layout()
{
   // Layout the internal GUI elements in use.

   if (fBox == 0) {
      return;
   }
   UInt_t w = fBox->GetWidth();
   UInt_t h = fBox->GetHeight();
   UInt_t upw = 2 * h / 3;
   UInt_t numw = (w > h) ? w - upw : w;
   UInt_t numh = h;
   TGTextEntry* te = ((KVTimeEntry*)fBox)->GetTextEntry();
   if (te)
      te->MoveResize(0, 0, numw, numh);
   TGNumberEntryLayout::Layout();

}


ClassImp(KVTimeEntry)

//__________________________________________
KVTimeEntry::KVTimeEntry(const TGWindow* parent, Double_t val,
                         Int_t digitwidth, Int_t id,
                         EStyle ,
                         EAttribute attr,
                         ELimit limits,
                         Double_t min, Double_t max):
   TGNumberEntry(parent, val, digitwidth, id,
                 kNESHourMinSec, attr, limits, min, max)
{
//
// Contructor
//
   // first remove all things
   UnmapWindow();
   RemoveFrame(fNumericEntry);
   /* current layout manager is deleted by TGCompositeFrame::SetLayoutManager
     therefore the following is not necessary and will lead to a seg violation
     when SetLayoutManager is called. Strangely, until I compiled this with
     ROOT 5.24/00 and gcc 4.1.2 at ccali, this didn't seem to matter...??? */
   //delete GetLayoutManager();

   // Constructs a numeric entry widget.

   // create gui elements
   fTimeEntry = new TGTextEntry(this, "01234567890123456789" , id);
   fTimeEntry->SetAlignment(kTextRight);
   fTimeEntry->Associate(this);

   //AddFrame(fTimeEntry, 0);
   // Add the fTimeEntry as the first element of the composite frame
   TGFrameElement* nw = new TGFrameElement(fTimeEntry,
                                           new TGLayoutHints(kLHintsExpandX | kLHintsTop,
                                                 0, 20, 0, 0));
   fList->AddFirst(nw);
   // in case of recusive cleanup, propagate cleanup setting to all
   // child composite frames
   if (fMustCleanup == kDeepCleanup)
      fTimeEntry->SetCleanup(kDeepCleanup);

   // resize
   Int_t h = fTimeEntry->GetDefaultHeight();
   Int_t charw = fTimeEntry->GetWidth();
   Int_t w = charw * TMath::Abs(digitwidth) / 10 + 8 + 2 * h / 3;
   SetLayoutManager(new KVTimeEntryLayout(this));
   MapSubwindows();
   Resize(w, h);

}

//__________________________________________
KVTimeEntry::~KVTimeEntry()
{
//
// Destructor
//
}

//__________________________________________
void KVTimeEntry::IncreaseNumber(EStepSize step, Int_t sign, Bool_t)
{
//
// Increse or Decrease value according to the step
//
   Long_t mag = 0;
   Long_t l = GetIntNumber();
   switch (step) {
      case kNSSSmall:
         mag = 1;
         break;
      case kNSSMedium:
         mag = 60;
         break;
      case kNSSLarge:
         mag = 3600;
         break;
      case kNSSHuge:
         mag = 36000;
         break;
   }
   l += sign * mag;
   if (l < 0)
      l = 0;
   SetIntNumber(l);
}

//__________________________________________
void KVTimeEntry::SetIntNumber(Long_t val)
{
//
// Set text field according to the numeric value
//
   Int_t h = val / 3600;
   Int_t m = (val - 3600 * h) / 60;
   Int_t s = (val - 3600 * h - 60 * m);
   SetTime(h, m, s);
}

//__________________________________________
Long_t KVTimeEntry::GetIntNumber() const
{
//
// Get int number according to the text
//
   Long_t val = 0;
   TString vs = fTimeEntry->GetText();
   while (vs.Contains(":")) {
      val *= 60;
      val += vs.Atoi();
      vs.Remove(0, vs.Index(":") + 1);
   }
   val *= 60;
   val += vs.Atoi();
   return val;
}

//__________________________________________
void KVTimeEntry::SetTimeFromTextEntry()
{
//
// Adjust time display
//
   Long_t val = GetIntNumber();
   SetIntNumber(val);
}

//__________________________________________
void KVTimeEntry::SetTime(Int_t hour, Int_t min, Int_t sec)
{
//
// Set time in the text field
//
   fTimeEntry->SetText(Form("%d:%02d:%02d", hour, min, sec));
}

//__________________________________________
void KVTimeEntry::GetTime(Int_t& hour, Int_t& min, Int_t& sec) const
{
//
// Get hours, minutes and seconds from the text field
//
   Long_t val = GetIntNumber();
   hour = val / 3600;
   min = (val - 3600 * hour) / 60;
   sec = (val - 3600 * hour - 60 * min);
}

//__________________________________________
Bool_t KVTimeEntry::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
   // Process the up/down button messages. If fButtonToNum is false the
   // following message is sent: kC_COMMAND, kCM_BUTTON, widget id, param
   // param % 100 is the step size
   // param % 10000 / 100 != 0 indicates log step
   // param / 10000 != 0 indicates button down

   switch (GET_MSG(msg)) {
      case kC_COMMAND: {
            if ((GET_SUBMSG(msg) == kCM_BUTTON) &&
                  (parm1 >= 1) && (parm1 <= 2)) {
               if (fButtonToNum) {
                  Int_t sign = (parm1 == 1) ? 1 : -1;
                  EStepSize step = (EStepSize)(parm2 % 100);
                  Bool_t logstep = (parm2 >= 100);
                  IncreaseNumber(step, sign, logstep);
               } else {
                  SendMessage(fMsgWindow, msg, fWidgetId,
                              10000 * (parm1 - 1) + parm2);
                  ValueChanged(10000 * (parm1 - 1) + parm2);
               }
               // Emit a signal needed by pad editor
               ValueSet(10000 * (parm1 - 1) + parm2);
            }
            break;
         }
      case kC_TEXTENTRY: {
            switch (GET_SUBMSG(msg)) {
               case kTE_ENTER: {
                     SetTimeFromTextEntry();
                     return kTRUE;
                     break;
                  }
               case kTE_TEXTCHANGED: {
                     break;
                  }
            }
            break;
         }
   }
   return kTRUE;

}

