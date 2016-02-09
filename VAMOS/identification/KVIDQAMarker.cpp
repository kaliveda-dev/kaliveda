//Created by KVClassFactory on Wed Jul  9 15:38:39 2014
//Author: Guilain ADEMARD

#include "KVIDQAMarker.h"
#include "KVIDQALine.h"
#include "TROOT.h"
#include "TClass.h"
#include "TCanvas.h"

#ifndef ROOT_Buttons
#include "Buttons.h"
#endif

ClassImp(KVIDQAMarker)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDQAMarker</h2>
<h4>Base class for identification markers corresponding to differents couples of mass and charge state</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDQAMarker::KVIDQAMarker()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVIDQAMarker::KVIDQAMarker(KVIDQALine* parent, Int_t a)
{
   init();
   SetParent(parent);
   SetA(a);
}
//________________________________________________________________

KVIDQAMarker::~KVIDQAMarker()
{
   // Destructor

   // delete associated ID line point.
   if (fIdx > 0 && (fDelta == 0)) {
      if (fParent && (fParent->KVIDZALine::RemovePoint(fIdx) >= 0))
         fParent->IncrementPtIdxOfMarkers(fIdx, -1);
   }
}
//________________________________________________________________

void KVIDQAMarker::init()
{
   // Initialization of data members. Called by constructors.
   fParent = NULL;
   SetA(0);
   fIdx = -1;
   fDelta = 0;
   SetMarkerStyle(21);
}
//________________________________________________________________

void KVIDQAMarker::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVIDQAMarker::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TMarker::Copy(obj);
   KVIDQAMarker& CastedObj = (KVIDQAMarker&)obj;
   //CastedObj.SetParent(fParent);
   CastedObj.SetA(fA);
   CastedObj.fIdx = fIdx;
   CastedObj.fDelta    = fDelta;
}
//________________________________________________________________

Int_t KVIDQAMarker::Compare(const TObject* obj) const
{
   // Compares numerical valur of X coordinate for sorting lists (such as KVList)

   const KVIDQAMarker* other = dynamic_cast<const KVIDQAMarker*>(obj);
   if (!other) return 0;
   // check for equality
   return ((other->GetX()) > GetX() ? -1 : 1);
}
//________________________________________________________________

void KVIDQAMarker::SetParent(KVIDQALine* parent)
{
   fParent = parent;
}
//________________________________________________________________

void KVIDQAMarker::SetPointIndex(Int_t idx, Double_t delta)
{
   fDelta = delta;
   fIdx   = idx;
   UpdateXandY();
}

//________________________________________________________________

Int_t KVIDQAMarker::GetQ() const
{
   return (fParent ? fParent->GetQ() : 0);
}
//________________________________________________________________

void KVIDQAMarker::ls(Option_t*) const
{
   // List TNamed name and title.

   TROOT::IndentLevel();
   std::cout << "OBJ: " << IsA()->GetName() << "\t" << GetName() << " : "
             << Form("PtIdx= %d, fDelta= %f, X=%f, Y=%f, marker type=%d :", fIdx, fDelta, fX, fY, fMarkerStyle)
             << Int_t(TestBit(kCanDelete)) << " at: " << this << std::endl;
}
//________________________________________________________________

void KVIDQAMarker::UpdateXandY()
{
   // Set X and Y of this marker calculated from the coordinates of low point, up point, and fDelta.
   // fDelta is the distance between this marker end the low point,
   // normalized to the distance between low and up points;
   if (!fParent) return;
   Double_t x_low, y_low;
   if (fParent->GetPoint(fIdx, x_low, y_low) > -1) {
      if (fDelta) { // marker is between low and up points
         Double_t x_up, y_up;
         if (fParent->GetPoint(fIdx + 1, x_up, y_up) > -1) {
            SetX(x_low + (x_up - x_low)*fDelta);
            SetY(y_low + (y_up - y_low)*fDelta);
         }
      } else {      // marker is over one point
         SetX(x_low);
         SetY(y_low);
      }
   }
}
//________________________________________________________________

