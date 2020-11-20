#include "KVTemplateEvent.h"

ClassImp(KVEvent)

void KVEvent::Streamer(TBuffer& R__b)
{
   // Customised Streamer for KVEvent.
   // This is just the automatic Streamer with the addition of a call to the Clear()
   // method before reading a new object (avoid memory leaks with lists of parameters).

   if (R__b.IsReading()) {
      Clear();
      R__b.ReadClassBuffer(KVEvent::Class(), this);
   }
   else {
      R__b.WriteClassBuffer(KVEvent::Class(), this);
   }
}
