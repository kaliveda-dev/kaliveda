//Created by KVClassFactory on Fri Nov 30 09:01:11 2012
//Author: dgruyer

#ifndef __KVSPIDERPSALINE_H
#define __KVSPIDERPSALINE_H

#include "KVSpiderLine.h"

class KVSpiderPSALine : public KVSpiderLine {

public:
   KVSpiderPSALine();
   KVSpiderPSALine(const KVSpiderPSALine&) ;
   virtual ~KVSpiderPSALine();
   void Copy(TObject&) const;

   ClassDef(KVSpiderPSALine, 1) //KVSpiderLine specialized for PSA matrix
};

#endif
