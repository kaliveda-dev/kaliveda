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
