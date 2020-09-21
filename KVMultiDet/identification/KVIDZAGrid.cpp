/***************************************************************************
                          KVIDZAGrid.cpp  -  description
                             -------------------
    begin                : Nov 24 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDZAGrid.cpp,v 1.24 2009/05/05 15:57:52 franklan Exp $
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "KVIDZAGrid.h"
#include "KVIDZALine.h"
#include "KVIDCutLine.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDZAGrid)
KVIDZAGrid::KVIDZAGrid()
{
   //default ctor.
   init();
}

KVIDZAGrid::~KVIDZAGrid()
{
   //default dtor.
}

KVIDZAGrid::KVIDZAGrid(const KVIDZAGrid& grid)
{
   //Copy constructor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   grid.Copy(*this);
#else
   ((KVIDZAGrid&) grid).Copy(*this);
#endif
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVIDZAGrid::Copy(TObject& obj) const
#else
void KVIDZAGrid::Copy(TObject& obj)
#endif
{
   //Copy this to 'obj'
   KVIDGrid::Copy(obj);
   ((KVIDZAGrid&) obj).SetZmax(const_cast <KVIDZAGrid*>(this)->GetZmax());
}

//_________________________________________________________________________//

void KVIDZAGrid::init()
{
   //initialisation
   fZMax = 0;
   fZMaxLine = 0;

   kinfi = kinf = ksup = ksups = -1;
   dinf = dsup = dinfi = dsups = 0.;
   winf = wsup = winfi = wsups = 0.;
   Zinfi = Zinf = Zsup = Zsups = Ainfi = Ainf = Asup = Asups = 0;
   fDistanceClosest = -1.;
   fClosest = fLsups = fLsup = fLinf = fLinfi = 0;
   fIdxClosest = -1;
}

//_________________________________________________________________________//

void KVIDZAGrid::RemoveLine(Int_t Z, Int_t A)
{
   // Remove and destroy identifier

   gROOT->ProcessLine("if(gROOT->FindObject(\"gIDGridEditorCanvas\")) gIDGridEditor->Clear()");

   Int_t toto = -1;
   KVIDZALine* tmpline = 0;
   if ((A > 0) && (!IsOnlyZId())) {
      if ((tmpline = GetZALine(Z, A, toto))) RemoveIdentifier((KVIDentifier*)tmpline);
   }
   else {
      if (!IsOnlyZId()) {
         KVList* tmplist = (KVList*)fIdentifiers->GetSubListWithMethod(Form("%d", Z), "GetZ");
         TIter next_id(tmplist);
         while ((tmpline = (KVIDZALine*)next_id())) {
            if (tmpline) RemoveIdentifier((KVIDentifier*)tmpline);
         }
         delete tmplist;
      }
      else if ((tmpline = GetZLine(Z, toto))) RemoveIdentifier((KVIDentifier*)tmpline);
   }
}

//_________________________________________________________________________//

void KVIDZAGrid::RemoveZLines(const Char_t* ZList)
{
   // Remove and destroy identifiers
   KVNumberList ZL(ZList);
   ZL.Begin();
   while (!ZL.End()) {
      Int_t Z = ZL.Next();
      RemoveLine(Z, -1);
   }
}

//_________________________________________________________________________//

KVIDZALine* KVIDZAGrid::GetZLine(Int_t z, Int_t& index) const
{
   //Returns ID line for which GetZ() returns 'z'. index=index of line found in
   //fIDLines list (-1 if not found).
   //To increase speed, this is done by dichotomy, rather than by looping over
   //all the lines in the list.

   index = -1;
   Int_t nlines = GetNumberOfIdentifiers();
   UInt_t idx_min = 0;          //minimum index
   UInt_t idx_max = nlines - 1; // maximum index
   //as a first guess, we suppose that the lines are numbered sequentially from
   //Z=1 (index 0) to Z=zmax=nlines (index nlines-1) with no gaps
   UInt_t idx = ((UInt_t) z - 1 > idx_max ? idx_max : (UInt_t) z - 1);

   while (idx_max > idx_min + 1) {

      KVIDZALine* line = (KVIDZALine*) GetIdentifierAt(idx);
      Int_t zline = line->GetZ();
      //Found it ?
      if (zline == z) {
         index = idx;
         return line;
      }


      if (z > zline) {
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
   //if one of these two lines has the right Z, return its pointer
   KVIDZALine* line = (KVIDZALine*) GetIdentifierAt(idx_min);
   if (line->GetZ() == z) {
      index = idx_min;
      return line;
   }
   line = (KVIDZALine*) GetIdentifierAt(idx_max);
   if (line->GetZ() == z) {
      index = idx_max;
      return line;
   }
   //if not, there is no line in the grid with the right Z
   return 0;
}

//_________________________________________________________________________//

KVIDZALine* KVIDZAGrid::GetZALine(Int_t z, Int_t a, Int_t& index) const
{
   //Returns ID line corresponding to nucleus (Z,A) and its index in fIDLines.
   //First we use GetLine(z) to find a line with the right Z (in
   //principal there are several in the grid), then search for the correct
   //isotope among these.

   Int_t idx;
   index = -1;
   KVIDZALine* line = GetZLine(z, idx);
   //no line with correct Z ?
   if (!line)
      return 0;
   //got right mass?
   if (line->GetA() == a) {
      index = idx;
      return line;
   }
   //increase/decreas index depending on if mass of line is greater than or less than a
   if (a > line->GetA()) {
      for (int i = idx; i < (int) GetNumberOfIdentifiers(); i++) {
         line = (KVIDZALine*) GetIdentifierAt(i);
         if (line->GetZ() != z)
            return 0;           //no longer correct Z
         if (line->GetA() == a) {
            index = i;
            return line;
         }
      }
      //gone to the end of the list and still not found it
      return 0;
   }
   else if (a < line->GetA()) {
      for (int i = idx; i > 0; i--) {
         line = (KVIDZALine*) GetIdentifierAt(i);
         if (line->GetZ() != z)
            return 0;           //no longer correct Z
         if (line->GetA() == a) {
            index = i;
            return line;
         }
      }
      //got to start of list and still not found
      return 0;
   }
   return 0;
}

//_______________________________________________________________________________________________//

void KVIDZAGrid::CalculateLineWidths()
{
   //Calculate natural "width" of each line in the grid.
   //The lines in the grid are first sorted so that they are in order of ascending 'Y'
   //i.e. first line is 1H, last line is the heaviest isotope (highest line).
   //
   //Then, for a given line :
   //
   //   ****  if the grid is to be used for A & Z identification (default)**** :
   //      - if the next line (above) has the same Z, we use the separation between these
   //        two lines corresponding to different isotopes of the same element
   //      - if the next line (above) has a different Z, but the line before (below) has
   //        the same Z, we use the separation between the line below and this one
   //      - if neither adjacent line has the same Z, the width is set to 16000 (huge).
   //
   //  **** if the grid is to be used for Z identification (fOnlyZid=kTRUE)**** :
   //      - we use the separation between each pair of lines
   //
   //In each case we find D_L (the separation between the two lines at their extreme left)
   //and  D_R (their separation at extreme right). The width of the line is then calculated
   //from these two using the method KVIDZALine::SetAsymWidth (which may be overridden in child
   //classes).


//    Info("CalculateLineWidths",
//          "For grid %s (%s vs. %s, runs %s).", GetName(), GetVarY(), GetVarX(), GetRunList());

   for (Int_t i = 0; i < (Int_t) GetNumberOfIdentifiers(); i++) {

      KVIDentifier* obj =  GetIdentifierAt(i);
      if (!obj->InheritsFrom("KVIDZALine")) continue;
      KVIDZALine* _line = (KVIDZALine*) obj;
//      KVIDZALine* _line = (KVIDZALine*) GetIdentifierAt(i);

      //Z of lines above and below this line - Zxx=-1 if there is no line above or below
      Int_t Zhi =
         (i <
          (Int_t) GetNumberOfIdentifiers() -
          1 ? ((KVIDZALine*) GetIdentifierAt(i + 1))->GetZ() : -1);
      Int_t Zlo = (i > 0 ? ((KVIDZALine*) GetIdentifierAt(i - 1))->GetZ() : -1);
      Int_t Z = _line->GetZ();

      // find line for comparison.
      // if we are dealing with a grid with several isotopes for each Z, the priority
      // is to calculate isotopic line widths using lines with same Z, different A.
      // if the grid is 'OnlyZId' it should have only one line per Z, so we calculate
      // widths by comparing neighbouring Z.
      // if the grid is not 'OnlyZId', it may still have some lines with just one isotope
      // per Z, in this case we calculate a width based on the separation from the
      // neighbouring Z, as if it were 'OnlyZId'.
      Int_t i_other;// index of line used to calculate width
      if (IsOnlyZId()) { //grid is 'OnlyZId':calculate widths by comparing neighbouring Z.
         i_other = (Zhi > -1 ? i + 1 : (Zlo > -1 ? i - 1 : -1));
      }
      else {
         //grid with several isotopes for each Z:calculate isotopic widths using lines with same Z, different A
         i_other = (Zhi == Z ? i + 1 : (Zlo == Z ? i - 1 : -1));
      }
      if (!IsOnlyZId() && i_other < 0) { //grid is not 'OnlyZId' but only one isotope for this Z
         i_other = (Zhi > -1 ? i + 1 : (Zlo > -1 ? i - 1 : -1));
      }

      //default width of 16000 in case of "orphan" line
      if (i_other < 0) {
         //Info("CalculateLineWidths", "No line found for comparison with %s. Width set to 16000", _line->GetName());
         _line->SetWidth(16000.);
         continue;              // skip to next line
      }

      KVIDZALine* _otherline = (KVIDZALine*) GetIdentifierAt(i_other);

      //calculate asymptotic distances between lines at left and right.
      //do this by finding which line's endpoint is between both endpoints of the other line
      Double_t D_L, D_R;

      //***************    LEFT SIDE    ********************************

      //get coordinates of first point of our line
      Double_t x1, y1;
      _line->GetStartPoint(x1, y1);
      KVIDZALine* _aline, *_bline;

      if (_otherline->IsBetweenEndPoints(x1, y1, "x")) {

         //first point of our line is "inside" the X-coords of the other line:
         //asymptotic distance LEFT is distance from our line's starting point (x1,Y1) to the other line
         _aline = _otherline;
         _bline = _line;

      }
      else {

         //first point of other line is "inside" the X-coords of the our line:
         //asymptotic distance LEFT is distance from other line's starting point (x1,Y1) to our line
         _aline = _line;
         _bline = _otherline;
         _otherline->GetStartPoint(x1, y1);

      }

      //calculate D_L
      Int_t dummy = 0;
      D_L = _aline->DistanceToLine(x1, y1, dummy);

      //make sure that D_L is positive : if not, we try to calculate other way (inverse line and starting point)
      if (D_L < 0.) {

         Double_t oldD_L = D_L;

         _aline->GetStartPoint(x1, y1);
         D_L = _bline->DistanceToLine(x1, y1, dummy);

         //still no good ? then we keep the smallest absolute value
         if (D_L < 0.) {
            D_L = TMath::Abs(TMath::Max(D_L, oldD_L));
         }
      }
      //***************   RIGHT SIDE    ********************************

      //get coordinates of last point of our line
      _line->GetEndPoint(x1, y1);

      if (_otherline->IsBetweenEndPoints(x1, y1, "x")) {

         //last point of our line is "inside" the X-coords of the other line:
         //asymptotic distance RIGHT is distance from our line's end point (x1,Y1) to the other line
         _aline = _otherline;
         _bline = _line;

      }
      else {

         //last point of other line is "inside" the X-coords of the our line:
         //asymptotic distance RIGHT is distance from other line's end point (x1,Y1) to our line
         _aline = _line;
         _bline = _otherline;
         _otherline->GetEndPoint(x1, y1);

      }

      //calculate D_R
      D_R = _aline->DistanceToLine(x1, y1, dummy);

      //make sure that D_R is positive : if not, we try to calculate other way (inverse line and end point)
      if (D_R < 0.) {

         Double_t oldD_R = D_R;

         _aline->GetEndPoint(x1, y1);
         D_R = _bline->DistanceToLine(x1, y1, dummy);

         //still no good ? then we keep the smallest absolute value
         if (D_R < 0.) {
            D_R = TMath::Abs(TMath::Max(D_R, oldD_R));
         }
      }
      //***************  SET NATURAL WIDTH OF LINE   ********************************

      _line->SetAsymWidth(D_L, D_R);


      //Info("CalculateLineWidths", "...width for line %s set to : %f (D_L=%f,D_R=%f)", _line->GetName(), _line->GetWidth(), D_L, D_R);
   }
}

//_______________________________________________________________________________________________//

void KVIDZAGrid::DrawLinesWithWidth()
{
   //This method displays the grid as in KVIDGrid::Draw, but
   //the natural line widths are shown as error bars

   if (!gPad) {
      new TCanvas("c1", GetName());
   }
   else {
      gPad->SetTitle(GetName());
   }
   if (!gPad->GetListOfPrimitives()->GetSize()) {
      //calculate size of pad necessary to show grid
      if (GetXmin() == GetXmax())
         const_cast < KVIDZAGrid* >(this)->FindAxisLimits();
      gPad->DrawFrame(GetXmin(), GetYmin(), GetXmax(), GetYmax());
   }
   TIter nextID(GetIdentifiers());
   KVIDZALine* line;
   while ((line = (KVIDZALine*)nextID())) {
      line->GetLineWithWidth()->Draw("3PL");
   }
   {
      GetCuts()->R__FOR_EACH(KVIDLine, Draw)("PL");
   }
   gPad->Modified();
   gPad->Update();
}

//______________________________________________________________________________________________//

Bool_t KVIDZAGrid::FindFourEmbracingLines(Double_t x, Double_t y, const Char_t* position)
{
   // This method will locate (at most) four lines close to the point (x,y), the point must
   // lie within the endpoints (in X) of each line (the lines "embrace" the point).
   // Returns kTRUE if at least one line is found. Identification can then be carried out with
   // either IdentZA or IdentZ (see Identify).
   // Returns kFALSE if no lines are found (not even a closest embracing line).
   //
   // We look for two lines above the point and two lines below the point, as in one of the following two cases:
   //
   //                      ------------------------     ksups       ---------------------
   //
   //  closest --->  ------------------------      ksup        ---------------------
   //                                     X
   //
   //                                                                                             X
   //                      ------------------------      kinf        ---------------------     <--- closest
   //
   //                      ------------------------     kinfi        ---------------------
   //
   // First we find the closest embracing line to the point, using FindNearestEmbracingIDLine.
   // Then we search above and below for the other 'embracing' lines. Note that no condition is
   // applied regarding the distances to these lines: the lines must have been sorted in order of increasing
   // ordinate before hand in Initialize(), we simply use the order of lines in the list of identifiers.
   // The Z, A, width and distance to each of these lines are stored in the variables
   //      Zsups, Asups, wsups, dsups
   // etc. etc. to be used by IdentZA or IdentZ.

   kinfi = kinf = ksup = ksups = -1;
   dinf = dsup = dinfi = dsups = 0.;
   winf = wsup = winfi = wsups = 16000.;
   Zinfi = Zinf = Zsup = Zsups = Ainfi = Ainf = Asup = Asups = -1;
   fDistanceClosest = -1.;
   fClosest = fLsups = fLsup = fLinf = fLinfi = 0;
   fIdxClosest = -1;

   fClosest = FindNearestEmbracingIDLine(x, y, position, "x", fIdxClosest, kinf, ksup, fDistanceClosest, dinf, dsup);

   if (!fClosest) return kFALSE; // no lines found

   Int_t dummy = 0;
   if (kinf > -1 && kinf == fIdxClosest) {
      //point is above closest line, closest line is "kinf"
      //need to look for 2 lines above (ksup, ksups) and 1 line below (kinfi)
      fLinf = fClosest;
      if (fLinf->InheritsFrom("KVIDZALine")) {
         winf = ((KVIDZALine*)fLinf)->GetWidth();
         Zinf = fLinf->GetZ();
         Ainf = fLinf->GetA();
      }
      if (ksup > -1) {
         fLsup = (KVIDLine*)GetIdentifierAt(ksup);
         if (fLsup->InheritsFrom("KVIDZALine")) {
            wsup = ((KVIDZALine*)fLsup)->GetWidth();
            Zsup = fLsup->GetZ();
            Asup = fLsup->GetA();
         }
      }
   }
   else if (ksup > -1 && ksup == fIdxClosest) {
      //point is below closest line, closest line is "ksup"
      //need to look for 1 line above (ksups) and 2 lines below (kinf, kinfi)
      fLsup = fClosest;
      if (fLsup->InheritsFrom("KVIDZALine")) {
         wsup = ((KVIDZALine*)fLsup)->GetWidth();
         Zsup = fLsup->GetZ();
         Asup = fLsup->GetA();
      }
      if (kinf > -1) {
         fLinf = (KVIDLine*)GetIdentifierAt(kinf);
         if (fLinf->InheritsFrom("KVIDZALine")) {
            winf = ((KVIDZALine*)fLinf)->GetWidth();
            Zinf = fLinf->GetZ();
            Ainf = fLinf->GetA();
         }
      }
   }
   else {
      Error("FindFourEmbracingLines",
            "I do not understand the result of FindNearestEmbracingIDLine!!!");
      return kFALSE;
   }


   if (kinf > -1) {
      // look for kinfi line -> next line below 'inf' line
      kinfi = kinf;
      fLinfi = FindNextEmbracingLine(kinfi, -1, x, y, "x");
      if (!fLinfi) kinfi = -1;   // no 'infi' line found
      else {
         dinfi = TMath::Abs(fLinfi->DistanceToLine(x, y, dummy));
         if (fLinfi->InheritsFrom("KVIDZALine")) {
            winfi = ((KVIDZALine*)fLinfi)->GetWidth();
            Zinfi = fLinfi->GetZ();
            Ainfi = fLinfi->GetA();
         }
      }
   }
   if (ksup > -1) {
      // look for ksups line -> next line above 'sup' line
      ksups = ksup;
      fLsups = FindNextEmbracingLine(ksups, 1, x, y, "x");
      if (!fLsups) ksups = -1;   // no 'sups' line found
      else {
         dsups = TMath::Abs(fLsups->DistanceToLine(x, y, dummy));
         if (fLsups->InheritsFrom("KVIDZALine")) {
            wsups = ((KVIDZALine*)fLsups)->GetWidth();
            Zsups = fLsups->GetZ();
            Asups = fLsups->GetA();
         }
      }
   }
   return kTRUE;
}

//_________________________________________________________________________//

void KVIDZAGrid::IdentZA(Double_t x, Double_t y, Int_t& Z, Double_t& A)
{
   //Finds Z, A and 'real A' for point (x,y) once closest lines to point have been found
   //by calling method FindFourEmbracingLines beforehand.
   //This is a line-for-line copy of the latter part of IdnCsOr, even the same
   //variable names and comments have been used (as much as possible).

   fICode = kICODE0;
   Z = -1;
   A = -1;
   Aint = 0;
   /*    cout << "kinfi = " << kinfi << " Zinfi = " << Zinfi << "  Ainfi = " << Ainfi << "  winfi = " << winfi << "  dinfi = " << dinfi << endl;
      cout << "kinf = " << kinf << " Zinf = " << Zinf << "  Ainf = " << Ainf << "  winf = " << winf << "  dinf = " << dinf << endl;
      cout << "ksup = " << ksup << " Zsup = " << Zsup << "  Asup = " << Asup << "  wsup = " << wsup << "  dsup = " << dsup << endl;
      cout << "ksups = " << ksups << " Zsups = " << Zsups << "  Asups = " << Asups << "  wsups = " << wsups << "  dsups = " << dsups << endl;
    */
   Int_t ibif = 0;
   Int_t k = -1;
   Double_t yy, y1, y2;
   Int_t ix1, ix2;
   yy = y1 = y2 = 0;
   ix1 = ix2 = 0;
   if (ksup > -1) {
      if (kinf > -1) {
         //cout << " /******************* 2 lignes encadrant le point ont ete trouvees ************************/" << endl;
         Double_t dt = dinf + dsup;     //distance between the 2 lines
         if (Zinf == Zsup) {
            //   cout << "      /****************meme Z**************/" << endl;
            Z = Zinf;
            Int_t dA = Asup - Ainf;
            Double_t dist = dt / dA;    //distance between the 2 lines normalised to difference in A of lines
            /*** A = Asup ***/
            if (dinf > dsup) {  //point is closest to upper line, 'sup' line
               ibif = 1;
               k = ksup;
               yy = -dsup;
               A = Asup;
               Aint = Asup;
               if (ksups > -1) {        // there is a 'sups' line above the 2 which encadrent le point
                  y2 = dsups - dsup;
                  if (Zsups == Zsup) {
                     ibif = 0;
                     y2 /= 2.;
                     ix2 = Asups - Asup;
                  }
                  else {
                     y2 /= 2.;
                     Double_t x2 = wsup;
                     x2 = 0.5 * TMath::Max(x2, dist);
                     y2 = TMath::Min(y2, x2);
                     ix2 = 1;
                  }
               }
               else {         // ksups == -1 i.e. no 'sups' line
                  y2 = wsup;
                  y2 = 0.5 * TMath::Max(y2, dist);
                  ix2 = 1;
               }
               y1 = -dt / 2.;
               ix1 = -dA;
            }
            /*** A = Ainf ***/
            else {              //point is closest to lower line, 'inf' line
               ibif = 2;
               k = kinf;
               yy = dinf;
               A = Ainf;
               Aint = Ainf;
               if (kinfi > -1) {        // there is a 'infi' line below the 2 which encadrent le point
                  y1 = 0.5 * (dinfi - dinf);
                  if (Zinfi == Zinf) {
                     ibif = 0;
                     ix1 = Ainfi - Ainf;
                     y1 = -y1;
                  }
                  else {
                     Double_t x1 = winf;
                     x1 = 0.5 * TMath::Max(x1, dist);
                     y1 = -TMath::Min(y1, x1);
                     ix1 = -1;
                  }
               }
               else {         // kinfi = -1 i.e. no 'infi' line
                  y1 = winf;
                  y1 = -0.5 * TMath::Max(y1, dist);
                  ix1 = -1;
               }
               y2 = dt / 2.;
               ix2 = dA;
            }
         }
         else {
            //cout << "         /****************Z differents**************/ " << endl;
            /*** Z = Zsup ***/
            ibif = 3;
            if (dinf > dsup) {  // closest to upper 'sup' line
               k = ksup;
               yy = -dsup;
               Z = Zsup;
               A = Asup;
               Aint = Asup;
               y1 = 0.5 * wsup;
               if (ksups > -1) {        // there is a 'sups' line above the 2 which encadrent the point
                  y2 = dsups - dsup;
                  if (Zsups == Zsup) {
                     ibif = 2;
                     ix2 = Asups - Asup;
                     Double_t x1 = y2 / ix2 / 2.;
                     y1 = TMath::Max(y1, x1);
                     y1 = -TMath::Min(y1, dt / 2.);
                     ix1 = -1;
                     y2 /= 2.;
                  }
                  else {
                     y2 /= 2.;
                     y2 = TMath::Min(y1, y2);
                     ix2 = 1;
                     y1 = -TMath::Min(y1, dt / 2.);
                     ix1 = -1;
                  }
               }
               else {         // ksups == -1, i.e. no 'sups' line
                  fICode = kICODE7;     //a gauche de la ligne fragment, Z est alors un Zmin et le plus probable
                  y2 = y1;
                  ix2 = 1;
                  y1 = -TMath::Min(y1, dt / 2.);
                  ix1 = -1;
               }
            }
            /*** Z = Zinf ***/
            else {              // closest to lower 'inf' line
               k = kinf;
               yy = dinf;
               Z = Zinf;
               A = Ainf;
               Aint = Ainf;
               y2 = 0.5 * winf;
               if (kinfi > -1) {        // there is a 'infi' line below the 2 which encadrent the point
                  y1 = dinfi - dinf;
                  if (Zinfi == Zinf) {
                     ibif = 1;
                     ix1 = Ainfi - Ainf;
                     Double_t x2 = -y1 / ix1 / 2.;
                     y2 = TMath::Max(y2, x2);
                     y2 = TMath::Min(y2, dt / 2.);
                     ix2 = 1;
                     y1 /= -2.;
                  }
                  else {
                     y1 = -TMath::Min(y2, y1 / 2.);
                     ix1 = -1;
                     y2 = TMath::Min(y2, dt / 2.);
                     ix2 = 1;
                  }
               }
               else {         // no kinfi line, kinfi==-1
                  y1 = -y2;
                  ix1 = -1;
                  y2 = TMath::Min(y2, dt / 2.);
                  ix1 = 1;
               }
            }
         }
      }//if(kinf>-1)...
      else if (Zsup > 0) {
         //cout<<" /****************** Seule une ligne superieure a ete trouvee *********************/" << endl;
         ibif = 3;
         k = ksup;
         yy = -dsup;
         Z = Zsup;
         A = Asup;
         Aint = Asup;
         y1 = 0.5 * wsup;
         if (ksups > -1) {      // there is a 'sups' line above the closest line to the point
            y2 = dsups - dsup;
            if (Zsups == Zsup) {
               ibif = 2;
               ix2 = Asups - Asup;
               Double_t x1 = y2 / ix2 / 2.;
               y1 = -TMath::Max(y1, x1);
               ix1 = -1;
               y2 /= 2.;
            }
            else {
               y2 /= 2.;
               y2 = TMath::Min(y1, y2);
               ix2 = 1;
               y1 = -y1;
               ix1 = -1;
            }
         }
         else {               // no 'sups' line above closest line
            fICode = kICODE7;   //Z est alors un Zmin et le plus probable
            y2 = y1;
            ix2 = 1;
            y1 = -y1;
            ix1 = -1;
         }
         if (yy >= y1)
            fICode = kICODE0; // we are within the 'natural width' of the last line
         else {
            fICode = kICODE6; // we are too far from first line to extrapolate correctly
            Z = Zsup - 1; // give Z below first line of grid, but this is an upper limit
         }
      }
      else {
         fICode = kICODE8;      //  Z indetermine ou (x,y) hors limites
      }
   }
   else if (kinf > -1) {

      //cout <<"/****************** Seule une ligne inferieure a ete trouvee ***********************/" << endl;

      ibif = 3;
      k = kinf;
      Z = Zinf;
      A = Ainf;
      Aint = Ainf;
      yy = dinf;
      y2 = 0.5 * winf;
      if (kinfi > -1) {
         y1 = dinfi - dinf;
         if (Zinfi == Zinf) {
            ibif = 1;
            ix1 = Ainfi - Ainf;
            Double_t x2 = -y1 / ix1 / 2.;
            y2 = TMath::Max(y2, x2);
            ix2 = 1;
            y1 /= -2.;
         }
         else {
            y1 = -TMath::Min(y2, y1 / 2.);
            ix1 = -1;
            ix2 = 1;
         }
      }
      else {
         y1 = -y2;
         ix1 = -1;
         ix2 = 1;
      }
      if (yy <= y2)
         fICode = kICODE0; // we are within the 'natural width' of the last line
      else
         fICode = kICODE7; // we are too far from last line to extrapolate correctly

   }
   /*****************Aucune ligne n'a ete trouvee*********************************/
   else {
      fICode = kICODE8;         // Z indetermine ou (x,y) hors limites
   }
   /****************Test des bornes********************************************/
   if (k > -1 && fICode == kICODE0) {
      if (yy > y2)
         fICode = kICODE4;      // Z ok, masse hors limite superieure ou egale a A
   }
   if (k > -1 && (fICode == kICODE0 || fICode == kICODE7)) {
      if (yy < y1)
         fICode = kICODE5;      // Z ok, masse hors limite inferieure ou egale a A
   }
   if (fICode == kICODE4 || fICode == kICODE5) {
      A = -1;
      Aint = 0;
   }
   /****************Interpolation de la masse: da = f*log(1+b*dy)********************/
   if (fICode == kICODE0) {
      Double_t deltaA = 0.;
      Bool_t i = kFALSE;
      Double_t dt, dist = y1 * y2;
      dt = 0.;
      if (ix2 == -ix1) {        //dA1 = dA2
         if (dist != 0) {
            dt = -(y1 + y2) / dist;
            i = kTRUE;
         }
         /*else
            Warning("IdentZA","%s : cannot calculate interpolated mass, Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
               GetName(), Z, Aint, fICode);*/
      }
      else if (ix2 == -ix1 * 2) {     // dA2 = 2*dA1
         Double_t tmp = y1 * y1 - 4. * dist;
         if (tmp > 0 && dist != 0) {
            dt = -(y1 + 2. * y2 -
                   TMath::Sqrt(tmp)) / dist / 2.;
            i = kTRUE;
         }
         /*else
            Warning("IdentZA","%s : cannot calculate interpolated mass, Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
               GetName(), Z, Aint, fICode);*/
      }
      else if (ix1 == -ix2 * 2) {     // dA1 = 2*dA2
         Double_t tmp = y2 * y2 - 4. * dist;
         if (tmp > 0 && dist != 0) {
            dt = -(y2 + 2. * y1 +
                   TMath::Sqrt(tmp)) / dist / 2.;
            i = kTRUE;
         }
         /*else
            Warning("IdentZA","%s : cannot calculate interpolated mass, Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
               GetName(), Z, Aint, fICode);*/
      }
      if (i) {
         dist = dt * y2;
         if (TMath::Abs(dist) < 0.001) {
            if (y2 != 0)
               deltaA = yy * ix2 / y2 / 2.;
            /*else
               Warning("IdentZA","%s : cannot calculate interpolated mass (y2=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
                  GetName(), y2, Z, Aint, fICode);*/
         }
         else {
            if (dist > -1. && dt * yy > -1.)
               deltaA = ix2 / 2. / TMath::Log(1. + dist) * TMath::Log(1. + dt * yy);
            /*else
               Warning("IdentZA","%s : cannot calculate interpolated mass (dist=%f dt*yy=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
                  GetName(), dist, dt*yy, Z, Aint, fICode);*/
         }
         A += deltaA;
      }
   }
   /***************D'autres masses sont-elles possibles ?*************************/
   if (fICode == kICODE0 && (ibif > 0 && ibif < 4)) {
      if (ibif != 2) {        /***Masse superieure***/
         //We look at next line in the complete list of lines, after the closest line.
         //If it has the same Z as the closest line, but was excluded from research for closest line
         //because the point lies outside the endpoints, there remains a doubt about the mass:
         //on rajoute 1 a fICode, effectivement on le met = kICODE1
         Int_t idx = fIdxClosest; // index of closest line
         if (idx > -1 && ++idx < GetNumberOfIdentifiers()) {
            KVIDentifier* nextline = GetIdentifierAt(idx);
            if (nextline->GetZ() == Z
                  && !((KVIDLine*)nextline)->IsBetweenEndPoints(x, y, "x")) {
               fICode++;        // Z ok, mais les masses superieures a A sont possibles
               //cout <<"//on rajoute 1 a fICode, effectivement on le met = kICODE1" << endl;
            }
         }
      }
      if (ibif != 1) {   /***Masse inferieure***/
         //We look at line below the closest line in the complete list of lines.
         //If it has the same Z as the closest line, but was excluded from research for closest line
         //because the point lies outside the endpoints, there remains a doubt about the mass:
         //on rajoute 2 a fICode, so it can be = kICODE2 or kICODE3
         Int_t idx = fIdxClosest;
         if (idx > -1 && --idx >= 0) {
            KVIDentifier* nextline = GetIdentifierAt(idx);
            if (nextline->GetZ() == Z
                  && !((KVIDLine*)nextline)->IsBetweenEndPoints(x, y, "x")) {
               fICode += 2;
               //cout << "//on rajoute 2 a fICode, so it can be = kICODE2 or kICODE3" << endl;
            }
         }
      }
   }

   //cout << "Z = " << Z << " A = " << A << " icode = " << fICode << endl;
}

