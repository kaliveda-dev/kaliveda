/*
$Id: KVIDContour.h,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Mon Apr 14 14:08:54 2008
//Author: franklan

#ifndef __KVIDCONTOUR_H
#define __KVIDCONTOUR_H

#include "KVIDentifier.h"
#include "TCutG.h"

class KVIDContour : public KVIDentifier {

public:
   KVIDContour();
   KVIDContour(const KVIDContour&);
   KVIDContour(const TCutG&);
   virtual ~KVIDContour();

   virtual Int_t InsertPoint()
   {
      if (GetEditable()) {
         return TCutG::InsertPoint();
      } else {
         return -2;
      }
   }; // *MENU*

   ClassDef(KVIDContour, 1) //Basic graphical contour class for use in particle identification
};

#endif
