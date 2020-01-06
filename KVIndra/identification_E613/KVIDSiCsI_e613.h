//Author: Diego Gruyer,,,

#ifndef __KVIDSiCSI_e613_H
#define __KVIDSiCSI_e613_H

#include "KVIDSiCsI.h"
#include "KVIDZAGrid.h"
#include "KVSilicon.h"
#include "KVCsI.h"

class KVIDSiCsI_e613 : public KVIDSiCsI {

   KVIDZAGrid* fGGgrid;        //! grid used for Si(GG)-CsI(TotLight) charge identification
   KVIDZAGrid* fPGgrid;        //! grid used for Si(PG)-CsI(TotLight) charge identification
   KVSilicon* fSi;       //! the si
   KVCsI* fCsI;          //! the csi

   Double_t fCsIRPedestal;     //!CsI Rapide pedestal for current run
   Double_t fCsILPedestal;     //!CsI Lente pedestal for current run
   Double_t fSiPGPedestal;     //!Silicon PG pedestal for current run
   Double_t fSiGGPedestal;     //!Silicon GG pedestal for current run

public:
   KVIDSiCsI_e613();
   virtual ~KVIDSiCsI_e613();

   virtual void Initialize(void);
   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");
   virtual Double_t GetPedestalX(Option_t*  = "")
   {
      return 0.;
   };
   virtual Double_t GetPedestalY(Option_t*  = "")
   {
      return 0.;
   };

   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   ClassDef(KVIDSiCsI_e613, 1) //Si-CsI id with grids for INDRA_e613
};

#endif
