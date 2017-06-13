//Created by KVClassFactory on Fri Jun 20 12:07:39 2014
//Author: Guilain ADEMARD

#include "KVIDQALine.h"
#include "TClass.h"
#include "TPad.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "KVIDQAGrid.h"

using namespace std;

ClassImp(KVIDQALine)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDQALine</h2>
<h4>Base class for identification ridge lines and spots corresponding to different charge states and masses respectively</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDQALine::KVIDQALine()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVIDQALine::~KVIDQALine()
{
   // Destructor
   fMarkers->Delete();
   fMarkers = NULL;
}
//________________________________________________________________

void KVIDQALine::init()
{
   // Initialization, used by constructors.
   // Marker width is set to zero.

   fMarkers = new KVList;

   //-----------------------
   static Int_t iMarker = 0;
   fMarkers->SetName(Form("fMarkers%d__%p", iMarker++, fMarkers));
   //-----------------------

   fMarkers->SetCleanup();
   fNextA = 1;
   fNextAinc = kTRUE;

   //no draw points of this line, just draw its KVIDQAMarkers
   SetMarkerStyle(24);
   SetWidth(0.5);
}
//________________________________________________________________

void KVIDQALine::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVIDQALine::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVIDZALine::Copy(obj);
   KVIDQALine& CastedObj = (KVIDQALine&)obj;
   CastedObj.fNextA = fNextA;
   CastedObj.fNextAinc = fNextAinc;
   CastedObj.fWidth = fWidth;
   fMarkers->Copy((TObject&)(*CastedObj.GetMarkers()));
   CastedObj.GetMarkers()->R__FOR_EACH(KVIDQAMarker, SetParent)((&CastedObj));

}
//________________________________________________________________

void KVIDQALine::Draw(Option_t* chopt)
{
//Draw all KVIDQAMarkers in line on the current display, if one exists: i.e. in order to superimpose the grid
   //If the line is already displayed (i.e. if fParent->GetPad()!=0), we call UnDraw() in order to remove them from the display.
   //This is so that double-clicking a graph in the IDGridManagerGUI list makes them disappear if they are already drawn.
   KVIDZALine::Draw(chopt);
   fMarkers->R__FOR_EACH(KVIDQAMarker, Draw)();
}
//________________________________________________________________

void KVIDQALine::UnDraw()
{
   //Make the line and their markers disappear from the current canvas/pad
   //In case any objects have (accidentally) been drawn more than once, we keep calling
   //gPad->GetListOfPrimitives()->Remove() as long as gPad->GetListOfPrimitives()->FindObject()

   if (fParent && fParent->GetPad()) {
      //remove the rest of the markers of the line
      TIter next_mk(fMarkers);
      KVIDQAMarker* mk;
      while ((mk = (KVIDQAMarker*) next_mk())) {
         while (fParent->GetPad()->GetListOfPrimitives()->FindObject(mk))
            fParent->GetPad()->GetListOfPrimitives()->Remove(mk);
      }
   }
}
//________________________________________________________________

void KVIDQALine::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{

   KVIDZALine::ExecuteEvent(event, px, py);

   static Bool_t motion = kFALSE;
   switch (event) {
      case kButton1Motion:
         motion = kTRUE;
         break;
      case kButton1Up:
         if (motion) {
            motion = kFALSE;
            fMarkers->R__FOR_EACH(KVIDQAMarker, UpdateXandY)();
         }
         break;
   }
}
//________________________________________________________________

