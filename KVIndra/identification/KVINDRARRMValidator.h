/*
$Id: KVINDRARRMValidator.h,v 1.4 2007/07/05 13:37:12 franklan Exp $
$Revision: 1.4 $
$Date: 2007/07/05 13:37:12 $
$Author: franklan $
*/

#ifndef __KVINDRARRMVAL_H
#define __KVINDRARRMVAL_H
#include "TObject.h"
#include "KVNumberList.h"

class KVINDRARRMValidator {
private:

   KVNumberList fRuns;          //list of valid runs
   KVNumberList fRings;         //list of valid rings
   KVNumberList fModules;       //list of valid modules

public:

   KVINDRARRMValidator();
   virtual ~ KVINDRARRMValidator();

   KVNumberList& GetRunList()
   {
      return fRuns;
   };
   KVNumberList& GetRingList()
   {
      return fRings;
   };
   KVNumberList& GetModuleList()
   {
      return fModules;
   };

   void SetRunList(const Char_t* list)
   {
      fRuns.SetList(list);
   };

   void SetModuleList(const Char_t* list)
   {
      fModules.SetList(list);
   }

   void SetRingList(const Char_t* list)
   {
      fRings.SetList(list);
   }

   void SetRunList(TString list)
   {
      fRuns.SetList(list);
   };

   void SetModuleList(TString list)
   {
      fModules.SetList(list);
   }

   void SetRingList(TString list)
   {
      fRings.SetList(list);
   }


   ClassDef(KVINDRARRMValidator, 1)     //Defines validity of e.g. an ID grid in terms of lists of runs, rings and modules (INDRA detectors)
};
#endif