//_________________________________________________________________________//

void KVIDZAGrid::IdentZ(Double_t x, Double_t y, Double_t& Z)
{
   // Finds Z & 'real Z' for point (x,y) once closest lines to point have been found (see GetNearestIDLine).
   // This is is based on the algorithm developed by L. Tassan-Got in IdnCsOr, even the same
   // variable names and comments have been used (as much as possible).

   fICode = kICODE0;
   Z = -1;
   Aint = 0;
   Zint = 0;
   /*   cout << "kinfi = " << kinfi << " Zinfi = " << Zinfi << "  Ainfi = " << Ainfi << "  winfi = " << winfi << "  dinfi = " << dinfi << endl;
      cout << "kinf = " << kinf << " Zinf = " << Zinf << "  Ainf = " << Ainf << "  winf = " << winf << "  dinf = " << dinf << endl;
      cout << "ksup = " << ksup << " Zsup = " << Zsup << "  Asup = " << Asup << "  wsup = " << wsup << "  dsup = " << dsup << endl;
      cout << "ksups = " << ksups << " Zsups = " << Zsups << "  Asups = " << Asups << "  wsups = " << wsups << "  dsups = " << dsups << endl;
    */
   Int_t ibif = 0;
   Int_t k = -1;
   Double_t yy, y1, y2;
   Int_t ix1, ix2;
   yy = y1 = y2 = 0;
   ix1 = ix2 = 0;

   if (ksup > -1) {         // there is a line above the point
      if (kinf > -1) {              // there is a line below the point

         //printf("------------>/*  We found a line above and a line below the point */\n");

         Double_t dt = dinf + dsup;     //distance between the 2 lines
         Int_t dZ = Zsup - Zinf;
         Double_t dist = dt / (1.0 * dZ);  //distance between the 2 lines normalised to difference in Z of lines

         /*** Z = Zsup ***/
         if (dinf > dsup) {  //point is closest to upper line, 'sup' line
            ibif = 1;
            k = ksup;
            yy = -dsup;
            Z = Zsup;
            Zint = Zsup;
            Aint = Asup;
            if (ksups > -1) {        // there is a 'sups' line above the 2 which encadrent le point
               y2 = dsups - dsup;

               ibif = 0;
               y2 /= 2.;
               ix2 = Zsups - Zsup;
            }
            else {         // ksups == -1 i.e. no 'sups' line
               y2 = wsup;
               y2 = 0.5 * TMath::Max(y2, dist);
               ix2 = 1;
            }
            y1 = -dt / 2.;
            ix1 = -dZ;
         }
         /*** Z = Zinf ***/
         else {              //point is closest to lower line, 'inf' line
            ibif = 2;
            k = kinf;
            yy = dinf;
            Z = Zinf;
            Zint = Zinf;
            Aint = Ainf;
            if (kinfi > -1) {        // there is a 'infi' line below the 2 which encadrent le point
               y1 = 0.5 * (dinfi - dinf);

               ibif = 0;
               ix1 = Zinfi - Zinf;
               y1 = -y1;

            }
            else {         // kinfi = -1 i.e. no 'infi' line
               y1 = winf;
               y1 = -0.5 * TMath::Max(y1, dist);
               ix1 = -1;
            }
            y2 = dt / 2.;
            ix2 = dZ;
         }

      }//if(kinf>-1)...*******************************************************
      else {
         //printf("------------>/*  Only a line above the point was found, no line below */\n");
         /* This means the point is below the first Z line of the grid (?) */
         ibif = 3;
         k = ksup;
         yy = -dsup;
         Z = Zsup;
         Zint = Zsup;
         Aint = Asup;
         y1 = 0.5 * wsup;
         if (ksups > -1) {      // there is a 'sups' line above the closest line to the point
            y2 = dsups - dsup;

            ibif = 2;
            ix2 = Zsups - Zsup;
            Double_t x1 = y2 / ix2 / 2.;
            y1 = -TMath::Max(y1, x1);
            ix1 = -1;
            y2 /= 2.;

         }
         else {               // no 'sups' line above closest line
            y2 = y1;
            ix2 = 1;
            y1 = -y1;
            ix1 = -1;
         }
         if (yy >= y1)
            fICode = kICODE0; // we are within the 'natural width' of the last line
         else {
            fICode = kICODE6; // we are too far from first line to extrapolate correctly
            Z = Zsup - 1; // give Z below first line of grid, but this is an upper limit
            Zint = Zsup - 1;
            Aint = 0;
         }
      }
   }  //if(ksup>-1)***************************************************************
   else if (kinf > -1) {

      //printf("------------>/*  Only a line below the point was found, no line above */\n");
      /* This means the point is above the last Z line of the grid (?) */
      ibif = 3;
      k = kinf;
      Z = Zinf;
      Zint = Zinf;
      Aint = Ainf;
      yy = dinf;
      y2 = 0.5 * winf;
      if (kinfi > -1) { // there is a 'infi' line below the closest line to the point
         y1 = dinfi - dinf;
         ibif = 1;
         ix1 = Zinfi - Zinf;
         Double_t x2 = -y1 / ix1 / 2.;
         y2 = TMath::Max(y2, x2);
         ix2 = 1;
         y1 /= -2.;
      }
      else {
         y1 = -y2;
         ix1 = -1;
         ix2 = 1;
      }
      if (yy <= y2)
         fICode = kICODE0; // we are within the 'natural width' of the last line
      else {
         fICode = kICODE7; // we are too far from last line to extrapolate correctly
         Z = Zinf + 1; // give Z above last line in grid, it is a lower limit
         Zint = Zinf + 1;
         Aint = 0;//calculate mass from Z
      }

   }
   /*no lines found at all*/
   else {
      fICode = kICODE8;         // Z indetermine ou (x,y) hors limites
   }


   /****************Test des bornes********************************************/
   if (k > -1 && fICode == kICODE0) {
      if (yy > y2)
         fICode = kICODE4;      // Z ok, masse hors limite superieure ou egale a A
   }
   if (k > -1 && fICode == kICODE0) {
      if (yy < y1)
         fICode = kICODE5;      // Z ok, masse hors limite inferieure ou egale a A
   }
   if (fICode == kICODE4 || fICode == kICODE5) Aint = 0;

   /****************Interpolation to find 'real Z': dz = f*log(1+b*dy)********************/

   if (fICode < kICODE6) {
      Double_t deltaZ = 0.;
      Bool_t i = kFALSE;
      Double_t dt, dist = y1 * y2;
      dt = 0.;
      if (ix2 == -ix1) {        //dZ1 = dZ2
         if (dist != 0) {
            dt = -(y1 + y2) / dist;
            i = kTRUE;
         }
         /*else
            Warning("IdentZ","%s : cannot calculate interpolated charge, Zreal will equal Zint (Zint=%d fICode=%d)",
               GetName(), Zint, fICode);*/
      }
      else if (ix2 == -ix1 * 2) {     // dZ2 = 2*dZ1
         Double_t tmp = y1 * y1 - 4. * dist;
         if (tmp > 0. && dist != 0) {
            dt = -(y1 + 2. * y2 -
                   TMath::Sqrt(tmp)) / dist / 2.;
            i = kTRUE;
         }
         /*else
            Warning("IdentZ","%s : cannot calculate interpolated charge, Zreal will equal Zint (Zint=%d fICode=%d)",
               GetName(), Zint, fICode);*/
      }
      else if (ix1 == -ix2 * 2) {     // dZ1 = 2*dZ2
         Double_t tmp = y2 * y2 - 4. * dist;
         if (tmp > 0. && dist != 0) {
            dt = -(y2 + 2. * y1 +
                   TMath::Sqrt(tmp)) / dist / 2.;
            i = kTRUE;
         }
         /*else
            Warning("IdentZ","%s : cannot calculate interpolated charge, Zreal will equal Zint (Zint=%d fICode=%d)",
               GetName(), Zint, fICode);*/
      }
      if (i) {
         dist = dt * y2;
         if (TMath::Abs(dist) < 0.001) {
            if (y2 != 0)
               deltaZ = yy * ix2 / y2 / 2.;
            /*else
               Warning("IdentZ","%s : cannot calculate interpolated charge (y2=%f), Zreal will equal Zint (Zint=%d fICode=%d)",
                  GetName(), y2, Zint, fICode);*/
         }
         else {
            if (dist > -1. && dt * yy > -1.)
               deltaZ = ix2 / 2. / TMath::Log(1. + dist) * TMath::Log(1. + dt * yy);
            /*else
               Warning("IdentZ","%s : cannot calculate interpolated charge (dist=%f dt*yy=%f), Zreal will equal Zint (Zint=%d fICode=%d)",
                  GetName(), dist, dt*yy, Zint, fICode);*/
         }
         Z += deltaZ;
      }
   }
   /***************Is there still a doubt about the Z ?*************************/
   if (fICode == kICODE0 && (ibif > 0 && ibif < 4)) {
      /***z superieure***/
      if (ibif != 2) {
         //We look at next line in the complete list of lines, after the closest line.
         //If it was excluded from research for closest line
         //because the point lies outside the endpoints, there remains a doubt about the Z:
         //on rajoute 1 a fICode, effectivement on le met = kICODE1
         Int_t idx = fIdxClosest;
         if (idx > -1 && ++idx < GetNumberOfIdentifiers()) {
            KVIDLine* nextline = (KVIDLine*)GetIdentifierAt(idx);
            if (!nextline->IsBetweenEndPoints(x, y, "x")) {
               fICode++;        // Z might be bigger than we think
               //cout <<"//on rajoute 1 a fICode, effectivement on le met = kICODE1" << endl;
            }
         }
      }
      /***z inferieure***/
      if (ibif != 1) {
         //We look at line below the closest line in the complete list of lines.
         //If it was excluded from research for closest line
         //because the point lies outside the endpoints, there remains a doubt about the Z:
         //on rajoute 2 a fICode, so it can be = kICODE2 or kICODE3
         Int_t idx = fIdxClosest;
         if (idx > -1 && --idx >= 0) {
            KVIDLine* nextline = (KVIDLine*) GetIdentifierAt(idx);
            if (!nextline->IsBetweenEndPoints(x, y, "x")) {
               fICode += 2;
               //cout << "//on rajoute 2 a fICode, so it can be = kICODE2 or kICODE3" << endl;
            }
         }
      }
   }

}

