//Created by KVClassFactory on Fri Feb 17 17:47:35 2012
//Author: dgruyer

#ifndef __KVIDGridEditorCanvas_H
#define __KVIDGridEditorCanvas_H

#include "TCanvas.h"
#include "KVCanvas.h"
#include "TGFrame.h"

class KVIDGridEditorCanvas : public KVCanvas {
//friend class KeyHandler;

   // variables for pan & scan
   Int_t X0, Y0;  // coordinates of initial click in pad pixels
   Int_t NdisXbins, NdisYbins; // number of displayed bins on X & Y
   Int_t NXbins, Xfirst0, Xlast0; // number of bins on x-axis, initial displayed bins
   Int_t NYbins, Yfirst0, Ylast0; // number of bins on y-axis, initial displayed bins
   TAxis* theXaxis, *theYaxis;  // the axes of the histogram
   Double_t XbinPixel, YbinPixel; // size of bins in pixels
   Int_t Xf1, Xl1, Yf1, Yl1; // last modification to axis limits

//   KeyHandler       *fKeyHandler;         // handler for arrow keys

protected:
//   Bool_t HandleKey(Event_t *event){return kTRUE;}
   virtual Bool_t HandleKey(Int_t px, Int_t py);

public:
   KVIDGridEditorCanvas();
   KVIDGridEditorCanvas(const char* name, const char* title, Int_t ww, Int_t wh);//:KVCanvas(name,title,ww,wh);

   virtual ~KVIDGridEditorCanvas();

   void HandleInput(EEventType event, Int_t px, Int_t py);

   ClassDef(KVIDGridEditorCanvas, 1) //bidouille
};

#endif
