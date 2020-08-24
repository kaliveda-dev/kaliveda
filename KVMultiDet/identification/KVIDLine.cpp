/***************************************************************************
$Id: KVIDLine.cpp,v 1.22 2009/05/05 15:57:52 franklan Exp $
                          KVIDLine.cpp  -  description
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

#include "KVIDLine.h"
#include "Riostream.h"
#include "TF1.h"
#include "TProfile.h"
#include "TH2.h"
#include "TCutG.h"
#include "TPad.h"
#include "TList.h"
#include "TROOT.h"
#include "TVirtualX.h"

using namespace std;

ClassImp(KVIDLine)
/////////////////////////////////////////////////////////////////////////////////////////
//KVIDLine
//
//Base class for lines/cuts used for particle identification in 2D data maps.
//This provides all methods for calculating the shortest distance between a point (x,y)
//in the map and the line, or for finding the relative orientation of the point and
//the line (above/below, left/right). These methods are modified versions of methods
//existing in the TGraph base class.
//
//Note: by default, a KVIDLine cannot be graphically modified using the mouse.
//This is to avoid accidents! This can of course be changed either with the context menu
//of the line, or when using the KVIDGridManagerGUI in Edit mode.
//
//Double_t KVIDLine::DistanceToLine(Double_t px, Double_t py, Int_t &i_near)
//
//Compute the closest distance of approach from point px,py to this line.
//The units of px, py are the same as the coordinates of the graph
//WARNING: can be NEGATIVE - see below.
//
//The distance from the point to each segment is calculated in turn.
//
//If the point lies between the endpoints of at least one segment, the returned
//distance will be the closest of these segments. In this case i_near gives the
//index of the first point of the closest segment.
//
//If not then the distance is the smallest distance between the endpoint
//of a segment and the point. In this case i_near gives the index of the
//closest endpoint.
//If the closest segment-endpoint is not one of the endpoints of the whole line,
//the returned distance is positive.
//On the other hand, if the closest part of the line to the point is one of the
//two endpoints of the line, the returned distance is NEGATIVE.
//
//Double_t KVIDLine::DistanceToLine(Double_t px, Double_t py, Double_t xp1, Double_t yp1, Double_t xp2, Double_t yp2, Int_t &i_nearest_point)
//
//Given a line segment defined by endpoints (xp1,yp1) and (xp2,yp2) find the
//  shortest distance between point (px,py) and the line.
//
//                                                   M
//      P1 (xp1,yp1) x---------------------------------------x P2 (xp2,yp2)
//                                                      |
//                                                      |
//                                                      |
//                                                      |
//                                                      |
//                                                      x
//                                                                P (px,py)
//This is simply the magnitude of the component of vector P1->P  (or P2->P)
//perpendicular to P1->P2.
//
//If the point is indeed between the two endpoints as shown, then
//P1M + P2M = P1P2
//If not, P1M + P2M > P1P2. In this case the closest distance is that to the nearer
//of the two endpoints, but the value returned is negative; i_nearest_point gives the index (0 or 1)
//of the nearer endpoint
//
//Bool_t KVIDLine::PosRelToLine(Option_t* opt, Double_t px, Double_t py, Double_t xp1, Double_t yp1, Double_t xp2, Double_t yp2 )
//
//Given a line segment defined by endpoints (xp1,yp1) and (xp2,yp2) test the
//  direction of (px,py) with respect to the line in the (x,y) plane.
//
//                                                        M
//      P1 (xp1,yp1) x---------------------------------------x P2 (xp2,yp2)
//                                                         |
//                                                         |
//                                                         |
//                                                         |
//                                                         |
//                                                         x
//                                                                P (px,py)
//This is given by the angle phi between vector MP (component of vector P1->P  (or P2->P)
//perpendicular to P1->P2) and the +ve x-axis.
//Point is left of line if 90<phi<270
//Point is right of line if 270<phi<90
//Point is above line if 0<phi<180
//Point is below line if 180<phi<360
//The option string is "left", "right", "above" or "below", and the result is either kTRUE or kFALSE
//depending on the relative position of the point and the line.
//E.g. in the diagram shown above, PosRelToLine("below", ...) would give kTRUE - the point is
//below the line.
//
//Bool_t KVIDLine::WhereAmI(Double_t px, Double_t py, Option_t *opt)
//
//The relative position of point (px,py) with respect to the line is tested.
//The option string can be "left", "right", "above" or "below".
//WhereAmI( px, py, "above") returns kTRUE if the point is above the line etc. etc.
//
//First of all, the closest segment/point of the line to the point is found.
//Then the relative position of the point and this segment/point is tested.
//
//void KVIDLine::GetStartPoint(Double_t& x, Double_t& y) const
//
//Coordinates of first point in line
//
//void KVIDLine::GetEndPoint(Double_t& x, Double_t& y) const
//
//Coordinates of last point in line
//
//Bool_t KVIDLine::IsBetweenEndPoints(Double_t x, Double_t y, const Char_t* axis) const
//
//Returns kTRUE for point (x,y) if :
// axis = "" (default) and both x and y lie inside the endpoints of the line x1 < x < x2, y1 < y < y2
// axis = "x" and x lies inside the endpoints x1 < x < x2
// axis = "y" and y lies inside the endpoints y1 < y < y2
/////////////////////////////////////////////////////////////////////////////////////////


KVIDLine::KVIDLine()
{
   //Default ctor
}

KVIDLine::~KVIDLine()
{
   //Default dtor
}

KVIDLine::KVIDLine(const KVIDLine& obj)
   : KVIDentifier(obj)
{
   // copy constructor
}

KVIDLine::KVIDLine(const TGraph& gr): KVIDentifier(gr)
{
   //initialize KVIDLine using TGraph copy ctor
}

//_____________________________________________________________________________________________

KVIDLine* KVIDLine::MakeIDLine(TObject* obj, Double_t xdeb, Double_t xfin, Double_t np, Bool_t save)
{
   // create an IDLine from different objects (Inherited from TProfile, TGraph, TF1)
   // - "xdeb" and "xfin" stand for the delimitation of the line - dafault values are -1
   // in this case range is the one of the considered object
   // if the choosen range is greater than the object one, one point for each limit is added with the value
   // corresponding to the first and/or last of the object
   // - "np" is in the TF1 case the number point (default value 20), in the TProfile case is the minimum threshold
   // for the bin entries (default value is one); for the TGraph case it has no effect;
   // In the TProfile and TGraph cases the number of points are determined by the object considering the interval
   // - "save" allow to save the TF1 or TProfile object which can be recall with the GetListofFunction()  method
   Double_t xdeb_bis = xdeb, xfin_bis = xfin, np_bis = np;
   if (obj) {
      KVIDLine* line = new KVIDLine();
      if (obj->InheritsFrom("TF1")) {
         Double_t xmin, xmax;
         dynamic_cast<TF1*>(obj)->GetRange(xmin, xmax);
         if (xdeb_bis == -1) xdeb_bis = xmin;
         if (xfin_bis == -1) xfin_bis = xmax;
         if (np_bis == 1) np_bis = Double_t(dynamic_cast<TF1*>(obj)->GetNpx());
         Double_t inter = (xfin_bis - xdeb_bis) / (np_bis);
         Int_t pp = 0;
         for (Double_t xx = xdeb_bis; xx <= xfin_bis; xx += inter) {
            line->SetPoint(pp++, xx, dynamic_cast<TF1*>(obj)->Eval(xx));
         }
         if (save) line->Fit(dynamic_cast<TF1*>(obj), "0+", "", xdeb_bis, xfin_bis);
      }
      else if (obj->InheritsFrom("TGraph")) {   // np has no effect in this case
         Int_t nx = dynamic_cast<TGraph*>(obj)->GetN(), np2 = 0;
         Double_t* xtab = dynamic_cast<TGraph*>(obj)->GetX();
         Double_t* ytab = dynamic_cast<TGraph*>(obj)->GetY();
         if (xdeb_bis == -1) xdeb_bis = xtab[0];
         if (xfin_bis == -1) xfin_bis = xtab[nx - 1];
         if (xdeb_bis < xtab[0]) line->SetPoint(np2++, xdeb_bis, ytab[0]);
         for (Int_t pp = 0; pp < nx; pp += 1) if (xdeb_bis <= xtab[pp] && xtab[pp] <= xfin_bis) line->SetPoint(np2++, xtab[pp], ytab[pp]);
         if (xfin_bis > xtab[nx - 1]) line->SetPoint(np2, xfin_bis, ytab[nx - 1]);
      }
      else if (obj->InheritsFrom("TProfile")) {
         TProfile* pf = dynamic_cast<TProfile*>(obj);
         Int_t nx = pf->GetNbinsX(), np2 = 0;
         Int_t pp = 1;
         while (pf->GetBinEntries(pp) < np && pp < nx) pp += 1;
         Int_t xmin = pp;
         pp = nx;
         while (pf->GetBinEntries(pp) < np && pp > xmin) pp -= 1;
         Int_t xmax = pp;
         if (xdeb_bis == -1) xdeb_bis = pf->GetBinCenter(xmin);
         if (xfin_bis == -1) xfin_bis = pf->GetBinCenter(xmax);
         if (xdeb_bis < pf->GetBinCenter(xmin)) line->SetPoint(np2++, xdeb_bis, pf->GetBinContent(xmin));
         for (pp = xmin; pp <= xmax; pp += 1) {
            Double_t xx = pf->GetBinCenter(pp);
            if (xdeb_bis <= xx && xx <= xfin_bis && pf->GetBinEntries(pp) >= np) line->SetPoint(np2++, xx, pf->GetBinContent(pp));
         }
         if (xfin_bis > pf->GetBinCenter(xmax)) line->SetPoint(np2, xfin_bis, pf->GetBinContent(xmax));
         if (save) line->GetListOfFunctions()->Add(pf);
      }
      else cout << "le type ne correspond pas " << endl;
      line->SetName(Form("from_%s", obj->GetName()));
      return line;
   }
   else return NULL;
}

//_____________________________________________________________________________________________

/*
KVIDLine *KVIDLine::MakeIDLine(TH2 *hh,TCutG *cut,Double_t xdeb,Double_t xfin,Double_t np,Bool_t save)
{
   if (hh){
      TProfile *gg = NULL;
      if (cut){
         if (cut->InheritsFrom("TCutG")){
            Double_t xmin=1e6,xmax=-1e6;
            Double_t ymin=1e6,ymax=-1e6;
            for (Int_t pp=0;pp<cut->GetN();pp+=1){
               Double_t xx,yy; cut->GetPoint(pp,xx,yy);
               if (xx<xmin) xmin=xx; if (xx>xmax) xmax=xx;
               if (yy<ymin) ymin=yy; if (yy>ymax) ymax=yy;
            }

            Int_t bxmin = hh->GetXaxis()->FindBin(xmin); xmin = hh->GetXaxis()->GetBinLowEdge(bxmin);
            Int_t bxmax = hh->GetXaxis()->FindBin(xmax); xmax = hh->GetXaxis()->GetBinUpEdge(bxmax);
            Int_t bymin = hh->GetYaxis()->FindBin(ymin); ymin = hh->GetYaxis()->GetBinLowEdge(bymin);
            Int_t bymax = hh->GetYaxis()->FindBin(ymax); ymax = hh->GetYaxis()->GetBinUpEdge(bymax);

            Int_t dx = bxmax - bxmin +1;
            gg = new TProfile(Form("%s_%s",hh->GetName(),cut->GetName()),"prof",dx,xmin,xmax,ymin,ymax);
            for (Int_t xx=bxmin;xx<=bxmax;xx+=1)
               for (Int_t yy=bymin;yy<=bymax;yy+=1)
                  if ( cut->IsInside(hh->GetXaxis()->GetBinCenter(xx),hh->GetYaxis()->GetBinCenter(yy)) )
                     gg->Fill(hh->GetXaxis()->GetBinCenter(xx),hh->GetYaxis()->GetBinCenter(yy),hh->GetBinContent(xx,yy));
         }
      }
      else { gg = hh->ProfileX(); }
      return KVIDLine::MakeIDLine(gg,xdeb,xfin,np,save);
   }
   else return NULL;
}
*/