//_______________________________________________________________________________________________//

void KVIDZAGrid::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // Fill the KVIdentificationResult object with the results of identification for point (x,y)
   // corresponding to some physically measured quantities related to a reconstructed nucleus.
   //
   // By default (OnlyZId()=kFALSE) this means identifying the Z & A of the nucleus.
   // In this case, we consider that the nucleus' Z & A have been correctly measured
   // if the 'quality code' returned by IdentZA() is < kICODE4:
   //   we set idr->Zident and idr->Aident to kTRUE if fICode<kICODE4
   //
   // If OnlyZId()=kTRUE, only the Z of the nucleus is established.
   // In this case, we consider that the nucleus' Z has been correctly measured
   // if the 'quality code' returned by IdentZ() is < kICODE4, thus:
   //   we set idr->Zident to kTRUE if fICode<kICODE4
   // The mass idr->A is set to the mass of the nearest line.
   //
   // Real & integer masses for isotopically identified particles
   // ===================================================
   // For points lying between two lines of same Z and different A (fICode<kIDCode4)
   // the "real" mass is given by interpolation between the two masses.
   // The integer mass is the A of the line closest to the point.
   // This means that the integer A is not always = nint("real" A),
   // as for example if a grid is drawn with lines for 7Be & 9Be but not 8Be
   // (usual case), then particles between the two lines can have "real" masses
   // between 7.5 and 8.5, but their integer A will be =7 or =9, never 8.
   //
   idr->IDOK = kFALSE;
   idr->Aident = idr->Zident = kFALSE;

   if (!const_cast<KVIDZAGrid*>(this)->FindFourEmbracingLines(x, y, "above")) {
      //no lines corresponding to point were found
      const_cast < KVIDZAGrid* >(this)->fICode = kICODE8;         // Z indetermine ou (x,y) hors limites
      idr->IDquality = kICODE8;
      idr->SetComment("no identification: (x,y) out of range covered by grid");
      return;
   }
   if (IsOnlyZId()) {
      Double_t Z;
      const_cast < KVIDZAGrid* >(this)->IdentZ(x, y, Z);
      idr->IDquality = fICode;
      if (fICode < kICODE4 || fICode == kICODE7) {
         idr->Zident = kTRUE;
      }
      if (fICode < kICODE4) {
         idr->IDOK = kTRUE;
      }
      idr->Z = Zint;
      idr->PID = Z;
      idr->A = Aint;

      switch (fICode) {
         case kICODE0:
            idr->SetComment("ok");
            break;
         case kICODE1:
            idr->SetComment("slight ambiguity of Z, which could be larger");
            break;
         case kICODE2:
            idr->SetComment("slight ambiguity of Z, which could be smaller");
            break;
         case kICODE3:
            idr->SetComment("slight ambiguity of Z, which could be larger or smaller");
            break;
         case kICODE4:
            idr->SetComment("point is in between two lines of different Z, too far from either to be considered well-identified");
            break;
         case kICODE5:
            idr->SetComment("point is in between two lines of different Z, too far from either to be considered well-identified");
            break;
         case kICODE6:
            idr->SetComment("(x,y) is below first line in grid");
            break;
         case kICODE7:
            idr->SetComment("(x,y) is above last line in grid");
            break;
         default:
            idr->SetComment("no identification: (x,y) out of range covered by grid");
      }
   }
   else {

      Int_t Z;
      Double_t A;
      const_cast < KVIDZAGrid* >(this)->IdentZA(x, y, Z, A);
      idr->IDquality = fICode;
      idr->Z = Z;
      idr->PID = A;

      if (fICode < kICODE4 || fICode == kICODE7) {
         idr->Zident = kTRUE;
      }
      idr->A = Aint;
      if (fICode < kICODE4) {
         idr->Aident = kTRUE;
         idr->IDOK = kTRUE;
      }
      switch (fICode) {
         case kICODE0:
            idr->SetComment("ok");
            break;
         case kICODE1:
            idr->SetComment("slight ambiguity of A, which could be larger");
            break;
         case kICODE2:
            idr->SetComment("slight ambiguity of A, which could be smaller");
            break;
         case kICODE3:
            idr->SetComment("slight ambiguity of A, which could be larger or smaller");
            break;
         case kICODE4:
            idr->SetComment("point is in between two isotopes of different Z, too far from either to be considered well-identified");
            break;
         case kICODE5:
            idr->SetComment("point is in between two isotopes of different Z, too far from either to be considered well-identified");
            break;
         case kICODE6:
            idr->SetComment("(x,y) is below first line in grid");
            break;
         case kICODE7:
            idr->SetComment("(x,y) is above last line in grid");
            break;
         default:
            idr->SetComment("no identification: (x,y) out of range covered by grid");
      }
   }
}

