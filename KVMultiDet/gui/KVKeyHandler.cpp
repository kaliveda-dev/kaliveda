//Created by KVClassFactory on Fri Feb 17 17:47:35 2012
//Author: dgruyer

#include "KVKeyHandler.h"
#include "TROOT.h"
#include "TRootCanvas.h"
#include "TGWindow.h"
#include <KeySymbols.h>
#include "TVirtualX.h"
#include <Riostream.h>

using namespace std;

ClassImp(KVKeyHandler)


KVKeyHandler::KVKeyHandler(KVCanvas* Canvas) : TGFrame(gClient->GetRoot(), 0, 0)
{
   // Key handler constructor.
   if (!Canvas) {
      cout << "WARNING: KVKeyHandler::KVKeyHandler(): invalid canvas!" << endl;
      return;
   }

   fCanvas = Canvas;

   TRootCanvas* main_frame = (TRootCanvas*)(fCanvas->GetCanvasImp());

   // bind arrow keys and space-bar key
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_Up),    0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_Left),  0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_Right), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_Down),  0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_Space), 0);

   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F1), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F2), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F3), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F4), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F5), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F6), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F7), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F8), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F9), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F10), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F11), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_F12), 0);

   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_a), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_b), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_c), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_d), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_e), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_f), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_g), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_h), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_i), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_j), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_k), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_l), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_m), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_n), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_o), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_p), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_q), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_r), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_s), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_t), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_u), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_v), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_w), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_x), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_y), 0);
   main_frame->BindKey((const TGWindow*)this, gVirtualX->KeysymToKeycode(kKey_z), 0);

   gVirtualX->SetKeyAutoRepeat(kTRUE);
}

KVKeyHandler::~KVKeyHandler()
{
   // Cleanup key handler.

   // get main frame of fCanvas
   TRootCanvas* main_frame = (TRootCanvas*)(fCanvas->GetCanvasImp());

   // remove binding of arrow keys and space-bar key
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_Up),    0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_Left),  0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_Right), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_Down),  0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_Space), 0);

   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F1), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F2), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F3), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F4), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F5), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F6), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F7), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F8), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F9), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F10), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F11), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_F12), 0);

   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_a), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_b), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_c), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_d), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_e), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_f), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_g), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_h), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_i), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_j), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_k), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_l), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_m), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_n), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_o), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_p), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_q), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_r), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_s), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_t), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_u), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_v), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_w), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_x), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_y), 0);
   main_frame->RemoveBind(this, gVirtualX->KeysymToKeycode(kKey_z), 0);

   gVirtualX->SetKeyAutoRepeat(kTRUE);
}


Bool_t KVKeyHandler::HandleKey(Event_t*)
{
   // Handle keys
//   if(fCanvas) fCanvas->HandleKey(event);
   return kTRUE;
}
