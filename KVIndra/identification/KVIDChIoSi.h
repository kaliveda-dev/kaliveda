/***************************************************************************
                          KVIDChIoSi.h  -  description
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

#ifndef KVIDChIoSi_H
#define KVIDChIoSi_H

#include "KVINDRAIDTelescope.h"
#include "KVIDGChIoSi.h"

/**
  \class KVIDChIoSi
  \ingroup INDRAIdent
  \brief Identification in ChIo-Si telescopes of INDRA
 */
class KVIDChIoSi: public KVINDRAIDTelescope {

protected:

   KVIDGChIoSi* ChIoSiGrid;//!
   KVDetector* fchio;//!
   KVDetector* fsi;//!
   Double_t fsipgped;//!
   Double_t fchiopgped;//!


public:

   KVIDChIoSi();
   virtual ~ KVIDChIoSi();

   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual void Initialize();

   virtual Bool_t CanIdentify(Int_t Z, Int_t /*A*/)
   {
      // Used for filtering simulations
      // Returns kTRUE if this telescope is theoretically capable of identifying a given nucleus,
      // without considering thresholds etc.
      // For INDRA ChIo-Si telescopes, identification is possible for Z>1
      // (protons are difficult to distinguish from pedestal)
      return (Z > 1);
   }

   Bool_t CheckTheoreticalIdentificationThreshold(KVNucleus* /*ION*/, Double_t /*EINC*/ = 0.0);

   ClassDef(KVIDChIoSi, 1)     //INDRA identification using ChIo-Si matrices
};

#endif
