/*
$Id: KVIDCutLine.cpp,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Fri Feb 15 15:00:17 2008
//Author: franklan

#include "KVIDCutLine.h"
#include "Riostream.h"
#include "KVIDGraph.h"

using namespace std;

ClassImp(KVIDCutLine)

KVIDCutLine::KVIDCutLine() : fAcceptedDirection("above")
{
   // Default constructor
}

KVIDCutLine::~KVIDCutLine()
{
   // Destructor
}

//__________________________________________________________

void KVIDCutLine::WriteAsciiFile_extras(ofstream& file, const Char_t*)
{
   // Write accepted direction for cut

   if (fAcceptedDirection == "") file << "[undefined accepted direction]" << endl;
   else file << fAcceptedDirection.Data() << endl;
}

//__________________________________________________________

void KVIDCutLine::ReadAsciiFile_extras(ifstream& file)
{
   // Read accepted direction for cut

   fAcceptedDirection.ReadLine(file);
   if (fAcceptedDirection == "[undefined accepted direction]") fAcceptedDirection = "";
}

void KVIDCutLine::SetAcceptedDirection(const Char_t* dir)
{
   // Set the direction of the acceptable region relative to the cut line
   // E.g. if points to identify must be above this cut, use
   //    cut->SetAcceptedDirection("above")
   // Possible values are: "above", "below", "left" or "right"
   // (see KVIDLine::WhereAmI).
   fAcceptedDirection = dir;
   if (GetParent()) GetParent()->Modified();
}

