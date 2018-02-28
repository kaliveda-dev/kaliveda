//Created by KVClassFactory on Fri Feb 17 17:47:35 2012
//Author: dgruyer

#include "KVCanvas.h"
#include "TROOT.h"
#include "TBox.h"
#include "TAxis.h"
#include "TContextMenu.h"
#include "TRootCanvas.h"
#include "TGWindow.h"
#include "KVKeyHandler.h"
#include <KeySymbols.h>
#include "TMath.h"
#include "KVHistoManipulator.h"
#include "TProfile.h"
#include "TF1.h"
#include "TGMsgBox.h"

#include <Riostream.h>

ClassImp(KVCanvas)

TObject* gCopyObject = 0x0;

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCanvas</h2>
TCanvas with mouse-controlled dynamic zoom and pan & scan.<br>

<img alt="KVCanvas" src="http://indra.in2p3.fr/KaliVedaDoc/images/KVCanvas.png"><br><br>

Dynamic zoom: left-click to draw rectangle around region of interest, release mouse button to zoom in<br>
Zoom in: mouse wheel up<br>
Zoom out: mouse wheel down<br>
Pan: hold down mouse wheel (centre button) and move
<!-- */
// --> END_HTML
// KEYBOARD SHORTCUTS:
//<ctrl> e   show editor
//<ctrl> f   start fit panel (TH1)
//<ctrl> g   set/unset grid on X and Y axes
//<ctrl> i   show shortcuts infos
//<ctrl> l   set/unset log scale on Y axis (TH1) or Z axis (TH2)
//<ctrl> n   normalize drawn histogram to its integral
//<ctrl> p x   draw profile X (TH2)
//<ctrl> p y   draw profile Y (TH2)
//<ctrl> s   save canvas as
//<ctrl> u   update canvas
//<ctrl> v   set/unset 'vener' mode (TH2)
//<ctrl> w   set/unset 'Age Of Empire' mode (TH2)
//<ctrl> +   set minimum +1 (TH2)
//<ctrl> -   set minimum -1 (TH2)
//      F9   set/unset log scale on X axis
//     F10   set/unset log scale on X axis
//     F11   set/unset log scale on X axis
//     F12   unzoom
//  Arrows   move on histogram or axis
////////////////////////////////////////////////////////////////////////////////



//________________________________________________________________
KVCanvas::KVCanvas(): TCanvas()
{
   fKeyHandler  = new KVKeyHandler(this);
   fAgeOfEmpire = false;
   fVenerMode   = false;
   fHasDisabledClasses = false;
   fDisabledClasses = "";
   fFreezed = kFALSE;
   fPPressed = kFALSE;
   fJPressed = kFALSE;
   InitInfos();

   // Default constructor
}

//________________________________________________________________
KVCanvas::~KVCanvas()
{
   gCopyObject = 0;
   // Destructor
}

//________________________________________________________________
KVCanvas::KVCanvas(const char* name, const char* title, Int_t ww, Int_t wh, Bool_t): TCanvas(name, title, ww, wh)
{
   //    if(keyHandler) fKeyHandler = new KVKeyHandler(this);
   fAgeOfEmpire = false;
   fVenerMode   = false;
   fHasDisabledClasses = false;
   fDisabledClasses = "";
   fFreezed = kFALSE;
   fPPressed = kFALSE;
   fJPressed = kFALSE;
   InitInfos();
}

//________________________________________________________________
KVCanvas::KVCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid): TCanvas(name, ww, wh, winid)
{
   //  fKeyHandler = new KVKeyHandler(this);
   fAgeOfEmpire = false;
   fVenerMode   = false;
   fHasDisabledClasses = false;
   fHasDisabledObject = false;
   fDisabledClasses = "";
   fFreezed = kFALSE;
   fPPressed = kFALSE;
   fJPressed = kFALSE;
   InitInfos();
}

//________________________________________________________________
bool KVCanvas::IsLogz()
{
   return fLogz;
}

//________________________________________________________________
bool KVCanvas::IsLogy()
{
   return fLogy;
}

//________________________________________________________________
bool KVCanvas::IsLogx()
{
   return fLogx;
}

//______________________________________________________________________________
void KVCanvas::RunAutoExec()
{
   // Execute the list of TExecs in the current pad.

   if (!TestBit(kAutoExec)) return;
   if (!gPad) return;
   ((TPad*)gPad)->AutoExec();

}

//______________________________________________________________________________
void KVCanvas::DisableClass(const char* className)
{
   fHasDisabledClasses = true;
   fDisabledClasses += className;

}

//______________________________________________________________________________
void KVCanvas::ResetDisabledClass()
{
   fHasDisabledClasses = false;
   fDisabledClasses = "";
}

//______________________________________________________________________________
void KVCanvas::ResetDisabledObject()
{
   fHasDisabledObject = true;
   fDisabledObjects.Clear();
}

//______________________________________________________________________________
void KVCanvas::DisableObject(TObject* obj)
{
   if (!obj) return;
   fHasDisabledObject = true;
   fDisabledObjects.AddLast(obj);
}

