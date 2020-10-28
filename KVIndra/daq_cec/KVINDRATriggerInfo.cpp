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
