// $Id: GTOneScaler.H,v 1.2 2007/09/17 12:33:27 franklan Exp $
// Author: $Author: franklan $
//-*************************************************************************
//                        GTGanilData.cpp  -  Main Header to ROOTGAnilTape
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr
//////////////////////////////////////////////////////////////////////////

/**
 \class GTOneScaler
 \brief Scaler class for the scaler structure.
  \ingroup DAQ
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// CVS Log:
// ---------------------------------------------------------------------------

#ifndef GT_GTOneScaler_H
#define GT_GTOneScaler_H

#include <TObject.h>
#include "GanTape/GEN_TYPE.H"
#include "GanTape/gan_acq_buf.h"

// ---------------------------------------------------------------------------
class GTOneScaler : public TObject {
public:
   GTOneScaler(void);
   GTOneScaler(scale_struct* s);
   void Set(scale_struct* s);

   UInt_t GetCount(void) const
   {
      return (fCount);
   }
   UInt_t GetStatus(void) const
   {
      return (fStatus);
   }
   UInt_t GetFrequency(void) const
   {
      return (fFreq);
   }
   UInt_t GetLabel(void) const
   {
      return (fLabel);
   }

   void ls(Option_t* opt = "") const;

protected:
   UInt_t fLabel;      // Comment here
   Int_t  fStatus;     // Comment here
   UInt_t fCount;      // Comment here
   UInt_t fFreq;       // Comment here
   UInt_t fTics;       // Comment here
   UInt_t fReserve[3]; // Comment here
public:
   ClassDef(GTOneScaler, 1) // Scaler structure
};


#endif

