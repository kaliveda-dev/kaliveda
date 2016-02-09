/*
$Id: KVReconIdent_e475s.h,v 1.1 2007/11/16 15:41:55 ebonnet Exp $
$Revision: 1.1 $
$Date: 2007/11/16 15:41:55 $
*/

//Created by KVClassFactory on Tue Nov 13 10:47:51 2007
//Author: bonnet

#ifndef __KVRECONIDENT_E475S_H
#define __KVRECONIDENT_E475S_H

#include "KVINDRAReconIdent.h"

class KVReconIdent_e475s : public KVINDRAReconIdent {
public:

   KVReconIdent_e475s();
   virtual ~KVReconIdent_e475s();
   virtual Bool_t Analysis();

   ClassDef(KVReconIdent_e475s, 1) //Identification of E475S events
};

#endif
