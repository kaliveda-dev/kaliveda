//Created by KVClassFactory on Fri Feb 17 17:47:35 2012
//Author: dgruyer

#ifndef __KVCanvas_H
#define __KVCanvas_H

#include "TCanvas.h"
#include "TH2.h"
#include "TGFrame.h"
#include "TList.h"
#include "TGFileDialog.h"
#include "TSystem.h"
#include "TGClient.h"

/**
   \class KVCanvas
\brief TCanvas with mouse-controlled dynamic zoom and pan & scan
\ingroup GUI

\image html http://indra.in2p3.fr/KaliVedaDoc/images/KVCanvas.png

#### MOUSE ACTIONS

  - Dynamic zoom: left-click to draw rectangle around region of interest, release mouse button to zoom in<br>
  - Zoom in: mouse wheel up<br>
  - Zoom out: mouse wheel down<br>
  - Pan: hold down mouse wheel (centre button) and move

#### KEYBOARD SHORTCUTS

  - `<ctrl> e`   show editor
  - `<ctrl> f`   start fit panel (TH1)
  - `<ctrl> g`   set/unset grid on X and Y axes
  - `<ctrl> i`   show shortcuts infos
  - `<ctrl> l`   set/unset log scale on Y axis (TH1) or Z axis (TH2)
  - `<ctrl> n`   normalize drawn histogram to its integral
  - `<ctrl> p x`   draw profile X (TH2)
  - `<ctrl> p y`   draw profile Y (TH2)
  - `<ctrl> s`   save canvas as
  - `<ctrl> u`   update canvas
  - `<ctrl> v`   set/unset 'vener' mode (TH2)
  - `<ctrl> w`   set/unset 'Age Of Empire' mode (TH2)
  - `<ctrl> +`   set minimum +1 (TH2)
  - `<ctrl> -`   set minimum -1 (TH2)
  - `F9`   set/unset log scale on X axis
  - `F10`   set/unset log scale on X axis
  - `F11`   set/unset log scale on X axis
  - `F12`   unzoom
  - `Arrows`   move on histogram or axis

  */

class KVCanvas : public TCanvas {
   friend class KVKeyHandler;

protected:
   Double_t xmin, xmax, ymin, ymax;
   Double_t oldx, oldy;
   //! variables for pan & scan
   Int_t X0, Y0;           //! coordinates of initial click in pad pixels
   Int_t NdisXbins, NdisYbins;      //! number of displayed bins on X & Y
   Int_t NXbins, Xfirst0, Xlast0;   //! number of bins on x-axis, initial displayed bins
   Int_t NYbins, Yfirst0, Ylast0;   //! number of bins on y-axis, initial displayed bins
   TAxis* theXaxis, *theYaxis;   //! the axes of the histogram
   Double_t XbinPixel, YbinPixel;   //! size of bins in pixels
   Int_t Xf1, Xl1, Yf1, Yl1;  //! last modification to axis limits

   Bool_t   moved;
   Bool_t   fPPressed;
   Bool_t   fJPressed;
   Bool_t   fAgeOfEmpire;
   Bool_t   fVenerMode;
   Bool_t   fHasDisabledClasses;
   TString  fDisabledClasses;
   Bool_t   fHasDisabledObject;

   TList    fDisabledObjects;
   TList    fShortCuts;
   Int_t    fEnabledShortcuts;
   TString  fSavedAs;

   TGFrame* fKeyHandler;         //! handler for keys
   Bool_t   fFreezed;

public:
   KVCanvas();
//   using TCanvas::TCanvas;
   KVCanvas(const char* name, const char* title, Int_t ww, Int_t wh, Bool_t keyHandler = kTRUE);
   KVCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid);
   virtual ~KVCanvas();

   void HandleInput(EEventType event, Int_t px, Int_t py);
   Bool_t IsLogz();
   Bool_t IsLogy();
   Bool_t IsLogx();

   void DisableClass(const char* className);
   void DisableObject(TObject* obj);
   void ResetDisabledClass();
   void ResetDisabledObject();

   void FreezCavans(Bool_t freez)
   {
      fFreezed = freez;
   }
   void ShowShortcutsInfos(); // *MENU*

   void SetVenerMode(Int_t value = 1); // *TOGGLE*
   Int_t GetVenerMode()
   {
      return fVenerMode;
   }

   void SetEnabledShortcuts(Int_t value = 1); // *TOGGLE*
   Int_t GetEnabledShortcuts()
   {
      return fEnabledShortcuts;
   }

protected:

//   virtual Bool_t HandleKey(Event_t* /*event*/){return kTRUE;}
   virtual Bool_t HandleKey(Int_t, Int_t py);

   void DynamicZoom(Int_t Sign, Int_t px, Int_t py);
   void DynamicZoomTH1(Int_t Sign, Int_t px, Int_t py);
   void RunAutoExec();
   void DrawEventStatus(Int_t event, Int_t px, Int_t py, TObject* selected);
   void ZoomSelected(TH2* TheHisto);

   void MoveAxis(TAxis* ax, Int_t sign);
   void ProfileX(TH2* hh);
   void ProfileY(TH2* hh);
   void ProjectionX(TH2* hh);
   void ProjectionY(TH2* hh);

   void SaveCanvasAs();
   Bool_t ExpandFunctionRange();
   TH1* FindHisto();

   void InitInfos();
   void AddShortcutsInfo(const char* cut, const char* desc);

   ClassDef(KVCanvas, 1) //TCanvas with mouse-controlled dynamic zoom and pan & scan
};

//................  global variable
R__EXTERN TObject* gCopyObject;

#endif
