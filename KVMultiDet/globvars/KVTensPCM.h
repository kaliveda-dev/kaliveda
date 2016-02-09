//$Id: KVTensPCM.h,v 1.1 2007/11/27 08:05:30 franklan Exp $
//J.D. Frankland
//1/3/2004
//Calculate tensor in reaction CM Frame
//

#ifndef KVTensPCM_h
#define KVTensPCM_h
#include "KVTensP.h"

class KVTensPCM: public KVTensP {
public:
   KVTensPCM(void);             // constructeur par defaut
   KVTensPCM(Char_t* nom);

   virtual ~ KVTensPCM(void);  // destructeur

   ClassDef(KVTensPCM, 1)      // Global variable CM Momentum tensor
};
#endif
