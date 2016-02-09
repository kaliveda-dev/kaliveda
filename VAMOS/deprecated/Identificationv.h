#ifndef __VAMOS_IDENTIFICATION_E503__
#define __VAMOS_IDENTIFICATION_E503__

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
#include <cmath>

// C++
#include <vector>
#include <iostream>

// ROOT
#include "Rtypes.h"
#include "TCutG.h"
#include "TGraph.h"
#include "TList.h"
#include "TString.h"
#include "TMath.h"

// KaliVeda (Standard)
#include "KV2Body.h"
#include "KVDetector.h"
#include "KVINDRAReconNuc.h"
#include "KVIdentificationResult.h"
#include "KVMaterial.h"
#include "KVNucleus.h"
#include "KVReconstructedNucleus.h"
#include "KVIDZAGrid.h"
#include "KVIDGridManager.h"
#include "KVINDRADB_e503.h"
#include "KVINDRAe503.h"
#include "KVUnits.h"
#include "KVIonRangeTable.h"
#include "KVIonRangeTableMaterial.h"

// Kaliveda (VAMOS)
#include "CsICalib.h"
#include "CsIv.h"
#include "Defines.h"
#include "Deprecation.h"
#include "DriftChamberv.h"
#include "IonisationChamberv.h"
#include "LogFile.h"
#include "Random.h"
#include "Reconstructionv.h"
#include "Sive503.h"

#ifdef __MAKECINT__
#pragma link C++ class vector<Int_t>+;
#pragma link C++ class vector<Float_t>+;
#pragma link C++ class vector<Double_t>+;
#endif

struct GridMissCounters {
   UInt_t sicsi;
   UInt_t chiosi;
   UInt_t sitof;
   UInt_t chiov2;
   UInt_t qaq;
   UInt_t qaq_chiosi;
   UInt_t cutscode2;
};

class Identificationv {
   Int_t kNumCsI;
   Int_t kMaxRuns;
   Int_t kNumSilicon;

   GridMissCounters* grid_misses;

   LogFile* L;
   Reconstructionv* Rec;
   DriftChamberv*   Dr;
   IonisationChamberv* Ic;
   Sive503* Si;
   CsIv* CsI;
   CsICalib* energytree;

   KVIdentificationResult* id;
   KVIdentificationResult* id_chiosi;
   KVIdentificationResult* id_sitof;
   KVIdentificationResult* id_chiov2;
   KVIdentificationResult* id_qaq;
   KVIdentificationResult* id_qaq_chiosi;

   KV2Body* kin;
   KVNucleus* kvn;

   KVTarget* target;
   KVDetector* drift_chamber_one;
   KVMaterial* sed;
   KVDetector* drift_chamber_two;
   KVDetector* chio;
   KVMaterial* isobutane_gap_one;
   KVMaterial* silicon;
   KVMaterial* isobutane_gap_two;
   KVMaterial* csi;

   UShort_t TFil1;
   UShort_t TFil2;
   UShort_t EFil1;
   UShort_t EFil2;

   Int_t runNumber;
   Bool_t grids_avail;

   std::vector<Float_t>* AA;
   std::vector<Double_t>* FragECsI;
   std::vector<Double_t>* FragEGap;

   Double_t a_bisec;
   Double_t e_bisec;

   Double_t initThickness;
   Double_t ECsI_corr;

   Double_t EEtot;
   Double_t NormVamos;
   Double_t DT;
   Double_t Brho_mag;
   Double_t FC_Indra;

   Float_t stat_tot;
   Float_t stat_indra;

   std::vector<Float_t>* Stat_Indra;

   std::vector<Double_t>* PID;
   std::vector<Double_t>* Z_PID;
   std::vector<Double_t>* A_PID;

   std::vector<Double_t>* PID_sitof;
   std::vector<Double_t>* Z_PID_sitof;

   std::vector<Double_t>* PID_chiov2;
   std::vector<Double_t>* Z_PID_chiov2;

   Double_t dif11[55];
   Double_t dif12[55];

   Double_t dif1[21];    //Z de 3 a 24
   Double_t dif2[21];

   std::vector<Int_t>* geom;
   std::vector<Int_t>* geom_cross;
   std::vector<Int_t>* geomchiosi;
   std::vector<Int_t>* geomchiocsi;

   std::vector<Int_t>* FragDetSi;
   std::vector<Int_t>* FragSignalSi;

   std::vector<Float_t>* FragESi;
   std::vector<Double_t>* FragTfinal;

   std::vector<Double_t>* Off_chiosi;
   std::vector<Double_t>* Off_dc1dc2;
   std::vector<Double_t>* TOF_chiosi;

   std::vector<Int_t>* FragSignalCsI;
   std::vector<Int_t>* FragDetCsI;
   std::vector<Int_t>* FragDetChio;
   std::vector<Int_t>* FragSignalChio;

   std::vector<Float_t>* FragEChio;

   Long64_t event_number;
   Int_t MaxM;
   Int_t fMultiplicity_case;
   Int_t Code_good_event;

   std::vector<Float_t>* Offset_relativiste_chiosi;

