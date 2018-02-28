/***************************************************************************
$Id: KVIDCsIRLLine.cpp,v 1.6 2009/03/03 13:36:00 franklan Exp $
                          KVIDCsIRLLine.cpp  -  description
                             -------------------
    begin                : Nov 10 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVIDCsIRLLine.h"
#include "Riostream.h"

using namespace std;

ClassImp(KVIDCsIRLLine)
/////////////////////////////////////////////////////////////////////////////////////////
//KVIDCsIRLLine
//
//Base class for CsI R-L identification lines (A and Z identification).
//The particularity of such lines is the order in which they appear on the grid
//(in order of increasing R):
//p, d, t, 3He, 4He, 6He, *** 8Be ***, 6Li, 7Li, 8Li, 9Li, 7Be, 9Be, ...
/////////////////////////////////////////////////////////////////////////////////////////

KVIDCsIRLLine::KVIDCsIRLLine()
{
   //Default ctor
}

KVIDCsIRLLine::KVIDCsIRLLine(const KVIDCsIRLLine& obj)
   : KVIDZALine()
{
   // copy constructor
   obj.Copy(*this);
}
KVIDCsIRLLine::~KVIDCsIRLLine()
{
   //Default dtor
}

//_____________________________________________________________________________________________

Int_t KVIDCsIRLLine::Compare(const TObject* obj) const
{
   //Used to sort lists of KVIDCsIRLLines.
   //Sorts the lines as a function of increasing Z, then increasing A
   //but 8Be is between 6He and 6Li

   int zline1 = GetZ();
   int zline2 =
      const_cast < KVIDZALine* >(static_cast <
                                 const KVIDZALine* >(obj))->GetZ();
   int aline1 = GetA();
   int aline2 =
      const_cast < KVIDCsIRLLine* >(static_cast <
                                    const KVIDCsIRLLine*
                                    >(obj))->GetA();
   //both same line ?
   if (zline1 == zline2 && aline1 == aline2)
      return 0;
   //special case : 8Be. If one of the two lines is 8Be, special treatment
   //all lines with Z<3 are below 8Be, all lines with Z>=3 are above 8Be
   if (zline1 == 4 && aline1 == 8) {
      if (zline2 < 3)
         return 1;
      else
         return -1;
   }
   if (zline2 == 4 && aline2 == 8) {
      if (zline1 < 3)
         return -1;
      else
         return 1;
   }
   //standard sorting when neither line is 8be
   return KVIDZALine::Compare(obj);
}

//_____________________________________________________________________________________________

void KVIDCsIRLLine::Print(Option_t* opt) const
{
   //Print out for line
   //The optional "opt" string, if given, is printed in parentheses after the line's name
   //This is used by KVIDGrid in order to show which lines are "ID" lines and which are
   //"OK" lines (i.e. used to define an identifiable area in a data map).
   //"Natural width" of line is given
   cout << ClassName() << " : " << GetName() << "(" << opt << ")" << endl;
   cout << "Natural Line Width : " << GetWidth() << endl;
   TGraph::Print();
}

//_____________________________________________________________________________________________

void KVIDCsIRLLine::SetAsymWidth(Double_t d_l, Double_t d_r)
{
   //Set the natural width of the line depending on its asymptotic separation
   //a gauche (d_l) and a droite (d_r) from a neighbouring line.
   //This method is used by KVIDZGrid::CalculateLineWidths
   //
   //For lines in a CsI R-L grid, this is the minimum of
   //three times the asymptotic distance a gauche & the asymptotic distance a droite.
   SetWidth(TMath::Min(3.*d_l, d_r));
}
