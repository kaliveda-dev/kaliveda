/***************************************************************************
$Id: KVDBElasticPeak.h,v 1.3 2006/10/19 14:32:43 franklan Exp $
                          KVDBElasticPeak.h  -  description
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
#ifndef __KVDBELASTIC_H
#define __KVDBELASTIC_H

#include "KVDBPeak.h"

class KVDBElasticPeak: public KVDBPeak {

private:

   UChar_t fZbeam;              //Z of projectile
   UChar_t fAbeam;              //A of projectile

public:

   KVDBElasticPeak();
   KVDBElasticPeak(const Char_t* detname);
   virtual ~ KVDBElasticPeak()
   {
   };

   void init();

   void SetZproj(UChar_t z)
   {
      fZbeam = z;
   };
   UChar_t GetZproj() const
   {
      return fZbeam;
   };
   void SetAproj(UChar_t a)
   {
      fAbeam = a;
   };
   UChar_t GetAproj() const
   {
      return fAbeam;
   };

   ClassDef(KVDBElasticPeak, 1) //elastic peaks for calibration
};

#endif
