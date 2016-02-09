/*
$Id: KVLVContainer.h,v 1.7 2009/04/28 09:11:29 franklan Exp $
$Revision: 1.7 $
$Date: 2009/04/28 09:11:29 $
*/

//Created by KVClassFactory on Wed Apr  9 13:54:31 2008
//Author: franklan

#ifndef __KVLVCONTAINER_H
#define __KVLVCONTAINER_H

#include "TGListView.h"
#include "TFunction.h"
#include "TString.h"
#include "TMethodCall.h"
#include "TClass.h"
#include "TTimer.h"
#include "Riostream.h"
#include "KVDatime.h"
#include "TContextMenu.h"
#include "KVList.h"
#include "KVNumberList.h"

class KVLVColumnData {
   // Utility class describing the data used to fill each
   // column of the list view container

   TString        fName;      // name used on button at top of column
   TString        fMethod;    // method used to retrieve data from objects
   TMethodCall*    fMethCall; // method call object
   Int_t          fRetType;   // return type of data retrieval method
   TString        result;     // string used to store object data
   Bool_t         fDate;      // kTRUE if column contains TDatime date & time info
   KVDatime::EKVDateFormat  fFmt;      // format for presenting date & time
   Bool_t         fIsKVDatime;   // kTRUE if date & time is in KVDatime object, TDatime if not
   Bool_t         fIsBoolean; // kTRUE if column data is a boolean (i.e. 1 or 0, kTRUE or kFALSE)
   TString         fDataFormat;// format for displaying numerical data

   enum {
      kDatimeRef = (Int_t)TMethodCall::kNone + 1,
      kDatimeInt
   };
   Int_t Compare_date(TObject* o1, TObject* o2);
   Int_t Compare_string(TObject* o1, TObject* o2);
   Int_t Compare_long(TObject* o1, TObject* o2);
   Int_t Compare_double(TObject* o1, TObject* o2);

public:
   KVLVColumnData(TClass* cl, const Char_t* name, const Char_t* method = "")
      : fName(name), fMethod(method), result(""), fDate(kFALSE), fFmt(KVDatime::kCTIME), fIsKVDatime(kFALSE), fIsBoolean(kFALSE)
   {
      if (fMethod == "") fMethod.Form("Get%s", name);
      fMethCall = new TMethodCall(cl, fMethod.Data(), "");
      if (!fMethCall->IsValid()) {
         std::cout << "Error in <KVLVColumnData::KVLVColumnData> : method " << fMethod.Data()
                   << " is not valid for class " << cl->GetName() << std::endl;
      }
      fRetType = (Int_t)fMethCall->ReturnType();
      fDataFormat = "";
      switch (fRetType) {
         case (Int_t)TMethodCall::kLong :
            fDataFormat = "%ld";
            break;

         case (Int_t)TMethodCall::kDouble :
            fDataFormat = "%lf";
            break;

         default:
            break;
      }
      if (!strcmp(fMethCall->GetMethod()->GetReturnTypeName(), "bool")
            || !strcmp(fMethCall->GetMethod()->GetReturnTypeName(), "Bool_t")) fIsBoolean = kTRUE;
      else fIsBoolean = kFALSE;
   };
   virtual ~KVLVColumnData()
   {
      delete fMethCall;
   };
   virtual void SetIsDateTime(KVDatime::EKVDateFormat fmt = KVDatime::kCTIME, Bool_t with_reference = kTRUE);
   virtual void SetIsBoolean(Bool_t isit = kTRUE)
   {
      fIsBoolean = isit;
   };
   virtual void SetDataFormat(const Char_t* fmt)
   {
      // Override default formatting for integer ("%ld") or double ("%lf") data types
      fDataFormat = fmt;
   }

   virtual Bool_t IsBoolean() const
   {
      return fIsBoolean;
   };
   const Char_t* GetDataString(TObject*);
   void GetData(TObject*, Long_t&);
   void GetData(TObject*, Double_t&);
   void GetData(TObject*, TString&);
   void GetData(TObject*, KVDatime&);
   Int_t Compare(TObject* ob1, TObject* ob2);

   ClassDef(KVLVColumnData, 0) //column data handler
};

class KVLVContainer : public TGLVContainer {

   friend class KVLVFrameElement;
   friend class KVLVEntry;

   Bool_t      fIsResized;    // used to resize columns exactly once
   Bool_t      fSort;

   Bool_t      fAllowContextMenu;   // can objects' context menu be opened with right-click ?
   TList*      fContextMenuClassExceptions; // list of classes for which we override value of fAllowContextMenu
   Bool_t      fAllowDoubleClick;   // do something when object double-clicked ?
   Bool_t      fUserDoubleClickAction; // user-defined double-click action instead of Browse() method
   Bool_t      fControlClick;      // set to kTRUE when user ctrl-clicks an item

   Bool_t      fKeepUserItems;      // internal use only, do not clear list of user items in RemoveAll()
protected:

   KVLVColumnData**    fColData;    // description of column data
   Int_t             fSortType;     // current sorting mode of contents (ascending or descending)
   KVLVColumnData*    fSortData;    // name of column (i.e. type of data) currently used to sort objects
   Int_t*             fSortDir;     // direction of sorting for each column
   Int_t             fNcols;        // number of data columns
   TContextMenu*      fContextMenu; // used to display popup context menu for items
   KVList*            fUserItems;   // list of currently displayed items, used by Refresh()
   KVList*              fPickOrderedObjects;// list of currently selected objects, in order of selection

   TClass* fObjClass;

   Bool_t fUseObjLabelAsRealClass; // if kTRUE, object's classname read from KVBase::GetLabel() (objects must be KVBase-derived!)

   virtual void FillList(const TCollection* = 0);
   void DeleteColData();
   void default_init();

   virtual void   ActivateItemFromSelectAll(TGFrameElement* el);
public:

   KVLVContainer(const TGWindow* p = 0, UInt_t w = 1, UInt_t h = 1,
                 UInt_t options = kSunkenFrame,
                 Pixel_t back = GetDefaultFrameBackground());
   KVLVContainer(TGCanvas* p, UInt_t options = kSunkenFrame,
                 Pixel_t back = GetDefaultFrameBackground());
   virtual ~KVLVContainer();

   void     AddFrame(TGFrame* f, TGLayoutHints* l = 0);
   void     Sort(int column);            // *MENU*
   virtual  void     Display(const TCollection* = 0);
   virtual  void     Refresh();
   virtual  void     SetDataColumns(Int_t ncols);
   virtual  void     SetDataColumn(Int_t index, TClass* cl, const Char_t* name, const Char_t* method = "");
   virtual  KVLVColumnData*      GetDataColumn(Int_t index) const
   {
      return (fColData ? (index < fNcols ? fColData[index] : 0) : 0);
   };
   TGLVEntry*     FindItemWithData(void* userData);
   void     ActivateItemWithData(void* userData, Bool_t activate = kTRUE);
   TGLVEntry*     FindItemWithColumnData(const Char_t* colname, const Char_t* data);
   TGLVEntry*     FindItemWithColumnData(const Char_t* colname, Long_t data);
   TGLVEntry*     FindItemWithColumnData(const Char_t* colname, Double_t data);
   void     ActivateItemWithColumnData(const Char_t* colname, const Char_t* data, Bool_t activate = kTRUE);
   void     ActivateItemWithColumnData(const Char_t* colname, Long_t data, Bool_t activate = kTRUE);
   void     ActivateItemWithColumnData(const Char_t* colname, Double_t data, Bool_t activate = kTRUE);
   void     ActivateItemsWithColumnData(const Char_t* colname, KVNumberList data, Bool_t activate = kTRUE);

   void     OpenContextMenu(TGFrame*, Int_t, Int_t, Int_t);
   void     DoDoubleClick(TGFrame*, Int_t, Int_t, Int_t);
   void     SetDoubleClickAction(const char* receiver_class, void* receiver, const char* slot);

   TObject* GetLastSelectedObject() const
   {
      // Returns object corresponding to last clicked item in list
      return GetLastSelectedItem() ? (TObject*)GetLastSelectedItem()->GetUserData() : 0;
   };
   TGLVEntry* GetLastSelectedItem() const
   {
      // Returns last clicked item in list
      return fLastActive;
   };
   TObject* GetFirstInList();
   TObject* GetLastInList();
   TList* GetSelectedItems();
   TList* GetSelectedObjects();
   KVList* GetPickOrderedSelectedObjects() const
   {
      // Returns list of currently selected objects, in the order they were picked.
      // Do NOT delete this list - it belongs to KVLVContainer.
      return fPickOrderedObjects;
   }
   TCollection* GetUserItems()
   {
      // return list of all objects in list (regardless of selection)
      return fUserItems;
   };
   Bool_t IsBeingSorted() const
   {
      return fSort;
   };
   void AllowContextMenu(Bool_t on = kTRUE)
   {
      // Call with on=kFALSE to disable objects' context menus opening with mouse right-click
      fAllowContextMenu = on;
   };
   void AddContextMenuClassException(TClass*);
   void AllowBrowse(Bool_t on = kTRUE)
   {
      // Deprecated: use AllowDoubleClick
      AllowDoubleClick(on);
   };
   void AllowDoubleClick(Bool_t on = kTRUE)
   {
      // Call with on=kFALSE to disable reactivity of objects to mouse double-click
      fAllowDoubleClick = on;
   };
   virtual void RemoveAll();

   void DoubleClickAction(TObject*); /* SIGNAL */
   Bool_t HandleButton(Event_t* event);

   void SetObjClass(TClass* cN)
   {
      fObjClass = cN;
   }
   void AddDataColumn(const char* columnName); // *MENU*
   //   void SetNewColumnName(const char* columnName);
   virtual void SelectAll(); /* SIGNAL */

   void SetUseObjLabelAsRealClass(Bool_t on = kTRUE)
   {
      fUseObjLabelAsRealClass = on;
   }

   ClassDef(KVLVContainer, 0) //List view container class
};

#endif