void KVIDQAMarker::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{

   if (fParent && !fParent->IsEditable()) return;

   static Double_t x_prev;
   static Bool_t motion = kFALSE;

   // keep memory of the current opaque moving and reset it
   // because if gPad->OpaqueMoving() is true then it is impossible
   // to move this marker everywhere you want
   Bool_t opaque  = gPad->OpaqueMoving();
   gPad->GetCanvas()->MoveOpaque(0);

   TMarker::ExecuteEvent(event, px, py);

   switch (event) {
      case kButton1Down:
         x_prev = GetX();
         break;

      case kButton1Motion:
         motion = kTRUE;
         break;
      case kButton1Up:
         if (motion) {
            motion = kFALSE;
            if ((fIdx > -1)) {
               if (fDelta) {// marker is between low and up points
                  fIdx = fParent->InsertPoint(fIdx + 1, GetX(), GetY(), x_prev);
                  fDelta   = 0;
                  fParent->GetMarkers()->R__FOR_EACH(KVIDQAMarker, UpdateXandY)();
               } else {
                  fParent->SetPoint(fIdx, GetX(), GetY());
                  fParent->GetMarkers()->R__FOR_EACH(KVIDQAMarker, UpdateXandY)();
               }
               gPad->Modified(kTRUE);
               gPad->Update();
            }
         }
         break;
   }

   // set the initial opaque moving status
   gPad->GetCanvas()->MoveOpaque(opaque);
}
//________________________________________________________________

void KVIDQAMarker::SetPointIndexAndX(Int_t idx, Double_t x)
{
   // Set index of the low neighbour point and the X coordinate of this
   // marker. Then fDelta and Y coordinates will be calculated and set to
   // this marker.

   if (!fParent) return;
   if (fParent->GetN() - 2 < idx) return;

   fIdx = idx;

   Double_t x_low, y_low, x_up, y_up;
   if ((fParent->GetPoint(idx, x_low, y_low) > -1) && (fParent->GetPoint(idx + 1, x_up, y_up) > -1)) {
      Double_t y;
      if (x_low == x_up) y = y_low;
      else y = y_up + (x - x_up) * (y_low - y_up) / (x_low - x_up);

      Double_t L     = TMath::Sqrt(TMath::Power(x_up - x_low, 2) + TMath::Power(y_up - y_low, 2));
      Double_t l     = TMath::Sqrt(TMath::Power(x - x_low, 2) + TMath::Power(y - y_low, 2));

      if (L == 0) fDelta = 0.;
      else fDelta = l / L;

      SetX(x);
      SetY(y_low + (y_up - y_low)*fDelta);
   }
}
//________________________________________________________________

void KVIDQAMarker::WriteAsciiFile(std::ofstream& file)
{
   // Write attributes of this KVIDQAMarker in ascii file

   file << fA << "\t" << fIdx << "\t" << fDelta << "\t" << fX << "\t" << fY << std::endl;
}
//________________________________________________________________

void KVIDQAMarker::ReadAsciiFile(std::ifstream& file)
{
   // Read attributes of this KVIDQAMarker in ascii file

   Int_t a;
   file >> a >> fIdx >> fDelta >> fX >> fY;
   SetA(a);
}
//________________________________________________________________

Double_t KVIDQAMarker::DistanceToMarker(Double_t px, Double_t py) const
{
   // Computes the closest distance of approach from point (px,py) to this marker.
   // This distance is calculated on a A/Q scale i.e. if the Y coordinate is A
   // then the Y coordinate will be divide by GetQ(). If the Y coordinate is
   // Q then Y is change by GetA()/Y. Keep in mind that the X coordinate is supposed
   // to give the ratio A/Q (mass number over the charge state).

   UNUSED(py);

   if (!fParent) return -1.;
   Double_t X = px - fX;
   Double_t Y = 0.;
   /*
   if( fParent->IsAvsAoQ() ){
      if( GetQ() ) Y = (py-fY)/GetQ();
      else return -1;
   }
   else{
      if( py*fY ) Y = GetA()*(fY-py)/(fY*py);
      else return -1.;
   }
   */
   return TMath::Sqrt(X * X + Y * Y);
}
