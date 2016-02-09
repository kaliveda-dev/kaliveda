/***************************************************************************
$Id: KVWidget.cpp,v 1.7 2006/10/19 14:32:43 franklan Exp $
                          kvwidget.cpp  -  description
                             -------------------
    begin                : Wed Jun 5 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Riostream.h"
#include "KVWidget.h"
#include "KVDataBaseBrowser.h"
#include "KVDetector.h"
#include "KVMaterial.h"
#include "KVList.h"
#include "KVDBTable.h"
#include "KVDataBase.h"

using namespace std;

ClassImp(KVWidget)
//__________________________________________________________________________________________
// Class holding all information relative to widgets in KVBrowser i.e. widget ID for
// interface, an identifier for the function associated to the widget, and a pointer
// to the object concerned.
Long_t KVWidget::fCurrentID;

KVWidget::KVWidget()
{
// Default constructor
   fWidID = ++fCurrentID;       //use current global ID for widget ID, and increment global ID by 1
   if (fCurrentID > kMAX_WIDGET_ID)
      fCurrentID = 0;
   fFunction = 0;
   fObject = 0;
   fWidget = 0;
}

//________________________________________________________________________________

KVWidget::~KVWidget()
{
   // destructor's principal role is to delete the TGFrame-derived GUI widget
   if (fWidget)
      delete fWidget;
}

//_______________________________________________________________________________

KVWidget::KVWidget(TObject* obj, Long_t func)
{
// Create a new widget associating the detector object *obj with the function func
   fWidID = fCurrentID++;
   // as the display is updated, and widgets are deleted and then redrawn,
   // fCurrentID will just keep increasing, eventually becoming too big for a Long_t (?)
   // So after a while it just starts out at zero again, which should work (I think...)
   if (fCurrentID > kMAX_WIDGET_ID)
      fCurrentID = 0;
   fFunction = func;
   fObject = obj;
   fWidget = 0;
}

//_______________________________________________________________________________

void KVWidget::Action(Long_t parm) const
{
   // perform the action associated with the widget

   switch (fFunction) {
      case CLOSE_BROWSER:
         cout << "CLOSE_BROWSER for " << fObject->
              ClassName() << " " << fObject->GetName() << endl;
         //((KVDetectorBrowser *) fObject)->CloseWindow();
         break;
      case kDET_CHOOSE_MATERIAL:
//                        cout << "CHOOSE MATERIAL: ";
//                    co        ut << ((KVMaterial*)((KVDetector*)fObject)->GetMaterialsList()->At(parm))->GetName() << endl;
         /*                    ((   KVDetector*)fObject)->SetMaterial(
                                ((  KVMaterial*)((KVDetector*)fObject)->GetMaterialsList()->At(parm))
                                );
           */
         break;
      case kDET_SET_THICKNESS:
//                       cout << "SET THICKNESS:";
         ((KVDetector*) fObject)->
         SetThickness(((TGNumberEntry*) GetWidget())->GetNumber());
         break;
      case kDB_TABLE_LIST:
         cout << "TABLE LIST: ";
         ((KVDataBaseBrowser*) fObject)->FillRecordsList(parm);
         break;
      case kDB_RECORD_LIST:
         cout << "RECORD LIST: ";
         break;
      default:
         cout << "KVWidget: Unknown widget or action" << endl;
   }
}

//___   ____________________________________________________________________________
void KVWidget::Print(Option_t*) const
{
   cout << " KVWidget ID = " << fWidID << endl;
   Action();
   cout << endl;
}