//______________________________________________________________________________
void KVCanvas::DrawEventStatus(Int_t event, Int_t px, Int_t py, TObject* selected)
{
   // Report name and title of primitive below the cursor.
   //
   //    This function is called when the option "Event Status"
   //    in the canvas menu "Options" is selected.

   const Int_t kTMAX = 256;
   static char atext[kTMAX];

   if (!TestBit(kShowEventStatus) || !selected) return;

   if (!fCanvasImp) return; //this may happen when closing a TAttCanvas

   TVirtualPad* savepad;
   savepad = gPad;
   gPad = GetSelectedPad();

   fCanvasImp->SetStatusText(selected->GetTitle(), 0);
   fCanvasImp->SetStatusText(selected->GetName(), 1);
   if (event == kKeyPress)
      snprintf(atext, kTMAX, "%c", (char) px);
   else
      snprintf(atext, kTMAX, "%d,%d", px, py);
   fCanvasImp->SetStatusText(atext, 2);
   fCanvasImp->SetStatusText(selected->GetObjectInfo(px, py), 3);
   gPad = savepad;
}

//______________________________________________________________________________
void KVCanvas::HandleInput(EEventType event, Int_t px, Int_t py)
{
   // Handle Input Events.
   //
   //  Handle input events, like button up/down in current canvas.

   if (fFreezed) return;

   TPad*    pad;
   TPad*    prevSelPad = (TPad*) fSelectedPad;
   TObject* prevSelObj = fSelected;

   fPadSave = (TPad*)gPad;
   cd();        // make sure this canvas is the current canvas

   fEvent  = event;
   fEventX = px;
   fEventY = py;

   Int_t sign = 0;
   Bool_t sendOrder = true;

   if (fHasDisabledClasses && fSelected) {
      if (fDisabledClasses.Contains(fSelected->ClassName())) sendOrder = false;
   }

   if (fHasDisabledObject && fSelected) {
      if (fDisabledObjects.Contains(fSelected)) sendOrder = false;
   }

   switch (event) {

      case kMouseMotion:
         // highlight object tracked over
         pad = Pick(px, py, prevSelObj);
         if (!pad) return;

         EnterLeave(prevSelPad, prevSelObj);

         gPad = pad;   // don't use cd() we will use the current
         // canvas via the GetCanvas member and not via
         // gPad->GetCanvas

         if (sendOrder) fSelected->ExecuteEvent(event, px, py);

         RunAutoExec();

         if (fAgeOfEmpire && (fSelected->InheritsFrom("TH2"))) {
            TH2* TheHisto = (TH2*) FindHisto();//fSelected;

            Double_t size = 0.4 - 0.35 * fVenerMode;

            Int_t dX = 0;
            Int_t dY = 0;

            Double_t ppx = AbsPixeltoX(px);
            Double_t ppy = AbsPixeltoY(py);

            TAxis* ax = TheHisto->GetXaxis();
            Int_t X0 = ax->GetFirst();
            Int_t X1 = ax->GetLast();
            Int_t NbinsX = ax->GetNbins();
            px = ax->FindBin(ppx);

            Double_t ddX   = (X1 + X0) * 0.5 - px;
            Double_t distX = TMath::Abs(ddX) / (X1 - X0);
            if (distX >= 0.5) return;

            TAxis* ay = TheHisto->GetYaxis();
            Int_t Y0 = ay->GetFirst();
            Int_t Y1 = ay->GetLast();
            Int_t NbinsY = ay->GetNbins();
            py = ay->FindBin(ppy);

            Double_t ddY   = (Y1 + Y0) * 0.5 - py;
            Double_t distY = TMath::Abs(ddY) / (Y1 - Y0);
            if (distY >= 0.5) return;

            if ((distX <= size) && (distY <= size)) return;
            dX = TMath::Nint(ddX * (0.05 + 0.05 * fVenerMode));
            dY = TMath::Nint(ddY * (0.05 + 0.05 * fVenerMode));

            if (TMath::Abs(dX) < 1) dX = TMath::Sign(1., ddX);
            if (TMath::Abs(dY) < 1) dY = TMath::Sign(1., ddY);


            Bool_t up = false;
            if ((X0 - dX > 0) && (X1 - dX < NbinsX)) {
               ax->SetRange(X0 - dX, X1 - dX);
               up = true;
            }
            if ((Y0 - dY > 0) && (Y1 - dY < NbinsY)) {
               ay->SetRange(Y0 - dY, Y1 - dY);
               up = true;
            }
            if (up) {
               Modified();
               Update();
            }
         }

         break;

      case kMouseEnter:
         // mouse enters canvas
         if (!fDoubleBuffer) FeedbackMode(kTRUE);
         break;

      case kMouseLeave:
         // mouse leaves canvas
         {
            // force popdown of tooltips
            TObject* sobj = fSelected;
            TPad*    spad = fSelectedPad;
            fSelected     = 0;
            fSelectedPad  = 0;
            EnterLeave(prevSelPad, prevSelObj);
            fSelected     = sobj;
            fSelectedPad  = spad;
            if (!fDoubleBuffer) FeedbackMode(kFALSE);
         }
         break;

      case kButton1Double:
      // triggered on the second button down within 350ms and within
      // 3x3 pixels of the first button down, button up finishes action

      case kButton1Down:
         // find pad in which input occured
         pad = Pick(px, py, prevSelObj);
         if (!pad) return;

         gPad = pad;   // don't use cd() because we won't draw in pad
         // we will only use its coordinate system

         if (fSelected) {
            FeedbackMode(kTRUE);   // to draw in rubberband mode
            fSelected->ExecuteEvent(event, px, py);

            RunAutoExec();

            if (fSelected->InheritsFrom("TH2")) {
               oldx = GetEventX();
               oldy = GetEventY();
               xmin = AbsPixeltoX(oldx);
               ymin = AbsPixeltoY(oldy);
               gVirtualX->DrawBox(XtoAbsPixel(xmin), YtoAbsPixel(ymin), oldx, oldy, TVirtualX::kHollow);
            }

         }

         break;

      case kButton1Motion:
         if (fSelected) {
            if (fSelected->InheritsFrom("TH2")) {
               gVirtualX->DrawBox(XtoAbsPixel(xmin), YtoAbsPixel(ymin), oldx, oldy, TVirtualX::kHollow);
               oldx = GetEventX();
               oldy = GetEventY();
               gVirtualX->DrawBox(XtoAbsPixel(xmin), YtoAbsPixel(ymin), oldx, oldy, TVirtualX::kHollow);
               moved = true;
            }
         }
      case kButton1ShiftMotion: //8 == kButton1Motion + shift modifier
         if (fSelected) {
            gPad = fSelectedPad;

            if (sendOrder) fSelected->ExecuteEvent(event, px, py);
            gVirtualX->Update();

            if (!fSelected->InheritsFrom(TAxis::Class())) {
               Bool_t resize = kFALSE;
               if (fSelected->InheritsFrom(TBox::Class()))
                  resize = ((TBox*)fSelected)->IsBeingResized();
               if (fSelected->InheritsFrom(TVirtualPad::Class()))
                  resize = ((TVirtualPad*)fSelected)->IsBeingResized();

               if ((!resize && TestBit(kMoveOpaque)) || (resize && TestBit(kResizeOpaque))) {
                  gPad = fPadSave;
                  Update();
                  FeedbackMode(kTRUE);
               }
            }

            RunAutoExec();
         }

         break;

      case kButton1Up:

         if (fSelected) {
            gPad = fSelectedPad;

            if (sendOrder) fSelected->ExecuteEvent(event, px, py);

            RunAutoExec();

            if (fPadSave)
               gPad = fPadSave;
            else {
               gPad     = this;
               fPadSave = this;
            }
            if (fSelected->InheritsFrom("TH2") && moved && !fSelected->InheritsFrom("TH3")) {
               xmax = AbsPixeltoX(GetEventX());
               ymax = AbsPixeltoY(GetEventY());
               Double_t toto = 0;
               if (xmax < xmin) {
                  toto = xmax;
                  xmax = xmin;
                  xmin = toto;
               }
               if (ymax < ymin) {
                  toto = ymax;
                  ymax = ymin;
                  ymin = toto;
               }
               ZoomSelected((TH2*)FindHisto());
//                ZoomSelected((TH2*)fSelected);
               moved = false;
            }
            Update();    // before calling update make sure gPad is reset
         }
         break;

      //*-*----------------------------------------------------------------------

      case kButton2Down:
         // find pad in which input occured
         pad = Pick(px, py, prevSelObj);
         if (!pad) return;

         gPad = pad;   // don't use cd() because we won't draw in pad
         // we will only use its coordinate system

         FeedbackMode(kTRUE);

         if (!fSelected->InheritsFrom("TH1")) fSelected->Pop();             // pop object to foreground
         pad->cd();                                 // and make its pad the current pad

         if (fSelected->InheritsFrom("TH2") && !fSelected->InheritsFrom("TH3")) {
            // implement pan & scan
            X0 = px;
            Y0 = py;  // u clikd here
            theXaxis = ((TH2*)FindHisto())->GetXaxis();
            theYaxis = ((TH2*)FindHisto())->GetYaxis();
            NXbins = theXaxis->GetNbins();  // maximum bin number in X
            NYbins = theYaxis->GetNbins();  // maximum bin number in Y
            Xf1 = Xfirst0 = theXaxis->GetFirst(); // initial displayed bin range in X
            Xl1 = Xlast0 = theXaxis->GetLast();
            Yf1 = Yfirst0 = theYaxis->GetFirst(); // initial displayed bin range in Y
            Yl1 = Ylast0 = theYaxis->GetLast();
            // size of axes in pixels
            Int_t pixelWidthX = gPad->XtoAbsPixel(gPad->GetUxmax()) - gPad->XtoAbsPixel(gPad->GetUxmin());
            Int_t pixelWidthY = gPad->YtoAbsPixel(gPad->GetUymax()) - gPad->YtoAbsPixel(gPad->GetUymin());
            // sizes of bins in pixels
            NdisXbins = Xlast0 - Xfirst0 + 1;
            NdisYbins = Ylast0 - Yfirst0 + 1;
            XbinPixel = pixelWidthX / (1.0 * NdisXbins);
            YbinPixel = pixelWidthY / (1.0 * NdisYbins);
         }




         if (gDebug)
            printf("Current Pad: %s / %s\n", pad->GetName(), pad->GetTitle());

         // loop over all canvases to make sure that only one pad is highlighted
         {
            TIter next(gROOT->GetListOfCanvases());
            TCanvas* tc;
            while ((tc = (TCanvas*)next()))
               tc->Update();
         }

         /*if (pad->GetGLDevice() != -1 && fSelected)
          fSelected->ExecuteEvent(event, px, py);*/

         break;   // don't want fPadSave->cd() to be executed at the end

      case kButton2Motion:
         //was empty!
         if (fSelected && fSelected->InheritsFrom("TH2") && !fSelected->InheritsFrom("TH3")) {
            // implement pan & scan
            Int_t dX = px - X0; // how far have i moved ?
            Int_t dY = py - Y0;
            Int_t dXbins = dX / XbinPixel;
            Int_t dYbins = dY / YbinPixel;
            Bool_t changed = kFALSE;
            Int_t newXfirst = Xfirst0 - dXbins;
            Int_t newXlast;
            if (newXfirst < 1) {
               newXfirst = 1;
               newXlast = NdisXbins;
            } else {
               newXlast = Xlast0 - dXbins;
               if (newXlast > NXbins) {
                  newXlast = NXbins;
                  newXfirst = newXlast - NdisXbins + 1;
               }
            }
            if (newXfirst != Xf1) {
               Xf1 = newXfirst;
               Xl1 = newXlast;
               theXaxis->SetRange(Xf1, Xl1);
               changed = kTRUE;
            }
            Int_t newYfirst = Yfirst0 - dYbins;
            Int_t newYlast;
            if (newYfirst < 1) {
               newYfirst = 1;
               newYlast = NdisYbins;
            } else {
               newYlast = Ylast0 - dYbins;
               if (newYlast > NYbins) {
                  newYlast = NYbins;
                  newYfirst = newYlast - NdisYbins + 1;
               }
            }
            if (newYfirst != Yf1) {
               Yf1 = newYfirst;
               Yl1 = newYlast;
               theYaxis->SetRange(Yf1, Yl1);
               changed = kTRUE;
            }
            if (changed) {
               Modified();
               Update();
            }
         }


      case kButton2Up:
         if (fSelected) {
            gPad = fSelectedPad;

            if (sendOrder) fSelected->ExecuteEvent(event, px, py);
            RunAutoExec();
         }
         break;

      case kButton2Double:
         break;

      //*-*----------------------------------------------------------------------

      case kButton3Down:
         // popup context menu
         pad = Pick(px, py, prevSelObj);
         if (!pad) return;

         if (!fDoubleBuffer) FeedbackMode(kFALSE);

         if (fContextMenu && !fSelected->TestBit(kNoContextMenu) && !pad->TestBit(kNoContextMenu) && !TestBit(kNoContextMenu)) {
            if (sendOrder) fContextMenu->Popup(px, py, fSelected, this, pad);
            else fSelected->ExecuteEvent(event, px, py);
         }

         break;

      case kButton3Motion:
         break;

      case kButton3Up:
         if (!fDoubleBuffer) FeedbackMode(kTRUE);
         break;

      case kButton3Double:
         break;

      case kKeyDown:
         //       Info("HandleInput","Key down: %d %d",px,py);
         break;

      case kKeyUp:
         //       Info("HandleInput","Key up: %d %d",px,py);
         break;

      case kKeyPress:
         if (!fSelectedPad || !fSelected) return;
         gPad = fSelectedPad;   // don't use cd() because we won't draw in pad
         // we will only use its coordinate system
         fSelected->ExecuteEvent(event, px, py);

         HandleKey(px, py);
         RunAutoExec();

         break;
      case kButton1Shift:
         // Try to select
         pad = Pick(px, py, prevSelObj);

         if (!pad) return;

         EnterLeave(prevSelPad, prevSelObj);

         gPad = pad;   // don't use cd() we will use the current
         // canvas via the GetCanvas member and not via
         // gPad->GetCanvas
         fSelected->ExecuteEvent(event, px, py);
         RunAutoExec();

         break;
      case kWheelUp:
      case kWheelDown:
         pad = Pick(px, py, prevSelObj);
         if (!pad) return;

         sign = (event == kWheelUp ? 1 : -1);

         gPad = pad;
         if (fSelected->InheritsFrom("TAxis")) fSelected->ExecuteEvent(event, px, py);
         else if (fSelected->InheritsFrom("TH2")) DynamicZoom(sign, px, py);

         RunAutoExec();

         break;
      default:
         break;
   }

   if (fPadSave && event != kButton2Down)
      fPadSave->cd();

   if (event != kMouseLeave) { // signal was already emitted for this event
      ProcessedEvent(event, px, py, fSelected);  // emit signal
      DrawEventStatus(event, px, py, fSelected);
   }
}

