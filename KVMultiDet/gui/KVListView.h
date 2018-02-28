/*
$Id: KVListView.h,v 1.5 2009/04/28 09:11:29 franklan Exp $
$Revision: 1.5 $
$Date: 2009/04/28 09:11:29 $
*/

//Created by KVClassFactory on Wed Apr  9 11:51:38 2008
//Author: franklan

#ifndef __KVLISTVIEW_H
#define __KVLISTVIEW_H

#include "TGListView.h"
#include "KVLVContainer.h"
#include "KVLVContainer.h"
#include "KVNumberList.h"

class KVListView : public TGListView {
protected:
   int      nselected;     //number of selected items
   TClass*   fObjClass;    //class of objects in list
   UInt_t fMaxColumnSize;//maximum width of columns
   TContextMenu* fContextMenu;

public:
   KVListView(TClass* obj_class, const TGWindow* p, UInt_t w, UInt_t h,
              UInt_t options = kSunkenFrame | kDoubleBorder,
              Pixel_t back = GetDefaultFrameBackground());
   virtual ~KVListView() {}

   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
   virtual void  ActivateSortButtons(); // *MENU*
   virtual void SetMaxColumnSize(UInt_t width)
   {
      fMaxColumnSize = width;
   }
   virtual void   SetDataColumns(Int_t ncolumns);
   virtual void   SetDataColumn(Int_t index, const Char_t* name, const Char_t* method = "",
                                Int_t mode = kTextCenterX);
   virtual  KVLVColumnData*      GetDataColumn(Int_t index) const
   {
      return ((KVLVContainer*)GetContainer())->GetDataColumn(index);
   }

   virtual void Display(const TCollection* l)
   {
      ((KVLVContainer*)GetContainer())->Display(l);   // *MENU*
   }
   virtual void Sort(Int_t column)
   {
      // Sort list according to data in given column (=0, 1, ...)
      ((KVLVContainer*)GetContainer())->Sort(column);
   }
   virtual void SelectAll()
   {
      ((KVLVContainer*)GetContainer())->SelectAll();
   }
   virtual void UnSelectAll()
   {
      ((KVLVContainer*)GetContainer())->UnSelectAll();
   }
   virtual void RemoveAll()
   {
      ((KVLVContainer*)GetContainer())->RemoveAll();
   }
   virtual Int_t GetColumnNumber(const Char_t* colname);
   TGLVEntry*     FindItemWithData(void* userData)
   {
      return ((KVLVContainer*)GetContainer())->FindItemWithData(userData);
   }
   void    ActivateItemWithData(void* userData, Bool_t activate = kTRUE)
   {
      ((KVLVContainer*)GetContainer())->ActivateItemWithData(userData, activate);
   }
   TGLVEntry*    FindItemWithColumnData(const Char_t* colname, const Char_t* data)
   {
      return ((KVLVContainer*)GetContainer())->FindItemWithColumnData(colname, data);
   }
   TGLVEntry*    FindItemWithColumnData(const Char_t* colname, Long_t data)
   {
      return ((KVLVContainer*)GetContainer())->FindItemWithColumnData(colname, data);
   }
   TGLVEntry*    FindItemWithColumnData(const Char_t* colname, Double_t data)
   {
      return ((KVLVContainer*)GetContainer())->FindItemWithColumnData(colname, data);
   }
   void     ActivateItemWithColumnData(const Char_t* colname, const Char_t* data, Bool_t activate = kTRUE)
   {
      ((KVLVContainer*)GetContainer())->ActivateItemWithColumnData(colname, data, activate);
   }
   void     ActivateItemWithColumnData(const Char_t* colname, Long_t data, Bool_t activate = kTRUE)
   {
      ((KVLVContainer*)GetContainer())->ActivateItemWithColumnData(colname, data, activate);
   }
   void     ActivateItemWithColumnData(const Char_t* colname, Double_t data, Bool_t activate = kTRUE)
   {
      ((KVLVContainer*)GetContainer())->ActivateItemWithColumnData(colname, data, activate);
   }
   void     ActivateItemsWithColumnData(const Char_t* colname, KVNumberList data, Bool_t activate = kTRUE)
   {
      ((KVLVContainer*)GetContainer())->ActivateItemsWithColumnData(colname, data, activate);
   }
   TObject* GetLastSelectedObject() const
   {
      return ((KVLVContainer*)GetContainer())->GetLastSelectedObject();
   }
   TGLVEntry* GetLastSelectedItem() const
   {
      return ((KVLVContainer*)GetContainer())->GetLastSelectedItem();
   }
   TList* GetSelectedItems() const
   {
      // list of selected KVLVEntry objects
      // DELETE AFTER USE !!!
      return ((KVLVContainer*)GetContainer())->GetSelectedItems();
   }
   TList* GetSelectedObjects() const
   {
      // list of selected objects (derived from TObject)
      // DELETE AFTER USE !!!
      return ((KVLVContainer*)GetContainer())->GetSelectedObjects();
   }
   KVList* GetPickOrderedSelectedObjects() const
   {
      // list of selected objects (derived from TObject) in the order they were picked
      // DO NOT DELETE!!!
      return ((KVLVContainer*)GetContainer())->GetPickOrderedSelectedObjects();
   }
   TCollection* GetUserItems()
   {
      // return list of all objects (selected or unselected) in list view
      return ((KVLVContainer*)GetContainer())->GetUserItems();
   }
   TObject* GetFirstInList() const
   {
      // return first object in currently displayed list
      return ((KVLVContainer*)GetContainer())->GetFirstInList();
   }
   TObject* GetLastInList() const
   {
      // return last object in currently displayed list
      return ((KVLVContainer*)GetContainer())->GetLastInList();
   }
   void AllowDoubleClick(Bool_t on = kTRUE)
   {
      // Call with on=kFALSE to disable objects' Browse() method being called with mouse double-click
      ((KVLVContainer*)GetContainer())->AllowDoubleClick(on);
   }
   void SetDoubleClickAction(const char* receiver_class, void* receiver, const char* slot);
   void AllowBrowse(Bool_t on = kTRUE)
   {
      // Deprecated: use AllowDoubleClick()
      AllowDoubleClick(on);
   }
   void AllowContextMenu(Bool_t on = kTRUE)
   {
      // Call with on=kFALSE to disable objects' context menus opening with mouse right-click
      ((KVLVContainer*)GetContainer())->AllowContextMenu(on);
   }
   void AddContextMenuClassException(TClass*);
   virtual void  SetDefaultColumnWidth(TGVFileSplitter* splitter);

   void AllowMultipleSelection(Bool_t yes = kTRUE)
   {
      ((TGLVContainer*)GetContainer())->SetMultipleSelection(yes);
   }
   Bool_t GetMultipleSelection() const
   {
      return ((TGLVContainer*)GetContainer())->GetMultipleSelection();
   }

   void SetUseObjLabelAsRealClass(Bool_t yes = kTRUE);

   TClass* GetObjClass() const
   {
      return fObjClass;
   };
   void SetObjClass(TClass* cl)
   {
      fObjClass = cl;
      ((KVLVContainer*)GetContainer())->SetObjClass(fObjClass);
   }

   ClassDef(KVListView, 0) //Slightly modified TGListView
};

#endif
