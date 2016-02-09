#ifndef KVEtransLCP_h
#define KVEtransLCP_h
#include "KVEtrans.h"


class KVEtransLCP: public KVEtrans {

public:
   KVEtransLCP(void);
   KVEtransLCP(Char_t*);

   virtual ~ KVEtransLCP(void);

   virtual void Fill(KVNucleus* c);

   ClassDef(KVEtransLCP, 1)    //Global variable total transverse energy of LCP (Z < 3)
};
#endif