//________________________________________________________________
void KVCanvas::ZoomSelected(TH2* TheHisto)
{
   if (!TheHisto) return;
   TAxis* ax = TheHisto->GetXaxis();

   Double_t ratio1 = (xmin - GetUxmin()) / (GetUxmax() - GetUxmin());
   Double_t ratio2 = (xmax - GetUxmin()) / (GetUxmax() - GetUxmin());
   if ((ratio2 - ratio1 > 0.05)) ax->SetRangeUser(xmin, xmax);

   ax = TheHisto->GetYaxis();

   ratio1 = (ymin - GetUymin()) / (GetUymax() - GetUymin());
   ratio2 = (ymax - GetUymin()) / (GetUymax() - GetUymin());
   if ((ratio2 - ratio1 > 0.05)) ax->SetRangeUser(ymin, ymax);

   xmax = xmin = ymax = ymin = 0.;
   return;
}

//________________________________________________________________
void KVCanvas::DynamicZoomTH1(Int_t Sign, Int_t px, Int_t)
{
   // Zoom in or out of histogram with mouse wheel

   if (!fSelected) return;
   TH1* TheHisto = (TH1*) FindHisto();//fSelected;

   Double_t percent = 0.15 - Sign * 0.05;

   Int_t dX = 0;

   px = AbsPixeltoX(px);

   TAxis* ax = TheHisto->GetXaxis();
   Int_t NbinsXtmp = ax->GetNbins();
   Int_t X0tmp = ax->GetFirst();
   Int_t X1tmp = ax->GetLast();
   Int_t step = TMath::Min(TMath::Max(1, (Int_t)(percent * (X1tmp - X0tmp))), NbinsXtmp / 2);
   step *= Sign;
   X0tmp = TMath::Min(TMath::Max(X0tmp + step, 1), X1tmp - step);
   X1tmp = TMath::Max(TMath::Min(X1tmp - step, NbinsXtmp), X0tmp);
   if (X0tmp >= X1tmp) X0tmp = X1tmp - 1;
   if (Sign > 0) dX = (Int_t)(X0tmp + (X1tmp - X0tmp) * 0.5 - ax->FindBin(px));
   if ((X0tmp - dX) < 0) ax->SetRange(0, X1tmp - X0tmp);
   else if ((X1tmp - dX) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (X1tmp - X0tmp), ax->GetNbins());
   else ax->SetRange(X0tmp - dX, X1tmp - dX);


   Modified();
   Update();
   return;
}

