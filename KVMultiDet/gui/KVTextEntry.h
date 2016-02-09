/*
$Id: KVTextEntry.h,v 1.1 2007/12/11 16:03:45 franklan Exp $
$Revision: 1.1 $
$Date: 2007/12/11 16:03:45 $
*/

//Created by KVClassFactory on Tue Dec 11 14:36:47 2007
//Author: John Frankland

#ifndef __KVTEXTENTRY_H
#define __KVTEXTENTRY_H

#include "TGTextEntry.h"

class KVTextEntry : public TGTextEntry {

public:
   KVTextEntry(const TGWindow* p, TGTextBuffer* text, Int_t id = -1,
               GContext_t norm = GetDefaultGC()(),
               FontStruct_t font = GetDefaultFontStruct(),
               UInt_t option = kSunkenFrame | kDoubleBorder,
               Pixel_t back = GetWhitePixel())
      : TGTextEntry(p, text, id, norm, font, option, back)
   {};
   KVTextEntry(const TGWindow* parent = 0, const char* text = 0, Int_t id = -1)
      : TGTextEntry(parent, text, id)
   {};
   KVTextEntry(const TString& contents, const TGWindow* parent, Int_t id = -1)
      : TGTextEntry(contents, parent, id)
   {};
   virtual ~KVTextEntry() {};

   virtual  void        SetText(const char* text, Bool_t /*emit*/ = kTRUE);                               //*MENU*
   virtual  void        SetMaxLength(Int_t maxlen);                              //*MENU*
   virtual  void        Insert(const char*);

   ClassDef(KVTextEntry, 0) //TGTextEntry without any limit on the length of the text
};

#endif
