/*
$Id: KVListView.cpp,v 1.6 2009/04/28 09:11:29 franklan Exp $
$Revision: 1.6 $
$Date: 2009/04/28 09:11:29 $
*/

//Created by KVClassFactory on Wed Apr  9 11:51:38 2008
//Author: franklan

#include "KVListView.h"
#include "TGButton.h"
#include "TVirtualX.h"

ClassImp(KVListView)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVListView</h2>
<h4>Slightly modified TGListView</h4>

<img src="http://indra.in2p3.fr/KaliVedaDoc/images/KVLVContainer.png"><br>

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
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVListView::KVListView(TClass* obj_class, const TGWindow* p, UInt_t w, UInt_t h,
                       UInt_t options,
                       Pixel_t back)
   : TGListView(p, w, h, options, back), nselected(0), fObjClass(obj_class)
{
   // Create list view widget for objects of class obj_class.
   // A KVLVContainer object is also created and set as the container in use.
   // The view mode is set to kLVDetails (only one possible, no icons)
   // Scrolling increments are set to 1 (vertical) & 19 (horizontal)

   SetContainer(new KVLVContainer(this, kHorizontalFrame, fgWhitePixel));
   SetViewMode(kLVDetails);
   SetIncrements(1, 19);
   fMaxColumnSize = 100;
   fContextMenu = new TContextMenu("fCurrentContext");
   ((KVLVContainer*)GetContainer())->SetObjClass(fObjClass);
}

Bool_t KVListView::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
   // Handle messages generated by the list view container
   switch (GET_SUBMSG(msg)) {
      case kCT_SELCHANGED:
         // only emit signal if number selected has really changed
         if ((int)parm2 != nselected) {
            nselected = (int)parm2;
            SelectionChanged();
         }
         return kTRUE;
         break;
      default:
         break;
   }
   return TGCanvas::ProcessMessage(msg, parm1, parm2);
}

void KVListView::ActivateSortButtons()
{
   // Connects the column header buttons (if defined) to the container's
   // Sort(const Char_t*) method. Each button, when pressed, sends a "Clicked"
   // signal which activates "Sort" with the name of the button/column as argument.

   TGTextButton** buttons = GetHeaderButtons();
   if (buttons) {
      for (int i = 0; i < (int)GetNumColumns(); i++) {
         buttons[i]->Connect("Clicked()", "KVLVContainer",
                             GetContainer(), Form("Sort(=%d)", i));
      }
   }
}

void KVListView::SetDataColumns(Int_t ncolumns)
{
   TGListView::SetHeaders(ncolumns);
   ((KVLVContainer*)GetContainer())->SetDataColumns(ncolumns);
}

void KVListView::SetDataColumn(Int_t index, const Char_t* name, const Char_t* method,
                               Int_t mode)
{
   // Define column with index = 0, 1, ...
   // name = name of column (shown in column header button)
   // method = method to call to fill column with data (by default, "Getname")
   // mode = text alignment for data in column (kTextCenterX [default], kTextLeft, kTextRight)
   // column header name is always center aligned
   SetHeader(name, kTextCenterX, mode, index);
   ((KVLVContainer*)GetContainer())->SetDataColumn(index, fObjClass, name, method);
}

Int_t KVListView::GetColumnNumber(const Char_t* colname)
{
   // Returns index of data column from its name.
   // Return -1 if not found.

   for (Int_t idx = 0; idx < fNColumns - 1; idx++) {
      if (fColHeader[idx]) {
         if (fColNames[idx] == colname) return idx;
      }
   }
   return -1;
}

//______________________________________________________________________________

void KVListView::SetDefaultColumnWidth(TGVFileSplitter* splitter)
{
   // Set default column width of the columns headers.
   // Limit minimum size of a column to total width / number of columns
   // If only one column it will span the whole viewport

   TGLVContainer* container = (TGLVContainer*) fVport->GetContainer();

   if (!container) {
      Error("SetDefaultColumnWidth", "no listview container set yet");
      return;
   }
   container->ClearViewPort();
   UInt_t minWidth = container->GetPageDimension().fWidth / (fNColumns - 1);

   for (int i = 0; i < fNColumns; ++i) {
      if (fSplitHeader[i] == splitter) {
         TString dt = fColHeader[i]->GetString();
         UInt_t bsize = gVirtualX->TextWidth(fColHeader[i]->GetFontStruct(),
                                             dt.Data(), dt.Length());
         UInt_t w = TMath::Max(fColHeader[i]->GetDefaultWidth(), bsize + 20);
         if (i == 0) {
            //w = TMath::Max(fMaxSize.fWidth + 10, w);
            w = TMath::Max(w, minWidth);
         }
         if (i > 0)  {
            w = TMath::Max(container->GetMaxSubnameWidth(i) + 40, (Int_t)w);
            //printf("w=%ud\n",w);
            //w = TMath::Min(w, fMaxColumnSize);
            w = TMath::Max(w, minWidth);
         }
         fColHeader[i]->Resize(w, fColHeader[i]->GetHeight());
         Layout();
      }
   }
}

void KVListView::SetUseObjLabelAsRealClass(Bool_t yes)
{
   // If list contains KVBase-derived objects, calling this method with yes=kTRUE
   // will cause the string returned by the objects' KVBase::GetLabel() method
   // to be used as the class name of the object
   ((KVLVContainer*)GetContainer())->SetUseObjLabelAsRealClass(yes);
}

//______________________________________________________________________________

void KVListView::SetDoubleClickAction(const char* receiver_class, void* receiver, const char* slot)
{
   // Overrides the default 'double-click' action.
   // By default, double-clicking on an object in the list will call the Browse(TBrowser*)
   // method of the selected object.
   // Use this method to override this behaviour.
   // When an object is double-clicked the method 'slot' of the object 'receiver' of class
   // 'receiver_class' will be called. The method in question must have the signature
   //       receiver_class::slot(TObject*)
   // The address of the selected (T)object is passed as argument.

   ((KVLVContainer*)GetContainer())->SetDoubleClickAction(receiver_class, receiver, slot);
}

void KVListView::AddContextMenuClassException(TClass* cl)
{
   // The global context menu status (allowed or not allowed) is set by AllowContextMenu().
   // If required, this can be overridden for specific classes by calling this
   // method for each required class.
   // In this case, any objects in the list of precisely this class (not derived classes)
   // will have the opposite behaviour to that defined by AllowContextMenu(),
   // i.e. if context menus are globally disabled, this method defines the classes for
   // which a context menu is authorised, and vice-versa.

   ((KVLVContainer*)GetContainer())->AddContextMenuClassException(cl);
}