//________________________________________________________________
void KVCanvas::DynamicZoom(Int_t Sign, Int_t px, Int_t py)
{
   // Zoom in or out of histogram with mouse wheel

   //    Info("DynamicZoom","px=%d py=%d",px,py);

   if (!fSelected) return;
   TH2* TheHisto = (TH2*) FindHisto();//fSelected;

   Double_t percent = 0.15 - Sign * 0.05;

   Int_t dX = 0;
   Int_t dY = 0;

   Double_t ppx = AbsPixeltoX(px);
   Double_t ppy = AbsPixeltoY(py);

   TAxis* ax = TheHisto->GetXaxis();
   Int_t NbinsXtmp = ax->GetNbins();
   Int_t X0tmp = ax->GetFirst();
   Int_t X1tmp = ax->GetLast();
   Int_t step = TMath::Min(TMath::Max(1, (Int_t)(percent * (X1tmp - X0tmp))), NbinsXtmp / 2);
   step *= Sign;
   X0tmp = TMath::Min(TMath::Max(X0tmp + step, 1), X1tmp - step);
   X1tmp = TMath::Max(TMath::Min(X1tmp - step, NbinsXtmp), X0tmp);
   if (X0tmp >= X1tmp) X0tmp = X1tmp - 1;
   if (Sign > 0) dX = (Int_t)(X0tmp + (X1tmp - X0tmp) * 0.5 - ax->FindBin(ppx));
   if ((X0tmp - dX) < 0) ax->SetRange(0, X1tmp - X0tmp);
   else if ((X1tmp - dX) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (X1tmp - X0tmp), ax->GetNbins());
   else ax->SetRange(X0tmp - dX, X1tmp - dX);

   ax = TheHisto->GetYaxis();
   Int_t NbinsYtmp = ax->GetNbins();
   Int_t Y0tmp = ax->GetFirst();
   Int_t Y1tmp = ax->GetLast();
   step = TMath::Min(TMath::Max(1, (Int_t)(percent * (Y1tmp - Y0tmp))), NbinsYtmp / 2);
   step *= Sign;
   Y0tmp = TMath::Min(TMath::Max(Y0tmp + step, 1), Y1tmp - step);
   Y1tmp = TMath::Max(TMath::Min(Y1tmp - step, NbinsYtmp), Y0tmp);
   if (Y0tmp >= Y1tmp) Y0tmp = Y1tmp - 1;
   if (Sign > 0) dY = (Int_t)(Y0tmp + (Y1tmp - Y0tmp) * 0.5 - ax->FindBin(ppy));
   if ((Y0tmp - dY) < 0) ax->SetRange(0, Y1tmp - Y0tmp);
   else if ((Y1tmp - dY) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (Y1tmp - Y0tmp), ax->GetNbins());
   else ax->SetRange(Y0tmp - dY, Y1tmp - dY);

   Modified();
   Update();
   return;
}

