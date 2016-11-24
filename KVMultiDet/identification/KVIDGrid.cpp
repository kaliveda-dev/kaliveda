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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDGrid</h2>
Abstract base class for 2D identification grids in e.g. (dE,E) maps.
Such a grid is supposed to be made up of two parts:
<ul>
<li>a set of lines for the identification i.e. indicating the position of the
       loci of points corresponding to a given nucleus</li>
<li>a set of lines used to indicate the area of the map in which identification
       is possible (or conversely, the limits beyond which identification is
       not possible).</li>
</ul>
<br>
Specific implementations of such identification grids must define the following
methods :

     <h4>Bool_t IsIdentifiable(Double_t x, Double_t y) const</h4>

     - returns either kTRUE or kFALSE for each point (x,y) of the identification
       map depending on whether such a point can lead to a successful identification
       or not

     <h4>void    Identify        (Double_t x, Double_t y, KVNucleus *nuc) const</h4>

     - attribute an identification to the KVNucleus for a given point (x,y) in the map.
       This is most often based on the result of FindNearestIDLine(), which is a general
     algorithm for finding the ID line which is the closest to a given point.
<br>
Make sure to call the Initialize() method of the grid once before using it.

<h3>Get/SetParameter methods</h3>
You can associate any number of parameters with the grid (KVParameter<Double_t> objects)
A parameter associates a name and a value. These parameters are used to name the grid.

<h3>Grid scaling factors</h3>
Get/SetX/YScaleFactor methods can be used to apply global linear scaling factor to grids
in one or both dimensions. i.e. if we have a grid pointer

<code>
     KVIDGrid* g;//pointer to a grid
     g->SetXScaleFactor(1.2);//we apply a 20% scaling factor to the X-coordinates
     g->Draw(); // X-coordinates of all points in lines are 1.2*original value
</code>

To remove the scaling factor, call SetX/YScaleFactor() or SetX/YScaleFactor(0).
Note that this scaling is taken into account also in IsIdentifiable and Identify methods
<!-- */
// --> END_HTML
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
   } else {
      if (!clas->InheritsFrom("KVIDLine")) {
         Error("AddIDLine",
               "%s is not a valid class deriving from KVIDLine.",
               _cl.Data());
      } else {
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

Int_t KVIDGrid::GetIDLinesEmbracingPoint(const Char_t* direction,
      Double_t x, Double_t y, TList& tmp) const
{
   //Replaces contents of TList 'tmp' with subset of ID lines for which IsBetweenEndPoints(x,y,direction) == kTRUE.
   //nlines = number of lines in list

   TIter next(GetIdentifiers());
   Int_t nlines = 0;
   KVIDLine* line;
   tmp.Clear();
   while ((line = (KVIDLine*) next())) {
      if (line->IsBetweenEndPoints(x, y, direction)) {
         tmp.Add(line);
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