//___________________________________________________________________________________

void KVIDZAGrid::Initialize()
{
   // General initialisation method for identification grid.
   // This method MUST be called once before using the grid for identifications.
   // The ID lines are sorted.
   // The natural line widths of all ID lines are calculated.
   // The line with the largest Z (Zmax line) is found.

   KVIDGrid::Initialize();
   // Zmax should be Z of last line in sorted list
   fZMaxLine = (KVIDZALine*) GetIdentifiers()->Last();
   if (fZMaxLine) fZMax = fZMaxLine->GetZ();
   else           fZMax = 0;     // protection au cas ou il n y a aucune ligne de Z
}


//______________________________________________________________________________

void KVIDZAGrid::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDZAGrid.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 2) {
         R__v = R__b.ReadVersion(&R__s, &R__c);// read version of KVIDZGrid
         if (R__v != 1) {
            Warning("Streamer", "Reading KVIDZGrid with version=%d", R__v);
         }
         KVIDGrid::Streamer(R__b);
         R__b >> fZMax;
      }
      else {
         R__b.ReadClassBuffer(KVIDZAGrid::Class(), this, R__v, R__s, R__c);
      }
   }
   else {
      R__b.WriteClassBuffer(KVIDZAGrid::Class(), this);
   }
}

//______________________________________________________________________________