//_____________________________________________________________________________________________

void KVIDLine::WaitForPrimitive()
{
   // Method used to draw a new identifier in the active pad
   // Override in child classes so that gPad->WaitPrimitive has correct arguments

   if (!gPad) return;
   TGraph* gr = (TGraph*) gPad->WaitPrimitive("Graph", "PolyLine");
   //copy coordinates of user's line
   CopyGraph(gr);
   //remove TGraph and draw the KVIDLine in its place
   gPad->GetListOfPrimitives()->Remove(gr);
   delete gr;
   Draw("PL");
}

//______________________________________________________________________________
void KVIDLine::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
   // We override the TGraph::ExecuteEvent which contains a TCutG-specific part
   // making sure that the first and last point are the same. This is a copy
   // of that method with the "if(InheritsFrom("TCutG"))" part commented out.
   //
   // Execute action corresponding to one event.
   //
   //  This member function is called when a graph is clicked with the locator
   //
   //  If Left button clicked on one of the line end points, this point
   //     follows the cursor until button is released.
   //
   //  if Middle button clicked, the line is moved parallel to itself
   //     until the button is released.

   Int_t i, d;
   Double_t xmin, xmax, ymin, ymax, dx, dy, dxr, dyr;
   const Int_t kMaxDiff = 10;
   static Bool_t middle, badcase;
   static Int_t ipoint, pxp, pyp;
   static Int_t px1, px2, py1, py2;
   static Int_t pxold, pyold, px1old, py1old, px2old, py2old;
   static Int_t dpx, dpy;
   static Int_t* x = 0, *y = 0;

   if (!IsEditable()) {
      gPad->SetCursor(kHand);
      return;
   }
   if (!gPad->IsEditable()) return;

   switch (event) {

      case kButton1Down:
         badcase = kFALSE;
         gVirtualX->SetLineColor(-1);
         TAttLine::Modify();  //Change line attributes only if necessary
         px1 = gPad->XtoAbsPixel(gPad->GetX1());
         py1 = gPad->YtoAbsPixel(gPad->GetY1());
         px2 = gPad->XtoAbsPixel(gPad->GetX2());
         py2 = gPad->YtoAbsPixel(gPad->GetY2());
         ipoint = -1;


         if (x || y) break;
         x = new Int_t[fNpoints + 1];
         y = new Int_t[fNpoints + 1];
         for (i = 0; i < fNpoints; i++) {
            pxp = gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
            pyp = gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
            if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                  pyp < -kMaxPixel || pyp >= kMaxPixel) {
               badcase = kTRUE;
               continue;
            }
            gVirtualX->DrawLine(pxp - 4, pyp - 4, pxp + 4,  pyp - 4);
            gVirtualX->DrawLine(pxp + 4, pyp - 4, pxp + 4,  pyp + 4);
            gVirtualX->DrawLine(pxp + 4, pyp + 4, pxp - 4,  pyp + 4);
            gVirtualX->DrawLine(pxp - 4, pyp + 4, pxp - 4,  pyp - 4);
            x[i] = pxp;
            y[i] = pyp;
            d   = TMath::Abs(pxp - px) + TMath::Abs(pyp - py);
            if (d < kMaxDiff) ipoint = i;
         }
         dpx = 0;
         dpy = 0;
         pxold = px;
         pyold = py;
         if (ipoint < 0) return;
         if (ipoint == 0) {
            px1old = 0;
            py1old = 0;
            px2old = gPad->XtoAbsPixel(fX[1]);
            py2old = gPad->YtoAbsPixel(fY[1]);
         }
         else if (ipoint == fNpoints - 1) {
            px1old = gPad->XtoAbsPixel(gPad->XtoPad(fX[fNpoints - 2]));
            py1old = gPad->YtoAbsPixel(gPad->YtoPad(fY[fNpoints - 2]));
            px2old = 0;
            py2old = 0;
         }
         else {
            px1old = gPad->XtoAbsPixel(gPad->XtoPad(fX[ipoint - 1]));
            py1old = gPad->YtoAbsPixel(gPad->YtoPad(fY[ipoint - 1]));
            px2old = gPad->XtoAbsPixel(gPad->XtoPad(fX[ipoint + 1]));
            py2old = gPad->YtoAbsPixel(gPad->YtoPad(fY[ipoint + 1]));
         }
         pxold = gPad->XtoAbsPixel(gPad->XtoPad(fX[ipoint]));
         pyold = gPad->YtoAbsPixel(gPad->YtoPad(fY[ipoint]));

         break;


      case kMouseMotion:

         middle = kTRUE;
         for (i = 0; i < fNpoints; i++) {
            pxp = gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
            pyp = gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
            d   = TMath::Abs(pxp - px) + TMath::Abs(pyp - py);
            if (d < kMaxDiff) middle = kFALSE;
         }


         // check if point is close to an axis
         if (middle) gPad->SetCursor(kMove);
         else gPad->SetCursor(kHand);
         break;

      case kButton1Motion:
         if (middle) {
            for (i = 0; i < fNpoints - 1; i++) {
               gVirtualX->DrawLine(x[i] + dpx, y[i] + dpy, x[i + 1] + dpx, y[i + 1] + dpy);
               pxp = x[i] + dpx;
               pyp = y[i] + dpy;
               if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                     pyp < -kMaxPixel || pyp >= kMaxPixel) continue;
               gVirtualX->DrawLine(pxp - 4, pyp - 4, pxp + 4,  pyp - 4);
               gVirtualX->DrawLine(pxp + 4, pyp - 4, pxp + 4,  pyp + 4);
               gVirtualX->DrawLine(pxp + 4, pyp + 4, pxp - 4,  pyp + 4);
               gVirtualX->DrawLine(pxp - 4, pyp + 4, pxp - 4,  pyp - 4);
            }
            pxp = x[fNpoints - 1] + dpx;
            pyp = y[fNpoints - 1] + dpy;
            gVirtualX->DrawLine(pxp - 4, pyp - 4, pxp + 4,  pyp - 4);
            gVirtualX->DrawLine(pxp + 4, pyp - 4, pxp + 4,  pyp + 4);
            gVirtualX->DrawLine(pxp + 4, pyp + 4, pxp - 4,  pyp + 4);
            gVirtualX->DrawLine(pxp - 4, pyp + 4, pxp - 4,  pyp - 4);
            dpx += px - pxold;
            dpy += py - pyold;
            pxold = px;
            pyold = py;
            for (i = 0; i < fNpoints - 1; i++) {
               gVirtualX->DrawLine(x[i] + dpx, y[i] + dpy, x[i + 1] + dpx, y[i + 1] + dpy);
               pxp = x[i] + dpx;
               pyp = y[i] + dpy;
               if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                     pyp < -kMaxPixel || pyp >= kMaxPixel) continue;
               gVirtualX->DrawLine(pxp - 4, pyp - 4, pxp + 4,  pyp - 4);
               gVirtualX->DrawLine(pxp + 4, pyp - 4, pxp + 4,  pyp + 4);
               gVirtualX->DrawLine(pxp + 4, pyp + 4, pxp - 4,  pyp + 4);
               gVirtualX->DrawLine(pxp - 4, pyp + 4, pxp - 4,  pyp - 4);
            }
            pxp = x[fNpoints - 1] + dpx;
            pyp = y[fNpoints - 1] + dpy;
            gVirtualX->DrawLine(pxp - 4, pyp - 4, pxp + 4,  pyp - 4);
            gVirtualX->DrawLine(pxp + 4, pyp - 4, pxp + 4,  pyp + 4);
            gVirtualX->DrawLine(pxp + 4, pyp + 4, pxp - 4,  pyp + 4);
            gVirtualX->DrawLine(pxp - 4, pyp + 4, pxp - 4,  pyp - 4);
         }
         else {
            if (px1old) gVirtualX->DrawLine(px1old, py1old, pxold,  pyold);
            if (px2old) gVirtualX->DrawLine(pxold,  pyold,  px2old, py2old);
            gVirtualX->DrawLine(pxold - 4, pyold - 4, pxold + 4,  pyold - 4);
            gVirtualX->DrawLine(pxold + 4, pyold - 4, pxold + 4,  pyold + 4);
            gVirtualX->DrawLine(pxold + 4, pyold + 4, pxold - 4,  pyold + 4);
            gVirtualX->DrawLine(pxold - 4, pyold + 4, pxold - 4,  pyold - 4);
            pxold = px;
            pxold = TMath::Max(pxold, px1);
            pxold = TMath::Min(pxold, px2);
            pyold = py;
            pyold = TMath::Max(pyold, py2);
            pyold = TMath::Min(pyold, py1);
            if (px1old) gVirtualX->DrawLine(px1old, py1old, pxold,  pyold);
            if (px2old) gVirtualX->DrawLine(pxold,  pyold,  px2old, py2old);
            gVirtualX->DrawLine(pxold - 4, pyold - 4, pxold + 4,  pyold - 4);
            gVirtualX->DrawLine(pxold + 4, pyold - 4, pxold + 4,  pyold + 4);
            gVirtualX->DrawLine(pxold + 4, pyold + 4, pxold - 4,  pyold + 4);
            gVirtualX->DrawLine(pxold - 4, pyold + 4, pxold - 4,  pyold - 4);
         }
         break;

      case kButton1Up:

         if (gROOT->IsEscaped()) {
            gROOT->SetEscape(kFALSE);
            delete [] x;
            x = 0;
            delete [] y;
            y = 0;
            break;
         }

         // Compute x,y range
         xmin = gPad->GetUxmin();
         xmax = gPad->GetUxmax();
         ymin = gPad->GetUymin();
         ymax = gPad->GetUymax();
         dx   = xmax - xmin;
         dy   = ymax - ymin;
         dxr  = dx / (1 - gPad->GetLeftMargin() - gPad->GetRightMargin());
         dyr  = dy / (1 - gPad->GetBottomMargin() - gPad->GetTopMargin());

         if (fHistogram) {
            // Range() could change the size of the pad pixmap and therefore should
            // be called before the other paint routines
            gPad->Range(xmin - dxr * gPad->GetLeftMargin(),
                        ymin - dyr * gPad->GetBottomMargin(),
                        xmax + dxr * gPad->GetRightMargin(),
                        ymax + dyr * gPad->GetTopMargin());
            gPad->RangeAxis(xmin, ymin, xmax, ymax);
         }
         if (middle) {
            for (i = 0; i < fNpoints; i++) {
               if (badcase) continue;  //do not update if big zoom and points moved
               if (x) fX[i] = gPad->PadtoX(gPad->AbsPixeltoX(x[i] + dpx));
               if (y) fY[i] = gPad->PadtoY(gPad->AbsPixeltoY(y[i] + dpy));
            }
         }
         else {
            fX[ipoint] = gPad->PadtoX(gPad->AbsPixeltoX(pxold));
            fY[ipoint] = gPad->PadtoY(gPad->AbsPixeltoY(pyold));
//          if (InheritsFrom("TCutG")) {
//             //make sure first and last point are the same
//             if (ipoint == 0) {
//                fX[fNpoints-1] = fX[0];
//                fY[fNpoints-1] = fY[0];
//             }
//             if (ipoint == fNpoints-1) {
//                fX[0] = fX[fNpoints-1];
//                fY[0] = fY[fNpoints-1];
//             }
//          }
         }
         badcase = kFALSE;
         delete [] x;
         x = 0;
         delete [] y;
         y = 0;
         gPad->Modified(kTRUE);
         gVirtualX->SetLineColor(-1);

   }

}


void KVIDLine::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDLine

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 2) {
         TGraph::Streamer(R__b);
      }
      else {
         R__b.ReadClassBuffer(KVIDLine::Class(), this, R__v, R__s, R__c);
      }
   }
   else {
      R__b.WriteClassBuffer(KVIDLine::Class(), this);
   }
}