void KVIDQALine::IdentA(Double_t x, Double_t y, Int_t& A, Double_t& realA, Int_t& code) const
{

   // Default values
   A     = -1;
   realA = -1.;

   Double_t dist, dist_r, dist_l;
   Int_t     idx,  idx_r,  idx_l;
   KVIDQAMarker* closest_mk = const_cast<KVIDQALine*>(this)->FindNearestIDMarker(x, y, idx, idx_l, idx_r, dist, dist_l, dist_r);

   if (!closest_mk) {
      //no marker corresponding to point was found
      //we assume that realA = Q * A/Q and set kICODE0
      realA = x * GetQ();
      code = KVIDQAGrid::kICODE0;
   } else {
      //the closest marker is found
      //Info("IdentA","ID marker found: %s",closest_mk->GetName());

      //marker at the left of the point
      KVIDQAMarker* l_mk = GetMarkerAt(idx_l);

      //marker at the right of the point
      KVIDQAMarker* r_mk = GetMarkerAt(idx_r);

      // WARNING: the method KVIDQALine::FindNearestIDMarker always find
      // 2 nearest markers even for point left (right) the first (last) marker.
      // we have to remove the second found marker for these points

      if ((closest_mk == GetMarkers()->First()) && !ProjIsBetween(x, y, l_mk, r_mk))
         r_mk = NULL;
      else if (closest_mk == GetMarkers()->Last() && !ProjIsBetween(x, y, l_mk, r_mk))
         l_mk = NULL;

      Double_t deltaA = 0.;

      // case where 2 embracing markers are found
      if (l_mk && r_mk && (l_mk != r_mk)) {
         if (dist > closest_mk->GetWidth() / 2.) {
            if (closest_mk == l_mk) code = KVIDQAGrid::kICODE1;  // "slight ambiguity of A, which could be larger"
            else code = KVIDQAGrid::kICODE2; // "slight ambiguity of A, which could be smaller"
         } else code = KVIDQAGrid::kICODE0; // ok

      }
      // case where only 1 embracing marker is found.
      // in this case, the distance between the point and the marker
      // has to be lower than the marker width
      else {
         if (dist > closest_mk->GetWidth() / 2.) {
            // the distance from the closest marker is to high
            if (closest_mk == l_mk) code = KVIDQAGrid::kICODE6;  // "(x,y) is below first marker in line"
            else code = KVIDQAGrid::kICODE7; // "(x,y) is above last marker in line"
         } else {
            code = KVIDQAGrid::kICODE3; // "slight ambiguity of A, which could be larger or smaller"
         }
      }

      deltaA = (x - closest_mk->GetX()) * GetQ();
      realA = closest_mk->GetA() + deltaA;
   }
   A     = TMath::Nint(realA);   // since some A-markers are missing in the line
}
//________________________________________________________________

KVIDQAMarker* KVIDQALine::FindNearestIDMarker(Double_t x, Double_t y, Int_t& idx, Int_t& idx_low, Int_t& idx_up, Double_t& dist, Double_t& dist_low, Double_t& dist_up) const
{
   // the list of Markers has to be sorted before to use this method


   Int_t n_mk = GetNumberOfMasses();
   if (n_mk == 0) return NULL;

   idx_low = 0;       // minimum index
   idx_up = n_mk - 1; // maximum index

   while (idx_up > idx_low + 1) {
      dist_low = GetMarkerAt(idx_low)->DistanceToMarker(x, y);
      dist_up  = GetMarkerAt(idx_up)->DistanceToMarker(x, y);

      if (dist_low < dist_up) {
         //deacrease index of idx_max
         idx_up -= (Int_t)((idx_up - idx_low) / 2 + 0.5);
      } else {
         //increase index
         idx_low += (Int_t)((idx_up - idx_low) / 2 + 0.5);
      }
   }
   KVIDQAMarker* mk_low = GetMarkerAt(idx_low);
   KVIDQAMarker* mk_up  = GetMarkerAt(idx_up);

   // calculate distance of point (x,y) to the two closest markers
   dist_low = mk_low->DistanceToMarker(x, y);
   dist_up  = mk_up->DistanceToMarker(x, y);

   KVIDQAMarker* nearest = NULL;
   if (dist_low < dist_up) {
      dist    = dist_low;
      idx     = idx_low;
      nearest = mk_low;
   } else {
      dist    = dist_up;
      idx     = idx_up;
      nearest = mk_up;
   }

   //Info("FindNearestIDMarker","X %f, Y %f: low %s, dist_low %f, up %s, dist_up %f: nearest %s",x,y,mk_low->GetName(),dist_low,mk_up->GetName(),dist_up,nearest->GetName());


   return nearest;
}
//________________________________________________________________

Int_t KVIDQALine::InsertMarker(Int_t a)
{
   // Insert a new Q-A identification marker at the mouse position

   Int_t idx = InsertPoint();
   if (idx >= 0) {
      if ((fNextAinc ? 1 : -1) * (a - fNextA) < 0) fNextAinc = !fNextAinc;
      fNextA = a + (fNextAinc ? 1 : -1);

      KVIDQAMarker* m = new KVIDQAMarker(this, a);
      AddMarker(m);
      m->SetPointIndex(idx);
      m->Draw();
   }
   return idx;
}
//________________________________________________________________

