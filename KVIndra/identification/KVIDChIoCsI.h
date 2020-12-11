/***************************************************************************
                          KVIDChIoCsI.h  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
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

#ifndef KVIDChIoCsI_H
#define KVIDChIoCsI_H

#include "KVINDRAIDTelescope.h"
#include "KVChIo.h"
#include "KVCsI.h"

/**
  \class KVIDChIoCsI
  \ingroup INDRAIdent
  \brief Identification in ChIo-CsI telescopes of INDRA
 */
class KVIDChIoCsI: public KVINDRAIDTelescope {

protected:
   KVChIo* fChIo;//!the chio
   KVCsI* fCsI;//!the csi

public:

   KVIDChIoCsI();
   virtual ~ KVIDChIoCsI()
   {
   }
   virtual Bool_t CanIdentify(Int_t Z, Int_t /*A*/)
   {
      // Used for filtering simulations
      // Returns kTRUE if this telescope is theoretically capable of identifying a given nucleus,
      // without considering thresholds etc.
      // For INDRA ChIo-CsI telescopes, identification is possible for Z>1
      // (protons are difficult to distinguish from pedestal)
      return (Z > 1);
   }
   Bool_t CheckTheoreticalIdentificationThreshold(KVNucleus* /*ION*/, Double_t /*EINC*/ = 0.0);
   virtual void Initialize(void);

   ClassDef(KVIDChIoCsI, 1)     //INDRA identification using ChIo-CsI matrices
};

#endif
