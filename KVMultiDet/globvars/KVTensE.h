/*
$Id: KVTensE.h,v 1.1 2008/04/11 09:58:58 franklan Exp $
$Revision: 1.1 $
$Date: 2008/04/11 09:58:58 $
*/

//Created by KVClassFactory on Fri Apr 11 11:34:23 2008
//Author: John Frankland

#ifndef __KVTENSE_H
#define __KVTENSE_H

#include "KVTensP.h"

class KVTensE : public KVTensP {

public:
   KVTensE() {};
   KVTensE(const Char_t* nom): KVTensP(nom) {};
   KVTensE(const Char_t* nom, const Char_t* frame): KVTensP(nom, frame) {};
   virtual ~KVTensE() {};
   void Fill(KVNucleus* c);

   ClassDef(KVTensE, 1) //Kinetic energy tensor
};

#endif
