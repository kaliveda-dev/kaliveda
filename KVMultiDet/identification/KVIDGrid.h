/***************************************************************************
                          KVIDGrid.h  -  description
                             -------------------
    begin                : Nov 10 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDGrid.h,v 1.51 2009/04/03 14:30:27 franklan Exp $
***************************************************************************/

#ifndef KVIDGrid_H
#define KVIDGrid_H

#include "KVIDGraph.h"
#include "KVIDLine.h"
#include "Riostream.h"
#include "TObjArray.h"
#include <TPad.h>
#include "TGWindow.h"
#include "TF1.h"
#include "TClass.h"
#include "TMath.h"

/**
\class KVIDGrid
\brief Abstract base class for 2D identification grids in e.g. (dE,E) maps.
\ingroup Identification

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
*/

class KVIDGrid : public KVIDGraph {

   mutable TObjArray fEmbracingLines;//! temporary array used by GetIDLinesEmbracingPoint

protected:

   void init();
   void ReadIdentifierFromAsciiFile(TString& name, TString& type, TString& cl, std::ifstream& gridfile);

public:

   KVIDGrid();
   virtual ~ KVIDGrid();

   KVIDLine* NewLine(const Char_t* idline_class = "");
   Int_t GetIDLinesEmbracingPoint(const Char_t* direction, Double_t x, Double_t y) const;

   KVIDLine* FindNearestIDLineFast(Double_t x, Double_t y, const Char_t* position,
                                   Int_t& idx, Int_t& idx_min, Int_t& idx_max, Double_t& dist, Double_t& dist_min, Double_t& dist_max) const
   {
      //Fast algorithm for finding the ID line the closest to a given point (x,y) by dichotomy.
      //
      // WARNING: will not always give right result if the distance from the point to each line
      //           is not a smooth function of the line index with a single minimum. In this case
      //           you should use FindNearestIDLine.
      // IF the grid's lines all start and end at the same abscissa, this will work fine.
      // IF NOT, (i.e. lines start and end at different 'x' values), it will sometimes give the wrong answer.
      //            -----------------------------------------------------------------
      //The returned pointer is the closest line, idx is its index in the list of identifiers,
      //whereas idx_min and idx_max are the indices of the two closest lines between which (x,y) lies.
      //dist, dist_min, and dist_max are the distances between the point and these lines.
      //
      //The order of the indices follows the order of the sorting of the ID line list which is in turn
      //determined by the Compare() function of the class of which the ID lines are members.
      //For the algorithm to work, the ID lines must be sorted according to the Compare() function defined for the
      //KVIDLine-based class of which they are members. The "position" string then corresponds to the
      //way lines are sorted in the map (vertically or horizontally, and in which direction). For this reason
      //it is essential to call the Initialise() method of the grid before using it (this will sort the lines).
      //
      //For example:
      //      Suppose we have lines of Z identification (KVIDZALine) in a map with x=E and y=dE.
      //The KVIDZALine::Compare() function sorts lines in order of increasing Z, i.e. in terms of increasing
      //dE, or from bottom to top when looking at the (E,dE) map. Thus the "position" string to use is
      //"above" : if the point (E,dE) is "above" the line currently tested, the algorithm will choose a line
      //which is "higher up" i.e. has a larger Z, or in fact, has a larger index in the list of ID lines. The indices
      //idx_min and idx_max would correspond to the line immediately below and above the point, respectively.
      //       This is for a case of more-or-less horizontal lines sorted from bottom to top.
      //In a map of more-or-less vertical ID lines, with the sorting going from left to right,
      //we would use position="right". (see KVIDLine::WhereAmI for an explanation of "above", "below", etc.).

      Int_t nlines = GetNumberOfIdentifiers();
      idx_min = 0;                 //minimum index
      idx_max = nlines - 1;        // maximum index
      idx = idx_max / 2;     //current index i.e. we begin in the middle
      dist = dist_min = dist_max = -1.;

      while (idx_max > idx_min + 1) {

         KVIDLine* line = (KVIDLine*)GetIdentifierAt(idx);
         Bool_t point_above_line = line->WhereAmI(x, y, position);

         if (point_above_line) {
            //increase index
            idx_min = idx;
            idx += (Int_t)((idx_max - idx) / 2 + 0.5);
         }
         else {
            //decrease index
            idx_max = idx;
            idx -= (Int_t)((idx - idx_min) / 2 + 0.5);
         }
      }
      //calculate distance of point to the two lines above and below
      //and return pointer of the closest one
      KVIDLine* upper = (KVIDLine*)GetIdentifierAt(idx_max);
      KVIDLine* lower = (KVIDLine*)GetIdentifierAt(idx_min);
      Int_t dummy = 0;
      dist_max = TMath::Abs(upper->DistanceToLine(x, y, dummy));
      dist_min = TMath::Abs(lower->DistanceToLine(x, y, dummy));
      if (dist_max < dist_min) {
         dist = dist_max;
         idx = idx_max;
         return upper;
      }
      dist = dist_min;
      idx = idx_min;
      return lower;
   }

