/*
$Id: KVIDZoneContour.h,v 1.2 2016/10/09 13:36:00 pastore g3dott0 $
$Revision: 1.2 $
$Date: 2016/10/09 13:36:00 $
*/

//Author: PastoreGiu

#include "KVIDZoneContour.h"
#include "KVIDGraph.h"

using namespace std;

ClassImp(KVIDZoneContour)

KVIDZoneContour::KVIDZoneContour()
{
   // Default constructor
}

KVIDZoneContour::KVIDZoneContour(const KVIDZoneContour& g)
   : KVIDContour(g)
{
   // Copy constructor
}

KVIDZoneContour::KVIDZoneContour(const TCutG& g)
   : KVIDContour(g)
{
   // Copy the TCutG contour
}

KVIDZoneContour::~KVIDZoneContour()
{
   // Destructor
}

//_____________________________________________________________________________________________

void KVIDZoneContour::WriteAsciiFile_extras(ofstream& file,
      const Char_t* name_prefix)
{
   // Write fExclusive

   KVIDContour::WriteAsciiFile_extras(file, name_prefix);
   file << fExclusive << endl;
}

//_____________________________________________________________________________________________

void KVIDZoneContour::ReadAsciiFile_extras(ifstream& file)
{
   // Read fExclusive
   KVIDContour::ReadAsciiFile_extras(file);
   file >> fExclusive;
}

//_____________________________________________________________________________________________

void KVIDZoneContour::SetExclusive(Bool_t e)
{
   // Make contour exclusive i.e. only accept points outside contour
   fExclusive = e;
   if (GetParent()) GetParent()->Modified();
}



