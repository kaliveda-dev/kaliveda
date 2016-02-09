/***************************************************************************
                          kvwidget.h  -  description
                             -------------------
    begin                : Wed Jun 5 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVWidget.h,v 1.4 2004/07/02 13:54:13 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVWIDGET_H
#define KVWIDGET_H

#include "TGFrame.h"
#include "KVBase.h"

#define kMAX_WIDGET_ID 65535

// definition of tags for all functions associated with widgets in interface
enum KV_functions {
   ADD_LAYER,
   UPDATE_ARRAY,
   QUIT_KVBROWSER,
   CLOSE_BROWSER,
   ADD_RING,
   REMOVE_LAYER,
   SET_NUMBER_TELESCOPES_RING,
   SET_THETA_MIN_RING,
   SET_THETA_MAX_RING,
   SET_PHI_MIN_RING,
   SET_PHI_MAX_RING,
   ADD_TELESCOPE,
   REMOVE_RING,
   LAUNCH_BROWSER_TELESCOPE,
   SET_PHI_MIN_TELESCOPE,
   SET_PHI_MAX_TELESCOPE,
   kDET_CHOOSE_MATERIAL,
   kDET_SET_THICKNESS,
   NAMEDIALOG_OK,
   NAMEDIALOG_CANCEL,
   kDB_TABLE_LIST,
   kDB_RECORD_LIST
};

class KVWidget: public KVBase {

private:
   static Long_t fCurrentID;    // current widget ID known by all widgets - it basically counts
   //     the total number of all KVWidgets in existence
   Long_t fWidID;               // this widget's unique ID
   Long_t fFunction;            // function of widget
   TObject* fObject;            //! base class pointer to KVMultiDetectorArray object associated with widget
   TGFrame* fWidget;            //! base pointer to GUI widget associated with widget

public:
   KVWidget();
   KVWidget(TObject* obj, Long_t func);
   virtual ~ KVWidget();
   Long_t GetID()
   {
      return fWidID;
   };
   void SetFunction(Long_t func)
   {
      fFunction = func;
   };
   void SetObject(TObject* obj)
   {
      fObject = obj;
   };
   Long_t GetFunction() const
   {
      return fFunction;
   };
   TObject* GetObject() const
   {
      return fObject;
   };
   void SetWidget(TGFrame* wid)
   {
      fWidget = wid;
   };
   TGFrame* GetWidget() const
   {
      return fWidget;
   };
   void ResetAllWidgets()
   {
      fCurrentID = 0;
   }                            //next created widget will have ID = 1
   virtual void Action(Long_t parm = 0) const;
   virtual void Print(Option_t* opt = "") const;

   ClassDef(KVWidget, 0)        // Class holding all information relative to widgets in KVBrowser
};

#endif