//________________________________________________________________
Bool_t KVCanvas::HandleKey(Int_t, Int_t py)
{
   // Handle keys

//    Info("HandleKey","key pressed : %d %d",px,py);

   TObject* obj = 0;
   TIter next(GetListOfPrimitives());

   if (!fEnabledShortcuts) return kTRUE;

   if (fSelected->InheritsFrom("TFrame")) fSelected = FindHisto();
   if (fSelected->InheritsFrom("TH2"))    fSelected = FindHisto();
   if (!fSelected) return kTRUE;

   switch ((EKeySym)py) {
      case kKey_F1:
         break;

      case kKey_F2:
         break;

      case kKey_F3:
         break;

      case kKey_F4:
         break;

      case kKey_F5:
         break;

      case kKey_F6:
         break;

      case kKey_F7:
         break;

      case kKey_F8:
         break;

      case kKey_F9:
         SetLogx(!fLogx);
         Modified();
         Update();
         break;

      case kKey_F10:
         SetLogy(!fLogy);
         Modified();
         Update();
         break;

      case kKey_F11:
         SetLogz(!fLogz);
         Modified();
         Update();
         break;

      case kKey_F12:
         if (fSelected->InheritsFrom("TH1")) {
            gPad->cd();
            ((TH1*)fSelected)->GetXaxis()->UnZoom();
            ((TH1*)fSelected)->GetYaxis()->UnZoom();
            Modified();
            Update();
         } else if (fSelected->InheritsFrom("TAxis")) {
            ((TAxis*)fSelected)->UnZoom();
            Modified();
            Update();
         }
         break;

      case kKey_a:
         break;

      case kKey_b:
         break;

      case kKey_c:
         if (gCopyObject) {
            gCopyObject->Delete();
            gCopyObject = 0;
         }
         if (fSelected) gCopyObject = fSelected->Clone();
//        gCopyObject = fSelected;
         break;

      case kKey_d:
         if (fSelected->InheritsFrom("TF1")) {
            TH1* hh = 0;
            if ((hh = FindHisto())) hh->GetListOfFunctions()->Remove(fSelected);
         } else GetListOfPrimitives()->Remove(fSelected);
         Modified();
         Update();
         break;

      case kKey_e:
         GetCanvasImp()->ShowEditor(!GetCanvasImp()->HasEditor());
         break;

      case kKey_f:
         if (fSelected->InheritsFrom("TH1"))((TH1*)fSelected)->FitPanel();
         break;

      case kKey_g:
         if (GetGridx() && GetGridy()) {
            SetGrid(0, 0);
            while ((obj = next())) {
               if (obj->InheritsFrom(TPad::Class())) {
                  ((TPad*)obj)->SetGrid(0, 0);
               }
            }
         } else {
            SetGrid();
            while ((obj = next())) {
               if (obj->InheritsFrom(TPad::Class())) {
                  ((TPad*)obj)->SetGrid();
               }
            }
         }
         Modified();
         Update();
         break;

      case kKey_i:
         ShowShortcutsInfos();
         break;

      case kKey_j:
         fJPressed = kTRUE;
         return kTRUE;
         break;

      case kKey_l:
         if (fSelected->InheritsFrom("TH2"))         SetLogz(!fLogz);
         else if (fSelected->InheritsFrom("TH1"))    SetLogy(!fLogy);
         Modified();
         Update();
         break;

      case kKey_n:
         if (fSelected->InheritsFrom("TH1")) {
            ((TH1*)fSelected)->Sumw2();
            ((TH1*)fSelected)->Scale(1. / ((TH1*)fSelected)->Integral());
         }
         Modified();
         Update();
         break;

      case kKey_p:
         fPPressed = kTRUE;
         return kTRUE;
         break;

//    case kKey_q:
//        Close();
//        return kTRUE;
//        break;

      case kKey_r:
         if (ExpandFunctionRange()) {
            Modified();
            Update();
         }
         break;

      case kKey_s:
         SaveCanvasAs();
         break;

      case kKey_t:
         break;

      case kKey_u:
         Modified();
         Update();
         break;

      case kKey_v:
         if (gCopyObject) {
            cd();
            Modified();
            Update();
            TString option = "";
            if (FindHisto()) option += "same";
            gCopyObject->Draw(option.Data());
            Modified();
            Update();
            gCopyObject = 0;
         }
         break;

      case kKey_w:
         fAgeOfEmpire = !fAgeOfEmpire;
         break;

      case kKey_x:
         if (fPPressed && fSelected->InheritsFrom("TH2")) ProfileX((TH2*)fSelected);
         if (fJPressed && fSelected->InheritsFrom("TH2")) ProjectionX((TH2*)fSelected);
         if (!fPPressed && !fJPressed) {
            gCopyObject = fSelected;
            GetListOfPrimitives()->Remove(gCopyObject);
            Modified();
            Update();
         }
         break;

      case kKey_y:
         if (fPPressed && fSelected->InheritsFrom("TH2")) ProfileY((TH2*)fSelected);
         if (fJPressed && fSelected->InheritsFrom("TH2")) ProjectionY((TH2*)fSelected);
         break;

      case kKey_Left:
         if (fSelected->InheritsFrom("TAxis"))    MoveAxis((TAxis*)fSelected, -1);
         else if (fSelected->InheritsFrom("TH1")) MoveAxis(FindHisto()->GetXaxis(), -1);
         break;

      case kKey_Down:
         if (fSelected->InheritsFrom("TAxis"))    MoveAxis((TAxis*)fSelected, -1);
         else if (fSelected->InheritsFrom("TH1")) MoveAxis(FindHisto()->GetYaxis(), -1);
         break;

      case kKey_Right:
         if (fSelected->InheritsFrom("TAxis"))    MoveAxis((TAxis*)fSelected, 1);
         else if (fSelected->InheritsFrom("TH1")) MoveAxis(FindHisto()->GetXaxis(), 1);
         break;

      case kKey_Up:
         if (fSelected->InheritsFrom("TAxis"))    MoveAxis((TAxis*)fSelected, 1);
         else if (fSelected->InheritsFrom("TH1")) MoveAxis(FindHisto()->GetYaxis(), 1);
         break;

      case kKey_Plus:
         if (fSelected->InheritsFrom("TH2")) {
            ((TH2*)fSelected)->SetMinimum(((TH1*)fSelected)->GetMinimum() + 1);
            Modified();
            Update();
         } else if (fSelected->InheritsFrom("TF1")) {
            ((TF1*)fSelected)->SetNpx(((TF1*)fSelected)->GetNpx() + 50);
            Modified();
            Update();
         } else if (fSelected->InheritsFrom("TH1")) {
            TObject* obj = 0;
            TIter it(((TH1*)fSelected)->GetListOfFunctions());
            while ((obj = it())) {
               ((TF1*)obj)->SetNpx(((TF1*)obj)->GetNpx() + 50);
            }
            Modified();
            Update();
         }
         break;

      case kKey_Minus:
         if (fSelected->InheritsFrom("TH2")) {
            if (((TH1*)fSelected)->GetMinimum() > 0)((TH2*)fSelected)->SetMinimum(((TH1*)fSelected)->GetMinimum() - 1);
            Modified();
            Update();
         } else if (fSelected->InheritsFrom("TF1")) {
            ((TF1*)fSelected)->SetNpx(((TF1*)fSelected)->GetNpx() - 50);
            Modified();
            Update();
         } else if (fSelected->InheritsFrom("TH1")) {
            TObject* obj = 0;
            TIter it(((TH1*)fSelected)->GetListOfFunctions());
            while ((obj = it()))((TF1*)obj)->SetNpx(((TF1*)obj)->GetNpx() - 50);
            Modified();
            Update();
         }
         break;

      case kKey_Space:
         break;

      default:
         fPPressed = kFALSE;
         fJPressed = kFALSE;
         return kTRUE;
   }
   fPPressed = kFALSE;
   fJPressed = kFALSE;
   return kTRUE;
}