// void KVIDZAGrid::MakeEDeltaEZGrid(Int_t Zmin, Int_t Zmax, Int_t npoints, Double_t gamma)
// {
//     // Generate dE-Eres grid for associated ID telescope.
//     // 1 line per Z is generated, using mass formula set for grid.
//     // For each (Z,A) we calculate npoints corresponding to incident energies from the punch-through
//     // of the first member up to the punch-through of the second.
//     // gamma controls the spacing of the incident energies:
//     //    gamma = 1 : equidistant steps in Einc
//     //    gamma > 1 : steps at low Einc are more closely spaced, more & more for gamma >> 1
//
//     KVIDTelescope* tel = ((KVIDTelescope*)fTelescopes.At(0));
//     if (!tel)
//     {
//         Error("MakeEDeltaEZGrid",
//               "No identification telescope associated to this grid");
//         return;
//     }
//     KVDetector *dEDet = tel->GetDetector(1);
//     KVDetector *ErDet = tel->GetDetector(2);
//     if (!ErDet)
//     {
//         Error("MakeEDeltaEZGrid",
//               "This identification telescope only has one member !");
//         return;
//     }
//     Double_t x_scale = GetXScaleFactor();
//     Double_t y_scale = GetYScaleFactor();
//     //clear old lines from grid (and scaling parameters)
//     Clear();
//
//     //loop over Z
//     KVNucleus part;
//     //set mass formula used for calculating A from Z
//     part.SetMassFormula(GetMassFormula());
//
//     Info("MakeEDeltaEZGrid",
//          "Calculating grid: dE detector = %s, E detector = %s",
//          dEDet->GetName(), ErDet->GetName());
//
//     for (Int_t z=Zmin; z<=Zmax; z++)
//     {
//
//         part.SetZ(z);
//
//         //loop over energy
//         //first find :
//         //      E1 = dE punch through + 0.1 MeV
//         //      E2 = 95% of energy at which particle passes Eres
//         //then perform npoints calculations between these two energies and use these
//         //to construct a KVIDZLine
//
//         Double_t E1, E2, E1bis;
//          E1bis = dEDet->GetEIncOfMaxDeltaE(z, part.GetA());
//          E1 = dEDet->GetPunchThroughEnergy(z, part.GetA()) + 0.1;
//          if(E1 < E1bis) E1 = E1bis;
//          E2 = 0.95*dEDet->GetIncidentEnergyFromERes(z, part.GetA(), ErDet->GetPunchThroughEnergy(z, part.GetA()));
//          Info("MakeEDeltaEZGrid","Z= %d, E1=%lf E2=%lf",z,E1,E2);
//
//         // check we are within limits of validity of energy loss tables
//         if ( E2 > dEDet->GetEmaxValid(z, part.GetA()) )
//         {
//             Warning("MakeEDeltaEZGrid",
//                     "Emax=%f MeV for Z=%d : beyond validity of range tables. Will use max limit=%f MeV",
//                     E2, z, dEDet->GetEmaxValid(z,part.GetA()));
//             E2 = dEDet->GetEmaxValid(z,part.GetA());
//         }
//         if ( E2 > dEDet->GetEmaxValid(z, part.GetA()) )
//         {
//             Warning("MakeEDeltaEZGrid",
//                     "Emax=%f MeV for Z=%d : beyond validity of range tables. Will use max limit=%f MeV",
//                     E2, z, dEDet->GetEmaxValid(z,part.GetA()));
//             E2 = dEDet->GetEmaxValid(z,part.GetA());
//         }
//
//         KVIDentifier *line = NewLine("ID");
//         Add("ID", line);
//         line->SetZ(z);
//         line->SetMassFormula(part.GetMassFormula());
//
//         Double_t dE = (E2 - E1) / pow((npoints - 1.),gamma);
//
//         Int_t npoints_added = 0;
//
//         for (int i = 0; npoints_added < npoints; i++)
//         {
//
//             Double_t E = E1 + dE*pow(i,gamma);
//
//             Double_t Eres = 0.0;
//                 dEDet->Clear();
//                 ErDet->Clear();
//                 part.SetEnergy(E);
//                 dEDet->DetectParticle(&part);
//                 ErDet->DetectParticle(&part);
//                 Eres = ErDet->GetEnergy();
//
//             line->SetPoint(npoints_added, Eres, dEDet->GetEnergy());
//             npoints_added++;
//         }
//     }
//     //if this grid has scaling factors, we need to apply them to the result
//     if (x_scale != 1)
//         SetXScaleFactor(x_scale);
//     if (y_scale != 1)
//         SetYScaleFactor(y_scale);
// }

