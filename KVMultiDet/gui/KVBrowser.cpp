/***************************************************************************
                          kvbrowser.cpp  -  description
                             -------------------
    begin                : Mon Nov 18 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVBrowser.cpp,v 1.3 2003/08/23 09:17:05 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVBrowser.h"
#include "KVWidget.h"
#include "KVWidgetList.h"

ClassImp(KVBrowser)
//______________________________________________________________________________
//Class providing basic functionality for all KaliVeda graphical interface tools
//All KaliVeda Browsers derive from this class, which takes care of house-keeping
//for TGLayoutHints and other widgets...
//______________________________________________________________________________
KVBrowser::KVBrowser()
{
   // for deleting stray GUI objects
   fCleanup = new TList;
   fWidgets = 0;
}

//_______________________________________________________________________________

KVBrowser::~KVBrowser()
{

   //Clean up all stray TGLayoutHints and delete widgets

   if (fCleanup) {
      fCleanup->Delete();
      delete fCleanup;
      fCleanup = 0;
   }

   if (fWidgets) {
      //fWidgets->Delete();
      delete fWidgets;
      fWidgets = 0;
   }
}

//_______________________________________________________________________________

void KVBrowser::AddToWidgetList(KVWidget* widg)
{
   //Add a new widget to the list

   if (!fWidgets)
      fWidgets = new KVWidgetList;
   fWidgets->Add(widg);
}