void KVCanvas::MoveAxis(TAxis* ax, Int_t sign)
{
   Int_t nBins = ax->GetNbins();
   Int_t first = ax->GetFirst();
   Int_t last  = ax->GetLast();

   Int_t dX = (last - first) / 10;
   if (dX == 0) dX++;

   if ((last + 2 < nBins) && (sign > 0)) {
      ax->SetRange(first + dX, last + dX);
      Modified();
      Update();
   }
   if ((first - 2 >= 0) && (sign < 0)) {
      ax->SetRange(first - dX, last - dX);
      Modified();
      Update();
   }
}

void KVCanvas::AddShortcutsInfo(const char* cut, const char* desc)
{
   fShortCuts.AddLast(new TNamed(cut, desc));
}

void KVCanvas::ShowShortcutsInfos()
{
   std::cout << std::endl << std::endl;
   TNamed* info = 0;
   TIter it(&fShortCuts);
   while ((info = (TNamed*)it())) {
      std::cout << Form("%20s", info->GetName()) << "   " << info->GetTitle() << std::endl;
   }
   std::cout << std::endl;
}

void KVCanvas::SetVenerMode(Int_t value)
{
   fVenerMode = value;
}

void KVCanvas::InitInfos()
{
   fEnabledShortcuts = 1;
   fSavedAs = "";

   AddShortcutsInfo("<ctrl> c", "copy the object under cursor");
   AddShortcutsInfo("<ctrl> d", "undraw the object under cursor (object not deleted)");
   AddShortcutsInfo("<ctrl> e", "show editor");
   AddShortcutsInfo("<ctrl> f", "start fit panel (TH1)");
   AddShortcutsInfo("<ctrl> g", "set/unset grid on X and Y axes");
   AddShortcutsInfo("<ctrl> i", "show shortcuts infos");
   AddShortcutsInfo("<ctrl> j x", "draw projection X (TH2)");
   AddShortcutsInfo("<ctrl> j y", "draw projection Y (TH2)");
   AddShortcutsInfo("<ctrl> l", "set/unset log scale on Y axis (TH1) or Z axis (TH2)");
   AddShortcutsInfo("<ctrl> n", "normalize histogram");
   AddShortcutsInfo("<ctrl> p x", "draw profile X (TH2)");
   AddShortcutsInfo("<ctrl> p y", "draw profile Y (TH2)");
   AddShortcutsInfo("<ctrl> s", "save canvas as");
   AddShortcutsInfo("<ctrl> u", "update canvas");
   AddShortcutsInfo("<ctrl> v", "paste");
   AddShortcutsInfo("<ctrl> w", "set/unset 'Age Of Empire' mode (TH2)");
   AddShortcutsInfo("<ctrl> x", "cut the object under cursor");
   AddShortcutsInfo("<ctrl> +", "set minimum +1 (TH2)");
   AddShortcutsInfo("<ctrl> -", "set minimum -1 (TH2)");
   AddShortcutsInfo("F9", "set/unset log scale on X axis");
   AddShortcutsInfo("F10", "set/unset log scale on Y axis");
   AddShortcutsInfo("F11", "set/unset log scale on Z axis");
   AddShortcutsInfo("F12", "unzoom");
   AddShortcutsInfo("Arrows", "move on histogram or axis");
}

