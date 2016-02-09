/***************************************************************************
                          kvDataBasebrowser.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVDataBaseBrowser.h,v 1.2 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVDBB_H
#define KVDBB_H

#include "KVBrowser.h"
#include "TGFrame.h"
#include "TGComboBox.h"

class KVDataBase;

class KVDataBaseBrowser: public KVBrowser, public TGMainFrame {

   friend class KVDataBase;

private:

   KVDataBase* fDbase;         //! the DataBase under study
   TGComboBox* fCB_Tables;      //combo box for database tables
   TGComboBox* fCB_Records;     //combo box for records in a table

public:

   KVDataBaseBrowser(const TGWindow* p, KVDataBase* d, UInt_t w,
                     UInt_t h);
   virtual ~ KVDataBaseBrowser()
   {
   };
   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);
   virtual void FillRecordsList(Long_t);
   ClassDef(KVDataBaseBrowser, 0)       // KaliVeda DataBase Browser and Configuration Tool
};

#endif
