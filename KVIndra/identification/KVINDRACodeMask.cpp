/***************************************************************************
$Id: KVINDRACodeMask.cpp,v 1.2 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVINDRACodeMask.h"

ClassImp(KVINDRACodeMask)

Bool_t KVINDRACodeMask::TestIDCode(UShort_t code)
{
   //check identification code against code mask
   //always kTRUE if no mask set (fIDMask==0)
   if (!fIDMask)
      return kTRUE;
   return (Bool_t)((fIDMask & code) != 0);
}

Bool_t KVINDRACodeMask::TestECode(UChar_t code)
{
   //check calibration code against code mask
   //always kTRUE if no mask set (fEMask==0)
   if (!fEMask)
      return kTRUE;
   return (Bool_t)((fEMask & code) != 0);
}
