/***************************************************************************
                          KVIDGrid.cpp  -  description
                             -------------------
    begin                : Nov 10 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDGrid.cpp,v 1.60 2009/05/05 15:57:52 franklan Exp $
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVIDGrid.h"
#include "TCanvas.h"
#include "TObjString.h"
#include "Riostream.h"
#include "TObjArray.h"
#include "TROOT.h"
#include "KVString.h"
#include "TVirtualFitter.h"
#include "TClass.h"
#include "TContextMenu.h"
#include "TSystem.h"
#include "TF1.h"
#include "KVIDZALine.h"
#include "KVIDCutLine.h"


using namespace std;

ClassImp(KVIDGrid)

//________________________________________________________________________________

KVIDGrid::KVIDGrid()
{
   //Default constructor
   init();
}

//________________________________________________________________________________

void KVIDGrid::init()
{
   //Initialisations, used by constructors
}

//________________________________________________________________________________

KVIDGrid::~KVIDGrid()
{
}

//_______________________________________________________________________________________________//

KVIDLine* KVIDGrid::NewLine(const Char_t* idline_class)
{
   // Create a new line compatible with this grid.
   //
   // If idline_class = "id" or "ID":
   //       create default identification line object for this grid
   //
   // If idline_class = "ok" or "OK":
   //       create default 'OK' line object for this grid
   //
   // If idline_class = class name:
   //       create line object of given class
   //
   // If idline_class = "":
   //       create KVIDLine object

   TString _cl, _type(idline_class);
   _type.ToUpper();
   TClass* clas = 0;

   if (_type == "ID") clas = DefaultIDLineClass();
   else if (_type == "OK") clas = DefaultOKLineClass();
   else _cl = idline_class;

   if (_cl == "") _cl = "KVIDLine";
   if (!clas) clas = gROOT->GetClass(_cl.Data());

   KVIDLine* line = 0;

   if (!clas) {
      Error("AddIDLine",
            "%s is not a valid classname. No known class.", _cl.Data());
   }
   else {
      if (!clas->InheritsFrom("KVIDLine")) {
         Error("AddIDLine",
               "%s is not a valid class deriving from KVIDLine.",
               _cl.Data());
      }
      else {
         line = (KVIDLine*) clas->New();
      }
   }
   return line;
}

//_______________________________________________________________________________________________//

void KVIDGrid::ReadIdentifierFromAsciiFile(TString& name, TString& type, TString& cl, ifstream& gridfile)
{
   // Read in new identifier object from file
   // Backwards-compatibility fixes

   KVIDentifier* line = 0;
   /************ BACKWARDS COMPATIBILITY FIX *************
      transform all 'OK' KVIDLines into KVIDCutLines
   */
   Bool_t oldcutline = kFALSE;
   if (type == "OK" && cl == "KVIDLine") {
      oldcutline = kTRUE;
   }
   /************ BACKWARDS COMPATIBILITY FIX *************
      transform all 'ID' KVIDZLines into KVIDZALines
   */
   Bool_t zline = kFALSE;
   if (type == "ID" && cl == "KVIDZLine") {
      cl = "KVIDZALine";
      zline = kTRUE;
   }
   line = New(cl.Data());
   //now use ReadAscii method of class to read coordinates and other informations
   /************ BACKWARDS COMPATIBILITY FIX *************
      special read method for old KVIDZLines
   */
   if (zline)((KVIDZALine*)line)->ReadAsciiFile_KVIDZLine(gridfile);
   else {
      line->ReadAsciiFile(gridfile);
      line->SetName(name.Data());
   }
   if (oldcutline) {
      KVIDentifier* oldcut = line;
      line = new KVIDCutLine;
      line->CopyGraph(oldcut);
      delete oldcut;
   }
   if (type == "OK") line->SetName(name.Data());
   Add(type, line);
}

//________________________________________________________________________________________//

Int_t KVIDGrid::GetIDLinesEmbracingPoint(const Char_t* direction, Double_t x, Double_t y) const
{
   //Replaces contents of fEmbracingLines with subset of ID lines for which IsBetweenEndPoints(x,y,direction) == kTRUE.
   //nlines = number of lines in list

   TIter next(GetIdentifiers());
   Int_t nlines = 0;
   KVIDLine* line;
   fEmbracingLines.Clear();
   while ((line = (KVIDLine*) next())) {
      if (line->IsBetweenEndPoints(x, y, direction)) {
         fEmbracingLines.Add(line);
         nlines++;
      }
   }
   return nlines;
}

//___________________________________________________________________________________

void KVIDGrid::Initialize()
{
   // General initialisation method for identification grid.
   // This method MUST be called once before using the grid for identifications.
   // The ID lines are sorted.
   // The natural line widths of all ID lines are calculated.

   SortIdentifiers();
   CalculateLineWidths();
}

