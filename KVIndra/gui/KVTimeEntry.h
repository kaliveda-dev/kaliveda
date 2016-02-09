//
// Entete de la classe KVTimeEntry
//
#ifndef KVTimeEntry_h
#define KVTimeEntry_h

#include "TGNumberEntry.h"

class KVTimeEntry : public TGNumberEntry {
protected:
   TGTextEntry* fTimeEntry;
public:
   KVTimeEntry(const TGWindow* parent = 0, Double_t val = 0,
               Int_t digitwidth = 5, Int_t id = -1,
               EStyle style = kNESHourMinSec,
               EAttribute attr = kNEANonNegative,
               ELimit limits = kNELNoLimits,
               Double_t min = 0, Double_t max = 1);
   virtual ~KVTimeEntry();

   virtual void IncreaseNumber(EStepSize step = kNSSSmall,
                               Int_t sign = 1, Bool_t logstep = kFALSE);

   virtual void SetIntNumber(Long_t val);
   virtual Long_t GetIntNumber() const;
   virtual void SetTime(Int_t hour, Int_t min, Int_t sec);
   virtual void GetTime(Int_t& hour, Int_t& min, Int_t& sec) const;

   virtual void SetTimeFromTextEntry();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   virtual TGTextEntry* GetTextEntry()
   {
      return fTimeEntry;
   }


   ClassDef(KVTimeEntry, 0)
};

class KVTimeEntryLayout : public TGNumberEntryLayout {

public:
   KVTimeEntryLayout(TGNumberEntry* box): TGNumberEntryLayout(box) { }
   virtual void Layout();

   ClassDef(KVTimeEntryLayout, 0) // Layout manager for time entry widget
};

#endif
