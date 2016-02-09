/***************************************************************************
                          kvindratriggerinfo.cpp  -  description
                             -------------------
    begin                : 28 sep 2005
    copyright            : (C) 2005 jdf
    email                : frankland@ganil.fr
$Id: KVINDRATriggerInfo.cpp,v 1.5 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVINDRATriggerInfo.h"
#include "Hexa_t.h"

using namespace std;

ClassImp(KVINDRATriggerInfo)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Utility class, holds values of the STAT_EVE, R_DEC and CONFIG parameters
//in raw INDRA data which represent the status of the Selecteur for an event.
//
//When reading raw data, the values of the three parameters
//are filled from the data event by event.
//For calibrated data, this information is accessible by using
//      KVINDRAReconEvent::EventTrigger()
//which returns a KVTriggerInfo object.
//
//GetSTAT_EVE(), GetR_DEC(), GetCONFIG() : binary contents of each register
//PHY_EVT(), MRQ(), GEN_ELEC(), GEN_TST(), GEN_LAS() :  kTRUE/kFALSE depending on state of corresponding bit in STAT_EVE
//IsGene() : kTRUE if any of the three GEN_xxx bits is set to 1
//IsPhysics() : kTRUE if PHY_EVT bit = 1 and MRQ bit is not set*
//(*because if MRQ=1 physical events cannot trigger the acquisition).
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
KVINDRATriggerInfo::KVINDRATriggerInfo()
{
   //Initialises number of bits (representation) for each register read from Selecteur
   fSTAT_EVE.SetNBits(6);
   fR_DEC.SetNBits(9);
   fVXCONFIG.SetNBits(14);
   fSTAT_EVE_PAR = fR_DEC_PAR = fVXCONFIG_PAR = 0;
};

void KVINDRATriggerInfo::Print(Option_t*)
{
   //Print contents of trigger registers, in binary, hexadecimal and decoded forms

   cout << "/************ KVINDRATriggerInfo ************/" << endl;
   cout << "    STAT_EVE = " << (const Char_t*) GetSTAT_EVE().Hexa() <<
        " / " << (const Char_t*) GetSTAT_EVE() << endl;
   cout << "    R_DEC    = " << (const Char_t*) GetR_DEC().Hexa() <<
        " / " << (const Char_t*) GetR_DEC() << endl;
   cout << "    CONFIG   = " << (const Char_t*) GetCONFIG().Hexa() <<
        " / " << (const Char_t*) GetCONFIG() << endl;
   cout << "/********************************************/" << endl;
   cout << "  IsPhysics()=" << IsPhysics() << ", IsGene()=" << IsGene() <<
        ", MRQ=" << MRQ() << endl;
   cout << "  PHY_EVT=" << PHY_EVT() << "  GEN_ELEC=" << GEN_ELEC() <<
        ", GEN_TST=" << GEN_TST() << ", GEN_LAS=" << GEN_LAS() << endl;
   cout << "/********************************************/" << endl;
}
