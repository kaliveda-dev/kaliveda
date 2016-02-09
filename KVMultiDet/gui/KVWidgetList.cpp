//$Id: KVWidgetList.cpp,v 1.5 2006/10/19 14:32:43 franklan Exp $
#include "KVWidgetList.h"
#include "KVBase.h"
#include "KVWidget.h"

ClassImp(KVWidgetList)
//________________________________________________________________________________________________
// Container for KVWidget objects
// Extends KVList class with GetWidget method, which is basically a GetObjectByWidgetID method
//________________________________________________________________________________________________
KVWidgetList::KVWidgetList()
{
}

KVWidgetList::~KVWidgetList()
{
}

KVWidget* KVWidgetList::GetWidget(Long_t id)
{
// return pointer to widget in list with given id

   TIter next((KVList*) this);
   KVBase* widg;
   while ((widg = (KVBase*) next())) {
      if ((((KVWidget*) widg)->GetID()) == id)
         return (KVWidget*) widg;
   }
   return 0;
}

void KVWidgetList::PrintWidgets() const
{
   TIter next((KVList*) this);
   KVBase* widg;
   while ((widg = (KVBase*) next())) {
      ((KVWidget*) widg)->Print();
   }
}
