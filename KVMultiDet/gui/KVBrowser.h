/***************************************************************************
                          KVBrowser.h  -  description
                             -------------------
    begin                : Mon Nov 18 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVBrowser.h,v 1.5 2006/11/03 08:01:32 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVBROWSER_H
#define KVBROWSER_H

#include "TROOT.h"
#include "TApplication.h"
#include "TVirtualX.h"
#include "TGClient.h"
#include "TGFrame.h"
#include "TGMenu.h"
#include "TGTab.h"
#include "TGLabel.h"
#include "TGFileDialog.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include "TList.h"

class KVWidget;
class KVWidgetList;

class KVBrowser {

private:

   TList* fCleanup;             //!for keeping track of TGLayoutHints and deleting them at the end
   KVWidgetList* fWidgets;      //!list of widgets in mainwindow

public:

   KVBrowser();
   void AddToWidgetList(KVWidget* widg);
   KVWidgetList* GetWidgetList()
   {
      return fWidgets;
   } void AddToCleanupList(TObject* obj)
   {
      fCleanup->Add(obj);
   }
   virtual ~ KVBrowser();

   ClassDef(KVBrowser, 0)       //Basic functionality for KV graphical interface objects
};

#endif