Int_t KVIDQALine::RemoveMarker(Int_t a)
{

   Int_t idx = -1;
   KVIDQAMarker* m = NULL;
   TIter next(fMarkers);

   // look for a marker with 'a'
   if (a > 0) {
      while ((m = (KVIDQAMarker*)next())) {
         if (a == m->GetA()) break;
      }
   }
   // look for a marker at mouse position
   else {
      Int_t px = gPad->GetEventX();
      Int_t py = gPad->GetEventY();

      //localize makrer to be deleted
      // start with a small window (in case the mouse is very close to one marker)
      while ((m = (KVIDQAMarker*)next())) {
         Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(m->GetX()));
         Int_t dpy = py - gPad->YtoAbsPixel(gPad->YtoPad(m->GetY()));
         if (dpx * dpx + dpy * dpy < 100) break;
      }
   }

   // if a marker is found ...
   if (m) {
      //... remove it from the marker list
      fMarkers->Remove(m);
      delete m;
   }
   return idx;
};
//________________________________________________________________

Int_t KVIDQALine::InsertPoint()
{
   Int_t i = KVIDZALine::InsertPoint();
   if (i >= 0) IncrementPtIdxOfMarkers(i);
// Info("InsertPoint","Point %d inserted at X=%f, Y=%f",i,fX[i],fY[i]);
   return i;
}
//________________________________________________________________

Int_t KVIDQALine::InsertPoint(Int_t i, Double_t x, Double_t y, Double_t x_prev)
{

   if ((i < 0) || (i > fNpoints)) return -1;

   Double_t y_prev = Eval(x_prev);

   Double_t** ps = ExpandAndCopy(fNpoints + 1, i);
   CopyAndRelease(ps, i, fNpoints++, i + 1);

   // To avoid redefenitions in descendant classes
   FillZero(i, i + 1);

   fX[i] = x_prev;
   fY[i] = y_prev;
   IncrementPtIdxOfMarkers(i);
// Info("InsertPoint","Point %d inserted at X=%f, Y=%f",i,fX[i],fY[i]);
   fX[i] = x;
   fY[i] = y;

   return i;
}
//________________________________________________________________

Int_t KVIDQALine::RemovePoint(Int_t i)
{
   // Delete point number ipoint and the associated ID marker
   // if it has one.

   KVIDQAMarker* m = NULL;
   TIter next(fMarkers);
   while ((m = (KVIDQAMarker*)next())) {
      if ((i == m->GetPointIndex()) && !m->GetDelta()) break;
   }
   if (m) {
      fMarkers->Remove(m);
      delete m;
   } else {
      i = KVIDZALine::RemovePoint(i);
      if (i >= 0) IncrementPtIdxOfMarkers(i, -1);
   }
   return i;
}
//________________________________________________________________

void KVIDQALine::IncrementPtIdxOfMarkers(Int_t idx, Int_t ival)
{
   TIter next(fMarkers);
   KVIDQAMarker* m = NULL;
   while ((m = (KVIDQAMarker*)next())) {
      Int_t    idx_m;
      Double_t delta_m;
      m->GetPointIndexAndDelta(idx_m, delta_m);

      if ((idx - 1) <= idx_m) {

         Double_t delta = 0;
         if ((idx_m == (idx - 1)) && (delta_m)) {
            //vector givent by the initial low and up neighbour points
            Double_t xi = fX[idx - 1] - fX[idx + 1];
            Double_t yi = fY[idx - 1] - fY[idx + 1];
            Double_t li = TMath::Sqrt(TMath::Power(xi, 2) + TMath::Power(yi, 2));

            //vector givent by the final low and up points
            Double_t xf = fX[idx - 1] - fX[idx];
            Double_t yf = fY[idx - 1] - fY[idx];
            Double_t lf = TMath::Sqrt(TMath::Power(xf, 2) + TMath::Power(yf, 2));

            if (delta_m * li < lf) {
               delta = delta_m * li / lf;
            } else {
               delta = (delta_m * li - lf) / (li - lf);
               idx_m += ival;
            }
         } else if (idx <= idx_m) {
            delta  = delta_m;
            idx_m += ival;
         }

         if (idx_m < 0) {
            idx_m = 0;
            delta = 0.;
         }
         if (idx_m > fNpoints - 2) {
            idx_m = fNpoints - 2;
            delta = 1.;
         }

         m->SetPointIndex(idx_m, delta);
      }
   }
}
//________________________________________________________________

void KVIDQALine::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDQALine.
   // Resets the 'fParent' pointer of each KVIDQAMarker after reading them in.
   //
   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVIDQALine::Class(), this);
      fMarkers->R__FOR_EACH(KVIDQAMarker, SetParent)((this));
   } else {
      R__b.WriteClassBuffer(KVIDQALine::Class(), this);
   }
}
//________________________________________________________________

