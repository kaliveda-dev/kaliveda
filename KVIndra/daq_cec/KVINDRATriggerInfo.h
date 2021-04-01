/***************************************************************************
                          kvindratriggerinfo.h  -  description
                             -------------------
    begin                : 28 sep 2005
    copyright            : (C) 2005 jdf
    email                : frankland@ganil.fr
$Id: KVINDRATriggerInfo.h,v 1.7 2007/06/28 16:00:49 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __KVINDRATriggerInfo_H
#define  __KVINDRATriggerInfo_H

#include "Binary_t.h"
#include "KVEBYEDAT_ACQParam.h"

//bit numbers in STAT_EVE register
#define BIT_PHY_EVT 3
#define BIT_MRQ 5
#define BIT_GEN_ELEC 1
#define BIT_GEN_TST 0
#define BIT_GEN_LAS 2

/**
  \class KVINDRATriggerInfo
  \brief Information on INDRA event from DAQ trigger
  \ingroup DAQ

Utility class, holds values of the `STAT_EVE`, `R_DEC` and `CONFIG` parameters
in raw INDRA data which represent the status of the Selecteur for an event.

When reading raw data, the values of the three parameters
are filled from the data event by event.
For calibrated data, this information is accessible by using

     KVINDRAReconEvent::EventTrigger()

which returns a KVTriggerInfo object.

GetSTAT_EVE(), GetR_DEC(), GetCONFIG() : binary contents of each register

PHY_EVT(), MRQ(), GEN_ELEC(), GEN_TST(), GEN_LAS() :  kTRUE/kFALSE depending on state of corresponding bit in STAT_EVE

IsGene() : kTRUE if any of the three GEN_xxx bits is set to 1

IsPhysics() : kTRUE if PHY_EVT bit = 1 and MRQ bit is not set*

(*because if MRQ=1 physical events cannot trigger the acquisition).
*/

class KVINDRATriggerInfo {

   Binary16_t fSTAT_EVE;        // value of register STAT_EVE for event
   Binary16_t fR_DEC;           // value of register R_DEC for event
   Binary16_t fVXCONFIG;        // value of register VXCONFIG for event
   KVEBYEDAT_ACQParam* fSTAT_EVE_PAR;   // STAT_EVE parameter read from raw data
   KVEBYEDAT_ACQParam* fR_DEC_PAR;      // R_DEC parameter read from raw data
   KVEBYEDAT_ACQParam* fVXCONFIG_PAR;   // VXCONFIG parameter read from raw data

public:

   KVINDRATriggerInfo();
   virtual ~ KVINDRATriggerInfo()
   {
   };

   void SetSTAT_EVE_PAR(KVEBYEDAT_ACQParam* p)
   {
      fSTAT_EVE_PAR = p;
   };
   void SetR_DEC_PAR(KVEBYEDAT_ACQParam* p)
   {
      fR_DEC_PAR = p;
   };
   void SetVXCONFIG_PAR(KVEBYEDAT_ACQParam* p)
   {
      fVXCONFIG_PAR = p;
   };

   inline void SetSTAT_EVE(Binary16_t);
   inline void SetR_DEC(Binary16_t);
   inline void SetCONFIG(Binary16_t);

   inline Binary16_t GetSTAT_EVE();
   inline Binary16_t GetR_DEC();
   inline Binary16_t GetCONFIG();

   inline Bool_t IsPhysics()
   {
      return (PHY_EVT() && !MRQ());
   };
   inline Bool_t IsGene()
   {
      return (GEN_ELEC() || GEN_TST() || GEN_LAS());
   };

   inline Bool_t PHY_EVT();
   inline Bool_t MRQ();
   inline Bool_t GEN_ELEC();
   inline Bool_t GEN_TST();
   inline Bool_t GEN_LAS();
   inline Bool_t IsPulser()
   {
      return GEN_ELEC();
   };
   inline Bool_t IsLaser()
   {
      return GEN_LAS();
   };
   inline Bool_t IsTest()
   {
      return GEN_TST();
   };
   virtual Bool_t IsINDRAEvent()
   {
      //Valid INDRA events have the acquisition parameter STAT_EVE present i.e. not equal to -1.
      //Test this after reading a DAQ event to know if a valid INDRA event has been read.
      return fSTAT_EVE_PAR->Fired();
   };

   void Print(Option_t* opt = "");

   ClassDef(KVINDRATriggerInfo, 3)      //Information on INDRA event from DAQ trigger
};

//_________ inline methodes _______________

inline Binary16_t KVINDRATriggerInfo::GetSTAT_EVE()
{
   //Returns value of STAT_EVE read from raw data
   fSTAT_EVE = fSTAT_EVE_PAR->GetData();
   return fSTAT_EVE;
}
inline Binary16_t KVINDRATriggerInfo::GetR_DEC()
{
   //Returns value of R_DEC read from raw data
   fR_DEC = fR_DEC_PAR->GetData();
   return fR_DEC;
}
inline Binary16_t KVINDRATriggerInfo::GetCONFIG()
{
   //Returns value of CONFIG read from raw data
   fVXCONFIG = fVXCONFIG_PAR->GetData();
   return fVXCONFIG;
}

inline Bool_t KVINDRATriggerInfo::PHY_EVT()
{
   return GetSTAT_EVE().TestBit(BIT_PHY_EVT);
}
inline Bool_t KVINDRATriggerInfo::MRQ()
{
   return GetSTAT_EVE().TestBit(BIT_MRQ);
}
inline Bool_t KVINDRATriggerInfo::GEN_ELEC()
{
   return GetSTAT_EVE().TestBit(BIT_GEN_ELEC);
}
inline Bool_t KVINDRATriggerInfo::GEN_TST()
{
   return GetSTAT_EVE().TestBit(BIT_GEN_TST);
}
inline Bool_t KVINDRATriggerInfo::GEN_LAS()
{
   return GetSTAT_EVE().TestBit(BIT_GEN_LAS);
}
inline void KVINDRATriggerInfo::SetSTAT_EVE(Binary16_t b)
{
   fSTAT_EVE = b;
};
inline void KVINDRATriggerInfo::SetR_DEC(Binary16_t b)
{
   fR_DEC = b;
};
inline void KVINDRATriggerInfo::SetCONFIG(Binary16_t b)
{
   fVXCONFIG = b;
};

#endif
