/***************************************************************************
$Id: KVDBPeak.cpp,v 1.4 2007/04/02 17:43:38 ebonnet Exp $
                          KVDBPeak.cpp  -  description
                             -------------------
    begin                : dec 5 2003
    copyright            : (C) 2003 by J.D.Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVDBPeak.h"

ClassImp(KVDBPeak)
void KVDBPeak::init()
{
   //default initialisations
   fAvecGaz = kTRUE;
   fGain = 1.0;                 //gain of ampli for Si = 1.00 or 1.41
   fEnergy = 0.0;               //theoretical energy of peak
   fDetector = "";
   fSignal = "";
   fRing = 0;
   fModule = 0;
   SetRunList("");
   SetRingList("");
   SetModuleList("");
}

KVDBPeak::KVDBPeak()
{
   //default ctor
   init();
}

KVDBPeak::KVDBPeak(const Char_t* detname, const Char_t* type,
                   UShort_t pnum): KVDBParameterSet(Form("%s_%s", detname,
                            type), type, pnum)
{
   //create a calibration peak of type "type" for detector "detname" with pnum parameters
   //the name of the peak is "detname_type"

   init();
   SetDetector(detname);
}
