/***************************************************************************
                          kvindrafnl.h  -  description
                             -------------------
    begin                : Wed 29th Oct 2003
    copyright            : (C) 2003 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVINDRAFNL.h,v 1.7 2007/03/02 14:36:03 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVINDRAFNL_H
#define KVINDRAFNL_H

#include "KVINDRA4.h"

/**
\class KVINDRAFNL
\brief Describes INDRA as used in FNL experiment E416 (June-July 2003)
\ingroup INDRAGeometry
 */

class KVINDRAFNL: public KVINDRA4 {

   virtual void MakeListOfDetectorTypes();
   virtual void PrototypeTelescopes();
   virtual void BuildGeometry();

public:

   KVINDRAFNL();
   virtual ~ KVINDRAFNL();

   virtual void Build(Int_t run = -1);

   KVINDRADetector* GetDetectorByType(UInt_t cou, UInt_t mod,
                                      UInt_t type) const;

   ClassDef(KVINDRAFNL, 1)     //INDRA multidetector array for FNL experiment
};

#endif
