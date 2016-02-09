/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed Jun 10 16:07:32 2009
//Author: Abdelouahao Chbihi

#ifndef __KVSILICONVAMOS_H
#define __KVSILICONVAMOS_H

#include "KVSilicon.h"

class KVSiliconVamos : public KVSilicon {

public:
   KVSiliconVamos();
   KVSiliconVamos(Float_t thick);
   virtual ~KVSiliconVamos();
   const Char_t* GetName() const;
   void SetACQParams();


   ClassDef(KVSiliconVamos, 1) //KVSilicon for Vamos
};

#endif
