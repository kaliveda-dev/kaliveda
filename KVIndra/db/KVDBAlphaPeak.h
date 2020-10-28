/***************************************************************************
$Id: KVDBAlphaPeak.h,v 1.4 2006/10/19 14:32:43 franklan Exp $
                          KVDBAlphaPeak.h  -  description
                             -------------------
    begin                : december 5th 2003
    copyright            : (C) 2003 by John D. Frankland
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
#ifndef __KVDBALPHA_H
#define __KVDBALPHA_H

#include "KVDBPeak.h"

#define PEAK_THORON_1 6.062
#define PEAK_THORON_2 8.785

/**
  \class KVDBAlphaPeak
\brief Wrapper for KVDBPeak describing alpha (thoron) peaks for calibration
\ingroup INDRADB

GetEnergy gives the energy of the alpha-peak in MeV.
*/

class KVDBAlphaPeak: public KVDBPeak {

public:

   KVDBAlphaPeak();
   KVDBAlphaPeak(const Char_t* detname);
   virtual ~ KVDBAlphaPeak()
   {
   };

   ClassDef(KVDBAlphaPeak, 1)   //alpha peaks for calibration
};

#endif
