//Created by KVClassFactory on Mon Jan 16 13:55:08 2012
//Author: bonnet

#ifndef __KVIDCHIOSILI_H
#define __KVIDCHIOSILI_H

#include "KVIDChIoSi.h"
#include "KVINDRACodes.h"

/**
  \class KVIDChIoSiLi
  \ingroup INDRAIdent
  \brief Identification in ChIo-Si(Li) (etalon) telescopes of INDRA
 */
class KVIDChIoSiLi : public KVIDChIoSi {

public:
   KVIDChIoSiLi();
   KVIDChIoSiLi(const KVIDChIoSiLi&) ;
   virtual ~KVIDChIoSiLi();
   void Copy(TObject&) const;

   ClassDef(KVIDChIoSiLi, 1) //Identification in ChIo-SiLi matrices of INDRA
};

#endif