   KVIDLine* FindNearestEmbracingIDLine(Double_t x, Double_t y, const Char_t* position, const Char_t* axis,
                                        Int_t& idx, Int_t& idx_min, Int_t& idx_max, Double_t& dist, Double_t& dist_min, Double_t& dist_max) const
   {
      // This is the same as FindNearestIDLineFast except that only lines for which
      // KVIDLine::IsBetweenEndPoints(x,y,axis) returns kTRUE are considered.
      // As we only consider lines between whose endpoints our point lies, this method
      // always gives the correct answer.

      Int_t nlines = GetIDLinesEmbracingPoint(axis, x, y);
      if (!nlines) return 0;   // no lines
      idx_min = 0;                 //minimum index
      idx_max = nlines - 1;        // maximum index
      idx = idx_max / 2;     //current index i.e. we begin in the middle
      dist = dist_min = dist_max = -1.;

      while (idx_max > idx_min + 1) {

         KVIDLine* line = (KVIDLine*)fEmbracingLines.UncheckedAt(idx);
         Bool_t point_above_line = line->WhereAmI(x, y, position);

         if (point_above_line) {
            //increase index
            idx_min = idx;
            idx += (Int_t)((idx_max - idx) / 2 + 0.5);
         }
         else {
            //decrease index
            idx_max = idx;
            idx -= (Int_t)((idx - idx_min) / 2 + 0.5);
         }
      }
      //calculate distance of point to the two lines above and below
      KVIDLine* upper = (KVIDLine*)fEmbracingLines.UncheckedAt(idx_max);
      KVIDLine* lower = (KVIDLine*)fEmbracingLines.UncheckedAt(idx_min);
      Int_t dummy = 0;
      //if idx_max = nlines-1, the point may be above the last line
      //in which case we put idx_max = -1 (no line above point)
      if (idx_max == nlines - 1) {
         if (upper->WhereAmI(x, y, position)) {
            // above last line
            idx = idx_min = fIdentifiers->IndexOf(upper); // index of last line
            idx_max = -1;
            dist = dist_min = TMath::Abs(upper->DistanceToLine(x, y, dummy));
            return upper;
         }
      }
      //if idx_min = 0, the point may be below the first line
      //in which case we put idx_min = -1 (no line below point)
      if (idx_min == 0) {
         if (!lower->WhereAmI(x, y, position)) {
            // below first line
            idx_min = -1;
            idx = idx_max = fIdentifiers->IndexOf(lower); // index of first line
            dist = dist_max = TMath::Abs(lower->DistanceToLine(x, y, dummy));
            return lower;
         }
      }

      dist_max = TMath::Abs(upper->DistanceToLine(x, y, dummy));
      dist_min = TMath::Abs(lower->DistanceToLine(x, y, dummy));
      // convert indices back to index in main list
      idx_min = fIdentifiers->IndexOf(lower);
      idx_max = fIdentifiers->IndexOf(upper);
      if (dist_max < dist_min) {
         dist = dist_max;
         idx = idx_max;
         return upper;
      }
      dist = dist_min;
      idx = idx_min;
      return lower;
   }

   KVIDLine* FindNextEmbracingLine(Int_t& index, Int_t inc_index, Double_t x, Double_t y, const Char_t* axis) const
   {
      // Starting from the line with given 'index', we search for the next line in the list of identifiers for which
      // KVIDLine::IsBetweenEndPoints(x,y,axis) returns kTRUE.
      // 'inc_index' is the step used to scan the list, i.e. inc_index=1 will scan index+1, index+2, ...
      // inc_index=-1 will scan index-1, index-2, ...
      // Returns pointer to line (or 0x0 if not found) and 'index' contains index of this line (or -1 if no line found)

      Int_t ii = index + inc_index;
      Int_t nlines = GetNumberOfIdentifiers();
      KVIDLine* l = 0;
      while ((ii > -1 && ii < nlines)) {
         l = (KVIDLine*)GetIdentifierAt(ii);
         if (l->IsBetweenEndPoints(x, y, axis)) break;
         ii += inc_index;
      }
      if (ii < 0 || ii >= nlines) {
         // no line found
         index = -1;
         return 0;
      }
      index = ii;
      return l;
   }


   void Initialize();

   virtual void CalculateLineWidths() {}

   virtual TClass* DefaultIDLineClass()
   {
      return TClass::GetClass("KVIDLine");
   }
   virtual TClass* DefaultOKLineClass()
   {
      return TClass::GetClass("KVIDLine");
   }


   ClassDef(KVIDGrid, 5)        //Base class for 2D identification grids
};

#endif
