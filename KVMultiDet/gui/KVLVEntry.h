/*
$Id: KVLVEntry.h,v 1.3 2009/04/28 09:11:29 franklan Exp $
$Revision: 1.3 $
$Date: 2009/04/28 09:11:29 $
*/

//Created by KVClassFactory on Wed Apr  9 13:55:03 2008
//Author: franklan

#ifndef __KVLVENTRY_H
#define __KVLVENTRY_H

#include "TGListView.h"
#include "TFunction.h"
#include "KVLVContainer.h"

class KVLVEntry : public TGLVEntry {

protected:
   Bool_t fEditMode;
   Bool_t fColoured;//=kTRUE if object has an associated color

   static Pixel_t fgBGColor;// current background color for items
   static Pixel_t fgGreyPixel;// light grey color used to distinguish adjacent list items
   Pixel_t fBGColor;// background color of this item

   Bool_t* fBoolean;//'IsBoolean' attribute of each column

   Bool_t fDisconnectRefresh;//! disconnect object's Modified method from this object's Refresh when deleted

public:
   KVLVEntry(TObject* obj, const KVLVContainer* cnt,
             UInt_t ncols, KVLVColumnData** coldata);
   KVLVEntry(TObject* obj, const Char_t* objclass, const KVLVContainer* cnt,
             UInt_t ncols, KVLVColumnData** coldata);
   virtual ~KVLVEntry();

   virtual void Refresh();
   virtual void   DrawCopy(Handle_t id, Int_t x, Int_t y);
   void SetEditMode(Bool_t edit = kTRUE)
   {
      fEditMode = edit;
   }

   ClassDef(KVLVEntry, 0) //List view item class
};

#endif
