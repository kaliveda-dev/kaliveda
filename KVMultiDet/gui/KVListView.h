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

/**
\class KVListView
\brief Enhanced version of ROOT TGListView widget
\ingroup GUI

\image http http://indra.in2p3.fr/KaliVedaDoc/images/KVLVContainer.png

<h3>Example of use</h3>
KVListView takes a list of objects (TObject or derived class) and presents them
as a list of items with different data for each object presented in separate columns.<br>
The user specifies the common base-class of the objects to display, the number of columns,
and the data to be presented in each column.
<br>
The example in the figure shown above (displaying a list of <a href="KVDBRun.html">KVDBRun</a>
objects) was generated with the following code:
<br>
<br>
<pre>
   lvRuns = new KVListView(<a href="KVDBRun.html">KVDBRun</a>::Class(), cfRuns, 500, 250); // 'cfRuns' is pointer to parent frame

   lvRuns->SetDataColumns(4);   //set number of columns to 4

   lvRuns->SetDataColumn(0, "Run", "GetNumber");       //by default, the data presented in a column with title "Toto" will be retrieved
   lvRuns->SetDataColumn(1, "Events", "", kTextRight);      //from each object by calling the "GetToto" method of the class given to the KVListView ctor.
   lvRuns->SetDataColumn(2, "Date", "GetDatime");      //in case the "getter" method has a non-standard name, it can be given explicitly.

   lvRuns->GetDataColumn(2)-><a href="KVLVColumnData.html#KVLVColumnData:SetIsDateTime">SetIsDateTime()</a>;      //some special treatment is afforded to date/time data. see method doc.
   lvRuns->SetDataColumn(3, "Comments", "", kTextLeft);// specify text alignment for data

   lvRuns->ActivateSortButtons();//when clicking a column header, the list is sorted according to that column's data.
   ...                           //clicking a second time the same column sorts objects in the opposite sense.
</pre>
<br>
To display objects, put them in a TList and call
<br>
<br>
<pre>
   lvRuns->Display( pointer_to_TList );
</pre>
<br>
<h3>Object selection</h3>
Items in the list can be selected using either:
      <ul>
      <li>single left click (single object selection),</li>
      <li>CTRL+left-click (selection of multiple non-neighbouring objects),</li>
        <li>or SHIFT+left-click (selection of multiple & neighbouring objects)</li>
        <li>CTRL+a (select all objects in list)</li>
        </ul>
(By default, multiple selection of objects is allowed. Call AllowMultipleSelection(kFALSE) to disable).
<br>
<br>
When the selection changes, the KVListView emits the
<pre>
   SelectionChanged()
</pre> signal.<br>
The list of the currently selected objects can be retrieved with the method
<pre>
   GetSelectedObjects()
</pre>
which returns a TList of the currently selected objects (delete list after use), or
<pre>
   GetPickOrderedSelectedObjects()
</pre>
which returns a TList of the currently selected objects IN THE ORDER THEY WERE CLICKED
- this list must NOT be deleted by the user.
<br>
<br>
<h3>Double-click action</h3>
By default, double-left-clicking an object will execute the
<pre>
   Browse()
</pre>
method of the object. This method is defined for all TObjects,
it can be overridden in derived classes in order to do something interesting and/or useful.<br>
Alternatively, using method
<pre>
   SetDoubleClickAction(const char* receiver_class, void* receiver, const char* slot)
</pre>
you can define a different behaviour when objects are double-clicked using signals & slots.<br>
If you want to deactivate the double-click actions, call
<pre>
   AllowDoubleClick(kFALSE)
</pre>
<br>
<h3>Context menus</h3>
By default, context menus are active, which means that right-clicking an object
opens its context menu. Call method
<pre>
   AllowContextMenu(kFALSE)
</pre>
to deactivate all context menus, or
<pre>
   AddContextMenuClassException(TClass*)
</pre>
to activate/deactivate context menus for specific classes, depending on whether
context menus are globally activated or not.
<br>
See also 'Wrapper classes' below.
<br>
<h3>Icons</h3>
An icon is shown next to each item in the list. This icon is defined (for most ROOT objects
e.g. histograms of different types) in the $ROOTSYS/etc/root.mimes file. Other icons can be
added in .kvrootrc as in the following example:
<pre>
   +KaliVeda.GUI.MimeTypes:   TCutG
   KaliVeda.GUI.MimeTypes.TCutG.Icon:   cut_t.xpm
</pre>
which adds an icon (pair of scissors) for the TCutG class (not defined in root.mimes).
<br>
See also 'Wrapper classes' below.
<br>
<h3>Wrapper classes</h3>
For convenience, the objects placed in the list may be of a class which provides
a 'wrapper' around the actual object of interest.
<br>
In this case, it is possible (if the 'wrapper'
inherits from <a href="KVBase.html">KVBase</a>) to have the icon and the context menu
of each object reflect the object of interest, rather than the wrapper.
<br>
To do so, call method
<pre>
   SetUseObjLabelAsRealClass(kTRUE)
</pre>
and have the wrapper class' <code>GetLabel()</code> and <code>GetObject()</code> methods return
the class name and address of the wrapped object.
*/

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