KVIDGraph* KVIDZAGrid::MakeSubsetGraph(TList* lines, TClass* graph_class)
{
   // Create a new graph/grid using the subset of lines of this grid contained in TList 'lines'.
   // By default the new graph/grid will be of the same class as this one, unless graph_class !=0,
   // in which case it must contain the address of a TClass object representing a class which
   // derives from KVIDGraph.
   // A clone of each line will be made and added to the new graph, which will have the same
   // name and be associated with the same ID telescopes as this one.

   if (!graph_class) graph_class = IsA();
   if (!graph_class->InheritsFrom("KVIDGraph")) {
      Error("MakeSubsetGraph", "Called with graph class %s, does not derive from KVIDGraph",
            graph_class->GetName());
      return 0;
   }
   KVIDGraph* new_graph = (KVIDGraph*)graph_class->New();
   new_graph->AddIDTelescopes(&fTelescopes);
   new_graph->SetOnlyZId(IsOnlyZId());
   new_graph->SetRuns(GetRuns());
   new_graph->SetVarX(GetVarX());
   new_graph->SetVarY(GetVarY());
   if (IsOnlyZId()) {
      new_graph->SetMassFormula(GetMassFormula());
   }
   // loop over lines in list, make clones and add to graph
   TIter next(lines);
   KVIDentifier* id;
   while ((id = (KVIDentifier*)next())) {
      KVIDentifier* idd = (KVIDentifier*)id->Clone();
      //id->Copy(*idd);
      //idd->ResetBit(kCanDelete);
      new_graph->AddIdentifier(idd);
   }
   return new_graph;
}

