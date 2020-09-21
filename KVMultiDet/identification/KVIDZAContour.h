/*
$Id: KVIDZAContour.h,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Mon Apr 14 14:09:54 2008
//Author: franklan

#ifndef __KVIDZACONTOUR_H
#define __KVIDZACONTOUR_H

#include "KVIDContour.h"

/**
\class KVIDZAContour
\brief Graphical contour associated with a given nucleus for use in particle identification
\ingroup Identification
*/

class KVIDZAContour : public KVIDContour {
protected:

   virtual void WriteAsciiFile_extras(std::ofstream&, const Char_t* name_prefix = "");
   virtual void ReadAsciiFile_extras(std::ifstream&);

public:

   KVIDZAContour();
   KVIDZAContour(const KVIDZAContour&);
   KVIDZAContour(const TCutG&);
   virtual ~KVIDZAContour();


   ClassDef(KVIDZAContour, 1) //Graphical contour associated with a given nucleus for use in particle identification
};

#endif