   Double_t einc_csi;
   Double_t eloss_csi;
   Double_t einc_isogap2;
   Double_t eloss_isogap2;
   Double_t einc_si;
   Double_t einc_isogap1;
   Double_t eloss_isogap1;
   Double_t einc_ic;
   Double_t eloss_ic;
   Double_t einc_dc2;
   Double_t eloss_dc2;
   Double_t einc_sed;
   Double_t eloss_sed;
   Double_t einc_dc1;
   Double_t eloss_dc1;
   Double_t einc_tgt;
   Double_t eloss_tgt;

   Double_t fELosLayer_dc1[3];
   Double_t fELosLayer_dc2[3];
   Double_t fELosLayer_ic[3];

   Double_t E_tgt;
   Double_t E_dc1;
   Double_t E_dc2;
   Double_t E_sed;
   Double_t E_gap1;
   Double_t E_chio;
   Double_t E_gap2;
   Double_t E_csi;

   Float_t E_VAMOS;

   Random* Rnd;

   UShort_t T_Raw;

   Float_t D;
   Float_t dE;
   Float_t dE1;

   std::vector<Float_t>* Etot;

   Float_t E_corr;
   Float_t T;
   Float_t T_straightpaola;

   std::vector<Float_t>* V;
   std::vector<Float_t>* Vx;
   std::vector<Float_t>* Vy;
   std::vector<Float_t>* Vz;

   Float_t VCMx;
   Float_t VCMy;
   Float_t VCMz;

   Float_t V_Etot;
   Float_t T_FP;

   Float_t V2;

   std::vector<Float_t>* Beta;

   Float_t Gamma;

   std::vector<Float_t>* M_Q;
   std::vector<Float_t>* Q;
   std::vector<Float_t>* M;

   std::vector<Float_t>* RealQ_straight;
   std::vector<Float_t>* M_straight;
   std::vector<Int_t>* Q_straight;

   std::vector<Float_t>* Beta_chiosi;

   std::vector<Float_t>* M_Q_chiosi;
   std::vector<Float_t>* Q_chiosi;
   std::vector<Float_t>* M_chiosi;

   Float_t Mr;
   Float_t M_Qr;
   Float_t Qr;
   Float_t Qc;
   Float_t Mc;
   Float_t Mass;
   Float_t M_simul;
   Float_t Z1;
   Float_t Z2;

   Float_t Z_tot;
   Float_t Z_si;

   //Q Correction Function according to the CsI detector
   std::vector<Float_t>* P0;
   std::vector<Float_t>* P1;
   std::vector<Float_t>* P2;
   std::vector<Float_t>* P3;

   // Correction de M/Q
   std::vector<Float_t>* P0_mq;
   std::vector<Float_t>* P1_mq;

   // Correction de M
   std::vector<Float_t>* P0_m;
   std::vector<Float_t>* P1_m;

   // Correction de M/Q Chio-Si
   std::vector<Float_t>* P0_mq_chiosi;
   std::vector<Float_t>* P1_mq_chiosi;

   // Correction de M Chio-Si
   std::vector<Float_t>* P0_m_chiosi;
   std::vector<Float_t>* P1_m_chiosi;

   // Correction de M/Q Si-Tof
   // TODO: Is this even used?
   std::vector<Float_t>* P0_mq_sitof;

   // TODO: Is this even used?
   std::vector<Float_t>* P1_mq_sitof;

   // Correction de M Si-Tof
   // TODO: Is this even used?
   std::vector<Float_t>* P0_m_sitof;

   // TODO: Is this even used?
   std::vector<Float_t>* P1_m_sitof;

   // Tag des events
   std::vector<Float_t>* Brho_min;
   std::vector<Float_t>* Brho_max;

   std::vector<Int_t>* Code_Vamos;
   std::vector<Int_t>* Code_Ident_Vamos;

   // Correction de Tof pour identification Chio-Si
   std::vector<Float_t>* Tof0;
   std::vector<Float_t>* Esi0;
   std::vector<Float_t>* Brho0;

   // Correction de Tof pour identification Si-CsI
   std::vector<Float_t>* Tof0_code1;
   std::vector <Float_t>* Deltat_code1;

   // Correction Double tof Pics
   std::vector<Float_t>* Fit1;
   std::vector<Float_t>* Fit2;
   std::vector<Float_t>* Fit3;
   std::vector<Float_t>* Fit4;
   std::vector<Float_t>* Fit5;
   std::vector<Float_t>* Fit6;
   std::vector<Float_t>* Fit7;

   // Correction Offet relativiste
   // TODO: Check if this is even used!
   std::vector<Float_t>* Offset_rela;

   // Redressement des distributions A/Q
   std::vector<Float_t>* P0_aq_straight;
   std::vector<Float_t>* P1_aq_straight;
   std::vector<Float_t>* P2_aq_straight;

   std::vector<Float_t>* P0_aq_straight_chiosi;
   std::vector<Float_t>* P1_aq_straight_chiosi;
   std::vector<Float_t>* P2_aq_straight_chiosi;

   Double_t M_corr;
   Int_t Q_corr;
   Double_t Q_corr_D;
   Double_t M_corr_D;
   Double_t M_corr_D2;
   Double_t Q_corr_D2;
   Int_t Q2;

