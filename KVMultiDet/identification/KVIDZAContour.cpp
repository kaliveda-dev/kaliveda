/*
$Id: KVIDZAContour.cpp,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Mon Apr 14 14:09:54 2008
//Author: franklan

#include "KVIDZAContour.h"

using namespace std;

ClassImp(KVIDZAContour)

KVIDZAContour::KVIDZAContour()
{
   // Default constructor
}

KVIDZAContour::KVIDZAContour(const KVIDZAContour& g)
   : KVIDContour(g)
{
   // Copy constructor
   SetZ(g.GetZ());
   SetMassFormula(g.GetMassFormula());
   SetA(g.GetA());
}

KVIDZAContour::KVIDZAContour(const TCutG& g)
   : KVIDContour(g)
{
   // Copy the TCutG
}

KVIDZAContour::~KVIDZAContour()
{
   // Destructor
}


//_____________________________________________________________________________________________

void KVIDZAContour::WriteAsciiFile_extras(ofstream& file,
      const Char_t* name_prefix)
{
   // Write Z & A of contour

   KVIDContour::WriteAsciiFile_extras(file, name_prefix);
   file << GetZ() << "\t" << GetA() << endl;
}

//_____________________________________________________________________________________________

void KVIDZAContour::ReadAsciiFile_extras(ifstream& file)
{
   // Read Z & A of contour

   KVIDContour::ReadAsciiFile_extras(file);
   Int_t Z, A;
   file >> Z >> A;
   SetZ(Z);
   SetA(A);
}