void KVCanvas::ProfileX(TH2* hh)
{
   TObject* pfx = 0;
   if ((pfx = FindObject(Form("%s_pfx", hh->GetName())))) pfx->Delete();
   hh->ProfileX("_pfx", 1, -1, "i,d,same");
   if ((pfx = FindObject(Form("%s_pfx", hh->GetName()))))((TProfile*)pfx)->SetLineColor(kBlack);
   Modified();
   Update();
}

void KVCanvas::ProfileY(TH2* hh)
{
   TObject* obj = 0;
   if ((obj = gROOT->FindObject(Form("%s_pfy", hh->GetName())))) obj->Delete();
   TProfile* pfy = hh->ProfileY("_pfy", 1, -1, "i");
   TGraphErrors* gr = gHistoManipulator->MakeGraphFrom(pfy);
   pfy->Delete();
   TGraph* gg = gHistoManipulator->PermuteAxis(gr);
   gr->Delete();
   gg->SetName(Form("%s_pfy", hh->GetName()));
   gg->SetLineColor(kBlack);
   gg->Draw("PEZ");
   Modified();
   Update();
}

void KVCanvas::SaveCanvasAs()
{

   if (strcmp("", fSavedAs)) {
      Int_t ret_val;
      TString file = fSavedAs.Data();
      file.ReplaceAll(gSystem->DirName(fSavedAs.Data()), "");
      file.ReplaceAll("/", "");
      new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "File name exist",
                   Form("File name '%s' already exists, OK to owerwrite it?", file.Data()),
                   kMBIconExclamation, kMBOk | kMBCancel, &ret_val);

      if (ret_val & kMBOk) {
         fCanvas->SaveAs(fSavedAs);
         return;
      }
   }

   const char* SaveAsTypes[] = {
      "PDF",          "*.pdf",
      "PostScript",   "*.ps",
      "Encapsulated PostScript", "*.eps",
      "SVG",          "*.svg",
      "TeX",          "*.tex",
      "GIF",          "*.gif",
      "ROOT macros",  "*.C",
      "ROOT files",   "*.root",
      "XML",          "*.xml",
      "PNG",          "*.png",
      "XPM",          "*.xpm",
      "JPEG",         "*.jpg",
      "TIFF",         "*.tiff",
      "XCF",          "*.xcf",
      "All files",    "*",
      0,              0
   };

   TString workdir = gSystem->WorkingDirectory();
   static TString dir(".");
   static Int_t typeidx = 0;
   static Bool_t overwr = kFALSE;
   TGFileInfo fi;
   fi.fFileTypes   = SaveAsTypes;
   fi.fIniDir      = StrDup(dir);
   fi.fFileTypeIdx = typeidx;
   fi.fOverwrite   = overwr;
   new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
   gSystem->ChangeDirectory(workdir.Data());
   if (!fi.fFilename) return;
   TString fn = fi.fFilename;
   dir     = fi.fIniDir;
   typeidx = fi.fFileTypeIdx;
   overwr  = fi.fOverwrite;
   fCanvas->SaveAs(fn);
   fSavedAs = fn;
}

