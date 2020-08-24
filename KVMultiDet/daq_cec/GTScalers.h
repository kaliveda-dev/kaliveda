// $Id: GTScalers.H,v 1.2 2007/09/17 12:33:27 franklan Exp $
// Author: $Author: franklan $
//-*************************************************************************
//                        GTGanilData.cpp  -  Main Header to ROOTGAnilTape
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr
//////////////////////////////////////////////////////////////////////////
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

#ifndef GT_GTScalers_H
#define GT_GTScalers_H

#include <TObject.h>
#include <TClonesArray.h>
class GTOneScaler;

/**
  \class GTScalers
  \brief Handle scaler buffers in GANIL DAQ data
  \ingroup DAQ
 */
class GTScalers : public TObject {
public:
   GTScalers(void);
   ~GTScalers(void);
   void Fill(void*);                 // Vocabulary: Set or Fill ?
   void DumpScalers(void);

   const GTOneScaler* GetScalerPtr(Int_t index) const;
   Int_t              GetNbChannel(void) const
   {
      return fNbChannel;
   }

protected:
   Int_t fNbChannel;          // Number of individual scales
   TClonesArray fScalerArray; // Array of scalers
public:
   ClassDef(GTScalers, 2)     // Scaler events class
};


#endif