void KVIDQALine::FindAMarkers(TH1* h)
{

   //find mass peaks in histogram h.
   //Use TSpectrum to find the peak candidates.
   //Build KVIDQAMarker for each peak found.

   TSpectrum s(60);
   Int_t nfound = s.Search(h, 2, "goff");

   Info("FindAMarkers", "%d masses found in histogram %s for %s line", nfound, h->GetName(), GetName());

   // Clear list of IDQAMarkers to build a new one
   fMarkers->Clear();

   // Build new IDQAMarkers
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
   Double_t* xpeaks = s.GetPositionX();
#else
   Float_t* xpeaks = s.GetPositionX();
#endif
   for (int p = 0; p < nfound; p++) {
      Double_t x = xpeaks[p];
      KVIDQAMarker* m = new KVIDQAMarker;
      m->SetParent(this);
      m->SetX(x);
      AddMarker(m);
   }

   // sort fMarkers list from X coordinate
   SortMarkers();

   // find neighbours simply looping  all points
   // and find also the 2 adjacent points: (low2 < low < x < up < up2 )
   // needed in case x is outside the graph ascissa interval
   Int_t low  = -1;
   TIter next(fMarkers);
   KVIDQAMarker* m = NULL;

   Int_t a = 0; // we assum that xpeaks gives A/Q
   Bool_t stop = kFALSE;
   KVList del_list;
   while ((m = (KVIDQAMarker*)next())) {

      Double_t x = m->GetX();
      Double_t tmp = TMath::Nint(x * GetQ());
      if (a <= tmp) a = tmp;

      m->SetA(a++);
      Int_t up  = -1;

      if (!stop) {
         for (Int_t i = (low > -1 ? low : 0); i < fNpoints; ++i) {
            if (fX[i] < x) {
               if ((low == -1) || (fX[i] > fX[low])) low = i;
            } else if (fX[i] > x) {
               if ((up  == -1) || (fX[i] < fX[up])) up = i;
            } else { // case x == fX[i]
               low = up = i;
               break;
            }
         }
      }

      // treat cases when x is outside line min max abscissa
      if ((up == -1) || (low == -1)) {
         // list the markers which are outside line to be deleted
         del_list.Add(m);
         if (up == -1) stop = kTRUE;
         continue;
      }

      // set index and the X coordinate to the marker
      if (low == up) m->SetPointIndex(low);
      else m->SetPointIndexAndX(low, x);

      // draw the new marker if its parent Q-line is already drawn in the
      // current pad
      if (gPad && gPad->GetListOfPrimitives()->FindObject(this)) m->Draw();
   }

   if (del_list.GetEntries()) {
      Warning("FindAMarkers", "%d markers will be deleted (outside of %s-line)", del_list.GetEntries(), GetName());
      del_list.ls();
   }
}
//________________________________________________________________

void KVIDQALine::WriteAsciiFile_extras(ofstream& file,
                                       const Char_t* name_prefix)
{
   // Write each KVIDQAMarker listed in fMarkers

   KVIDLine::WriteAsciiFile_extras(file, name_prefix);
   file << GetQ() << "\t" << fMarkers->GetEntries() << endl;
   if (fMarkers->First()) {
      file << fMarkers->First()->ClassName() << endl;
      fMarkers->R__FOR_EACH(KVIDQAMarker, WriteAsciiFile)(file);
   }
}
//________________________________________________________________

void KVIDQALine::ReadAsciiFile_extras(ifstream& file)
{
   // Read all KVQAMarker's and set them in the list fMarkers

   KVIDLine::ReadAsciiFile_extras(file);

   Int_t q, a;
   file >> q >> a;
   SetQ(q);
   if (a > 0) {
      KVString cl;
      cl.ReadLine(file);
      for (Int_t i = 0; i < a; i++) {
         KVIDQAMarker* m = New(cl.Data());
         AddMarker(m);
         m->ReadAsciiFile(file);
      }
   }
}
//________________________________________________________________

KVIDQAMarker* KVIDQALine::New(const Char_t* m_class)
{
   //Create new object of class "m_class" which derives from KVIDQAMarker

   KVIDQAMarker* m = 0;
   TClass* clas = TClass::GetClass(m_class);
   if (!clas) {
      Error("New",
            "%s is not a valid classname. No known class.", m_class);
   } else {
      if (!clas->InheritsFrom(KVIDQAMarker::Class())) {
         Error("New",
               "%s is not a valid class deriving from KVIDQAMarker",
               m_class);
      } else {
         m = (KVIDQAMarker*) clas->New();
      }
   }
   return m;
}
//________________________________________________________________

