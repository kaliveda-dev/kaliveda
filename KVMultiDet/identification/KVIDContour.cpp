/*
$Id: KVIDContour.cpp,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Mon Apr 14 14:08:54 2008
//Author: franklan

#include "KVIDContour.h"

ClassImp(KVIDContour)


KVIDContour::KVIDContour()
{
   // Default constructor
}

KVIDContour::KVIDContour(const KVIDContour& g)
   : KVIDentifier(g)
{
   // Copy constructor
}

KVIDContour::KVIDContour(const TCutG& g)
   : KVIDentifier(g)
{
   // Copy the TCutG contour
}

KVIDContour::~KVIDContour()
{
   // Destructor
}

