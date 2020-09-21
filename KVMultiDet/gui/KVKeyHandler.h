//Created by KVClassFactory on Fri Feb 17 17:47:35 2017
//Author: dgruyer

#ifndef __KVKeyHandler_H
#define __KVKeyHandler_H

#include "TCanvas.h"
#include "KVCanvas.h"
#include "TGFrame.h"
#include "TGClient.h"

/**
\class KVKeyHandler
\brief Utility class to redirect keyboard events to KVCanvas
\ingroup GUI

\note Numerical keypad must be deactivated for signals to pass
 */
class KVKeyHandler : public TGFrame {

protected:
   KVCanvas* fCanvas;

public:
//   KVKeyHandler() : TGFrame(gClient->GetRoot(),0,0){};
   KVKeyHandler(KVCanvas* Canvas);
   ~KVKeyHandler();

//   void SetCanvas(KVCanvas* Canvas){};
   Bool_t HandleKey(Event_t* event);
   ClassDef(KVKeyHandler, 1) // handler of the key events
};

#endif
