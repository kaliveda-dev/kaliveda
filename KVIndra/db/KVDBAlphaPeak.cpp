/***************************************************************************
$Id: KVDBAlphaPeak.cpp,v 1.4 2006/10/19 14:32:43 franklan Exp $
                          KVDBAlphaPeak.cpp  -  description
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
#include "KVDBAlphaPeak.h"

ClassImp(KVDBAlphaPeak)
KVDBAlphaPeak::KVDBAlphaPeak()
{
   //default ctor
}

KVDBAlphaPeak::KVDBAlphaPeak(const Char_t* detname)
   : KVDBPeak(detname, "alpha", 7)
{
   //create alpha peak parameter set for detector "detname"
}
