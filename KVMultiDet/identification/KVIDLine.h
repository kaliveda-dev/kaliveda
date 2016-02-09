/***************************************************************************
                          KVIDLine.h  -  description
                             -------------------
    begin                : Nov 10 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDLine.h,v 1.16 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/

#ifndef KVIDLine_H
#define KVIDLine_H

#include "TVector2.h"
#include "TMath.h"
#include "TH2.h"
#include "KVIDentifier.h"

class KVIDLine : public KVIDentifier {

public:

   KVIDLine();
   KVIDLine(const TGraph& gr);
   KVIDLine(const KVIDLine&);

   virtual ~ KVIDLine();

   virtual void WaitForPrimitive();
   virtual void ExecuteEvent(Int_t event, Int_t px, Int_t py);

   inline Double_t DistanceToLine(Double_t px, Double_t py, Int_t&);
   inline Double_t DistanceToLine(Double_t px, Double_t py, Double_t xp1,
                                  Double_t yp1, Double_t xp2, Double_t yp2,
                                  Int_t&);
   inline Bool_t WhereAmI(Double_t px, Double_t py, Option_t* opt);
   inline Bool_t PosRelToLine(Option_t* opt, Double_t px, Double_t py,
                              Double_t xp1, Double_t yp1, Double_t xp2,
                              Double_t yp2);

   inline void GetStartPoint(Double_t& x, Double_t& y) const;
   inline void GetEndPoint(Double_t& x, Double_t& y) const;
   inline Bool_t IsBetweenEndPoints(Double_t x, Double_t y,
                                    const Char_t* axis = "") const;

   static KVIDLine* MakeIDLine(TObject* obj, Double_t xdeb = -1., Double_t xfin = -1., Double_t np = 1., Bool_t save = kFALSE);
// static KVIDLine *MakeIDLine(TH2 *obj,TCutG *cut,Double_t xdeb=-1.,Double_t xfin=-1.,Double_t np=1.,Bool_t save=kFALSE);

   ClassDef(KVIDLine, 2)       //Base class for lines/cuts used for particle identification
};

//______________________________________________________________________________

inline Double_t KVIDLine::DistanceToLine(Double_t px, Double_t py,
      Int_t& i_near)
{
   //Compute the closest distance of approach from point px,py to this line.
   //The units of px, py are the same as the coordinates of the graph
   //
   //WARNING: result can be NEGATIVE - see below.
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

   Double_t distance, dist2;
   Int_t i;
   Double_t d;
   distance = dist2 = 9999.;
   Int_t i_nearest_point = 0, inear1 = 0, inear2 = 0;
   //distance = closest approach to a segment with point lying between endpoints
   //dist2 = closest approach to a segment with point outside endpoints
   //if distance == 9999 at end, then dist2 is used

   for (i = 0; i < fNpoints - 1; i++) {
      d = DistanceToLine(px, py, fX[i], fY[i], fX[i + 1], fY[i + 1],
                         i_nearest_point);
      //cout << "i=" << i << "  X1="<<fX[i]<<" Y1="<<fY[i]<<" X2="<<fX[i+1]<<" Y2="<<fY[i+1]<<" --- d = "<<d<<endl;
      if (d >= 0.) {
         if (d < distance) {
            distance = d;
            inear1 = i;
         }
      } else {
         //point not between endpoints of segment
         if (-d < dist2) {
            dist2 = -d;
            inear2 = i + i_nearest_point;
         }
      }
   }

   i_near = inear1;
   if (distance < 9999.)
      return distance;
   i_near = inear2;
   //closest point is an 'internal' point of line - POSITIVE distance
   if (inear2 > 0 && inear2 < (fNpoints - 1))
      return dist2;
   //closest point is one of endpoints of line - NEGATIVE distance
   return -dist2;
}


//_________________________________________________________________________________________________________________________________________

inline Double_t KVIDLine::DistanceToLine(Double_t px, Double_t py,
      Double_t xp1, Double_t yp1,
      Double_t xp2, Double_t yp2,
      Int_t& i_nearest_point)
{
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

   TVector2 P1(xp1, yp1), P2(xp2, yp2), P(px, py);
   TVector2 P1P2 = P2 - P1;
   TVector2 P1P = P - P1;
   TVector2 P2P = P - P2;
   TVector2 MP = P1P.Norm((P1P2));
   i_nearest_point = 0;
   //check point lies between endpoints
   Double_t sum = (P1P - MP).Mod() + (P2P - MP).Mod();
   //cout << "sum = "<<sum<<"  P1P2 = " << P1P2.Mod() << " difference = " << sum-P1P2.Mod() << endl;
   if (TMath::Abs(sum - P1P2.Mod()) > 1.e-06) {
      //cout << "take endpoint" << endl;
      if (P1P.Mod() < P2P.Mod()) {
         i_nearest_point = 0;
         return -(P1P.Mod());
      }
      i_nearest_point = 1;
      return -(P2P.Mod());
   }
   //cout << "point is close to segment" << endl;
   return MP.Mod();
}


//______________________________________________________________________________

inline Bool_t KVIDLine::PosRelToLine(Option_t* opt, Double_t px,
                                     Double_t py, Double_t xp1,
                                     Double_t yp1, Double_t xp2,
                                     Double_t yp2)
{
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

   TVector2 P1(xp1, yp1), P2(xp2, yp2), P(px, py);
   TVector2 P1P2 = P2 - P1;
   TVector2 P1P = P - P1;
   TVector2 MP = P1P.Norm((P1P2));
   Double_t phi = MP.Phi() * TMath::RadToDeg();
   //Point is left of line if 90<phi<270
   //Point is right of line if 270<phi<90
   //Point is above line if 0<phi<180
   //Point is below line if 180<phi<360
   Bool_t result = kFALSE;
   if (!strcmp(opt, "left")) {
      result = (phi > 90. && phi < 270.);
   } else if (!strcmp(opt, "right")) {
      result = (phi < 90. || phi > 270.);
   } else if (!strcmp(opt, "above")) {
      result = (phi > 0. && phi < 180.);
   } else if (!strcmp(opt, "below")) {
      result = (phi > 180. && phi < 360.);
   }
   return result;
}

//______________________________________________________________________________

inline Bool_t KVIDLine::WhereAmI(Double_t px, Double_t py, Option_t* opt)
{
   //The relative position of point (px,py) with respect to the line is tested.
   //The option string can be "left", "right", "above" or "below".
   //WhereAmI( px, py, "above") returns kTRUE if the point is above the line etc. etc.
   //
   //First of all, the closest segment/point of the line to the point is found.
   //Then the relative position of the point and this segment/point is tested.

   Double_t* XX, *YY;
   Double_t  xx,  yy;
   Int_t    sign  = 1;

   if (!strcmp(opt, "left")) {
      XX = fY;
      xx = py;
      YY = fX;
      yy = px;
   } else if (!strcmp(opt, "right")) {
      XX = fY;
      xx = py;
      YY = fX;
      yy = px;
      sign = -1;
   } else if (!strcmp(opt, "above")) {
      XX = fX;
      xx = px;
      YY = fY;
      yy = py;
      sign = -1;
   } else if (!strcmp(opt, "below")) {
      XX = fX;
      xx = px;
      YY = fY;
      yy = py;
   } else return kFALSE;

   Int_t i_start     = 0;
   Int_t i_stop      = fNpoints - 1;
   Int_t prev_i_stop = 0;
   Bool_t same_sign = TMath::Sign(1., XX[i_start] - xx) == TMath::Sign(1., XX[i_stop] - xx);
   while ((i_start < i_stop - 1) || same_sign) {

      if (same_sign && (prev_i_stop == 0))  break;
      else if (same_sign) {
         i_start = i_stop;
         i_stop  = prev_i_stop;
      } else {
         prev_i_stop = i_stop;
         i_stop = (Int_t)((i_start + i_stop) / 2 + 0.5);
      }

      same_sign = TMath::Sign(1., XX[i_start] - xx) == TMath::Sign(1., XX[i_stop] - xx);
   }

   Double_t a = (YY[i_stop] - YY[i_start]) / (XX[i_stop] - XX[i_start]);
   Double_t b = YY[i_start] - a * XX[i_start];

   Bool_t res = (sign * yy < sign * (a * xx + b));

   return res;
}

//_____________________________________________________________________________________________

inline void KVIDLine::GetStartPoint(Double_t& x, Double_t& y) const
{
   //Coordinates of first point in line
#if ROOT_VERSION_CODE >= ROOT_VERSION(4,0,3)
   GetPoint(0, x, y);
#else
   const_cast < KVIDLine* >(this)->GetPoint(0, x, y);
#endif
}

//_____________________________________________________________________________________________

inline void KVIDLine::GetEndPoint(Double_t& x, Double_t& y) const
{
   //Coordinates of last point in line
#if ROOT_VERSION_CODE >= ROOT_VERSION(4,0,3)
   GetPoint((GetN() - 1), x, y);
#else
   const_cast < KVIDLine* >(this)->GetPoint((GetN() - 1), x, y);
#endif
}

//_____________________________________________________________________________________________

inline Bool_t KVIDLine::IsBetweenEndPoints(Double_t x, Double_t y,
      const Char_t* axis) const
{
   //Returns kTRUE for point (x,y) if :
   // axis = "" (default) and both x and y lie inside the endpoints of the line x1 < x < x2, y1 < y < y2
   // axis = "x" and x lies inside the endpoints x1 < x < x2
   // axis = "y" and y lies inside the endpoints y1 < y < y2

   TString ax(axis);
   ax.ToUpper();
   Double_t x1, y1, x2, y2;
   GetStartPoint(x1, y1);
   GetEndPoint(x2, y2);
   Double_t xmin = TMath::Min(x1, x2);
   Double_t xmax = TMath::Max(x1, x2);
   Double_t ymin = TMath::Min(y1, y2);
   Double_t ymax = TMath::Max(y1, y2);

   Bool_t in_range_x = (x <= xmax && x >= xmin);
   Bool_t in_range_y = (y <= ymax && y >= ymin);
   if (ax == "X") {
      return in_range_x;
   } else if (ax == "Y") {
      return in_range_y;
   }

   return (in_range_x && in_range_y);
}

#endif
