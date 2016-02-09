/*
$Id: KVCVSUpdateChecker.h,v 1.1 2008/03/05 16:13:45 franklan Exp $
$Revision: 1.1 $
$Date: 2008/03/05 16:13:45 $
*/

//Created by KVClassFactory on Tue Mar  4 17:49:56 2008
//Author: franklan

#ifndef __KVCVSUPDATECHECKER_H
#define __KVCVSUPDATECHECKER_H

#include "KVUpdateChecker.h"

class KVCVSUpdateChecker : public KVUpdateChecker {

public:
   KVCVSUpdateChecker();
   virtual ~KVCVSUpdateChecker();

   virtual Bool_t NeedsUpdate(const Char_t* filename = "");

   virtual void Update(const Char_t* filename = "");

   ClassDef(KVCVSUpdateChecker, 1) //Checks local sources against CVS version and updates if necessary
};

#endif
