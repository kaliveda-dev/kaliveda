#ifndef __CSICALIB_H
#define __CSICALIB_H

/**
   WARNING: This class has been deprecated and will eventually be removed. Do
   not use!

   This class is only compiled if the deprecated code is enabled in the build
   configuration (e.g. cmake -DUSE_DEPRECATED_VAMOS=yes). If you enable the
   deprecated code then a large number of warnings will be printed to the
   terminal. To disable these warnings (not advised) compile VAMOS with
   -Wno-deprecated-declarations. Despite these warnings the code should compile
   just fine. The warnings are there to prevent the unwitting use of the
   deprecated code (which should be strongly discouraged).

   BY DEFAULT THIS CLASS IS NOT COMPILED.

   Deprecated by: Peter Wigg (peter.wigg.314159@gmail.com)
   Date:          Thu 17 Dec 17:24:38 GMT 2015
*/

// C
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// C++
#include <string>

// ROOT
#include "Riostream.h"
#include "TCut.h"
#include "TEventList.h"
#include "TFile.h"
#include "TROOT.h"
#include "TString.h"
#include "TTree.h"

// KaliVeda (Standard)
#include "KVDetector.h"
#include "KVIDGraph.h"
#include "KVIDGridManager.h"
#include "KVIDTelescope.h"
#include "KVIDZAGrid.h"
#include "KVLightEnergyCsI.h"
#include "KVList.h"
#include "KVReconstructedNucleus.h"
#include "KVSeqCollection.h"
#include "KVTelescope.h"
#include "KVUnits.h"
#include "KVMacros.h" // UNUSED macro

// KaliVeda (VAMOS)
#include "CsIv.h"
#include "Deprecation.h"
#include "GridLoader.h"
#include "KVIDSiCsIVamos.h"
#include "KVIVReconIdent.h"
#include "KVLightEnergyCsIVamos.h"
#include "LogFile.h"
#include "Sive503.h"

class CsICalib {
   Int_t eN;
   Int_t status;

   UShort_t eLightSi;
   UShort_t eLightCsI;
   Double_t LightCsI;
   Double_t eEnergySi;
   Double_t eEnergyCsI;

   Double_t Einc;
   Double_t EEsi;
   Double_t Echio;

   Double_t sEnergySi;
   Double_t sEnergyCsI;
   Double_t sRefECsI;

   Double_t esi1;
   Double_t esi2;
   Double_t ecsi1;
   Double_t ecsi2;
   Double_t diff1;
   Double_t diff2;

   Double_t diffsi;
   Double_t diffcsi;
   Double_t diffetot;
   Double_t difflum;

   Double_t CanalCsI;

   Int_t right;
   Int_t left;

   Int_t eZ;
   Int_t eA;
   Int_t sA;
   Double_t iA;

   Double_t ePied;
   Double_t a;
   Double_t b;
   Double_t c;
   Double_t alpha;

   Double_t a1;
   Double_t a2;
   Double_t a3;
   Double_t a4;

   Double_t thick;

   KVReconstructedNucleus* frag;
   KVLightEnergyCsIVamos* lum;

   KVTelescope* ttel;

   KVTelescope* kvt_icsi;
   KVTelescope* kvt_sicsi;
   KVDetector* kvd_si;
   KVDetector* kvd_csi;

   KVDetector* kvd_gap;

   KVDetector* gap;
   KVDetector* si;
   KVDetector* csi;

   // Used to store the address of the reference detectors.
   const KVDetector* si_detector;
   const KVDetector* gap_detector;
   const KVDetector* csi_detector;

   KVNucleus part;
   KVNucleus part2;

   Sive503* Si;
   CsIv* CsI;
   LogFile* L;

   const KVIDGrid* kvid;
   const KVIDGrid* kvid_chiosi;
   const KVIDGrid* kvid_sitof;
   const KVIDGrid* kvid_cutscode2;
   const KVIDGrid* kvid_chiov2;
   const KVIDGrid* kvid_qaq;
   const KVIDGrid* kvid_qaq_chiosi;

   Bool_t good_bisection;
   Double_t eEnergyGap;

   GridLoader* grid_loader;
   Bool_t kInitialised;

   // Number of bisector simulations
   Int_t bisector_iterations_;

public:

   CsICalib(LogFile* Log, Sive503* Si);
   virtual ~CsICalib();

   Bool_t Init();
   Bool_t InitRun(const UInt_t run);

   Bool_t InitTelescope(Int_t num_si, Int_t num_csi);
   Bool_t InitTelescopeChioSi(Int_t num_chio, Int_t num_si);
   Bool_t InitTelescopeSiTof(Int_t num_si);
   Bool_t InitCode2Cuts(Float_t brho0);
   Bool_t InitChioV2(Int_t num_chio);
   Bool_t InitQStraight(Int_t num_csi);
   Bool_t InitQStraight_chiosi(Int_t num_chio);

   void InitSiCsI(Int_t);

   void SetCalibration(Sive503*, CsIv*, Int_t, Int_t);
   void SetSimCalibration(Sive503*, CsIv*, Int_t, Int_t);

   void SetFragmentZ(Int_t);
   void SetFragmentA(Int_t);

   //necessary methods to GetResidualEnergyCsI: best estimation of ECsI and A
   Double_t GetResidualEnergyCsI(Double_t, Double_t); //UShort_t,UShort_t

   void CalculateESi(Double_t); //UShort_t
   void Bisection(Int_t, Double_t); //UShort_t
   Double_t BisectionLight(Double_t , Double_t , Double_t);
   void CompleteSimulation(); //UShort_t

   void Interpolate();
   Double_t GetInterpolationD(Double_t, Double_t, Double_t, Double_t, Double_t);

   Double_t RetrieveEnergySi() const;
   Double_t RetrieveA() const;
   Double_t RetrieveLight() const;
   Double_t RetrieveEnergyCsI() const;

   void PrintAssertionStatus() const;

   // Accessor methods:

   Bool_t          get_good_bisection()      const;
   Double_t        get_eEnergyGap()          const;
   const KVIDGrid* get_kvid()                const;
   const KVIDGrid* get_kvid_chiosi()         const;
   const KVIDGrid* get_kvid_sitof()          const;
   const KVIDGrid* get_kvid_cutscode2()      const;
   const KVIDGrid* get_kvid_chiov2()         const;
   const KVIDGrid* get_kvid_qaq()            const;
   const KVIDGrid* get_kvid_qaq_chiosi()     const;
   Int_t           get_bisector_iterations() const;

   // Mutator Methods:

   void set_si_detector(const KVDetector* const detector);
   void set_gap_detector(const KVDetector* const detector);
   void set_csi_detector(const KVDetector* const detector);


   ClassDef(CsICalib, 1) //CsICalib
};

#endif // __CSICALIB_H is not set

#ifdef __CSICALIB_H
DEPRECATED_CLASS(CsICalib);
#endif

