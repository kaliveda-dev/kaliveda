/*
$Id: KVIDCutContour.cpp,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Mon Apr 14 14:10:56 2008
//Author: franklan

#include "KVIDCutContour.h"
#include "KVIDGraph.h"

using namespace std;

ClassImp(KVIDCutContour)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDCutContour</h2>
<h4>Graphical contour for excluding/including regions in particle identification maps</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDCutContour::KVIDCutContour()
{
   // Default constructor
}

KVIDCutContour::KVIDCutContour(const KVIDCutContour& g)
   : KVIDContour(g)
{
   // Copy constructor
}

KVIDCutContour::KVIDCutContour(const TCutG& g)
   : KVIDContour(g)
{
   // Copy the TCutG contour
}

KVIDCutContour::~KVIDCutContour()
{
   // Destructor
}

//_____________________________________________________________________________________________

void KVIDCutContour::WriteAsciiFile_extras(ofstream& file,
      const Char_t* name_prefix)
{
   // Write fExclusive

   KVIDContour::WriteAsciiFile_extras(file, name_prefix);
   file << fExclusive << endl;
}

//_____________________________________________________________________________________________

void KVIDCutContour::ReadAsciiFile_extras(ifstream& file)
{
   // Read fExclusive

   KVIDContour::ReadAsciiFile_extras(file);
   file >> fExclusive;
}

void KVIDCutContour::SetExclusive(Bool_t e)
{
   // Make contour exclusive i.e. only accept points outside contour
   fExclusive = e;
   if (GetParent()) GetParent()->Modified();
}

