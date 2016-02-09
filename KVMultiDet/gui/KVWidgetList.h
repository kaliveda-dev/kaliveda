/***************************************************************************
                          kvwidgetlist.h  -  description
                             -------------------
    begin                : Wed Jun 5 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVWidgetList.h,v 1.4 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVWL
#define KVWL

#include "KVList.h"

class KVWidget;

class KVWidgetList: public KVList {

public:

   KVWidgetList();
   virtual ~ KVWidgetList();
   KVWidget* GetWidget(Long_t id);
   virtual void PrintWidgets() const;

   ClassDef(KVWidgetList, 0)   // list container for KVWidgets
};

#endif
