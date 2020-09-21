/***************************************************************************
$Id: KVIDZALine.cpp,v 1.10 2009/05/05 15:57:52 franklan Exp $
                          KVIDZALine.cpp  -  description
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

#include "KVIDZALine.h"
#include "TVector2.h"
#include "Riostream.h"
#include "TMath.h"

using namespace std;

ClassImp(KVIDZALine)

KVIDZALine::KVIDZALine()
{
   // Default ctor
   // Line width is set to zero
   // Default identification is set to proton

   SetWidth(0.);
   fLineWithWidth = 0;
}

KVIDZALine::KVIDZALine(const KVIDZALine& obj)
   : KVIDLine()
{
   // copy constructor
   fLineWithWidth = 0;
   obj.Copy(*this);
}
void KVIDZALine::Copy(TObject& obj) const
{
   // copy 'this' to 'obj'
   KVIDLine::Copy(obj);
   ((KVIDZALine&)obj).SetWidth(fWidth);
}

KVIDZALine::~KVIDZALine()
{
   //Default dtor
}

//_____________________________________________________________________________________________

void KVIDZALine::WriteAsciiFile_extras(ofstream& file,
                                       const Char_t* name_prefix)
{
   // Write Z & A of line

   KVIDLine::WriteAsciiFile_extras(file, name_prefix);
   file << GetZ() << "\t" << GetA() << endl;
}

//_____________________________________________________________________________________________

void KVIDZALine::ReadAsciiFile_extras(ifstream& file)
{
   // Read Z & A of line

   KVIDLine::ReadAsciiFile_extras(file);
   Int_t Z, A;
   file >> Z >> A;
   SetZ(Z);
   SetA(A);
}

//_____________________________________________________________________________________________

void KVIDZALine::ReadAsciiFile_KVIDZLine(ifstream& file)
{
   //************ BACKWARDS COMPATIBILITY FIX *************
   //       special read method for old KVIDZLines
   //
   //Read coordinates of line in file buffer stream
   //Format is :
   //Z
   //number_of_points
   //x1    y1
   //x2    y2
   //...
   //etc. etc.
   Int_t N, Z;
   file >> Z;
   SetZ(Z);
   file >> N;
   for (Int_t i = 0; i < N; i++) {
      Double_t x, y;
      file >> x >> y;
      SetPoint(i, x, y);
   }
}

//_____________________________________________________________________________________________

void KVIDZALine::Print(Option_t* opt) const
{
   //Print out for line
   //The optional "opt" string, if given, is printed in parentheses after the line's name
   //This is used by KVIDGrid in order to show which lines are "ID" lines and which are
   //"OK" lines (i.e. used to define an identifiable area in a data map).
   cout << ClassName() << " : " << GetName() << "(" << opt << ")" << endl;
   cout << "Z=" << GetZ() << " A=" << GetA() << endl;
   if (GetWidth() > 0.0) cout << "Natural Line Width : " << GetWidth() << endl;
   TGraph::Print();
}

//_____________________________________________________________________________________________

TGraphErrors* KVIDZALine::GetLineWithWidth()
{
   //Return pointer to TGraphError object which can be used to visualise the natural width
   //of this identification line.
   //If line width = 16000, the error bar is set to 0

   if (fLineWithWidth) {
      delete fLineWithWidth;
   }
   fLineWithWidth = new TGraphErrors(GetN(), GetX(), GetY());
   fLineWithWidth->SetName(GetName());
   fLineWithWidth->SetFillColor(2);
   fLineWithWidth->SetFillStyle(3001);
   fLineWithWidth->SetLineStyle(2);
   fLineWithWidth->SetLineWidth(2);
   fLineWithWidth->SetMarkerStyle(4);
   fLineWithWidth->SetMarkerSize(0.8);
   fLineWithWidth->SetEditable(kFALSE);
   for (int i = 0; i < GetN(); i++) {
      fLineWithWidth->SetPointError(i, 0., 0.5 * (GetWidth() < 16000 ? GetWidth() : 0));
   }
   return fLineWithWidth;
}

//_____________________________________________________________________________________________

void KVIDZALine::SetAsymWidth(Double_t d_l, Double_t d_r)
{
   // Set the natural width of the line depending on its asymptotic separation
   // a gauche (d_l) and a droite (d_r) from a neighbouring line.
   // This method is used by KVIDGrid::CalculateLineWidths
   //
   // For ordinary Z/(Z,A) lines in a dE-E grid, this is just the average of
   // the two asymptotic distances.

   SetWidth((d_l + d_r) / 2.);
}

//______________________________________________________________________________
void KVIDZALine::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDZALine.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 2) {
         R__b.ReadVersion(&R__s, &R__c);// read version of KVIDZLine
         KVIDLine::Streamer(R__b);
         UShort_t z;
         R__b >> z;
         R__b >> fWidth;
         SetZ(z);
         UShort_t a;
         R__b >> a;
         SetA(a);
      }
      else {
         R__b.ReadClassBuffer(KVIDZALine::Class(), this, R__v, R__s, R__c);
      }
   }
   else {
      R__b.WriteClassBuffer(KVIDZALine::Class(), this);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////

ClassImp(KVIDZLine)

//////////////////////////////////////////////////
// This class is for backwards compatibility only
// and must not be used.
//////////////////////////////////////////////////

KVIDZLine::KVIDZLine()
{
   // This class is for backwards compatibility only
}

void KVIDZLine::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDZLine

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v != 2) {
         Warning("Streamer", "Reading KVIDZLine with version=%d", R__v);
      }
      KVIDLine::Streamer(R__b);
      UShort_t z;
      R__b >> z;
      R__b >> fWidth;
      SetZ(z);
   }
}
