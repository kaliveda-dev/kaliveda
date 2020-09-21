/*
$Id: KVIDGUITelescopeChooserDialog.h,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Nov 17 14:41:53 2008
//Author: franklan

#ifndef __KVIDGUITELESCOPECHOOSERDIALOG_H
#define __KVIDGUITELESCOPECHOOSERDIALOG_H

#ifndef ROOT_TGDockableFrame
#include "TGDockableFrame.h"
#endif
#ifndef ROOT_TGMenu
#include "TGMenu.h"
#endif
#ifndef ROOT_TGMdiDecorFrame
#include "TGMdiDecorFrame.h"
#endif
#ifndef ROOT_TG3DLine
#include "TG3DLine.h"
#endif
#ifndef ROOT_TGMdiFrame
#include "TGMdiFrame.h"
#endif
#ifndef ROOT_TGMdiMainFrame
#include "TGMdiMainFrame.h"
#endif
#ifndef ROOT_TRootBrowserLite
#include "TRootBrowserLite.h"
#endif
#ifndef ROOT_TGMdiMenu
#include "TGMdiMenu.h"
#endif
#ifndef ROOT_TGListBox
#include "TGListBox.h"
#endif
#ifndef ROOT_TGNumberEntry
#include "TGNumberEntry.h"
#endif
#ifndef ROOT_TGScrollBar
#include "TGScrollBar.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif
#ifndef ROOT_TGFileDialog
#include "TGFileDialog.h"
#endif
#ifndef ROOT_TGShutter
#include "TGShutter.h"
#endif
#ifndef ROOT_TGButtonGroup
#include "TGButtonGroup.h"
#endif
#ifndef ROOT_TGCanvas
#include "TGCanvas.h"
#endif
#ifndef ROOT_TGFSContainer
#include "TGFSContainer.h"
#endif
#ifndef ROOT_TGButton
#include "TGButton.h"
#endif
#ifndef ROOT_TGFSComboBox
#include "TGFSComboBox.h"
#endif
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif
#ifndef ROOT_TGMsgBox
#include "TGMsgBox.h"
#endif
#ifndef ROOT_TGTab
#include "TGTab.h"
#endif
#ifndef ROOT_TGListView
#include "TGListView.h"
#endif
#ifndef ROOT_TGSplitter
#include "TGSplitter.h"
#endif
#ifndef ROOT_TGStatusBar
#include "TGStatusBar.h"
#endif
#ifndef ROOT_TGListTree
#include "TGListTree.h"
#endif
#ifndef ROOT_TGToolTip
#include "TGToolTip.h"
#endif
#ifndef ROOT_TGToolBar
#include "TGToolBar.h"
#endif

#include "Riostream.h"
#include "KVMultiDetArray.h"
#include "TList.h"
#include "KVIDTelescope.h"

/**
\class KVIDGUITelescopeChooserDialog
\brief ID Grid Manager dialog for choice of ID telescope(s)
\ingroup GUI

\image http://indra.in2p3.fr/KaliVedaDoc/images/IDTelescopeChooser.png

This dialog box presents the user with a drop-down list of ID telescope
types, and a box containing the list of all ID telescopes in a given multidetector
array of the selected type. The user selects the type of ID telescope, then
selects one or more telescopes in the list, and presses "OK" in order to
validate the choice.
 */
class KVIDGUITelescopeChooserDialog {

   RQ_OBJECT("KVIDGUITelescopeChooserDialog")

   TList*                fSelection;      // list of selected telescopes
   KVMultiDetArray*      fMultiDet;       // multidetector array geometry
   TGTransientFrame*     fMainFrame1428;  // main frame
   TGComboBox*           fComboBox994;    // combo box with list of ID telescope types
   TGListBox*            fListBox980;     // list of telescopes of selected type
   Bool_t*               fCancelPress;

public:
   KVIDGUITelescopeChooserDialog(KVMultiDetArray* MDA, TList* selection, Bool_t*,
                                 const TGWindow* p, const TGWindow* main, Bool_t disable_type_select = false, UInt_t w = 1, UInt_t h = 1);
   virtual ~KVIDGUITelescopeChooserDialog();
   void DoClose();
   void CloseWindow();
   void GetSelection();
   void HighlightSelectedTelescopes();

   void FillTelescopeListWithType(const char*);

   ClassDef(KVIDGUITelescopeChooserDialog, 1) //ID Grid Manager dialog for choice of ID telescope(s)
};

#endif