KVIDGraph* KVIDZAGrid::MakeSubsetGraph(Int_t Zmin, Int_t Zmax, const Char_t* graph_class)
{
   // Create a new graph/grid using the subset of lines of this grid with Zmin <= Z <= Zmax.
   // By default the new graph/grid will be of the same class as this one, unless graph_class !="",
   // in which case it must contain the name of a class which derives from KVIDGraph.
   // A clone of each line will be made and added to the new graph, which will have the same
   // name and be associated with the same ID telescopes as this one.

   TList* lines = new TList; // subset of lines to clone
   TIter next(fIdentifiers);
   KVIDentifier* l;
   while ((l = (KVIDentifier*)next())) {
      if (l->GetZ() >= Zmin && l->GetZ() <= Zmax) lines->Add(l);
   }
   TClass* cl = 0;
   if (strcmp(graph_class, "")) cl = TClass::GetClass(graph_class);
   KVIDGraph* gr = MakeSubsetGraph(lines, cl);
   lines->Clear();
   delete lines;
   return gr;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ClassImp(KVIDZGrid)

//////////////////////////////////////////////////
// This class is for backwards compatibility only
// and must not be used.
//////////////////////////////////////////////////

void KVIDZGrid::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDZGrid.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v != 1) {
         Warning("Streamer", "Reading KVIDZGrid with version=%d", R__v);
      }
      KVIDGrid::Streamer(R__b);
      R__b >> fZMax;
   }
}
