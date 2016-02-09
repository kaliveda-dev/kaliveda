/*
$Id: GTGanilDataVAMOS.h,v 1.2 2007/06/08 15:49:10 franklan Exp $
$Revision: 1.2 $
$Date: 2007/06/08 15:49:10 $
*/

//Created by KVClassFactory on Thu May 31 15:04:01 2007
//Author: e_ivamos

#ifndef __GTGANILDATAVAMOS_H
#define __GTGANILDATAVAMOS_H

#include "GTGanilData.h"
#include "Parameters.h"

class GTGanilDataVAMOS : public GTGanilData {
protected:

   Parameters* Par;//->list of acquisition parameters
   virtual bool EventUnravelling(CTRL_EVENT*);
   virtual void ReadParameters(void);

public:

   GTGanilDataVAMOS();
   GTGanilDataVAMOS(const TString filename);
   virtual ~GTGanilDataVAMOS();
   virtual void SetUserTree(TTree*);

   ClassDef(GTGanilDataVAMOS, 1) //Reads and formats raw data from INDRA-VAMOS experiments
};

#endif