TH1* KVCanvas::FindHisto()
{
   TObject* hh = 0;
   TIter it(GetListOfPrimitives());
   while ((hh = (TObject*)it())) {
      if (hh->InheritsFrom("TH1")) return (TH1*) hh;
   }
   return 0;
}

Bool_t KVCanvas::ExpandFunctionRange()
{
   Bool_t up = kFALSE;
   TH1* hh = FindHisto();
   if (!hh) return up;
   TObject* obj = 0;
   TIter it(hh->GetListOfFunctions());
   while ((obj = it())) {
      ((TF1*)obj)->SetRange(hh->GetXaxis()->GetXmin(), hh->GetXaxis()->GetXmax());
      up = kTRUE;
   }
   return up;
}

void KVCanvas::SetEnabledShortcuts(Int_t value)
{
   fEnabledShortcuts = value;
}

void KVCanvas::ProjectionX(TH2* hh)
{
   TString pname = Form("%s_px", hh->GetName());
   Int_t ip = 1;
   while (gROOT->FindObject(pname.Data())) {
      pname = Form("%s_px%d", hh->GetName(), ip);
      ip++;
   }

   TH1* px = hh->ProjectionX(pname.Data());
   if (!px) return;
   Double_t minY = (hh->GetYaxis()->GetXmin());
   Double_t maxY = (hh->GetYaxis()->GetXmax());
   Double_t dY = (maxY - minY) * 0.8;

   Double_t maxH = px->GetBinContent(px->GetMaximumBin());

   TGraph* gg = 0;
   if ((gg = (TGraph*)gROOT->FindObject(Form("%s_gjx", hh->GetName())))) gg->Delete();

   gg = new TGraph;
   for (int i = 0; i < px->GetNbinsX(); i++) {
      gg->SetPoint(i, px->GetBinCenter(i), minY + px->GetBinContent(i)*dY / maxH);
   }

   gg->SetName(Form("%s_gjx", hh->GetName()));
   gg->SetTitle(Form("%s_gjx", hh->GetName()));
   gg->SetLineColor(kBlack);
   gg->SetMarkerColor(kBlack);
   gg->SetMarkerStyle(8);
   gg->Draw("PL");

   Modified();
   Update();
}

void KVCanvas::ProjectionY(TH2* hh)
{
   TString pname = Form("%s_py", hh->GetName());
   Int_t ip = 1;

   while (gROOT->FindObject(pname.Data())) {
      pname = Form("%s_py%d", hh->GetName(), ip);
      ip++;
   }

   TH1* py = hh->ProjectionY(pname.Data());
   if (!py) return;
   Double_t minY = (hh->GetXaxis()->GetXmin());
   Double_t maxY = (hh->GetXaxis()->GetXmax());
   Double_t dY = (maxY - minY) * 0.8;

   Double_t maxH = py->GetBinContent(py->GetMaximumBin());

   TGraph* gg = 0;
   if ((gg = (TGraph*)gROOT->FindObject(Form("%s_gjy", hh->GetName())))) gg->Delete();

   gg = new TGraph;
   for (int i = 0; i < py->GetNbinsX(); i++) {
      gg->SetPoint(i, minY + py->GetBinContent(i)*dY / maxH, py->GetBinCenter(i));
   }

   gg->SetName(Form("%s_gjy", hh->GetName()));
   gg->SetTitle(Form("%s_gjy", hh->GetName()));
   gg->SetLineColor(kBlack);
   gg->SetMarkerColor(kBlack);
   gg->SetMarkerStyle(8);
   gg->Draw("PL");

   Modified();
   Update();
}








