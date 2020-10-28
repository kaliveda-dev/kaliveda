/***************************************************************************
$Id: KVDBElasticPeak.cpp,v 1.3 2006/10/19 14:32:43 franklan Exp $
                          KVDBElasticPeak.cpp  -  description
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
#include "Riostream.h"
#include "KVDBElasticPeak.h"

ClassImp(KVDBElasticPeak)
void KVDBElasticPeak::init()
{
   //default initialisations
   fZbeam = 0;
   fAbeam = 0;
}

KVDBElasticPeak::KVDBElasticPeak()
{
   //default ctor
   init();
}

KVDBElasticPeak::KVDBElasticPeak(const Char_t* detname)
   : KVDBPeak(detname, "elastic", 7)
{
   //create elastic peak parameter set for detector "detname"
   init();
}