   std::vector<Int_t>* Z_corr;
   std::vector<Int_t>* Z_corr_sitof;
   std::vector<Int_t>* Z_corr_chiov2;

   Double_t Delta;
   Double_t M_realQ_D;
   Double_t M_realQ;
   Double_t realQ_D;
   Int_t realQ;
   Int_t Qid;
   Float_t M_Qcorr;

   Bool_t print;
   Bool_t debug;
   Bool_t Present; //true if coordinates determined

   void Geometry(); //method to reconstruct VAMOS telescopes
   void GeometryChioSi();
   void GeometryChioCsI();
   void GeometrySiCsICross();

   void Init(); //Init for every event
   void InitSavedQuantities();
   void InitReadVariables();

   void ReadQCorrection();
   void ReadAQCorrection();
   void ReadACorrection();
   void ReadFlagVamos();
   void ReadToFCorrectionChioSi();
   void ReadToFCorrectionCode1();
   void ReadStatIndra();
   void ReadStraightAQ();
   void ReadDoublingCorrection();
   void ReadOffsetsChiosi();

   Bool_t IsChioCsITelescope(Int_t, Int_t);
   Bool_t IsChioSiTelescope(Int_t, Int_t);
   Bool_t IsSiCsITelescope(Int_t, Int_t);
   Bool_t IsCsISiCrossTelescope(Int_t, Int_t);

   const Int_t print_modulo;
   const size_t print_max_array_size;

   // Don't use <typename T>, name collision with member variable
   template <typename Type>
   void PrintVector(const std::vector<Type>* const v) const;

   template <typename Type>
   void PrintArray(const Type* const a, const size_t& size) const;

   template <typename Type>
   void PrintArray(const Type* const* const a,
                   const size_t& size_1,
                   const size_t& size_2) const;

   template <typename Type>
   void PrintArray(const Type* const* const* const a,
                   const size_t& size_1,
                   const size_t& size_2,
                   const size_t& size_3) const;

   // Prohibit Copy and Copy Assign
   Identificationv(const Identificationv&);
   Identificationv& operator=(const Identificationv&);

public:

   Identificationv(
      LogFile* Log,
      Reconstructionv* Recon,
      DriftChamberv* Drift,
      IonisationChamberv* IonCh,
      Sive503* SiD,
      CsIv* CsID,
      CsICalib* Ecsicalibrated
   );

   void InitialiseComponents();
   void LoadGridForCode2Cuts();

   virtual ~Identificationv(void);

   Int_t GetMaxMultiplicity(void);
   void FragPropertiesInOrder(void);

   void Calculate(); // Calulate  Initial coordinates
   void Show(void);
   void Treat(void);
   void inAttach(TTree* inT);
   void outAttach(TTree* outT);
   void CreateHistograms();
   void FillHistograms();

   // Detector Mutators (Copies object given as argument)
   void set_target(const KVTarget* const targ);
   void set_drift_chamber_one(const KVDetector* const detector);
   void set_sed(const KVMaterial* const material);
   void set_drift_chamber_two(const KVDetector* const detector);
   void set_chio(const KVDetector* const detector);
   void set_isobutane_gap_one(const KVMaterial* const material);
   void set_silicon(const KVMaterial* const material);
   void set_isobutane_gap_two(const KVMaterial* const material);
   void set_csi(const KVMaterial* const material);

   // Detector Accessors
   const KVTarget* get_target() const;
   const KVDetector* get_drift_chamber_one() const;
   const KVMaterial* get_sed() const;
   const KVDetector* get_drift_chamber_two() const;
   const KVDetector* get_chio() const;
   const KVMaterial* get_isobutane_gap_one() const;
   const KVMaterial* get_silicon() const;
   const KVMaterial* get_isobutane_gap_two() const;
   const KVMaterial* get_csi() const;

   Double_t GetEnergyLossCsI(Int_t, Int_t, Double_t, Double_t);
   Double_t GetEnergyLossGap2(Int_t, Int_t, Double_t, Double_t);
   Double_t GetEnergyLossGap1(Int_t, Int_t, Double_t, Double_t);
   Double_t GetEnergyLossChio(Int_t, Double_t);
   Double_t GetEnergyLossDC2(Int_t, Double_t);
   Double_t GetEnergyLossSed(Int_t, Double_t);
   Double_t GetEnergyLossDC1(Int_t, Double_t);
   Double_t GetEnergyLossTarget(Int_t, Double_t);

   void SetBrhoRef(Float_t brho)
   {
      Brho_mag = brho;
   };

   void ResizeVectors();       // Resizes vectors to "MaxM"
   void ResetResizedVectors(); // Resets vector sizes
   void PrintParameters() const;

   // For testing only (called by old code handler)
   void PrintGridMisses() const;

   void PrintAssertionStatus() const;

   ClassDef(Identificationv, 1)

};

#endif // __VAMOS_IDENTIFICATION_E503__ is not set

#ifdef __VAMOS_IDENTIFICATION_E503__
DEPRECATED_CLASS(Identificationv);
#endif