void KVIDQALine::Initialize()
{
   // General initialisation method for Q-A identification line.
   // This method MUST be called once before using the line for identifications.
   // The ID markers are sorted.
   // The natural marker widths of all ID markers are calculated.
   SortMarkers();
   CalculateMarkerWidths();
}
//________________________________________________________________

void KVIDQALine::CalculateMarkerWidths()
{
//Calculate natural "width" of each ID markers in the line.
   //The markers in the line have first to sorted so that they are in order of ascending 'X'
   //i.e. first markers has lower A, last line is the heaviest mass .
   //
   // The width is supposed to be the distance between the first and the
   // last marker divided by the difference of A of both markers.

   KVIDQAMarker* first = (KVIDQAMarker*)fMarkers->First();
   KVIDQAMarker* last  = (KVIDQAMarker*)fMarkers->Last();
   if ((!last) || (!first)) return;
   Double_t width = first->DistanceToMarker(last->GetX(), last->GetY());
   Double_t dA   = last->GetA() - first->GetA();
   width = TMath::Abs(width / dA);
   fMarkers->R__FOR_EACH(KVIDQAMarker, SetWidth)(width);
}
//________________________________________________________________

Bool_t KVIDQALine::ProjIsBetween(Double_t x, Double_t y, KVIDQAMarker* m1, KVIDQAMarker* m2) const
{
   //Given a line segment defined by 2 markers m1 and m2 test if the
   //  projection of the point P(x,y) is between both markers.
   //
   //      m1 x---------------------------------------x m2
   //                                          |
   //                                          |
   //                                          |
   //                                          x
   //                                          P (x,y)
   //
   // returns kFALSE if the projection is outside the segment given
   // by both markers.

   if (!m1 || !m2) return kFALSE;

   Double_t x1 = m1->GetX();
   Double_t x2 = m2->GetX();
   Double_t y1 = m1->GetY();
   Double_t y2 = m2->GetY();
   // scale Y coordinate to A/Q

   if (IsAvsAoQ()) {
      if (GetQ()) {
         y  /= GetQ();
         y1 /= GetQ();
         y2 /= GetQ();
      } else return kFALSE;
   } else {
      if (y > 0 && y1 > 0 && y2 > 0) {
         y  = GetA() / y;
         y1 = GetA() / y1;
         y2 = GetA() / y2;
      } else return kFALSE;
   }
   TVector2 P1P2(x2 - x1, y2 - y1);
   if (P1P2.Mod2() == 0) return kFALSE;
   TVector2 P1M(x - x1, y - y1);

   Double_t proj = (P1M * P1P2) / P1P2.Mod();
   //Info("ProjIsBetween","P1M %f, P1P2 %f, proj %f",P1M.Mod(), P1P2.Mod(), proj);
   if ((0 <= proj) && (proj <= P1P2.Mod())) return kTRUE;
   return kFALSE;
}
//________________________________________________________________

void KVIDQALine::IncrementAMarkers(Int_t val, Option_t* dir)
{
   // Increment A value of A-markers from the mouse position.
   // In option 'dir', set the direction of the A-marker to be incremented.
   //   dir -- left, rigth, above, below, all

   Int_t px = gPad->GetEventX();
   Int_t py = gPad->GetEventY();
   Double_t x = gPad->PadtoX(gPad->AbsPixeltoX(px));
   Double_t y = gPad->PadtoY(gPad->AbsPixeltoY(py));

   Int_t opt = 0;
   if (!strcmp(dir, "right")) opt = 1;
   else if (!strcmp(dir, "left")) opt = 2;
   else if (!strcmp(dir, "above")) opt = 3;
   else if (!strcmp(dir, "below")) opt = 4;

   TIter next(fMarkers);
   KVIDQAMarker* mk = NULL;
   while ((mk = (KVIDQAMarker*)next())) {
      Int_t a = mk->GetA() + val;
      Bool_t inc = kFALSE;
      switch (opt) {
         case 0:
            inc = kTRUE;
            break;
         case 1:
            if (mk->GetX() >= x) inc = kTRUE;
            break;
         case 2:
            if (mk->GetX() <= x) inc = kTRUE;
            break;
         case 3:
            if (mk->GetY() >= y) inc = kTRUE;
            break;
         case 4:
            if (mk->GetY() <= y) inc = kTRUE;
            break;
      }
      if (inc) mk->SetA(a);
   }
}
