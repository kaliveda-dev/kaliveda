#include "Identificationv.h"

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

ClassImp(Identificationv)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Identificationv</h2>
<p>Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund
(GANIL).</p>
<body>


</body>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

class  MyFunctionObject {
public:

   TF1* dedx;
   Int_t fZ, fA;
   Double_t fRho;
   Double_t fEi;
   Double_t fEres;
   Double_t fDeltaE;

   MyFunctionObject(Int_t z, Int_t a, Int_t option)
   {
      fZ = z;
      fA = a;
      // fEi = ei;
      // fDeltaE = deltaE;

      Float_t pressure = 0.0;
      KVMaterial* gas(NULL);

      if (option == 0) {   // case DC1-DC2
         pressure = 13.;
         gas = new KVMaterial("C4H10", 2.*13.0);      // en cm - 2DC
         gas->SetPressure(pressure * KVUnits::mbar);
      } else if (option == 1) {  // case Chio
         pressure = 40.;
         gas = new KVMaterial("C4H10", 13.8);      // en cm
         gas->SetPressure(pressure * KVUnits::mbar);
      } else {
         // XXX: Undefined behaviour! terminate
         std::cerr << "Identificationv: (MyFunctionObject) Behaviour "
                   "undefined for option value of " << option << std::endl;
         std::exit(-1);
      }

      KVIonRangeTable* t = gas->GetRangeTable();
      KVIonRangeTableMaterial* mat = t->GetMaterial("C4H10");
      mat->SetTemperatureAndPressure(19., pressure * KVUnits::mbar);

      fRho = mat->GetDensity();
      dedx = mat->GetStoppingFunction(z, a);

   }

   // use constructor to customize your function object
   Double_t operator()(Double_t* x, Double_t* p)
   {
      // z:fZ a:fA

      UNUSED(p);

      Double_t e = *x;
      Double_t result = 1. / (e * dedx->Eval(e));

      return result;
   }

   Double_t GetRho() const
   {
      return fRho;
   }

   Double_t GetDeltaE() const
   {
      return fDeltaE;
   }

   Double_t GetEres() const
   {
      return fEres;
   }

   Double_t GetEi() const
   {
      return fEi;
   }

   Int_t GetA() const
   {
      return fA;
   }

   Int_t GetZ() const
   {
      return fZ;
   }

};


Double_t DrawDeDx(Int_t z, Int_t a, Int_t option, Double_t Einc, Double_t Eres)
{
   MyFunctionObject* fobj;
   TF1* f;

   fobj = new MyFunctionObject(z, a, option);
   f = new TF1("f", fobj, 1e-5, 2000, 0, "MyFunctionObject");
   f->SetNpx(10000);

   Double_t rho = fobj->GetRho();
   Double_t fact =
      TMath::Sqrt(0.7192 * static_cast<Double_t>(fobj->GetA())) * 1. / rho;
   // Double_t Einc = fobj->GetEi();
   // Double_t Eres = fobj->GetEres();
   Double_t timeNS = fact * f->Integral(Eres, Einc);

   // std::cout << "timeNS : " << timeNS << std::endl;
   delete fobj;
   delete f;

   return timeNS;
}


// It's hideous I know but it's the best I can do without refactoring the code
// and I'm not doing that as it's about to be replaced by the new code. At least
// this way the default construction leaves the object in a well defined state.
// All complex construction is done with 'InitialiseComponents()' which must be
// called immediately after this constructor!

Identificationv::Identificationv(
   LogFile* Log,
   Reconstructionv* Recon,
   DriftChamberv* Drift,
   IonisationChamberv* IonCh,
   Sive503* SiD,
   CsIv* CsID,
   CsICalib* Ecsicalibrated
)
   : kNumCsI(0),
     kMaxRuns(0),
     kNumSilicon(0),
     grid_misses(NULL),
     L(Log),
     Rec(Recon),
     Dr(Drift),
     Ic(IonCh),
     Si(SiD),
     CsI(CsID),
     energytree(Ecsicalibrated),
     id(NULL),
     id_chiosi(NULL),
     id_sitof(NULL),
     id_chiov2(NULL),
     id_qaq(NULL),
     id_qaq_chiosi(NULL),
     kin(NULL),
     kvn(NULL),
     target(NULL),
     drift_chamber_one(NULL),
     sed(NULL),
     drift_chamber_two(NULL),
     chio(NULL),
     isobutane_gap_one(NULL),
     silicon(NULL),
     isobutane_gap_two(NULL),
     csi(NULL),
     AA(NULL),
     FragECsI(NULL),
     FragEGap(NULL),
     Stat_Indra(NULL),
     PID(NULL),
     Z_PID(NULL),
     A_PID(NULL),
     PID_sitof(NULL),
     Z_PID_sitof(NULL),
     PID_chiov2(NULL),
     Z_PID_chiov2(NULL),
     geom(NULL),
     geom_cross(NULL),
     geomchiosi(NULL),
     geomchiocsi(NULL),
     FragDetSi(NULL),
     FragSignalSi(NULL),
     FragESi(NULL),
     FragTfinal(NULL),
     Off_chiosi(NULL),
     Off_dc1dc2(NULL),
     TOF_chiosi(NULL),
     FragSignalCsI(NULL),
     FragDetCsI(NULL),
     FragDetChio(NULL),
     FragSignalChio(NULL),
     FragEChio(NULL),
     Offset_relativiste_chiosi(NULL),
     Rnd(NULL),
     Etot(NULL),
     V(NULL),
     Vx(NULL),
     Vy(NULL),
     Vz(NULL),
     Beta(NULL),
     M_Q(NULL),
     Q(NULL),
     M(NULL),
     RealQ_straight(NULL),
     M_straight(NULL),
     Q_straight(NULL),
     Beta_chiosi(NULL),
     M_Q_chiosi(NULL),
     Q_chiosi(NULL),
     M_chiosi(NULL),
     P0(NULL),
     P1(NULL),
     P2(NULL),
     P3(NULL),
     P0_mq(NULL),
     P1_mq(NULL),
     P0_m(NULL),
     P1_m(NULL),
     P0_mq_chiosi(NULL),
     P1_mq_chiosi(NULL),
     P0_m_chiosi(NULL),
     P1_m_chiosi(NULL),
     P0_mq_sitof(NULL),
     P1_mq_sitof(NULL),
     P0_m_sitof(NULL),
     P1_m_sitof(NULL),
     Brho_min(NULL),
     Brho_max(NULL),
     Code_Vamos(NULL),
     Code_Ident_Vamos(NULL),
     Tof0(NULL),
     Esi0(NULL),
     Brho0(NULL),
     Tof0_code1(NULL),
     Deltat_code1(NULL),
     Fit1(NULL),
     Fit2(NULL),
     Fit3(NULL),
     Fit4(NULL),
     Fit5(NULL),
     Fit6(NULL),
     Fit7(NULL),
     Offset_rela(NULL),
     P0_aq_straight(NULL),
     P1_aq_straight(NULL),
     P2_aq_straight(NULL),
     P0_aq_straight_chiosi(NULL),
     P1_aq_straight_chiosi(NULL),
     P2_aq_straight_chiosi(NULL),
     Z_corr(NULL),
     Z_corr_sitof(NULL),
     Z_corr_chiov2(NULL),
     print_modulo(8),             // 8 element modulus for printing
     print_max_array_size(100000) // Maximum number of elements to print

{
   // Clean construction means the object is in a well defined state!
}

void Identificationv::InitialiseComponents()
{
   // Complex initialisation of the Identificationv object (should be called
   // immediately after construction)

   debug = kFALSE;

   kNumSilicon =  gEnv->GetValue("VAMOS.kNumSilicon", -1);
   kNumCsI   = gEnv->GetValue("VAMOS.kNumCsI", -1);
   kMaxRuns = gEnv->GetValue("VAMOS.kMaxRuns", -1);

   if (kNumSilicon < 0) {
      Error("InitialiseComponents",
            "Problem(s) reading environment variable \'VAMOS.kNumSilicon\'");
   }

   if (kMaxRuns < 0) {
      Error("InitialiseComponents",
            "Problem(s) reading environment variable \'VAMOS.kMaxRuns\'");
   }

   if (kNumCsI < 0) {
      Error("InitialiseComponents",
            "Problem(s) reading environment variable \'VAMOS.kNumCsI\'");
   }

   Info("InitialiseComponents", "Array Boundary Values:");
   Info("InitialiseComponents", "kNumSilicon: %d", kNumSilicon);
   Info("InitialiseComponents", "kNumCsI:     %d", kNumCsI);
   Info("InitialiseComponents", "kMaxRuns:    %d", kMaxRuns);

   grid_misses = new GridMissCounters;

   grid_misses->sicsi = 0;
   grid_misses->chiosi = 0;
   grid_misses->sitof = 0;
   grid_misses->chiov2 = 0;
   grid_misses->qaq = 0;
   grid_misses->qaq_chiosi = 0;
   grid_misses->cutscode2 = 0;

   Rnd = new Random;
   kvn = new KVNucleus();

   AA = new std::vector<Float_t>(kNumCsI + 1, -10.);

   FragECsI = new std::vector<Double_t>(kNumCsI + 1, -10.);
   FragEGap = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Stat_Indra = new std::vector<Float_t>(kMaxRuns + 1, -10.);

   PID = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Z_PID = new std::vector<Double_t>(kNumCsI + 1, -10.);
   A_PID = new std::vector<Double_t>(kNumCsI + 1, -10.);

   Z_corr = new std::vector<Int_t>(kNumCsI + 1, -10);

   PID_sitof = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Z_PID_sitof = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Z_corr_sitof = new std::vector<Int_t>(kNumCsI + 1, -10);

   PID_chiov2 = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Z_PID_chiov2 = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Z_corr_chiov2 = new std::vector<Int_t>(kNumCsI + 1, -10);

   // Q Correction Function according to the CsI detector
   P0 = new std::vector<Float_t>(kNumCsI + 1, 0.);
   P1 = new std::vector<Float_t>(kNumCsI + 1, 0.);
   P2 = new std::vector<Float_t>(kNumCsI + 1, 0.);
   P3 = new std::vector<Float_t>(kNumCsI + 1, 0.);

   // Correction de M/Q
   P0_mq = new std::vector<Float_t>(kMaxRuns + 1, 0.);
   P1_mq = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   // Correction de M
   P0_m = new std::vector<Float_t>((kMaxRuns + 1) * 25, 0.);
   P1_m = new std::vector<Float_t>((kMaxRuns + 1) * 25, 0.);

   // Correction de M Chio-Si
   P0_m_chiosi = new std::vector<Float_t>((kMaxRuns + 1) * 25, 0.);
   P1_m_chiosi = new std::vector<Float_t>((kMaxRuns + 1) * 25, 0.);

   // Correction de M Si-Tof
   P0_m_sitof = new std::vector<Float_t>((kMaxRuns + 1) * 25, 0.);
   P1_m_sitof = new std::vector<Float_t>((kMaxRuns + 1) * 25, 0.);

   // Correction de M/Q Chio-Si
   P0_mq_chiosi = new std::vector<Float_t>(kMaxRuns + 1, 0.);
   P1_mq_chiosi = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   // Correction de M/Q Si-Tof
   P0_mq_sitof = new std::vector<Float_t>(kMaxRuns + 1, 0.);
   P1_mq_sitof = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   // Tag des events
   Brho_min = new std::vector<Float_t>(kMaxRuns + 1, -10.);
   Brho_max = new std::vector<Float_t>(kMaxRuns + 1, -10.);

   Code_Ident_Vamos = new std::vector<Int_t>(kNumCsI + 1, -10);
   Code_Vamos = new std::vector<Int_t>(kNumCsI + 1, -10);

   // Correction de Tof pour identification Chio-Si
   Tof0 = new std::vector<Float_t>(kMaxRuns + 1, 0.);
   Esi0 = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   Brho0 = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   // Correction de Tof pour identification Si-CsI
   Tof0_code1 = new std::vector<Float_t>(kMaxRuns + 1, 0.);
   Deltat_code1 = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   // Correction Double tof Pics

   Fit1 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);
   Fit2 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);
   Fit3 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);
   Fit4 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);
   Fit5 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);
   Fit6 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);
   Fit7 = new std::vector<Float_t>(25 * 2 * (kMaxRuns + 1), -1000.);

   // Correction Offet relativiste
   Offset_rela = new std::vector<Float_t>(kMaxRuns + 1, 0.);
   Offset_relativiste_chiosi = new std::vector<Float_t>(kMaxRuns + 1, 0.);

   // Redressement des distributions A/Q
   P0_aq_straight = new std::vector<Float_t>(kNumSilicon + 1, 0.);
   P1_aq_straight = new std::vector<Float_t>(kNumSilicon + 1, 0.);
   P2_aq_straight = new std::vector<Float_t>(kNumSilicon + 1, 0.);

   P0_aq_straight_chiosi = new std::vector<Float_t>(8, 0.);
   P1_aq_straight_chiosi = new std::vector<Float_t>(8, 0.);
   P2_aq_straight_chiosi = new std::vector<Float_t>(8, 0.);

   FragDetSi = new std::vector<Int_t>(kNumCsI + 1, -10);
   FragSignalSi = new std::vector<Int_t>(kNumCsI + 1, -10);
   FragESi = new std::vector<Float_t>(kNumCsI + 1, -10.);

   FragTfinal = new std::vector<Double_t>(kNumCsI + 1, -10.);

   Off_chiosi = new std::vector<Double_t>(kNumCsI + 1, -10.);
   Off_dc1dc2 = new std::vector<Double_t>(kNumCsI + 1, -10.);

   TOF_chiosi = new std::vector<Double_t>(kNumCsI + 1, -10.);

   FragSignalCsI = new std::vector<Int_t>(kNumCsI + 1, -10);
   FragDetCsI = new std::vector<Int_t>(kNumCsI + 1, -10);

   FragDetChio = new std::vector<Int_t>(kNumCsI + 1, -10);
   FragSignalChio = new std::vector<Int_t>(kNumCsI + 1, -10);

   FragEChio = new std::vector<Float_t>(kNumCsI + 1, -10.);

   V = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Vx = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Vy = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Vz = new std::vector<Float_t>(kNumCsI + 1, -10.);

   Beta = new std::vector<Float_t>(kNumCsI + 1, -10.);

   M_Q = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Etot = new std::vector<Float_t>(kNumCsI + 1, -10.);
   M = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Q = new std::vector<Float_t>(kNumCsI + 1, -10.);

   Beta_chiosi = new std::vector<Float_t>(kNumCsI + 1, -10.);
   M_Q_chiosi = new std::vector<Float_t>(kNumCsI + 1, -10.);
   M_chiosi = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Q_chiosi = new std::vector<Float_t>(kNumCsI + 1, -10.);

   RealQ_straight = new std::vector<Float_t>(kNumCsI + 1, -10.);
   Q_straight = new std::vector<Int_t>(kNumCsI + 1, -10);
   M_straight = new std::vector<Float_t>(kNumCsI + 1, -10.);

   geom = new std::vector<Int_t> ((kNumSilicon + 1) * 6, -1);
   geom_cross = new std::vector<Int_t> ((kNumSilicon + 1) * 4, -1);
   geomchiosi = new std::vector<Int_t> ((kNumSilicon + 1) * 2, -1);
   geomchiocsi = new std::vector<Int_t> ((kNumCsI + 1) * 2, -1);

   InitReadVariables();
   Geometry();
   GeometryChioSi();
   GeometryChioCsI();
   GeometrySiCsICross();

   // Reading Files :
   ReadQCorrection();
   ReadAQCorrection();
   ReadACorrection();
   ReadFlagVamos();
   ReadToFCorrectionChioSi();
   ReadToFCorrectionCode1();
   ReadStatIndra();
   ReadStraightAQ();
   ReadDoublingCorrection();
   ReadOffsetsChiosi();

   event_number = 0;

   energytree->Init();

}

void Identificationv::LoadGridForCode2Cuts()
{

   std::cout << "run : " << gIndra->GetCurrentRunNumber() << " Brho0 : "
             << Brho0->at(gIndra->GetCurrentRunNumber()) << std::endl;

   energytree->InitCode2Cuts(Brho0->at(gIndra->GetCurrentRunNumber()));

   return;
}

Identificationv::~Identificationv(void)
{
   // Longest destructor in the world!

   if (target) {
      delete target;
      target = NULL;
   }

   if (drift_chamber_one) {
      delete drift_chamber_one;
      drift_chamber_one = NULL;
   }

   if (sed) {
      delete sed;
      sed = NULL;
   }

   if (drift_chamber_two) {
      delete drift_chamber_two;
      drift_chamber_two = NULL;
   }

   if (chio) {
      delete chio;
      chio = NULL;
   }

   if (isobutane_gap_one) {
      delete isobutane_gap_one;
      isobutane_gap_one = NULL;
   }

   if (silicon) {
      delete silicon;
      silicon = NULL;
   }

   if (isobutane_gap_two) {
      delete isobutane_gap_two;
      isobutane_gap_two = NULL;
   }

   if (csi) {
      delete csi;
      csi = NULL;
   }

   if (AA) {
      delete AA;
      AA = NULL;
   }

   if (FragECsI) {
      delete FragECsI;
      FragECsI = NULL;
   }

   if (FragEGap) {
      delete FragEGap;
      FragEGap = NULL;
   }

   if (Stat_Indra) {
      delete Stat_Indra;
      Stat_Indra = NULL;
   }

   if (PID) {
      delete PID;
      PID = NULL;
   }

   if (PID_sitof) {
      delete PID_sitof;
      PID_sitof = NULL;
   }

   if (PID_chiov2) {
      delete PID_chiov2;
      PID_chiov2 = NULL;
   }

   if (Z_PID) {
      delete Z_PID;
      Z_PID = NULL;
   }

   if (A_PID) {
      delete A_PID;
      A_PID = NULL;
   }

   if (Z_corr) {
      delete Z_corr;
      Z_corr = NULL;
   }

   if (Z_PID_sitof) {
      delete Z_PID_sitof;
      Z_PID_sitof = NULL;
   }

   if (Z_corr_sitof) {
      delete Z_corr_sitof;
      Z_corr_sitof = NULL;
   }

   if (Z_PID_chiov2) {
      delete Z_PID_chiov2;
      Z_PID_chiov2 = NULL;
   }

   if (Z_corr_chiov2) {
      delete Z_corr_chiov2;
      Z_corr_chiov2 = NULL;
   }

   // Q Correction Function according to the CsI detector

   if (P0) {
      delete P0;
      P0 = NULL;
   }

   if (P1) {
      delete P1;
      P1 = NULL;
   }

   if (P2) {
      delete P2;
      P2 = NULL;
   }

   if (P3) {
      delete P3;
      P3 = NULL;
   }

   // Correction de M/Q

   if (P0_mq) {
      delete P0_mq;
      P0_mq = NULL;
   }

   if (P1_mq) {
      delete P1_mq;
      P1_mq = NULL;
   }

   // Correction de M

   if (P0_m) {
      delete P0_m;
      P0_m = NULL;
   }

   if (P1_m) {
      delete P1_m;
      P1_m = NULL;
   }

   // Correction de M Chio-Si

   if (P0_m_chiosi) {
      delete P0_m_chiosi;
      P0_m_chiosi = NULL;
   }

   if (P1_m_chiosi) {
      delete P1_m_chiosi;
      P1_m_chiosi = NULL;
   }

   // Correction de M Si-Tof

   if (P0_m_sitof) {
      delete P0_m_sitof;
      P0_m_sitof = NULL;
   }

   if (P1_m_sitof) {
      delete P1_m_sitof;
      P1_m_sitof = NULL;
   }

   // Correction de M/Q Chio-Si

   if (P0_mq_chiosi) {
      delete P0_mq_chiosi;
      P0_mq_chiosi = NULL;
   }

   if (P1_mq_chiosi) {
      delete P1_mq_chiosi;
      P1_mq_chiosi = NULL;
   }

   // Correction de M/Q Si-Tof

   if (P0_mq_sitof) {
      delete P0_mq_sitof;
      P0_mq_sitof = NULL;
   }

   if (P1_mq_sitof) {
      delete P1_mq_sitof;
      P1_mq_sitof = NULL;
   }

   // Tag des events

   if (Brho_min) {
      delete Brho_min;
      Brho_min = NULL;
   }

   if (Brho_max) {
      delete Brho_max;
      Brho_max = NULL;
   }

   if (Code_Ident_Vamos) {
      delete Code_Ident_Vamos;
      Code_Ident_Vamos = NULL;
   }

   if (Code_Vamos) {
      delete Code_Vamos;
      Code_Vamos = NULL;
   }

   // Correction de Tof pour identification Chio-Si

   if (Tof0) {
      delete Tof0;
      Tof0 = NULL;
   }

   if (Esi0) {
      delete Esi0;
      Esi0 = NULL;
   }

   if (Brho0) {
      delete Brho0;
      Brho0 = NULL;
   }

   // Correction de Tof pour identification Si-CsI

   if (Tof0_code1) {
      delete Tof0_code1;
      Tof0_code1 = NULL;
   }

   if (Deltat_code1) {
      delete Deltat_code1;
      Deltat_code1 = NULL;
   }

   // Correction Double tof Pics

   if (Fit1) {
      delete Fit1;
      Fit1 = NULL;
   }

   if (Fit2) {
      delete Fit2;
      Fit2 = NULL;
   }

   if (Fit3) {
      delete Fit3;
      Fit3 = NULL;
   }

   if (Fit4) {
      delete Fit4;
      Fit4 = NULL;
   }

   if (Fit5) {
      delete Fit5;
      Fit5 = NULL;
   }

   if (Fit6) {
      delete Fit6;
      Fit6 = NULL;
   }

   if (Fit7) {
      delete Fit7;
      Fit7 = NULL;
   }

   // Correction Offet relativiste

   if (Offset_rela) {
      delete Offset_rela;
      Offset_rela = NULL;
   }

   if (Offset_relativiste_chiosi) {
      delete Offset_relativiste_chiosi;
      Offset_relativiste_chiosi = NULL;
   }

   // Redressement des distributions A/Q

   if (P0_aq_straight) {
      delete P0_aq_straight;
      P0_aq_straight = NULL;
   }

   if (P1_aq_straight) {
      delete P1_aq_straight;
      P1_aq_straight = NULL;
   }

   if (P2_aq_straight) {
      delete P2_aq_straight;
      P2_aq_straight = NULL;
   }

   if (P0_aq_straight_chiosi) {
      delete P0_aq_straight_chiosi;
      P0_aq_straight_chiosi = NULL;
   }

   if (P1_aq_straight_chiosi) {
      delete P1_aq_straight_chiosi;
      P1_aq_straight_chiosi = NULL;
   }

   if (P2_aq_straight_chiosi) {
      delete P2_aq_straight_chiosi;
      P2_aq_straight_chiosi = NULL;
   }

   if (FragDetSi) {
      delete FragDetSi;
      FragDetSi = NULL;
   }

   if (FragSignalSi) {
      delete FragSignalSi;
      FragSignalSi = NULL;
   }

   if (FragESi) {
      delete FragESi;
      FragESi = NULL;
   }

   if (FragSignalCsI) {
      delete FragSignalCsI;
      FragSignalCsI = NULL;
   }

   if (FragDetCsI) {
      delete FragDetCsI;
      FragDetCsI = NULL;
   }

   if (FragDetChio) {
      delete FragDetChio;
      FragDetChio = NULL;
   }

   if (FragSignalChio) {
      delete FragSignalChio;
      FragSignalChio = NULL;
   }

   if (FragEChio) {
      delete FragEChio;
      FragEChio = NULL;
   }

   if (FragTfinal) {
      delete FragTfinal;
      FragTfinal = NULL;
   }

   if (Off_chiosi) {
      delete Off_chiosi;
      Off_chiosi = NULL;
   }

   if (Off_dc1dc2) {
      delete Off_dc1dc2;
      Off_dc1dc2 = NULL;
   }

   if (TOF_chiosi) {
      delete TOF_chiosi;
      TOF_chiosi = NULL;
   }

   if (V) {
      delete V;
      V = NULL;
   }

   if (Vx) {
      delete Vx;
      Vx = NULL;
   }

   if (Vy) {
      delete Vy;
      Vy = NULL;
   }

   if (Vz) {
      delete Vz;
      Vz = NULL;
   }

   if (Beta) {
      delete Beta;
      Beta = NULL;
   }

   if (M_Q) {
      delete M_Q;
      M_Q = NULL;
   }

   if (Etot) {
      delete Etot;
      Etot = NULL;
   }

   if (M) {
      delete M;
      M = NULL;
   }

   if (Q) {
      delete Q;
      Q = NULL;
   }

   if (Beta_chiosi) {
      delete Beta_chiosi;
      Beta_chiosi = NULL;
   }

   if (M_Q_chiosi) {
      delete M_Q_chiosi;
      M_Q_chiosi = NULL;
   }

   if (M_chiosi) {
      delete M_chiosi;
      M_chiosi = NULL;
   }

   if (Q_chiosi) {
      delete Q_chiosi;
      Q_chiosi = NULL;
   }

   if (RealQ_straight) {
      delete RealQ_straight;
      RealQ_straight = NULL;
   }

   if (Q_straight) {
      delete Q_straight;
      Q_straight = NULL;
   }

   if (M_straight) {
      delete M_straight;
      M_straight = NULL;
   }

   if (geom) {
      delete geom;
      geom = NULL;
   }

   if (geom_cross) {
      delete geom_cross;
      geom_cross = NULL;
   }

   if (geomchiosi) {
      delete geomchiosi;
      geomchiosi = NULL;
   }

   if (geomchiocsi) {
      delete geomchiocsi;
      geomchiocsi = NULL;
   }

   if (Rnd) {
      delete Rnd;
      Rnd = NULL;
   }

   if (id) {
      delete id;
      id = NULL;
   }

   if (kvn) {
      delete kvn;
      kvn = NULL;
   }

   if (grid_misses) {
      delete grid_misses;
      grid_misses = NULL;
   }

}


void Identificationv::InitReadVariables(void)
{
   geom->assign((kNumSilicon + 1) * 6, -1);
   geomchiosi->assign((kNumSilicon + 1) * 2, -1);

   P0->assign(P0->size(), 0.);
   P1->assign(P1->size(), 0.);
   P2->assign(P2->size(), 0.);
   P3->assign(P3->size(), 0.);

   P0_mq->assign(P0_mq->size(), 0.);
   P1_mq->assign(P1_mq->size(), 0.);
   Tof0->assign(Tof0->size(), 0.);
   Esi0->assign(Esi0->size(), 0.);
   Brho0->assign(Brho0->size(), 0.);
   Tof0_code1->assign(Tof0_code1->size(), 0.);
   Deltat_code1->assign(Deltat_code1->size(), 0.);

   P0_m->assign(P0_m->size(), 0.);
   P1_m->assign(P1_m->size(), 0.);

   P0_mq_chiosi->assign(P0_mq_chiosi->size(), 0.);
   P1_mq_chiosi->assign(P1_mq_chiosi->size(), 0.);

   P0_m_chiosi->assign(P0_m_chiosi->size(), 0.);
   P1_m_chiosi->assign(P1_m_chiosi->size(), 0.);

   P0_mq_sitof->assign(P0_mq_sitof->size(), 0.);
   P1_mq_sitof->assign(P1_mq_sitof->size(), 0.);

   P0_m_sitof->assign(P0_m_sitof->size(), 0.);
   P1_m_sitof->assign(P1_m_sitof->size(), 0.);

   Stat_Indra->assign(Stat_Indra->size(), -10.);
   Brho_min->assign(Brho_min->size(), -10.);
   Brho_max->assign(Brho_max->size(), -10.);
   Offset_relativiste_chiosi->assign(Offset_relativiste_chiosi->size(), 0.);

   P0_aq_straight->assign(P0_aq_straight->size(), 0.);
   P1_aq_straight->assign(P1_aq_straight->size(), 0.);
   P2_aq_straight->assign(P2_aq_straight->size(), 0.);

   P0_aq_straight_chiosi->assign(P0_aq_straight_chiosi->size(), 0.);
   P1_aq_straight_chiosi->assign(P1_aq_straight_chiosi->size(), 0.);
   P2_aq_straight_chiosi->assign(P2_aq_straight_chiosi->size(), 0.);

   Fit1->assign(Fit1->size(), -1000.);
   Fit2->assign(Fit2->size(), -1000.);
   Fit3->assign(Fit3->size(), -1000.);
   Fit4->assign(Fit4->size(), -1000.);
   Fit5->assign(Fit5->size(), -1000.);
   Fit6->assign(Fit6->size(), -1000.);
   Fit7->assign(Fit7->size(), -1000.);

   return;
}

void Identificationv::Init(void)
{
   Present = false;
   dE = dE1 = V_Etot = T_FP = Mass = -10.;
   M_simul = Z1 = Z2 = Z_tot = Z_si = D = -10.0;
   M_Qr = Mr = Qr = -10.0;
   Qc = Mc = -10.0;

   Gamma = 1.;
   initThickness = 0.;

   einc_si = einc_isogap1 = eloss_isogap1 = 0.0;
   einc_ic = eloss_ic = einc_dc2 = eloss_dc2 = 0.0;
   einc_sed = eloss_sed = einc_dc1 = eloss_dc1 = 0.0;
   einc_tgt = eloss_tgt = 0.0;

   E_tgt = E_dc1 = E_dc2 = E_sed = E_gap1 = E_chio = 0.0;

   for (Int_t i = 0; i < 3; i++) {
      fELosLayer_dc1[i] = 0.0;
      fELosLayer_dc2[i] = 0.0;
      fELosLayer_ic[i] = 0.0;
   }

   runNumber = 0;
   runNumber = (Int_t)gIndra->GetCurrentRunNumber();

   AA->assign(AA->size(), -10.);
   FragECsI->assign(FragECsI->size(), -10.);
   FragEGap->assign(FragEGap->size(), -10.);

   PID->assign(PID->size(), -10.);
   Z_PID->assign(Z_PID->size(), -10.);
   A_PID->assign(A_PID->size(), -10.);

   PID_sitof->assign(PID_sitof->size(), -10.);
   Z_PID_sitof->assign(Z_PID_sitof->size(), -10.);
   PID_chiov2->assign(PID_chiov2->size(), -10.);
   Z_PID_chiov2->assign(Z_PID_chiov2->size(), -10.);

   FragDetSi->assign(FragDetSi->size(), -10);
   FragSignalSi->assign(FragSignalSi->size(), -10);
   FragESi->assign(FragESi->size(), -10.);

   FragTfinal->assign(FragTfinal->size(), -10.);
   Off_chiosi->assign(Off_chiosi->size(), -10.);
   Off_dc1dc2->assign(Off_dc1dc2->size(), -10.);
   TOF_chiosi->assign(TOF_chiosi->size(), -10.);

   FragSignalCsI->assign(FragSignalCsI->size(), -10);
   FragDetCsI->assign(FragDetCsI->size(), -10);

   FragDetChio->assign(FragDetChio->size(), -10);
   FragSignalChio->assign(FragSignalChio->size(), -10);
   FragEChio->assign(FragEChio->size(), -10.);

   Etot->assign(Etot->size(), -10.);

   V->assign(V->size(), -10.);
   Vx->assign(Vx->size(), -10.);
   Vy->assign(Vy->size(), -10.);
   Vz->assign(Vz->size(), -10.);

   Beta->assign(Beta->size(), -10.);

   M_Q->assign(M_Q->size(), -10.);
   Q->assign(Q->size(), -10.);
   M->assign(M->size(), -10.);

   RealQ_straight->assign(RealQ_straight->size(), -10.);

   M_straight->assign(M_straight->size(), -10.);
   Q_straight->assign(Q_straight->size(), -10);

   Beta_chiosi->assign(Beta_chiosi->size(), -10.);
   M_Q_chiosi->assign(M_Q_chiosi->size(), -10.);
   Q_chiosi->assign(Q_chiosi->size(), -10.);
   M_chiosi->assign(M_chiosi->size(), -10.);

   Code_Vamos->assign(Code_Vamos->size(), -10);
   Code_Ident_Vamos->assign(Code_Ident_Vamos->size(), -10);

   Z_corr->assign(Z_corr->size(), -10.);
   Z_corr_sitof->assign(Z_corr_sitof->size(), -10);
   Z_corr_chiov2->assign(Z_corr_chiov2->size(), -10);

   MaxM = 0;

   fMultiplicity_case = -10;
   Code_good_event = -10;

   print = kFALSE;

   return;
}


void Identificationv::InitSavedQuantities(void)
{
   FragDetSi->assign(FragDetSi->size(), -10);
   FragSignalSi->assign(FragSignalSi->size(), -10);
   FragESi->assign(FragESi->size(), -10.);

   FragTfinal->assign(FragTfinal->size(), -10.);
   Off_chiosi->assign(Off_chiosi->size(), -10.);
   Off_dc1dc2->assign(Off_dc1dc2->size(), -10.);
   TOF_chiosi->assign(TOF_chiosi->size(), -10.);

   FragSignalCsI->assign(FragSignalCsI->size(), -10);
   FragDetCsI->assign(FragDetCsI->size(), -10);

   FragDetChio->assign(FragDetChio->size(), -10);
   FragSignalChio->assign(FragSignalChio->size(), -10);
   FragEChio->assign(FragEChio->size(), -10.);

   return;
}

//===================================================

Int_t Identificationv::GetMaxMultiplicity(void)
{
   if (Rec->Brho < 0) {
      fMultiplicity_case = -3;
      Code_good_event = 3;
      return 0;
   }

   if (Si->EMSI >= Ic->EMIC && Si->EMSI >= CsI->EMCSI) {
      fMultiplicity_case = 1;
      return Si->EMSI;
   } else if (CsI->EMCSI > Si->EMSI && CsI->EMCSI >= Ic->EMIC) {
      fMultiplicity_case = 2;
      return CsI->EMCSI;
   } else if (Ic->EMIC > Si->EMSI && Ic->EMIC > CsI->EMCSI) {
      fMultiplicity_case = 0;
      return Ic->EMIC;
   } else {
      fMultiplicity_case = -10;
      std::cout << "Not suppose to enter here GetMaxMultiplicty()" <<
                std::endl;
      return 0;
   }

}

void Identificationv::FragPropertiesInOrder(void)
{
   Int_t nhit_sicsi = 0;
   Int_t nhit_chiosi = 0;
   Int_t nhit_chiocsi = 0;

   MaxM = GetMaxMultiplicity();

   Code_good_event = 0;

   std::vector<Bool_t> used_silicon;
   used_silicon.resize(Si->EMSI);

   Bool_t not_recoverable = kFALSE;

   for (Int_t i = 0; i < Si->EMSI; i++) {
      used_silicon[i] = kFALSE;
   }

   InitSavedQuantities();

   if (debug) {
      std::cout << "fMultiplicity_case : " << fMultiplicity_case << std::endl;
      std::cout << "code good event : " << Code_good_event << std::endl;

      for (Int_t i = 0; i < GetMaxMultiplicity(); i++) {
         std::cout << "Chio before : " << FragDetChio->at(i) << "	" <<
                   FragSignalChio->at(i) << "	" << FragEChio->at(i) <<
                   std::endl;

         std::cout << "Si before : " << FragDetSi->at(i) << "	" <<
                   FragSignalSi->at(i) << "	" << FragESi->at(i) <<
                   std::endl;

         std::cout << "CsI before : " << FragDetCsI->at(i) << "	" <<
                   FragSignalCsI->at(i) << std::endl;
      }
   }

   if (fMultiplicity_case == 1) {
      // Si->EMSI greater or equal to the other multiplicities

      Code_good_event = -10;
      for (Int_t i = 0; i < Si->EMSI; i++) {

         if (Si->EMSI == 1) {
            if (Si->DetSi[i] == 16) {
               if (Si->E[i] > ((4.36 * Si->T_final[i]) - 311.)) {
                  if (
                     (
                        (CsI->DetCsI[i] == 45) &&
                        Si->E[i] <
                        (
                           (-0.0548 * CsI->CsIERaw[i]) + 384.
                        ) &&
                        Si->E[i] >
                        (
                           (-0.0556 * CsI->CsIERaw[i]) + 340.
                        )
                     ) || (
                        (CsI->DetCsI[i] == 46) &&
                        Si->E[i] < (
                           (-0.0328 * CsI->CsIERaw[i]) + 365.
                        ) &&
                        Si->E[i] > (
                           (-0.0353 * CsI->CsIERaw[i]) + 345.
                        )
                     )
                  ) {

                     std::cout << "DetCsI : " << CsI->DetCsI[i] <<
                               std::endl;

                     std::cout << "ESi : " << Si->E[i] << "	Tof : " <<
                               Si->T_final[i] << "	CsICanal : " <<
                               CsI->CsIERaw[i] << std::endl;

                     print = kTRUE;
                  }
               }
            }
         }

         FragDetSi->at(i) = Si->DetSi[i];

         FragSignalSi->at(i) = Si->SiERaw[i];
         FragESi->at(i) = Si->E[i];
         FragTfinal->at(i) = Si->T_final[i];
         if (!(FragTfinal->at(i) > 0.)) {
            std::cerr << "ERR: TFINAL: " << FragTfinal->at(i) << std::endl;
         }

         assert(FragTfinal->at(i) > 0.);

         if (debug) {
            std::cout << "i : " << i << " FragDetSi[i] : " <<
                      FragDetSi->at(i) << " FragSignalSi[i] : " <<
                      FragSignalSi->at(i) << " FragESi[i] : " <<
                      FragESi->at(i) << "	FragTfinal : " <<
                      FragTfinal->at(i) << std::endl;
         }

         for (Int_t j = 0; j < CsI->EMCSI; j++) {

            if (debug) {
               std::cout << "j : " << j << " " << CsI->DetCsI[j] << " " <<
                         CsI->CsIERaw[j] << std::endl;

               std::cout << "geometry sicsi : " <<
                         IsSiCsITelescope(
                            Si->DetSi[i] - 1, CsI->DetCsI[j] - 1
                         ) << std::endl;
            }

            if (
               CsI->CsIERaw[j] > 0 &&
               IsSiCsITelescope(Si->DetSi[i] - 1, CsI->DetCsI[j] - 1) == 1
            ) {
               if (nhit_sicsi > 0) {
                  if (debug) {
                     std::cout << "Double Hit In SiCsI" << std::endl;
                  }
               }

               FragSignalCsI->at(i) = CsI->CsIERaw[j];
               FragDetCsI->at(i) = CsI->DetCsI[j];

               nhit_sicsi++;

            } // IsSiCsITelescope
         } // Multiplicity CsI

         for (Int_t k = 0; k < Ic->EMIC; k++) {

            if (debug) {

               std::cout << "k : " << k << " " << Ic->DetChio[k] << " " <<
                         Ic->E[k] << std::endl;

               std::cout << "geometry chiosi : " <<
                         IsChioSiTelescope(
                            Ic->DetChio[k] - 1, Si->DetSi[i] - 1
                         ) << std::endl;
            }

            if (
               Ic->E[k] > 0 &&
               IsChioSiTelescope(Ic->DetChio[k] - 1, Si->DetSi[i] - 1) == 1
            ) {
               if (nhit_chiosi > 0) {
                  if (debug) {
                     std::cout << "Double Hit In ChioSi" << std::endl;
                  }
               }

               FragSignalChio->at(i) = Ic->IcRaw[k];
               FragDetChio->at(i) = Ic->DetChio[k];
               FragEChio->at(i) = Ic->E[k];

               nhit_chiosi++;

            } // IsChioSiTelescope
         } // Multiplicity IC

         if (nhit_sicsi > 1 || nhit_chiosi > 1) {
            Code_good_event = 4;
            break;
         }
         nhit_sicsi = 0;
         nhit_chiosi = 0;

      } // Multiplicity Si

      if (Si->EMSI == CsI->EMCSI && Si->EMSI == 1) {
         // We restrict to events with multiplicity = 1, because after we
         // analyze only mulitplicity 1 events

         if (FragDetCsI->at(0) == -10) {
            if (
               IsCsISiCrossTelescope(
                  FragDetSi->at(0) - 1, CsI->DetCsI[0] - 1
               ) == 1
            ) {
               Code_good_event = 6;
               // Fragment hit one Si and enters in the CsI behind a
               // neighbour Si. We can't identify those events in Chio-Si,
               // because the ptcle. is not stopped in Si
            } else {
               Code_good_event = 7;
               // No relation between the Si and the CsI. Possible?
               // Chio-Si identification if there's a chio, but we lose the
               // information about the CsI.
            }

         }
      }
   } else if (fMultiplicity_case == 2) {

      for (Int_t i = 0; i < CsI->EMCSI; i++) {

         FragSignalCsI->at(i) = CsI->CsIERaw[i];
         FragDetCsI->at(i) = CsI->DetCsI[i];


         for (Int_t j = 0; j < Si->EMSI; j++) {
            if (
               Si->E[j] > 0 &&
               IsSiCsITelescope(Si->DetSi[j] - 1, CsI->DetCsI[i] - 1) == 1
            ) {
               if (used_silicon[j] == kTRUE) {
                  not_recoverable = kTRUE;
                  if (debug) {
                     std::cout << "2 CsI behind the same Si" <<
                               std::endl;
                  }
               }
               if (nhit_sicsi > 0) {
                  if (debug) {
                     std::cout << "Double Hit In SiCsI" << std::endl;
                  }
               }
               used_silicon[j] = kTRUE;

               FragDetSi->at(i) = Si->DetSi[j];
               FragSignalSi->at(i) = Si->SiERaw[j];
               FragESi->at(i) = Si->E[j];
               FragTfinal->at(i) = Si->T_final[j];
               assert(FragTfinal->at(i) > 0.);

               nhit_sicsi++;

            } // IsSiCsITelescope
         } // Multiplicity Si

         for (Int_t k = 0; k < Ic->EMIC; k++) {
            if (
               Ic->E[k] > 0 &&
               IsChioCsITelescope(
                  Ic->DetChio[k] - 1, CsI->DetCsI[i] - 1
               ) == 1
            ) {
               if (nhit_chiocsi > 0) {
                  if (debug) {
                     std::cout << "Double Hit In ChioCsI" << std::endl;
                  }
               }

               FragSignalChio->at(i) = Ic->IcRaw[k];
               FragDetChio->at(i) = Ic->DetChio[k];
               FragEChio->at(i) = Ic->E[k];

               nhit_chiocsi++;

            } // IsChioSiTelescope
         } // Multiplicity IC

         if (
            nhit_sicsi > 1 ||
            nhit_chiocsi > 1 ||
            not_recoverable == kTRUE
         ) {
            // if(nhit_sicsi>1 || nhit_chiocsi>1 ){
            // TODO: Rename these codes to something useful
            Code_good_event = 4;
            break;
         }
         nhit_sicsi = 0;
         nhit_chiocsi = 0;
      } // Multiplicity CsI

      if (
         (CsI->EMCSI == 2 && Si->EMSI == 1) &&
         ((FragDetSi->at(0) > 0) ^ (FragDetSi->at(1) > 0))
      ) {
         Code_good_event = -8;
      } else {
         Code_good_event = 9;
      }

      if (
         (CsI->EMCSI == 3 && Si->EMSI == 1) &&
         ((FragDetSi->at(0) > 0) ^ (FragDetSi->at(1) > 0) ^
          (FragDetSi->at(2) > 0)
         )
      ) {
         Code_good_event = -8;
      } else {
         Code_good_event = 9;
      }

      if (CsI->EMCSI == 2 && Si->EMSI == 1 && FragDetSi->at(0) == -10) {
         // We restrict to events with multiplicity = 1, because after we
         // analyze only mulitplicity 1 events
         for (Int_t i = 0; i < CsI->EMCSI; i++) {
            if (
               IsCsISiCrossTelescope(
                  Si->DetSi[0] - 1, CsI->DetCsI[i] - 1
               ) == 1
            ) {
               Code_good_event = 6;
               // Fragment hit one Si and enters in the CsI behind a
               // neighbour Si. We can't identify those events in Chio-Si,
               // because the ptcle. is not stopped in Si
            }
         }
         if (Code_good_event == 0) {
            Code_good_event = 9;

         }
      }

   } else if (fMultiplicity_case == 0) {

      Code_good_event = 10;

      for (Int_t i = 0; i < Ic->EMIC; i++) {

         FragSignalChio->at(i) = Ic->IcRaw[i];
         FragDetChio->at(i) = Ic->DetChio[i];
         FragEChio->at(i) = Ic->E[i];

         for (Int_t j = 0; j < Si->EMSI; j++) {
            if (
               Si->E[j] > 0 &&
               IsChioSiTelescope(
                  Ic->DetChio[i] - 1, Si->DetSi[j] - 1
               ) == 1
            ) {
               if (nhit_chiosi > 0) {
                  if (debug) {
                     std::cout << "Double Hit In ChioSi" << std::endl;
                  }
               }

               FragDetSi->at(i) = Si->DetSi[j];
               FragSignalSi->at(i) = Si->SiERaw[j];
               FragESi->at(i) = Si->E[j];
               assert((FragESi->at(i) >= 0.) && (FragESi->at(i) < 3000.));
               FragTfinal->at(i) = Si->T_final[j];
               assert((FragTfinal->at(i) > 0.) && (FragTfinal->at(i) < 240.));

               nhit_chiosi++;

            } // IsSiCsITelescope
         } // Multiplicity Si

         for (Int_t j = 0; j < CsI->EMCSI; j++) {
            if (
               CsI->CsIERaw[j] > 0 &&
               IsChioCsITelescope(
                  Ic->DetChio[i] - 1, CsI->DetCsI[j] - 1
               ) == 1
            ) {
               if (nhit_chiocsi > 0) {
                  if (debug) {
                     std::cout << "Double Hit In ChioCsI" << std::endl;
                  }
               }

               FragSignalCsI->at(i) = CsI->CsIERaw[j];
               FragDetCsI->at(i) = CsI->DetCsI[j];

               nhit_chiocsi++;

            } // IsSiCsITelescope
         } // Multiplicity CsI

         if (nhit_sicsi > 1 || nhit_chiocsi > 1) {
            Code_good_event = 4;
            break;
         }
         nhit_chiosi = 0;
         nhit_chiocsi = 0;
      }
   } else {
      Code_good_event = 4;
      if (debug) {
         std::cout << "Should not happen FragPropertiesInOrder()" <<
                   std::endl;
      }
   }

   if (debug) {
      std::cout << "Code good event at the end : " << Code_good_event <<
                std::endl;
   }

   if (Code_good_event == 4) {
      if (debug) std::cout << "Reset everything" << std::endl;

      FragDetSi->assign(FragDetSi->size(), -10);
      FragSignalSi->assign(FragSignalSi->size(), -10);
      FragESi->assign(FragESi->size(), -10.);

      FragTfinal->assign(FragTfinal->size(), -10.);
      FragSignalCsI->assign(FragSignalCsI->size(), -10);
      FragDetCsI->assign(FragDetCsI->size(), -10);

      FragDetChio->assign(FragDetChio->size(), -10);
      FragSignalChio->assign(FragSignalChio->size(), -10);
      FragEChio->assign(FragEChio->size(), -10.);

   }

   if (debug) {
      for (Int_t i = 0; i < GetMaxMultiplicity(); i++) {
         std::cout << "Chio : " << FragDetChio->at(i) << "	" <<
                   FragSignalChio->at(i) << "	" << FragEChio->at(i) <<
                   std::endl;
         std::cout << "Si : " << FragDetSi->at(i) << "	" <<
                   FragSignalSi->at(i) << "	" << FragESi->at(i) <<
                   std::endl;
         std::cout << "CsI : " <<
                   FragSignalCsI->at(i) << std::endl;
         std::cout << "Tof : " << FragTfinal->at(i) << std::endl;
      }
   }

   return;
}

void Identificationv::Calculate(void)
{
   //=================================================
   // Z Identification

   Bool_t identified = kFALSE;

   if (Code_good_event == -10  && Si->EMSI == 1) {

      for (Int_t y = 0; y < Si->EMSI ; y++) {
         identified = kFALSE;

         // if the channel is above the pedestal
         if (FragSignalCsI->at(y) > 0) {

            // IdentificationSi-CsI
            energytree->InitTelescope(FragDetSi->at(y) - 1,
                                      FragDetCsI->at(y) - 1);
            energytree->InitSiCsI(FragDetSi->at(y) - 1);
            energytree->SetCalibration(Si, CsI, FragDetSi->at(y) - 1,
                                       FragDetCsI->at(y) - 1);

            id = new KVIdentificationResult();
            id_sitof = new KVIdentificationResult();

            TString scope_name;

            const KVIDGrid* grid_kvid = energytree->get_kvid();
            if (!grid_kvid) {

               ++grid_misses->sicsi;

               // ID failed because no Si-CsI grid
               Code_Ident_Vamos->at(y) = 31;

               // TODO: Why is identified set?
               identified = kTRUE;

            } else {

               if (
                  grid_kvid->IsIdentifiable(
                     static_cast<Double_t>(FragSignalCsI->at(y)),
                     static_cast<Double_t>(FragESi->at(y))
                  )
               ) {
                  // Code_Ident_Vamos[y]=1;
                  // if(debug) std::cout << "Code1 : "
                  // << Code_Ident_Vamos[y]<< std::endl;

                  // energytree->kvid : KVIDGraph  Identify(x,y)
                  grid_kvid->Identify(
                     static_cast<Double_t>(FragSignalCsI->at(y)),
                     static_cast<Double_t>(FragESi->at(y)),
                     id);

                  A_PID->at(y) = id->A;
                  Z_PID->at(y) = id->Z;
                  Z_corr->at(y) = Z_PID->at(y);
                  PID->at(y) = id->PID;

                  energytree->SetFragmentZ(Z_corr->at(y));

                  // Method called for guessing A value by bissection
                  // method and getting CsI energy
                  energytree->GetResidualEnergyCsI(
                     static_cast<Double_t>(FragESi->at(y)),
                     static_cast<Double_t>(FragSignalCsI->at(y)));

                  if (energytree->get_good_bisection() == kFALSE) {
                     Code_Ident_Vamos->at(y) = 37;
                  }

                  FragECsI->at(y) = energytree->RetrieveEnergyCsI();
                  FragEGap->at(y) = energytree->get_eEnergyGap();

                  AA->at(y) = energytree->RetrieveA();

                  identified = kTRUE;

                  energytree->InitTelescopeSiTof(
                     FragDetSi->at(y) - 1
                  );

                  const KVIDGrid* grid_sitof =
                     energytree->get_kvid_sitof();

                  if (!grid_sitof) {
                     Code_Ident_Vamos->at(y) = 38;
                     ++grid_misses->sitof;

                  } else {

                     if (
                        grid_sitof->IsIdentifiable(
                           static_cast<Double_t>(
                              FragTfinal->at(y)
                           ),
                           static_cast<Double_t>(
                              FragESi->at(y)
                           )
                        )
                     ) {
                        // std::cout << "Identifiable" <<
                        // std::endl;
                        grid_sitof->Identify(
                           static_cast<Double_t>(
                              FragTfinal->at(y)
                           ),
                           static_cast<Double_t>(
                              FragESi->at(y)
                           ),
                           id_sitof
                        );

                        PID_sitof->at(y) = id_sitof->PID;
                        Z_PID_sitof->at(y) = id_sitof->Z;
                        Z_corr_sitof->at(y) =
                           Z_PID_sitof->at(y);

                        if (
                           (
                              id_sitof->PID >=
                              (PID->at(y) - 0.7)
                           ) && (
                              id_sitof->PID <=
                              (PID->at(y) + 0.7)
                           )
                        ) {
                           Code_Ident_Vamos->at(y) = 1;
                           // std::cout << "Good Z. Youppi!" <<
                           // std::endl;

                        } else {
                           Code_Ident_Vamos->at(y) = 39;
                           // std::cout << "Bad Z. Bou!" <<
                           // std::endl;
                           // std::cout << "TOF : " <<
                           // FragTfinal[y]<< "
                           // ESI : " << FragESi[y]<< "
                           // DetSi : " <<
                           // FragDetSi[y]<< "  Z Si-CsI : " <<
                           // PID[y]<<
                           //"   Z Chio-Si : " << id_sitof->PID
                           //<< std::endl;
                        }
                     } else {
                        Code_Ident_Vamos->at(y) = 40;
                        // std::cout << "Non identifiable
                        // Si-TOF" << std::endl;
                        // std::cout << "TOF : " <<
                        // FragTfinal[y]<< "
                        // ESI : " << FragESi[y]<< "  DetSi :
                        // " <<
                        // FragDetSi[y]<< "  Z Si-CsI : " <<
                        // PID[y]<<
                        // std::endl;
                     }


                  }

                  if (print) {
                     std::cout << "DetCsI : " << CsI->DetCsI[y]
                               << std::endl;

                     std::cout << "ESi : " << Si->E[y] << "	Tof : "
                               << Si->T_final[y] <<
                               "	CsICanal : " << CsI->CsIERaw[y]
                               << std::endl;

                     std::cout << "Z : " << PID->at(y)
                               << "	ECsI : " << FragECsI->at(y) <<
                               std::endl;
                  }

               }  // Is Identifiable
               else {
                  // Signal in CsI, but noise...
                  Code_Ident_Vamos->at(y) = 35;
                  identified = kTRUE;
               }

            }

            delete id;
            id = NULL;

            delete id_sitof;
            id_sitof = NULL;

         } // Signal CsI>piedestal

         if (FragEChio->at(y) > 0 && FragSignalCsI->at(y) <= 0) {
            // Identification Chio-Si

            energytree->InitTelescopeChioSi(
               FragDetChio->at(y) - 1, FragDetSi->at(y) - 1);

            id_chiosi = new KVIdentificationResult();

            TString scope_name;

            const KVIDGrid* grid_chiosi = energytree->get_kvid_chiosi();
            if (!grid_chiosi) {

               ++grid_misses->chiosi;

               // ID failed because no Chio-Si grid
               Code_Ident_Vamos->at(y) = 32;
               identified = kTRUE;

            } else {

               if (
                  grid_chiosi->IsIdentifiable(
                     FragESi->at(y), FragEChio->at(y)
                  )
               ) {
                  Code_Ident_Vamos->at(y) = 2;

                  // energytree->kvid : KVIDGraph  Identify(x,y)
                  grid_chiosi->Identify(
                     FragESi->at(y),
                     FragEChio->at(y),
                     id_chiosi);


                  A_PID->at(y) = id_chiosi->A;
                  Z_PID->at(y) = id_chiosi->Z;
                  Z_corr->at(y) = Z_PID->at(y);
                  PID->at(y) = id_chiosi->PID;


                  identified = kTRUE;

               } // IsIdentificable
               else {
                  // Signal in Chio, but noise...
                  Code_Ident_Vamos->at(y) = 36;
                  identified = kTRUE;
               }

            }

            delete id_chiosi;
            id_chiosi = NULL;

         } // identified==kFALSE && FragEChio[y]>0

         if (identified == kFALSE) {
            Code_Ident_Vamos->at(y) = 3;
         }

      } // Multiplicity Si

      if (debug) {
         for (Int_t y = 0; y < Si->EMSI ; y++) {
            if (Code_Ident_Vamos->at(y) < 0) {
               std::cout <<
                         "**************Code_Ident_Vamos=-10 "
                         "OUT**************" << std::endl;
            }
         }
      }
   } // good event

   if (Code_good_event == -8) {
      for (Int_t y = 0; y < CsI->EMCSI ; y++) {
         identified = kFALSE;

         if (FragESi->at(y) > 0.) { // if the channel is above the pedestal
            // IdentificationSi-CsI

            energytree->InitTelescope(FragDetSi->at(y) - 1,
                                      FragDetCsI->at(y) - 1);
            energytree->InitSiCsI(FragDetSi->at(y) - 1);
            energytree->SetCalibration(Si, CsI, FragDetSi->at(y) - 1,
                                       FragDetCsI->at(y) - 1);

            id = new KVIdentificationResult();
            id_sitof = new KVIdentificationResult();

            TString scope_name;

            const KVIDGrid* grid_kvid = energytree->get_kvid();
            if (!grid_kvid) {

               ++grid_misses->sicsi;

               // ID failed because no Si-CsI grid
               Code_Ident_Vamos->at(y) = 31;

            } else {

               if (
                  grid_kvid->IsIdentifiable(
                     static_cast<Double_t>(FragSignalCsI->at(y)),
                     static_cast<Double_t>(FragESi->at(y)))
               ) {
                  Code_Ident_Vamos->at(y) = 1;

                  if (debug) {
                     std::cout << "Code1 : " <<
                               Code_Ident_Vamos->at(y) <<
                               std::endl;
                  }

                  // energytree->kvid : KVIDGraph  Identify(x,y)
                  grid_kvid->Identify(
                     static_cast<Double_t>(FragSignalCsI->at(y)),
                     static_cast<Double_t>(FragESi->at(y)),
                     id);

                  A_PID->at(y) = id->A;
                  Z_PID->at(y) = id->Z;
                  Z_corr->at(y) = Z_PID->at(y);
                  PID->at(y) = id->PID;

                  energytree->SetFragmentZ(Z_corr->at(y));

                  // Method called for guessing A value by bissection
                  // method and getting CsI energy
                  energytree->GetResidualEnergyCsI(
                     static_cast<Double_t>(FragESi->at(y)),
                     static_cast<Double_t>(FragSignalCsI->at(y)));

                  if (energytree->get_good_bisection() == kFALSE) {
                     Code_Ident_Vamos->at(y) = 37;
                  }

                  FragECsI->at(y) = energytree->RetrieveEnergyCsI();
                  FragEGap->at(y) = energytree->get_eEnergyGap();

                  AA->at(y) = energytree->RetrieveA();

                  identified = kTRUE;

                  energytree->InitTelescopeSiTof(
                     FragDetSi->at(y) - 1
                  );

                  const KVIDGrid* grid_sitof =
                     energytree->get_kvid_sitof();

                  if (!grid_sitof) {
                     Code_Ident_Vamos->at(y) = 38;
                     ++grid_misses->sitof;
                  } else {

                     if (grid_sitof->IsIdentifiable(
                              static_cast<Double_t>(
                                 FragTfinal->at(y)
                              ),
                              static_cast<Double_t>(
                                 FragESi->at(y)
                              )
                           )
                        ) {

                        grid_sitof->Identify(
                           static_cast<Double_t>(
                              FragTfinal->at(y)
                           ),
                           static_cast<Double_t>(
                              FragESi->at(y)
                           ),
                           id_sitof
                        );

                        PID_sitof->at(y) = id_sitof->PID;
                        Z_PID_sitof->at(y) = id_sitof->Z;
                        Z_corr_sitof->at(y) =
                           Z_PID_sitof->at(y);

                        if (
                           (
                              id_sitof->PID >=
                              (PID->at(y) - 0.5)
                           ) && (
                              id_sitof->PID <=
                              (PID->at(y) + 0.5)
                           )
                        ) {
                           Code_Ident_Vamos->at(y) = 1;
                        } else {
                           Code_Ident_Vamos->at(y) = 39;
                        }
                     } else {
                        Code_Ident_Vamos->at(y) = 40;
                     }
                  }

               }  // Is Identifiable
               else {
                  Code_Ident_Vamos->at(y) = 36;
               }

            }

            delete id;
            id = NULL;

            delete id_sitof;
            id_sitof = NULL;

         } // Energy Si
         else {
            Code_Ident_Vamos->at(y) = 34;
            if (debug) {
               std::cout << "Code 34 : " << Code_Ident_Vamos->at(y) <<
                         std::endl;
            }
         }

         // if(identified == kFALSE){
         // Code_Ident_Vamos[y]=3;
         // std::cout << "Code Ident : " << Code_Ident_Vamos[y]<< std::endl;
         //}

      }// CsI Multiplicity

      if (debug) {
         for (Int_t y = 0; y < CsI->EMCSI ; y++) {
            if (Code_Ident_Vamos->at(y) < 0) {
               std::cout << "**************Code_Ident_Vamos=-10 "
                         "OUT**************" << std::endl;
            }
         }
      }

   } // Code good event

   //==========================================================================
   // Offset for Si-CsI events (Code_Ident_Vamos=1).
   // We observe a shift of 40ns that we can't understand...Shift them back!

   // Distance : 9423mm(si layer position) - 8702.5mm(focal plane position) =
   // 720.50 mm
   D = (1 / TMath::Cos(Dr->Pf / 1000.)) *
       (
          Rec->Path +
          (
             ((Rec->DSI - Dr->FocalPos) / 10.) /
             TMath::Cos(Dr->Tf / 1000.)
          )
       );

   if ((Code_good_event == -10  && Si->EMSI == 1) || (Code_good_event == -8)) {
      for (Int_t y = 0; y < MaxM ; y++) {
         if (Code_Ident_Vamos->at(y) == 1) {
            if (FragTfinal->at(y) < Tof0_code1->at(runNumber)) {

               FragTfinal->at(y) = FragTfinal->at(y) +
                                   Deltat_code1->at(runNumber);
               assert(FragTfinal->at(y) > 0.);

               Code_Ident_Vamos->at(y) = 40;
            }
         }
      }
   }


   // Reconstruct Tof for Chio-si events adding the RF period
   // Frequency for the two beams : 8.5500 MHz

   if ((Code_good_event == -10  && Si->EMSI == 1) || (Code_good_event == -8)) {
      for (Int_t y = 0; y < MaxM ; y++) {
         if (Code_Ident_Vamos->at(y) == 2) {
            const KVIDGrid* grid_cutscode2 =
               energytree->get_kvid_cutscode2();

            if (!grid_cutscode2) {
               ++grid_misses->cutscode2;
            } else {
               if ((runNumber > 510 && runNumber < 515) ||
                     runNumber == 424 || runNumber == 420 ||
                     runNumber == 423 || runNumber == 360 ||
                     runNumber == 516 || runNumber == 517 ||
                     (runNumber > 501 && runNumber < 507)
                  ) {
                  // Runs with a cut line, nothing to reject

                  if (grid_cutscode2->IsIdentifiable(
                           FragTfinal->at(y), FragESi->at(y))
                     ) {
                     if (runNumber == 360) {

                        FragTfinal->at(y) = FragTfinal->at(y) +
                                            (2. *
                                             (
                                                10.0E+09 /
                                                (8.5500 * 10.0E06)
                                             )
                                            );
                        assert(FragTfinal->at(y) > 0.);

                        Code_Ident_Vamos->at(y) = 22;

                        if (Z_corr->at(y) < 10) {
                           Code_Ident_Vamos->at(y) = 41;
                        }

                     } else {

                        FragTfinal->at(y) = FragTfinal->at(y) +
                                            (
                                               10.0E+09 /
                                               (8.5500 * 10.0E06)
                                            );
                        assert(FragTfinal->at(y) > 0.);

                        Code_Ident_Vamos->at(y) = 22;

                        if (runNumber == 424 || runNumber == 420 ||
                              runNumber == 423 || runNumber == 360 ||
                              runNumber == 516 || runNumber == 517
                           ) {
                           if (Z_corr->at(y) < 10) {
                              Code_Ident_Vamos->at(y) = 41;
                           }
                        } else if (
                           (runNumber > 501 && runNumber < 505)
                        ) {
                           if (Z_corr->at(y) < 7)  {
                              Code_Ident_Vamos->at(y) = 41;
                           }
                        } else if (
                           (runNumber > 504 && runNumber < 507)
                        ) {
                           if (Z_corr->at(y) < 9) {
                              Code_Ident_Vamos->at(y) = 41;
                           }
                        }
                     }
                  } else {
                     if (runNumber == 360) {

                        FragTfinal->at(y) = FragTfinal->at(y) +
                                            (
                                               10.0E+09 /
                                               (8.5500 * 10.0E06)
                                            );
                        assert(FragTfinal->at(y) > 0.);

                     } else   {
                        // TODO: Err...why?
                        FragTfinal->at(y) = FragTfinal->at(y);
                        assert(FragTfinal->at(y) > 0.);
                     }
                  }
               }  // runs with a cut line, nothing to reject
               else {
                  // runs where we reject something, we don't
                  // understand...

                  if
                  (
                     grid_cutscode2->IsIdentifiable(
                        FragTfinal->at(y), FragESi->at(y)
                     )
                  ) {
                     if (
                        runNumber > 516 &&
                        runNumber < 520 &&
                        (
                           FragTfinal->at(y) >
                           Tof0->at(runNumber)
                        )
                     ) {
                        // FragTfinal[y] =
                        // FragTfinal[y]-(10.0E+09/(8.5500*10.0E06));

                        // TODO: Why this assignment?
                        FragTfinal->at(y) = FragTfinal->at(y);
                        assert(FragTfinal->at(y) > 0.);
                        Code_Ident_Vamos->at(y) = 22;

                     }
                     // for those 2 runs, we have to substract the HF
                     // period, check done on Total Energy-ToF spectras
                     else {
                        if (
                           FragTfinal->at(y) <
                           Tof0->at(runNumber)
                        ) {

                           FragTfinal->at(y) = FragTfinal->at(y) +
                                               (
                                                  10.0E+09 /
                                                  (8.5500 * 10.0E06)
                                               );
                           assert(FragTfinal->at(y) > 0.);

                           Code_Ident_Vamos->at(y) = 22;
                        }
                     }
                  }  // fragment identifiable, good fragments!
                  else {
                     FragTfinal->at(y) = FragTfinal->at(y) +
                                         Deltat_code1->at(runNumber);
                     assert(FragTfinal->at(y) > 0.);

                     Code_Ident_Vamos->at(y) = 40;
                  }  // if not identifiable, fragment inside the contour
               }  // runs where we reject something
            }  // grid not NULL


            // Comparaison de l'identification en Z pour l'identification
            // Chio-Si (Code_Ident_Vamos=2)
            id_chiov2 = new KVIdentificationResult();

            TString scope_name;

            energytree->InitChioV2(FragDetChio->at(y) - 1);

            const KVIDGrid* grid_chiov2 = energytree->get_kvid_chiov2();

            if (!grid_chiov2) {
               ++grid_misses->chiov2;
               Code_Ident_Vamos->at(y) = 38;
            } else {

               if (
                  grid_chiov2->IsIdentifiable
                  (
                     static_cast<Double_t>(
                        (D / FragTfinal->at(y)) *
                        (D / FragTfinal->at(y))),

                     static_cast<Double_t>(FragEChio->at(y))
                  )
               ) {

                  grid_chiov2->Identify(

                     static_cast<Double_t>(
                        (D / FragTfinal->at(y)) *
                        (D / FragTfinal->at(y))),

                     static_cast<Double_t>(FragEChio->at(y)),

                     id_chiov2
                  );

                  PID_chiov2->at(y) = id_chiov2->PID;
                  Z_PID_chiov2->at(y) = id_chiov2->Z;
                  Z_corr_chiov2->at(y) = Z_PID_chiov2->at(y);

                  // std::cout << "PID chiov2 : " << PID_chiov2[y]<<
                  // std::endl;
                  if ((PID_chiov2->at(y) >= (PID->at(y) - 0.5)) &&
                        (PID_chiov2->at(y) <= (PID->at(y) + 0.5))
                     ) {
                     // std::cout << "diff : " <<
                     // id_chiov2->PID+1.-PID[y]<< std::endl;
                     Code_Ident_Vamos->at(y) = 2;
                  } else {
                     Code_Ident_Vamos->at(y) = 39;
                  }
               } else {
                  Code_Ident_Vamos->at(y) = 40;
               }
            }

            delete   id_chiov2;
            id_chiov2 = NULL;

         }  // code_ident_vamos==2
      }  // for MaxM
   }  // code_good_event

   //==========================================================================
   // V, A/Q, A and Q definitions for both cases

   const Double_t AMU = 931.494061;
   const Double_t m_elec = 0.510998928;
   const Double_t cte_tesla = (1.660538921E-27 * 299792458) / 1.602176565E-19;

   if ((Code_good_event == -10  && Si->EMSI == 1) || (Code_good_event == -8)) {

      // TODO: Z_corr can be undefined here! Why?

      Double_t x_aq = 0.0;
      Double_t Gamma = 0.0;

      for (Int_t y = 0; y < MaxM ; y++) {

         if (FragDetSi->at(y) < 0)  continue;

         Code_Vamos->at(y) = -10;

         if ((
                  Code_Ident_Vamos->at(y) == 1 ||
                  Code_Ident_Vamos->at(y) == 2 ||
                  Code_Ident_Vamos->at(y) == 22
               ) &&
               Rec->Brho >= Brho_min->at(runNumber) &&
               Rec->Brho <= Brho_max->at(runNumber)
            ) {
            Code_Vamos->at(y) = 1;
         } else {
            Code_Vamos->at(y) = 0;
         }

         assert((FragTfinal->at(y) > 0.) && (FragTfinal->at(y) < 500.));
         assert((D > 0.) && (D < 1500.));
         V->at(y) = D / FragTfinal->at(y); // Velocity given in cm/ns

         Vx->at(y) = V->at(y) * sin(Rec->ThetaL) * cos(
                        (Rec->PhiL) + (270.*(TMath::Pi() / 180.)));
         Vy->at(y) = V->at(y) * sin(Rec->ThetaL) * sin(
                        (Rec->PhiL) + (270.*(TMath::Pi() / 180.)));
         Vz->at(y) = V->at(y) * cos(Rec->ThetaL);

         Beta->at(y) = V->at(y) / 29.9792458;
         assert((Beta->at(y) > 0.) && (Beta->at(y) < 1.));

         x_aq = TMath::Sqrt((1 - (Beta->at(y) * Beta->at(y)))) /
                Beta->at(y);

         /////// TESTING START: TODO: Remove
         assert((FragDetSi->at(y) > 0) && (FragDetSi->at(y) < 19));

         Double_t lhs =
            (
               (1. / AMU) *
               (
                  (Rec->Brho * (AMU / cte_tesla) * x_aq) + m_elec
               )
            );

         Double_t rhs =
            P0_aq_straight->at(FragDetSi->at(y)) +
            (
               P1_aq_straight->at(FragDetSi->at(y)) *
               PID->at(y)
            ) + (
               P2_aq_straight->at(FragDetSi->at(y)) *
               TMath::Power(PID->at(y), 2)
            );

         /////// TESTING END

         M_Q->at(y) =
            (
               (1. / AMU) *
               (
                  (Rec->Brho * (AMU / cte_tesla) * x_aq) + m_elec
               )
            ) - (
               P0_aq_straight->at(FragDetSi->at(y)) +
               (
                  P1_aq_straight->at(FragDetSi->at(y)) *
                  PID->at(y)
               ) + (
                  P2_aq_straight->at(FragDetSi->at(y)) *
                  TMath::Power(PID->at(y), 2)
               )
            );

         if ((M_Q->at(y) <= 0.) || (M_Q->at(y) >= 10.)) {
            std::cout << "Error Report:" << std::endl;
            std::cout << "M_Q: " << M_Q->at(y) << std::endl;
            std::cout << "Brho: " << Rec->Brho << std::endl;
            std::cout << "FragDetSi: " << FragDetSi->at(y) << std::endl;
            std::cout << "PID: " << PID->at(y) << std::endl;
            std::cout << "x_aq: " << x_aq << std::endl;
            std::cout << "Beta: " << Beta->at(y) << std::endl;
            std::cout << "lhs: " << lhs << std::endl;
            std::cout << "rhs: " << rhs << std::endl;
            std::cout << "P0: " << P0_aq_straight->at(FragDetSi->at(y)) << std::endl;
            std::cout << "P1: " << P1_aq_straight->at(FragDetSi->at(y)) << std::endl;
            std::cout << "P2: " << P2_aq_straight->at(FragDetSi->at(y)) << std::endl;
         }

         assert((M_Q->at(y) > 0.) && (M_Q->at(y) < 10.));

         if (
            (Code_Ident_Vamos->at(y) == 1) &&
            (Z_corr->at(y) > 0)
         ) {

            // The 3 Dimensional array index is accessed by using
            // the appropriate linear offset. The dimensions of the
            // "3D array" are 25 * 2 * (kMaxRuns + 1).
            //
            // Each Z has 2 * (kMaxRuns + 1) data elements to store.
            // For each Z the "min/max" (0..1) segment must store
            // kMaxRuns + 1 data elements.
            //
            // E.g. To access Z2, Max, Run 5
            //
            // First we offset to the start of the Z2 data
            //
            // | Z1      | Z2      | ...   |
            // ----------^
            //
            // Then we offset to the start of the Max data for Z2;
            //
            // | Z2                  |
            // | Min Data | Max Data |
            // -----------^
            //
            // And finally we offset to the appropriate run for (Z2,
            // Max):
            //
            // | Z2                                     |
            // | Max Data                               |
            // | Run1 | Run2 | Run3 | Run4 | Run5 | ... |
            // ----------------------------^

            assert(y < (kNumCsI + 1));
            assert(Z_corr->at(y) > 0);
            assert(kMaxRuns > 0);
            assert(runNumber > 0);

            Long64_t low_index(
               (Z_corr->at(y) * 2 * (kMaxRuns + 1)) +
               (0 * (kMaxRuns + 1)) + runNumber
            );

            Long64_t high_index(
               (Z_corr->at(y) * 2 * (kMaxRuns + 1)) +
               (1 * (kMaxRuns + 1)) + runNumber
            );

            // Boundary conditions for FitX vectors
            assert((low_index >= 0) &&
                   (low_index < (25 * 2 * (kMaxRuns + 1)))
                  );
            assert((high_index >= 0) &&
                   (high_index < (25 * 2 * (kMaxRuns + 1)))
                  );

            if (
               (
                  M_Q->at(y) > Fit1->at(low_index) &&
                  M_Q->at(y) < Fit1->at(high_index)
               ) || (
                  M_Q->at(y) > Fit2->at(low_index) &&
                  M_Q->at(y) < Fit2->at(high_index)
               ) || (
                  M_Q->at(y) > Fit3->at(low_index) &&
                  M_Q->at(y) < Fit3->at(high_index)
               ) || (
                  M_Q->at(y) > Fit4->at(low_index) &&
                  M_Q->at(y) < Fit4->at(high_index)
               ) || (
                  M_Q->at(y) > Fit5->at(low_index) &&
                  M_Q->at(y) < Fit5->at(high_index)
               ) || (
                  M_Q->at(y) > Fit6->at(low_index) &&
                  M_Q->at(y) < Fit6->at(high_index)
               ) || (
                  M_Q->at(y) > Fit7->at(low_index) &&
                  M_Q->at(y) < Fit7->at(high_index)
               )
            ) {

               FragTfinal->at(y) = FragTfinal->at(y) + 1.235;

               // Velocity given in cm/ns
               assert((FragTfinal->at(y) > 0.) && (FragTfinal->at(y) < 500.));
               assert((D > 0.) && (D < 1500.));
               V->at(y) = D / FragTfinal->at(y);

               Vx->at(y) = V->at(y) * sin(Rec->ThetaL) * cos(
                              (Rec->PhiL) + (270.*(TMath::Pi() / 180.))
                           );
               Vy->at(y) = V->at(y) * sin(Rec->ThetaL) * sin(
                              (Rec->PhiL) + (270.*(TMath::Pi() / 180.))
                           );
               Vz->at(y) = V->at(y) * cos(Rec->ThetaL);

               Beta->at(y) = V->at(y) / 29.9792458;
               assert((Beta->at(y) > 0.) && (Beta->at(y) < 1.));

               x_aq = TMath::Sqrt((1 - (Beta->at(y) * Beta->at(y)))) /
                      Beta->at(y);

               M_Q->at(y) =
                  (
                     (1. / AMU) *
                     (
                        (Rec->Brho * (AMU / cte_tesla) * x_aq) + m_elec
                     )
                  ) - (
                     P0_aq_straight->at(FragDetSi->at(y)) +
                     (
                        P1_aq_straight->at(FragDetSi->at(y)) *
                        PID->at(y)
                     ) + (
                        P2_aq_straight->at(FragDetSi->at(y)) *
                        TMath::Power(PID->at(y), 2)
                     )
                  );

               if ((M_Q->at(y) <= 0.) || (M_Q->at(y) >= 10.)) {
                  std::cout << "Error Report:" << std::endl;
                  std::cout << "M_Q: " << M_Q->at(y) << std::endl;
                  std::cout << "Brho: " << Rec->Brho << std::endl;
                  std::cout << "FragDetSi: " << FragDetSi->at(y) << std::endl;
                  std::cout << "PID: " << PID->at(y) << std::endl;
                  std::cout << "x_aq: " << x_aq << std::endl;
                  std::cout << "Beta: " << Beta->at(y) << std::endl;
               }

               assert((M_Q->at(y) > 0.) && (M_Q->at(y) < 10.));


               Code_Ident_Vamos->at(y) = 50;
            }
         } else if (
            Code_Ident_Vamos->at(y) == 2 ||
            Code_Ident_Vamos->at(y) == 22
         ) {

            FragTfinal->at(y) = FragTfinal->at(y) +
                                Offset_relativiste_chiosi->at(
                                   runNumber
                                );

            assert((FragTfinal->at(y) > 0.) && (FragTfinal->at(y) < 500.));
            assert((D > 0.) && (D < 1500.));
            V->at(y) = D / FragTfinal->at(y); // Velocity given in cm/ns

            Vx->at(y) = V->at(y) * sin(Rec->ThetaL) * cos(
                           (Rec->PhiL) + (270.*(TMath::Pi() / 180.)));
            Vy->at(y) = V->at(y) * sin(Rec->ThetaL) * sin(
                           (Rec->PhiL) + (270.*(TMath::Pi() / 180.)));
            Vz->at(y) = V->at(y) * cos(Rec->ThetaL);

            Beta->at(y) = V->at(y) / 29.9792458;
            assert((Beta->at(y) > 0.) && (Beta->at(y) < 1.));

            x_aq = TMath::Sqrt((1 - (Beta->at(y) * Beta->at(y)))) /
                   Beta->at(y);

            M_Q->at(y) =
               (
                  (1. / AMU) *
                  (
                     (Rec->Brho * (AMU / cte_tesla) * x_aq) + m_elec
                  )
               ) - (
                  P0_aq_straight_chiosi->at(FragDetChio->at(y)) +
                  (
                     P1_aq_straight_chiosi->at(FragDetChio->at(y)) *
                     PID->at(y)
                  ) + (
                     P2_aq_straight_chiosi->at(FragDetChio->at(y)) *
                     TMath::Power(PID->at(y), 2)
                  )
               );

            if ((M_Q->at(y) <= 0.) || (M_Q->at(y) >= 10.)) {
               std::cout << "Error Report:" << std::endl;
               std::cout << "M_Q: " << M_Q->at(y) << std::endl;
               std::cout << "Brho: " << Rec->Brho << std::endl;
               std::cout << "FragDetChio: " << FragDetChio->at(y) << std::endl;
               std::cout << "PID: " << PID->at(y) << std::endl;
               std::cout << "x_aq: " << x_aq << std::endl;
               std::cout << "Beta: " << Beta->at(y) << std::endl;
            }

            assert((M_Q->at(y) > 0.) && (M_Q->at(y) < 10.));


         }

         // Z_corr may be uninitialised here, so we check it.

         if (Z_corr->at(y) > 0) {

            E_csi = GetEnergyLossCsI(FragDetSi->at(y), Z_corr->at(y),
                                     M_Q->at(y), FragESi->at(y));
            // gap Si-CsI:
            E_gap2 = GetEnergyLossGap2(FragDetSi->at(y), Z_corr->at(y),
                                       M_Q->at(y), FragESi->at(y));
            // gap Chio-Si:
            E_gap1 = GetEnergyLossGap1(FragDetSi->at(y), Z_corr->at(y),
                                       M_Q->at(y), FragESi->at(y));

            E_chio = GetEnergyLossChio(Z_corr->at(y), M_Q->at(y));
            E_dc2 = GetEnergyLossDC2(Z_corr->at(y), M_Q->at(y));
            E_sed = GetEnergyLossSed(Z_corr->at(y), M_Q->at(y));
            E_dc1 = GetEnergyLossDC1(Z_corr->at(y), M_Q->at(y));
            E_tgt = GetEnergyLossTarget(Z_corr->at(y), M_Q->at(y));

            if (
               Code_Ident_Vamos->at(y) == 1 ||
               Code_Ident_Vamos->at(y) == 38 ||
               Code_Ident_Vamos->at(y) == 39 ||
               Code_Ident_Vamos->at(y) == 40
            ) {

               // Identification Si-CsI

               if (FragEChio->at(y) <= 0) FragEChio->at(y) = E_chio;
               if (FragEGap->at(y) <= 0.) FragEGap->at(y) = 0.;
               if (FragECsI->at(y) <= 0.) FragECsI->at(y) = 0.;

               assert(FragEChio->at(y) >= 0.);
               assert(FragESi->at(y) >= 0.);
               assert(FragEGap->at(y) >= 0.);
               assert(FragECsI->at(y) >= 0.);

               Etot->at(y) = FragEChio->at(y) + FragESi->at(y) +
                             FragEGap->at(y) + FragECsI->at(y);

            } else if (
               Code_Ident_Vamos->at(y) == 2 ||
               Code_Ident_Vamos->at(y) == 22
            ) {

               // Identification Chio-Si
               Etot->at(y) = FragEChio->at(y) + FragESi->at(y);
            }

            // XXX: else? We can drop right through to here without
            // fulfilling any of the above conditions!

            assert(E_tgt >= 0.);
            assert(E_dc1 >= 0.);
            assert(E_dc2 >= 0.);
            assert(E_sed >= 0.);
            assert(E_gap1 >= 0.);

            if (Etot->at(y) >= 3000.) {
               std::cout << "Error Report:" << std::endl;
               std::cout << "Etot: " << Etot->at(y) << std::endl;
               std::cout << "E_tgt: " << E_tgt << std::endl;
               std::cout << "E_dc1: " << E_dc1 << std::endl;
               std::cout << "E_sed: " << E_sed << std::endl;
               std::cout << "E_dc2: " << E_dc2 << std::endl;
               std::cout << "E_gap1: " << E_gap1 << std::endl;
               std::cout << "FragEChio: " << FragEChio->at(y) << std::endl;
               std::cout << "FragESi: " << FragESi->at(y) << std::endl;
               std::cout << "FragEGap: " << FragEGap->at(y) << std::endl;
               std::cout << "FragECsI: " << FragECsI->at(y) << std::endl;
            }

            Etot->at(y) += E_tgt + E_dc1 + E_dc2 + E_sed + E_gap1;
            assert((Etot->at(y) >= 0.) && (Etot->at(y) < 3000.));

            Gamma = 1. / TMath::Sqrt(1. - TMath::Power(Beta->at(y), 2.));

            M->at(y) = Etot->at(y) / (AMU * (Gamma - 1));
            assert((M->at(y) > 0.) && (M->at(y) < 100.));

            Q->at(y) = M->at(y) / M_Q->at(y);;
            assert((Q->at(y) > 0.) && (Q->at(y) < 50.));

            if (
               (
                  (Code_Ident_Vamos->at(y) == 2) &&
                  Code_good_event == -10 &&
                  Si->EMSI == 1
               )
            ) {

               Off_chiosi->at(y) = DrawDeDx(
                                      Z_corr->at(y),
                                      M_Q->at(y) * Z_corr->at(y),
                                      1,
                                      (
                                         FragESi->at(y) + E_gap1 +
                                         FragEChio->at(y)
                                      ),
                                      E_gap1 + FragESi->at(y)
                                   );

               Off_dc1dc2->at(y) = DrawDeDx(
                                      Z_corr->at(y),
                                      M_Q->at(y) * Z_corr->at(y),
                                      0,
                                      (
                                         FragESi->at(y) + E_gap1 +
                                         FragEChio->at(y) + E_dc1 +
                                         E_dc2 + E_sed
                                      ),
                                      (
                                         FragESi->at(y) + E_gap1 +
                                         FragEChio->at(y)
                                      )
                                   );

               TOF_chiosi->at(y) = FragTfinal->at(y) -
                                   Off_chiosi->at(y) - Off_dc1dc2->at(y);

               if (TOF_chiosi->at(y) > 0) {
                  Beta_chiosi->at(y) =
                     (D / TOF_chiosi->at(y)) / 29.9792458;
                  assert((Beta_chiosi->at(y) > 0.) && (Beta_chiosi->at(y) < 1.));

                  x_aq = TMath::Sqrt(
                            1 -
                            (
                               Beta_chiosi->at(y) *
                               Beta_chiosi->at(y)
                            )
                         ) / Beta_chiosi->at(y);

                  M_Q_chiosi->at(y) =
                     (1. / AMU) *
                     (
                        (Rec->Brho * (AMU / cte_tesla) * x_aq) + m_elec
                     ) - (
                        P0_aq_straight->at(FragDetSi->at(y)) +
                        (
                           P1_aq_straight->at(FragDetSi->at(y)) *
                           PID->at(y)
                        ) + (
                           P2_aq_straight->at(FragDetSi->at(y)) *
                           TMath::Power(PID->at(y), 2)
                        )
                     );

                  if ((M_Q_chiosi->at(y) <= 0.) || (M_Q_chiosi->at(y) >= 10.)) {
                     std::cout << "Error Report:" << std::endl;
                     std::cout << "M_Q_chiosi: " << M_Q_chiosi->at(y) << std::endl;
                     std::cout << "Brho: " << Rec->Brho << std::endl;
                     std::cout << "FragDetSi: " << FragDetSi->at(y) << std::endl;
                     std::cout << "PID: " << PID->at(y) << std::endl;
                     std::cout << "x_aq: " << x_aq << std::endl;
                     std::cout << "Beta_chiosi: " << Beta_chiosi->at(y) << std::endl;
                  }

                  assert((M_Q_chiosi->at(y) > 0.) && (M_Q_chiosi->at(y) < 10.));

                  Gamma = 1. / TMath::Sqrt(
                             1. - TMath::Power(Beta_chiosi->at(y), 2.)
                          );

                  M_chiosi->at(y) = Etot->at(y) / (AMU * (Gamma - 1));
                  Q_chiosi->at(y) = M_chiosi->at(y) / M_Q_chiosi->at(y);
               }
            }
         }

         if (
            (
               Code_Ident_Vamos->at(y) == 1 ||
               Code_Ident_Vamos->at(y) == 40 ||
               Code_Ident_Vamos->at(y) == 50
            ) && (FragDetCsI->at(y) > 0) // XXX
         ) {
            // Redressement des distributions en Q

            id_qaq = new KVIdentificationResult();
            TString scope_name;

            energytree->InitQStraight(FragDetCsI->at(y) - 1);
            const KVIDGrid* grid_qaq = energytree->get_kvid_qaq();

            if (!grid_qaq) {

               ++grid_misses->qaq;

               RealQ_straight->at(y) = Q->at(y);
               Q_straight->at(y) = TMath::Floor(Q->at(y) + 0.5);
               M_straight->at(y) = Q_straight->at(y) * M_Q->at(y);

               Code_Ident_Vamos->at(y) = 45;
            } else {

               if (
                  grid_qaq->IsIdentifiable(
                     static_cast<Double_t>(M_Q->at(y)),
                     static_cast<Double_t>(Q->at(y))
                  )
               ) {
                  grid_qaq->Identify(
                     static_cast<Double_t>(M_Q->at(y)),
                     static_cast<Double_t>(Q->at(y)),
                     id_qaq
                  );

                  RealQ_straight->at(y) = id_qaq->PID;
                  Q_straight->at(y) = id_qaq->Z;
                  M_straight->at(y) = Q_straight->at(y) *
                                      M_Q->at(y);

               } else {
                  Code_Ident_Vamos->at(y) = 46;
               }
            }

            delete   id_qaq;
            id_qaq = NULL;

         } // Code_Ident_Vamos[y]=1
         else if (
            (
               Code_Ident_Vamos->at(y) == 2 ||
               Code_Ident_Vamos->at(y) == 22
            ) && (FragDetChio->at(y) > 0) //XXX
         ) {

            // Redressement des distributions en Q
            id_qaq_chiosi = new KVIdentificationResult();

            TString scope_name;

            energytree->InitQStraight_chiosi(FragDetChio->at(y) - 1);
            const KVIDGrid* grid_qaq_chiosi =
               energytree->get_kvid_qaq_chiosi();

            if (!grid_qaq_chiosi) {

               ++grid_misses->qaq_chiosi;

               RealQ_straight->at(y) = Q->at(y);
               Q_straight->at(y) = TMath::Floor(Q->at(y) + 0.5);
               M_straight->at(y) = Q_straight->at(y) * M_Q->at(y);

               Code_Ident_Vamos->at(y) = 55;
            } else {

               if (
                  grid_qaq_chiosi->IsIdentifiable(
                     static_cast<Double_t>(M_Q->at(y)),
                     static_cast<Double_t>(Q->at(y))
                  )

               ) {

                  grid_qaq_chiosi->Identify(
                     static_cast<Double_t>(M_Q->at(y)),
                     static_cast<Double_t>(Q->at(y)),
                     id_qaq_chiosi);

                  // std::cout << "Redressement" << std::endl;
                  RealQ_straight->at(y) = id_qaq_chiosi->PID;
                  Q_straight->at(y) = id_qaq_chiosi->Z;
                  M_straight->at(y) = Q_straight->at(y) *
                                      M_Q->at(y);

               } else {
                  Code_Ident_Vamos->at(y) = 56;
               }
            }

            delete   id_qaq_chiosi;
            id_qaq_chiosi = NULL;

         } // Code_Ident_Vamos[y]=2

      } // Loop MaxM
   }


   NormVamos =
      gIndraDB->GetRun(gIndra->GetCurrentRunNumber())->Get("NormVamos");
   // Correction du temps mort : 1.0 / (1.0-DT(%)) (/ 6.21295660000000000e+07
   // pour que correction soit de l'ordre de l'unité)
   DT = 1.0 /
        (
           1.0 -
           (
              gIndraDB->GetRun(gIndra->GetCurrentRunNumber())->Get("DT") /
              100.0
           )
        );

   FC_Indra =
      gIndraDB->GetRun(gIndra->GetCurrentRunNumber())->GetScaler("INDRA");

   stat_tot = 0.;
   stat_tot = Stat_Indra->at(gIndra->GetCurrentRunNumber());

}

void Identificationv::Treat(void)
{
#ifdef DEBUG
   std::cout << "Identificationv::Treat" << std::endl;
#endif

   Init();
   FragPropertiesInOrder();
   Calculate();

   ++event_number;
}

void Identificationv::inAttach(TTree* inT)
{
   UNUSED(inT);

#ifdef DEBUG
   std::cout << "Identificationv::inAttach " << std::endl;
#endif

#ifdef DEBUG
   std::cout << "Attaching Identification variables" << std::endl;
#endif

#ifdef ACTIVEBRANCHES
   std::cout << "Activating Branches: Identificationv" << std::endl;
   L->Log << "Activating Branches: Identificationv" << std::endl;
#endif
}


void Identificationv::outAttach(TTree* outT)
{

#ifdef DEBUG
   std::cout << "Identificationv::outAttach " << std::endl;
#endif

#ifdef DEBUG
   std::cout << "Attaching Identificationv variables" << std::endl;
#endif

   outT->Branch("RunNumber", &runNumber, "runNumber/I");

   // outT->Branch("ESiRaw",&SiRaw,"SiRaw/I");
   // outT->Branch("ECsIRaw",&CsIRaw,"CsIRaw/I");

   // outT->Branch("E_tgt",&E_tgt,"E_tgt/D");
   // outT->Branch("E_dc1",&E_dc1,"E_dc1/D");
   // outT->Branch("E_sed",&E_sed,"E_sed/D");
   // outT->Branch("E_dc2",&E_dc2,"E_dc2/D");
   // outT->Branch("E_chio",&E_chio,"E_chio/D");
   // outT->Branch("E_gap1",&E_gap1,"E_gap1/D");

   // outT->Branch("ESi",&ESi,"ESi/D");
   // outT->Branch("EGap",&EGap,"EGap/D");
   // outT->Branch("ECsI",&ECsI,"ECsI/D");
   // outT->Branch("ECsI_corr",&ECsI_corr,"ECsI_corr/D");
   // outT->Branch("DT",&DT,"DT/D");
   // outT->Branch("Brho_mag",&Brho_mag,"Brho_mag/D");
   // outT->Branch("FC_Indra",&FC_Indra,"FC_Indra/D");

   // outT->Branch("E_corr",&E_corr,"E_corr/F");

   // outT->Branch("Gamma",&Gamma,"Gamma/F");

   // outT->Branch("ESiRaw",&SiRaw,"SiRaw/I");
   // outT->Branch("ECsIRaw",&CsIRaw,"CsIRaw/I");
   // outT->Branch("ESi",&ESi,"ESi/D");

   outT->Branch("event_number", &event_number, "event_number/L");
   outT->Branch("MaxM", &MaxM, "MaxM/I");

   outT->Branch("FragDetChio", FragDetChio);
   outT->Branch("FragSignalChio", FragSignalChio);
   outT->Branch("FragEChio", FragEChio);
   outT->Branch("FragDetSi", FragDetSi);
   outT->Branch("FragSignalSi", FragSignalSi);

   outT->Branch("FragESi", FragESi);
   outT->Branch("FragTfinal", FragTfinal);
   outT->Branch("FragDetCsI", FragDetCsI);
   outT->Branch("FragSignalCsI", FragSignalCsI);
   outT->Branch("FragECsI", FragECsI);

   outT->Branch("Code_good_event", &Code_good_event, "Code_good_event/I");
   outT->Branch("fMultiplicity_case", &fMultiplicity_case,
                "fMultiplicity_case/I");

   outT->Branch("Brho_0", &Brho_mag, "Brho_mag/D");
   outT->Branch("DTcorr", &DT, "DT/D");
   outT->Branch("Stat_Indra", &stat_tot, "Stat_Indra/F");
   outT->Branch("NormVamos", &NormVamos, "NormVamos/D");

   outT->Branch("Code_Vamos", &Code_Vamos);
   outT->Branch("Code_Ident_Vamos", Code_Ident_Vamos);

   outT->Branch("E_VAMOS", Etot);
   // outT->Branch("E",&E_VAMOS,"E_VAMOS/F");
   // outT->Branch("T",&T,"T/F");

   outT->Branch("D", &D, "D/F");

   outT->Branch("V_VAMOS", V);
   outT->Branch("Vx_VAMOS", Vx);
   outT->Branch("Vy_VAMOS", Vy);
   outT->Branch("Vz_VAMOS", Vz);

   outT->Branch("Beta", Beta);

   // outT->Branch("AQ_VAMOS", &M_Qcorr, "M_Qcorr/F");

   outT->Branch("RealZ_VAMOS", PID);
   outT->Branch("Z_VAMOS", Z_corr);
   outT->Branch("RealZ_VAMOS_sitof", PID_sitof);
   outT->Branch("Z_VAMOS_sitof", Z_corr_sitof);

   outT->Branch("RealZ_VAMOS_chiov2", PID_chiov2);
   outT->Branch("Z_VAMOS_chiov2", Z_corr_chiov2);
   outT->Branch("A_VAMOS", M_straight);
   outT->Branch("Q_VAMOS", Q_straight);
   outT->Branch("RealQ_VAMOS", RealQ_straight);

   outT->Branch("AQ", M_Q);
   outT->Branch("Q", Q);
   outT->Branch("A", M);

   outT->Branch("AQ_chiosi", M_Q_chiosi);
   outT->Branch("Q_chiosi", Q_chiosi);
   outT->Branch("A_chiosi", M_chiosi);

   outT->Branch("Off_chiosi", Off_chiosi);
   outT->Branch("Off_dc1dc2", Off_dc1dc2);
   outT->Branch("Abisec", AA);

}

void Identificationv::CreateHistograms(void)
{
#ifdef DEBUG
   std::cout << "Identificationv::CreateHistograms " << std::endl;
#endif
}

void Identificationv::FillHistograms(void)
{
#ifdef DEBUG
   std::cout << "Identificationv::FillHistograms " << std::endl;
#endif
}

void Identificationv::Show(void)
{
#ifdef DEBUG
   std::cout << "Identificationv::Show__prova" << std::endl;
#endif
   std::cout.setf(ios::showpoint);
}

void Identificationv::GeometrySiCsICross()
{
   Int_t  num;
   Int_t csi1 = 0, csi2 = 0, csi3 = 0, csi4 = 0;
   ifstream in;
   TString sline;

   if (!gDataSet->OpenDataSetFile("geom_sicsi_biggercsi.dat", in)) {
      std::cout << "Could not open geom_sicsi_biggercsi.dat !!!" << std::endl;
      return;
   } else {
      while (!in.eof()) {
         sline.ReadLine(in);
         if (!in.eof()) {
            if (!sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %d %d %d %d", &num, &csi1, &csi2,
                      &csi3, &csi4);

               // 4 is the width of each data "element" in the vector
               geom_cross->at((4 * num) + 0) = csi1;
               geom_cross->at((4 * num) + 1) = csi2;
               geom_cross->at((4 * num) + 2) = csi3;
               geom_cross->at((4 * num) + 3) = csi4;

            }
         }
      }
   }

   in.close();

   return;
}


Bool_t Identificationv::IsCsISiCrossTelescope(Int_t sinum, Int_t csinum)
{
   for (Int_t i = 0; i < 4; i++) {
      // 4 is the width of each data "element" in the vector
      if (geom_cross->at((4 * sinum) + i) == csinum) {
         return kTRUE;
      }
   }

   return kFALSE;
}

// Method to reconstruct VAMOS telescopes
void Identificationv::GeometryChioSi()
{
   Int_t  num;
   Int_t chio1 = 0, chio2 = 0;
   ifstream in;
   TString sline;

   if (!gDataSet->OpenDataSetFile("geom_chiosi.dat", in)) {
      std::cout << "Could not open the calibration file geom_chiosi.dat !!!"
                << std::endl;
      return;
   } else {
      while (!in.eof()) {
         sline.ReadLine(in);
         if (!in.eof()) {
            if (!sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %d %d", &num, &chio1, &chio2);

               // 2 is the width of each data "element" in the vector
               geomchiosi->at((2 * num) + 0) = chio1;
               geomchiosi->at((2 * num) + 1) = chio2;
            }
         }
      }
   }

   in.close();

   return;

}


Bool_t Identificationv::IsChioSiTelescope(Int_t cinum, Int_t sinum)
{
   for (Int_t i = 0; i < 2; i++) {
      // 2 is the width of each data "element" in the vector
      if (geomchiosi->at((2 * sinum) + i) == cinum) {
         return kTRUE;
      }
   }
   return kFALSE;
}

void Identificationv::GeometryChioCsI()
{
   Int_t  num;
   Int_t chio1 = 0, chio2 = 0;
   ifstream in;
   TString sline;

   if (!gDataSet->OpenDataSetFile("geom_chiocsi.dat", in)) {
      std::cout <<
                "Could not open the calibration file geom_chiocsi.dat !!!" <<
                std::endl;
      return;
   } else {
      while (!in.eof()) {
         sline.ReadLine(in);
         if (!in.eof()) {
            if (!sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %d %d", &num, &chio1, &chio2);

               // 2 is the width of each data "element" in the vector
               geomchiocsi->at((2 * num) + 0) = chio1;
               geomchiocsi->at((2 * num) + 1) = chio2;

            }
         }
      }
   }

   in.close();

   return;
}


Bool_t Identificationv::IsChioCsITelescope(Int_t chionum, Int_t csinum)
{
   for (Int_t i = 0; i < 2; i++) {
      if (geomchiocsi->at((2 * csinum) + i) == chionum) {
         return kTRUE;
      }
   }

   return kFALSE;
}

void Identificationv::Geometry()
{
   Int_t  num;
   Int_t csi1 = 0, csi2 = 0, csi3 = 0, csi4 = 0, csi5 = 0, csi6 = 0;
   ifstream in;
   TString sline;

   if (!gDataSet->OpenDataSetFile("geom.dat", in)) {
      std::cout << "Could not open the calibration file geom.dat !!!" <<
                std::endl;
      return;
   } else {
      while (!in.eof()) {
         sline.ReadLine(in);
         if (!in.eof()) {
            if (!sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %d %d %d %d %d %d", &num, &csi1,
                      &csi2, &csi3, &csi4, &csi5, &csi6);

               // 6 is the width of each data "element" in the vector

               geom->at((6 * num) + 0) = csi1;
               geom->at((6 * num) + 1) = csi2;
               geom->at((6 * num) + 2) = csi3;
               geom->at((6 * num) + 3) = csi4;
               geom->at((6 * num) + 4) = csi5;
               geom->at((6 * num) + 5) = csi6;

            }
         }
      }
   }

   in.close();

   return;
}


Bool_t Identificationv::IsSiCsITelescope(Int_t sinum, Int_t csinum)
{
   for (Int_t i = 0; i < 6; i++) {
      if (geom->at((6 * sinum) + i) == csinum) {
         return kTRUE;
      }
   }

   return kFALSE;
}

void Identificationv::ReadQCorrection()
{
   //==========================================================================
   // Correction de l'état de charge en fonction du détecteur CsI
   // Charge state correction according to the CsI detector

   Int_t csi = 0;
   Float_t p0 = 0.;
   Float_t p1 = 0.;
   Float_t p2 = 0.;
   Float_t p3 = 0.;

   ifstream file;
   TString sline;

   if (!gDataSet->OpenDataSetFile("deprecated/q_function.dat", file)) {
      L->Log << "Could not open the calibration file q_function !!!" <<
             std::endl;
      return;
   } else {
      L->Log << "Reading q_function" << std::endl;
      while (file.good()) {         // reading the file
         sline.ReadLine(file);
         if (!file.eof()) {          // fin du fichier
            if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%u %f %f %f %f",
                      &csi, &p0, &p1, &p2, &p3);

               P0->at(csi) = p0;
               P1->at(csi) = p1;
               P2->at(csi) = p2;
               P3->at(csi) = p3;

               L->Log << p0 << "	" << p1 << "	" << p2 << "	" <<
                      p3 << std::endl;
            }
         }
      }
   }

   file.close();

   return;
}


void Identificationv::ReadAQCorrection()
{
   //==========================================================================

   Int_t run1 = 0;
   Int_t run2 = 0;
   Float_t brho = 0.;
   Float_t chi2 = 0.;
   Float_t p0mq = 0.;
   Float_t p1mq = 0.;

   //==========================================================================
   // Correction du rapport M/Q en fonction du Brho (ou RunNumber...)
   // M/Q correction according to the Brho value (or the RunNumber...)
   // Identification Si-CsI

   ifstream file2;
   TString sline2;
   if (!gDataSet->OpenDataSetFile("deprecated/mq_function.dat", file2)) {
      L->Log << "Could not open the calibration file q_function !!!" <<
             std::endl;
      return;
   } else {
      L->Log << "Reading mq_function" << std::endl;
      while (file2.good()) {         // reading the file
         sline2.ReadLine(file2);
         if (!file2.eof()) {          // fin du fichier
            if (sline2.Sizeof() > 1 && !sline2.BeginsWith("#")) {
               sscanf(sline2.Data(), "%f %u %u %f %f %f",
                      &brho, &run1, &run2, &p0mq, &p1mq, &chi2);
               for (Int_t i = run1; i < run2 + 1; i++) {

                  P0_mq->at(i) = p0mq;
                  P1_mq->at(i) = p1mq;

                  // L->Log << p0mq << "  " << p1mq << std::endl;
               }
            }
         }
      }
   }

   file2.close();

   //==========================================================================
   //==========================================================================
   // Correction du rapport M/Q en fonction du Brho (ou RunNumber...)
   // M/Q correction according to the Brho value (or the RunNumber...)
   // Identification Chio-Si

   ifstream file22;
   TString sline22;
   if (!gDataSet->OpenDataSetFile("deprecated/mq_function_chiosi.dat", file22)) {
      L->Log <<
             "Could not open the calibration file mq_function_chiosi !!!" <<
             std::endl;
      return;
   } else {
      L->Log << "Reading mq_function_chiosi" << std::endl;
      while (file22.good()) {         // reading the file
         sline22.ReadLine(file22);
         if (!file22.eof()) {          // fin du fichier
            if (sline22.Sizeof() > 1 && !sline22.BeginsWith("#")) {
               sscanf(sline22.Data(), "%f %u %u %f %f %f",
                      &brho, &run1, &run2, &p0mq, &p1mq, &chi2);
               for (Int_t i = run1; i < run2 + 1; i++) {

                  P0_mq_chiosi->at(i) = p0mq;
                  P1_mq_chiosi->at(i) = p1mq;

                  // L->Log << p0mq << "  " << p1mq << std::endl;
               }
            }
         }
      }
   }

   file22.close();

   return;
}


void Identificationv::ReadACorrection()
{
   //==========================================================================
   // Correction de la masse en fonction du Brho (ou RunNumber...) et de l'état
   // de charge entier Q
   // Mass correction according to the Brho value et charge state value Q
   // (which is an interger in that case...)
   // Identification Si-CsI

   ifstream file3;
   TString sline3;

   Int_t q;
   Float_t brho;
   Int_t run1;
   Int_t run2;
   Float_t p0m;
   Float_t p1m;
   Float_t chi2;

   if (!gDataSet->OpenDataSetFile("deprecated/m_function.dat", file3)) {
      L->Log << "Could not open the calibration file m_function !!!" <<
             std::endl;
      return;
   } else {
      L->Log << "Reading m_function" << std::endl;
      while (file3.good()) {         // reading the file
         sline3.ReadLine(file3);
         if (!file3.eof()) {          // fin du fichier
            if (sline3.Sizeof() > 1 && !sline3.BeginsWith("#")) {
               sscanf(sline3.Data(), "%f %u %u %u %f %f %f",
                      &brho, &run1, &run2, &q, &p0m, &p1m, &chi2);
               for (Int_t i = run1; i < run2 + 1; i++) {

                  // 25 is the width of each element in the vector
                  P0_m->at((25 * i) + q) = p0m;
                  P1_m->at((25 * i) + q) = p1m;

                  // L->Log << p0m << "   " << p1m << std::endl;
               }
            }
         }
      }
   }
   file3.close();

   //==========================================================================
   // Correction de la masse en fonction du Brho (ou RunNumber...) et de l'état
   // de charge entier Q
   // Mass correction according to the Brho value et charge state value Q
   // (which is an interger in that case...)

   // Identification Chio-Si

   ifstream file33;
   TString sline33;

   if (!gDataSet->OpenDataSetFile("deprecated/m_function_chiosi.dat", file33)) {
      L->Log << "Could not open the calibration file m_function_chiosi !!!" <<
             std::endl;
      return;
   } else {
      L->Log << "Reading m_function_chiosi" << std::endl;
      while (file33.good()) {         // reading the file
         sline33.ReadLine(file33);
         if (!file33.eof()) {          // fin du fichier
            if (sline33.Sizeof() > 1 && !sline33.BeginsWith("#")) {
               sscanf(sline33.Data(), "%f %u %u %u %f %f %f",
                      &brho, &run1, &run2, &q, &p0m, &p1m, &chi2);
               for (Int_t i = run1; i < run2 + 1; i++) {

                  // 25 is the width of each element in the vector
                  P0_m_chiosi->at((25 * i) + q) = p0m;
                  P1_m_chiosi->at((25 * i) + q) = p1m;

               }
            }
         }
      }
   }

   file33.close();

   return;
}


void Identificationv::ReadFlagVamos()
{
   //==========================================================================
   // Tag the event according to the fragment identification (Z,A,Q),
   // Brho0 (or RunNumber) and system

   // Int_t z;
   // Int_t a;
   // Int_t q;
   Float_t brho0;
   Int_t run1;
   Int_t run2;
   Float_t brhomin;
   Float_t brhomax;

   ifstream file4;
   TString sline4;

   if (!gDataSet->OpenDataSetFile("Flag_CodeVamos.dat", file4)) {
      L->Log << "Could not open the calibration file Flag_CodeVamos !!!" <<
             std::endl;
      return;
   } else {
      std::cout << "Reading Flag_CodeVamos" << std::endl;

      while (file4.good()) {         // reading the file
         sline4.ReadLine(file4);

         if (!file4.eof()) {          // fin du fichier
            if (sline4.Sizeof() > 1 && !sline4.BeginsWith("#")) {

               sscanf(sline4.Data(), "%f %u %u %f %f",
                      &brho0, &run1, &run2, &brhomin, &brhomax);

               for (Int_t i = run1; i < run2 + 1; i++) {
                  Brho_min->at(i) = brhomin;
                  Brho_max->at(i) = brhomax;
               }

               if (run1 == run2) {
                  Brho_min->at(run1) = brhomin;
                  Brho_max->at(run2) = brhomax;
               }

               if (TMath::Abs(run2 - run1) == 1) {
                  Brho_min->at(run1) = brhomin;
                  Brho_max->at(run1) = brhomax;

                  Brho_min->at(run2) = brhomin;
                  Brho_max->at(run2) = brhomax;
               }

            }
         }
      }
   }

   file4.close();

   return;
}


void Identificationv::ReadDoublingCorrection()
{
   Int_t z;
   Int_t system, run1, run2;
   Float_t min1, max1;
   Float_t min2, max2;
   Float_t min3, max3;
   Float_t min4, max4;
   Float_t min5, max5;
   Float_t min6, max6;
   Float_t min7, max7;

   ifstream file5;
   TString sline5;

   std::cout << "Reading deprecated/doublepeak_mins.dat" << std::endl;

   if (!gDataSet->OpenDataSetFile("deprecated/doublepeak_mins.dat", file5)) {
      L->Log << "Could not open the calibration file deprecated/doublepeak_mins.dat !!!"
             << std::endl;
      return;
   } else {
      while (file5.good()) {         // reading the file
         sline5.ReadLine(file5);
         if (!file5.eof()) {          // fin du fichier
            if (sline5.Sizeof() > 1 && !sline5.BeginsWith("#")) {

               sscanf(sline5.Data(), "%u %u %u %u "
                      "%f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                      &system, &z, &run1, &run2, &min1, &max1, &min2,
                      &max2, &min3, &max3, &min4, &max4, &min5, &max5,
                      &min6, &max6, &min7, &max7);

               for (Int_t i = run1; i < run2 + 1; i++) {

                  // The 3 Dimensional array index is accessed by using
                  // the appropriate linear offset. The dimensions of the
                  // "3D array" are 25 * 2 * (kMaxRuns + 1).
                  //
                  // Each Z has 2 * (kMaxRuns + 1) data elements to store.
                  // For each Z the "min/max" (0..1) segment must store
                  // kMaxRuns + 1 data elements.
                  //
                  // E.g. To access Z2, Max, Run 5
                  //
                  // First we offset to the start of the Z2 data
                  //
                  // | Z1      | Z2      | ...   |
                  // ----------^
                  //
                  // Then we offset to the start of the Max data for Z2;
                  //
                  // | Z2                  |
                  // | Min Data | Max Data |
                  // -----------^
                  //
                  // And finally we offset to the appropriate run for (Z2,
                  // Max):
                  //
                  // | Z2                                     |
                  // | Max Data                               |
                  // | Run1 | Run2 | Run3 | Run4 | Run5 | ... |
                  // ----------------------------^

                  Long64_t low_index(
                     (z * 2 * (kMaxRuns + 1)) +
                     (0 * (kMaxRuns + 1)) + i
                  );

                  Long64_t high_index(
                     (z * 2 * (kMaxRuns + 1)) +
                     (1 * (kMaxRuns + 1)) + i
                  );

                  Fit1->at(low_index) = min1;
                  Fit1->at(high_index) = max1;

                  Fit2->at(low_index) = min2;
                  Fit2->at(high_index) = max2;

                  Fit3->at(low_index) = min3;
                  Fit3->at(high_index) = max3;

                  Fit4->at(low_index) = min4;
                  Fit4->at(high_index) = max4;

                  Fit5->at(low_index) = min5;
                  Fit5->at(high_index) = max5;

                  Fit6->at(low_index) = min6;
                  Fit6->at(high_index) = max6;

                  Fit7->at(low_index) = min7;
                  Fit7->at(high_index) = max7;

               }
            }
         }
      }
   }

   file5.close();

   std::cout << "End Reading deprecated/doublepeak_mins.dat" << std::endl;

   return;
}


void Identificationv::ReadToFCorrectionChioSi()
{
   //==========================================================================
   // Correction du temps de vol pour les events Chio-Si (Code_Ident_Vamos==2)
   Float_t brho0;
   Int_t run1;
   Int_t run2;
   Int_t t0;
   Int_t esi0;

   ifstream file5;
   TString sline5;

   if (!gDataSet->OpenDataSetFile("deprecated/Tof_corr.dat", file5)) {
      L->Log << "Could not open the calibration file deprecated/Tof_corr.dat !!!" <<
             std::endl;
      return;
   } else {
      while (file5.good()) {         // reading the file
         sline5.ReadLine(file5);

         if (!file5.eof()) {          // fin du fichier
            if (sline5.Sizeof() > 1 && !sline5.BeginsWith("#")) {

               sscanf(sline5.Data(), "%f %u %u %u %u",
                      &brho0, &run1, &run2, &t0, &esi0);

               for (Int_t i = run1; i < run2 + 1; i++) {
                  Brho0->at(i) = brho0;
                  Tof0->at(i) = t0;
                  Esi0->at(i) = esi0;

               }
            }
         }
      }
   }

   file5.close();
   return;
}


void Identificationv::ReadToFCorrectionCode1()
{

   //==========================================================================
   // Correction du temps de vol pour les events Si-CsI (Code_Ident_Vamos=1)

   Int_t run1;
   Int_t run2;
   Int_t system;
   Float_t t0, biny1, biny2, toflow, err_toflow, tofgood, err_tofgood, deltat;

   ifstream file5;
   TString sline5;

   if (!gDataSet->OpenDataSetFile("deprecated/Tof_corr_code1.dat", file5)) {
      L->Log << "Could not open the calibration file deprecated/Tof_corr_code1.dat !!!"
             << std::endl;
      return;
   } else {
      while (file5.good()) {         // reading the file
         sline5.ReadLine(file5);

         if (!file5.eof()) {          // fin du fichier
            if (sline5.Sizeof() > 1 && !sline5.BeginsWith("#")) {

               sscanf(sline5.Data(), "%d %d %d %f %f %f %f %f %f %f %f",
                      &run1, &run2, &system, &t0, &biny1, &biny2, &toflow,
                      &err_toflow, &tofgood, &err_tofgood, &deltat);

               for (Int_t i = run1; i < run2 + 1; i++) {
                  Tof0_code1->at(i) = t0;
                  Deltat_code1->at(i) = deltat;

               }
            }
         }
      }
   }

   file5.close();
}


void Identificationv::ReadStatIndra()
{
   //==========================================================================
   // Calculate the total statistic for each nominal Brho.

   Float_t brho0;
   Int_t run1;
   Int_t run2;

   // Float_t brho0;
   ifstream file6;
   TString sline6;

   stat_indra = -10.0;

   if (!gDataSet->OpenDataSetFile("Stat_Indra.dat", file6)) {
      L->Log << "Could not open the calibration file  Stat_Indra!!!" <<
             std::endl;
      return;
   } else {
      while (file6.good()) {         // reading the file
         sline6.ReadLine(file6);

         if (!file6.eof()) {          // fin du fichier
            if (sline6.Sizeof() > 1 && !sline6.BeginsWith("#")) {

               sscanf(sline6.Data(), "%f %d %d %f", &brho0, &run1, &run2,
                      &stat_indra);

               for (Int_t i = run1; i < run2 + 1; i++) {
                  Stat_Indra->at(i) = stat_indra;
               }

               if (run1 == run2) {
                  Stat_Indra->at(run1) = stat_indra;
               }

               if (TMath::Abs(run2 - run1) == 1) {
                  Stat_Indra->at(run1) = stat_indra;
                  Stat_Indra->at(run2) = stat_indra;
               }

            }
         }
      }
   }

   file6.close();

   return;
}


void Identificationv::ReadOffsetsChiosi()
{

   TString sline3;
   ifstream in3;
   Float_t brho0, aq, sig_aq, off_relativiste;
   Int_t run1, run2;


   // ToF Offset for each run
   if (!gDataSet->OpenDataSetFile("Offset_relativiste_chiosi.dat", in3)) {
      std::cout <<
                "Could not open the calibration file "
                "Offset_relativiste.cal !!!" <<
                std::endl;
      return;
   } else {
      std::cout << "Reading Offset_relativiste.dat" << std::endl;
      L->Log << "Reading Offset_relativiste.dat" << std::endl;

      while (!in3.eof()) {
         sline3.ReadLine(in3);

         if (!in3.eof()) {
            if (!sline3.BeginsWith("+") && !sline3.BeginsWith("#")) {

               sscanf(sline3.Data(), "%f %d %d %f %f %f", &brho0, &run1,
                      &run2, &off_relativiste, &aq, &sig_aq);

               for (Int_t i = run1; i < run2 + 1; i++) {
                  Offset_relativiste_chiosi->at(i) = off_relativiste;
               }
            }
         }
      }
   }

   in3.close();

   return;
}


void Identificationv::ReadStraightAQ()
{
   ifstream file8;
   TString sline8;
   ifstream file80;
   TString sline80;

   Int_t si;
   Float_t p0, err_p0;
   Float_t p1, err_p1;
   Float_t p2, err_p2;

   si = 0;
   p0 = err_p0 = 0.0;
   p1 = err_p1 = 0.0;
   p2 = err_p2 = 0.0;

   if (!gDataSet->OpenDataSetFile("deprecated/straight_fct.dat", file8)) {
      L->Log << "Could not open the calibration file  straight_fct!!!" <<
             std::endl;
      return;
   } else {
      while (file8.good()) {         // reading the file
         sline8.ReadLine(file8);
         if (!file8.eof()) {          // fin du fichier
            if (sline8.Sizeof() > 0 && !sline8.BeginsWith("#")) {
               sscanf(sline8.Data(), "%u %f %f %f %f %f %f", &si, &p0,
                      &err_p0, &p1, &err_p1, &p2, &err_p2);

               P0_aq_straight->at(si) = p0;
               P1_aq_straight->at(si) = p1;
               P2_aq_straight->at(si) = p2;

            }
         }
      }
   }
   file8.close();

   if (!gDataSet->OpenDataSetFile("deprecated/straight_fct_chiosi.dat", file80)) {
      L->Log << "Could not open the calibration file  straight_fct_chiosi!!!"
             << std::endl;
      return;
   } else {
      while (file80.good()) {         // reading the file
         sline80.ReadLine(file80);

         if (!file80.eof()) {          // fin du fichier
            if (sline80.Sizeof() > 0 && !sline80.BeginsWith("#")) {

               sscanf(sline80.Data(), "%u %f %f %f %f %f %f", &si, &p0,
                      &err_p0, &p1, &err_p1, &p2, &err_p2);

               P0_aq_straight_chiosi->at(si) = p0;
               P1_aq_straight_chiosi->at(si) = p1;
               P2_aq_straight_chiosi->at(si) = p2;

            }
         }
      }
   }

   file80.close();
   return;
}


//===================================================

void Identificationv::set_target(const KVTarget* const targ)
{
   if (target) {
      delete target;
      target = NULL;
   }

   target = new KVTarget(*targ);
}

void Identificationv::set_drift_chamber_one(const KVDetector* const detector)
{
   if (drift_chamber_one) {
      delete drift_chamber_one;
      drift_chamber_one = NULL;
   }

   drift_chamber_one = new KVDetector(*detector);
}

void Identificationv::set_sed(const KVMaterial* const material)
{
   if (sed) {
      delete sed;
      sed = NULL;
   }

   sed = new KVMaterial(*material);
}

void Identificationv::set_drift_chamber_two(const KVDetector* const detector)
{
   if (drift_chamber_two) {
      delete drift_chamber_two;
      drift_chamber_two = NULL;
   }

   drift_chamber_two = new KVDetector(*detector);
}

void Identificationv::set_chio(const KVDetector* const detector)
{
   if (chio) {
      delete chio;
      chio = NULL;
   }

   chio = new KVDetector(*detector);
}

void Identificationv::set_isobutane_gap_one(const KVMaterial* const material)
{
   if (isobutane_gap_one) {
      delete isobutane_gap_one;
      isobutane_gap_one = NULL;
   }

   isobutane_gap_one = new KVMaterial(*material);
}

void Identificationv::set_silicon(const KVMaterial* const material)
{
   if (silicon) {
      delete silicon;
      silicon = NULL;
   }

   silicon = new KVMaterial(*material);
}
void Identificationv::set_isobutane_gap_two(const KVMaterial* const material)
{
   if (isobutane_gap_two) {
      delete isobutane_gap_two;
      isobutane_gap_two = NULL;
   }

   isobutane_gap_two = new KVMaterial(*material);
}

void Identificationv::set_csi(const KVMaterial* const material)
{
   if (csi) {
      delete csi;
      csi = NULL;
   }

   csi = new KVMaterial(*material);
}

//===================================================
const KVTarget* Identificationv::get_target() const
{
   return target;
}

const KVDetector* Identificationv::get_drift_chamber_one() const
{
   return drift_chamber_one;
}

const KVMaterial* Identificationv::get_sed() const
{
   return sed;
}

const KVDetector* Identificationv::get_drift_chamber_two() const
{
   return drift_chamber_two;
}

const KVDetector* Identificationv::get_chio() const
{
   return chio;
}

const KVMaterial* Identificationv::get_isobutane_gap_one() const
{
   return isobutane_gap_one;
}

const KVMaterial* Identificationv::get_silicon() const
{
   return silicon;
}

const KVMaterial* Identificationv::get_isobutane_gap_two() const
{
   return isobutane_gap_two;
}

const KVMaterial* Identificationv::get_csi() const
{
   return csi;
}

//===================================================
Double_t Identificationv::GetEnergyLossCsI(Int_t number, Int_t Z, Double_t AQ,
      Double_t ESi)
{
   if (AQ <= 0.) {
      std::cout << "Error Report: " << std::endl;
      std::cout << "AQ: " << AQ << std::endl;
   }

   // Boundary checks
   assert((number > 0) && (number < 19));
   assert((Z > 0 && Z < 120));
   assert(AQ > 0.);
   assert(ESi > 0.);

   silicon->SetThickness(Si->si_thick[number]*KVUnits::um);

   // Calcul de l'énergie perdue dans csi
   einc_isogap2 = silicon->GetEResFromDeltaE(Z, static_cast<Int_t>(Z * AQ),
                  ESi);

   eloss_isogap2 = isobutane_gap_two->GetDeltaE(Z, static_cast<Int_t>(Z * AQ),
                   einc_isogap2);

   einc_csi =
      isobutane_gap_two->GetEResFromDeltaE(Z, static_cast<Int_t>(Z * AQ),
                                           eloss_isogap2);

   eloss_csi = csi->GetDeltaE(Z, static_cast<Int_t>(Z * AQ),
                              einc_isogap2);

   silicon->Clear();

   return eloss_csi;
}

Double_t Identificationv::GetEnergyLossGap2(Int_t number, Int_t Z, Double_t AQ,
      Double_t ESi)
{
   assert((number > 0) && (number < 19));
   assert(Z > 0);
   assert(AQ > 0.);
   assert(ESi > 0.);

   silicon->SetThickness(Si->si_thick[number]*KVUnits::um);

   // Calcul de l'énergie perdue dans isogap2
   einc_isogap2 = silicon->GetEResFromDeltaE(Z, static_cast<Int_t>(Z * AQ),
                  ESi);

   eloss_isogap2 = isobutane_gap_two->GetDeltaE(Z, static_cast<Int_t>(Z * AQ),
                   einc_isogap2);

   silicon->Clear();

   return eloss_isogap2;
}

Double_t Identificationv::GetEnergyLossGap1(Int_t number, Int_t Z, Double_t AQ,
      Double_t ESi)
{
   assert((number > 0) && (number < 19));
   assert((Z > 0) && (Z < 120));
   assert((AQ > 0.) && (AQ < 10.));
   assert((ESi > 0.) && (ESi < 3000.));

   silicon->SetThickness(Si->si_thick[number]*KVUnits::um);
   einc_si = silicon->GetIncidentEnergy(Z, static_cast<Int_t>(Z * AQ), ESi);

   // Calcul de l'énergie perdue dans isogap1
   einc_isogap1 =
      isobutane_gap_one->GetIncidentEnergyFromERes(Z,
            static_cast<Int_t>(Z * AQ),
            einc_si);

   eloss_isogap1 =
      isobutane_gap_one->GetDeltaEFromERes(Z, static_cast<Int_t>(Z * AQ),
                                           einc_si);

   silicon->Clear();

   return eloss_isogap1;
}

Double_t Identificationv::GetEnergyLossChio(Int_t Z, Double_t AQ)
{
   assert((Z > 0) && (Z < 120));
   assert((AQ > 0.) && (AQ < 10.));

   eloss_ic = 0.;

   // Calcul de l'énergie perdue dans la chio
   einc_ic = chio->GetIncidentEnergyFromERes(Z, static_cast<Int_t>(Z * AQ),
             einc_isogap1);

   kvn->SetZ(Z);
   kvn->SetA(static_cast<Int_t>(AQ * Z));
   kvn->SetEnergy(einc_ic);
   KVMaterial* kvm_ic = 0;

   for (Int_t i = 0; i < 2; i++) {
      // Take account only the first two layers, because the calibration gives
      // the energy for the active layer, which is the last layer...
      fELosLayer_ic[i] = 0.;
      kvm_ic = (KVMaterial*) chio->GetAbsorber(i);
      kvm_ic->DetectParticle(kvn);

      fELosLayer_ic[i] = kvm_ic->GetEnergyLoss();
      eloss_ic += fELosLayer_ic[i];
   }

   kvn->Clear();
   chio->Clear();

   return eloss_ic;
}

Double_t Identificationv::GetEnergyLossDC2(Int_t Z, Double_t AQ)
{
   assert((Z > 0) && (Z < 120));
   assert((AQ > 0.) && (AQ < 10.));

   eloss_dc2 = 0.;

   // Calcul de l'énergie perdue dans la DC2
   einc_dc2 =
      drift_chamber_two->GetIncidentEnergyFromERes(Z,
            static_cast<Int_t>(AQ * Z),
            einc_ic);

   kvn->SetZ(Z);
   kvn->SetA(static_cast<Int_t>(AQ * Z));
   kvn->SetEnergy(einc_dc2);
   KVMaterial* kvm_dc2 = 0;

   for (Int_t i = 0; i < 3; i++) {
      fELosLayer_dc2[i] = 0.;
      kvm_dc2 = (KVMaterial*) drift_chamber_two->GetAbsorber(i);
      kvm_dc2->DetectParticle(kvn);

      fELosLayer_dc2[i] = kvm_dc2->GetEnergyLoss();
      eloss_dc2 += fELosLayer_dc2[i];
   }

   kvn->Clear();
   drift_chamber_two->Clear();

   return eloss_dc2;
}

Double_t Identificationv::GetEnergyLossSed(Int_t Z, Double_t AQ)
{
   assert((Z > 0) && (Z < 120));
   assert((AQ > 0.) && (AQ < 10.));

   eloss_sed = 0.;

   // Calcul de l'énergie perdue dans la SED
   TVector3 rot(0, 1, -1); // 45 deg
   einc_sed =
      sed->GetIncidentEnergyFromERes(Z, static_cast<Int_t>(AQ * Z),
                                     einc_dc2);

   kvn->SetZ(Z);
   kvn->SetA(static_cast<Int_t>(AQ * Z));
   kvn->SetEnergy(einc_sed);

   sed->DetectParticle(kvn, &rot);
   eloss_sed = sed->GetEnergyLoss();

   // einc_sed =
   // GetSed()->GetIncidentEnergyFromERes(int(Z_PID),int(M_Q*Z_PID),einc_dc2);
   // eloss_sed =
   // GetSed()->GetDeltaEFromERes(int(Z_PID),int(M_Q*Z_PID),einc_dc2);

   sed->Clear();

   return eloss_sed;
}

Double_t Identificationv::GetEnergyLossDC1(Int_t Z, Double_t AQ)
{
   assert((Z > 0) && (Z < 120));
   assert((AQ > 0.) && (AQ < 10.));

   eloss_dc1 = 0.;

   // Calcul de l'énergie perdue dans la DC1
   einc_dc1 =
      drift_chamber_one->GetIncidentEnergyFromERes(Z,
            static_cast<Int_t>(AQ * Z),
            einc_sed);

   kvn->SetZ(static_cast<Int_t>(Z));
   kvn->SetA(static_cast<Int_t>(AQ * Z));
   kvn->SetEnergy(einc_dc1);
   KVMaterial* kvm_dc1 = 0;

   for (Int_t i = 0; i < 3; i++) {
      fELosLayer_dc1[i] = 0.;
      kvm_dc1 = (KVMaterial*) drift_chamber_one->GetAbsorber(i);
      kvm_dc1->DetectParticle(kvn);

      fELosLayer_dc1[i] = kvm_dc1->GetEnergyLoss();
      eloss_dc1 += fELosLayer_dc1[i];
   }

   kvn->Clear();
   drift_chamber_one->Clear();

   return eloss_dc1;
}


Double_t Identificationv::GetEnergyLossTarget(Int_t Z, Double_t AQ)
{
   assert((Z > 0) && (Z < 120));
   assert((AQ > 0.) && (AQ < 10.));

   // Calcul de l'énergie perdue dans la cible
   einc_tgt =
      target->GetIncidentEnergyFromERes(Z, static_cast<Int_t>(AQ * Z),
                                        einc_dc1);
   eloss_tgt = target->GetDeltaEFromERes(Z, static_cast<Int_t>(AQ * Z),
                                         einc_dc1);

   target->Clear();

   return einc_tgt - einc_dc1;
   // return eloss_tgt;

}

void Identificationv::ResizeVectors()
{
   // Resize vectors which are to be written to the root tree.
   // Use initialisation value in resize method just to be safe.

   AA->resize(MaxM, -10.);
   FragECsI->resize(MaxM, -10.);
   PID->resize(MaxM, -10.);

   PID_sitof->resize(MaxM, -10.);
   PID_chiov2->resize(MaxM, -10.);

   FragDetSi->resize(MaxM, -10);
   FragSignalSi->resize(MaxM, -10);
   FragESi->resize(MaxM, -10.);

   FragTfinal->resize(MaxM, -10.);
   Off_chiosi->resize(MaxM, -10.);
   Off_dc1dc2->resize(MaxM, -10.);

   FragSignalCsI->resize(MaxM, -10);
   FragDetCsI->resize(MaxM, -10);

   FragDetChio->resize(MaxM, -10);
   FragSignalChio->resize(MaxM, -10);
   FragEChio->resize(MaxM, -10.);

   Etot->resize(MaxM, -10.);

   V->resize(MaxM, -10.);

   Vx->resize(MaxM, -10.);
   Vy->resize(MaxM, -10.);
   Vz->resize(MaxM, -10.);

   Beta->resize(MaxM, -10.);

   M_Q->resize(MaxM, -10.);
   Q->resize(MaxM, -10.);
   M->resize(MaxM, -10.);

   RealQ_straight->resize(MaxM, -10.);
   M_straight->resize(MaxM, -10.);
   Q_straight->resize(MaxM, -10);
   M_Q_chiosi->resize(MaxM, -10.);
   Q_chiosi->resize(MaxM, -10.);
   M_chiosi->resize(MaxM, -10.);

   Code_Vamos->resize(MaxM, -10);
   Code_Ident_Vamos->resize(MaxM, -10);

   Z_corr->resize(MaxM, -10);
   Z_corr_sitof->resize(MaxM, -10);
   Z_corr_chiov2->resize(MaxM, -10);

}

void Identificationv::ResetResizedVectors()
{
   // Resize vectors after they have been written to the root tree to restore
   // them to their original length.
   //
   // Use initialisation value in resize method.

   AA->resize(kNumCsI + 1, -10.);
   FragECsI->resize(kNumCsI + 1, -10.);
   PID->resize(kNumCsI + 1, -10.);

   PID_sitof->resize(kNumCsI + 1, -10.);
   PID_chiov2->resize(kNumCsI + 1, -10.);

   FragDetSi->resize(kNumCsI + 1, -10);

   FragSignalSi->resize(kNumCsI + 1, -10);
   FragESi->resize(kNumCsI + 1, -10.);

   FragTfinal->resize(kNumCsI + 1, -10.);
   Off_chiosi->resize(kNumCsI + 1, -10.);
   Off_dc1dc2->resize(kNumCsI + 1, -10.);

   FragSignalCsI->resize(kNumCsI + 1, -10);
   FragDetCsI->resize(kNumCsI + 1, -10);

   FragDetChio->resize(kNumCsI + 1, -10);
   FragSignalChio->resize(kNumCsI + 1, -10);
   FragEChio->resize(kNumCsI + 1, -10.);

   Etot->resize(kNumCsI + 1, -10.);

   V->resize(kNumCsI + 1, -10.);

   Vx->resize(kNumCsI + 1, -10.);
   Vy->resize(kNumCsI + 1, -10.);
   Vz->resize(kNumCsI + 1, -10.);

   Beta->resize(kNumCsI + 1, -10.);

   M_Q->resize(kNumCsI + 1, -10.);
   Q->resize(kNumCsI + 1, -10.);
   M->resize(kNumCsI + 1, -10.);

   RealQ_straight->resize(kNumCsI + 1, -10.);
   M_straight->resize(kNumCsI + 1, -10.);
   Q_straight->resize(kNumCsI + 1, -10);
   M_Q_chiosi->resize(kNumCsI + 1, -10.);
   Q_chiosi->resize(kNumCsI + 1, -10.);
   M_chiosi->resize(kNumCsI + 1, -10.);

   Code_Vamos->resize(kNumCsI + 1, -10);
   Code_Ident_Vamos->resize(kNumCsI + 1, -10);

   Z_corr->resize(kNumCsI + 1, -10);
   Z_corr_sitof->resize(kNumCsI + 1, -10);
   Z_corr_chiov2->resize(kNumCsI + 1, -10);

}

void Identificationv::PrintParameters() const
{
   printf("=== GEOM CROSS ===\n");
   PrintVector<Int_t>(geom_cross);

   printf("=== GEOM CHIOSI ===\n");
   PrintVector<Int_t>(geomchiosi);

   printf("=== GEOM CHIOCSI ===\n");
   PrintVector<Int_t>(geomchiocsi);

   printf("=== GEOM ===\n");
   PrintVector<Int_t>(geom);

   printf("=== P0 ===\n");
   PrintVector<Float_t>(P0);

   printf("=== P1 ===\n");
   PrintVector<Float_t>(P1);

   printf("=== P2 ===\n");
   PrintVector<Float_t>(P2);

   printf("=== P3 ===\n");
   PrintVector<Float_t>(P3);

   printf("=== P0_mq ===\n");
   PrintVector<Float_t>(P0_mq);

   printf("=== P1_mq ===\n");
   PrintVector<Float_t>(P1_mq);

   printf("=== P0_mq_chiosi ===\n");
   PrintVector<Float_t>(P0_mq_chiosi);

   printf("=== P1_mq_chiosi ===\n");
   PrintVector<Float_t>(P1_mq_chiosi);

   printf("=== P0_m ===\n");
   PrintVector<Float_t>(P0_m);

   printf("=== P1_m ===\n");
   PrintVector<Float_t>(P1_m);

   printf("=== P0_m_chiosi ===\n");
   PrintVector<Float_t>(P0_m_chiosi);

   printf("=== P1_m_chiosi ===\n");
   PrintVector<Float_t>(P1_m_chiosi);

   printf("=== Brho_min ===\n");
   PrintVector<Float_t>(Brho_min);

   printf("=== Brho_max ===\n");
   PrintVector<Float_t>(Brho_max);

   printf("=== Fit1 ===\n");
   PrintVector<Float_t>(Fit1);

   printf("=== Fit2 ===\n");
   PrintVector<Float_t>(Fit2);

   printf("=== Fit3 ===\n");
   PrintVector<Float_t>(Fit3);

   printf("=== Fit4 ===\n");
   PrintVector<Float_t>(Fit4);

   printf("=== Fit5 ===\n");
   PrintVector<Float_t>(Fit5);

   printf("=== Fit6 ===\n");
   PrintVector<Float_t>(Fit6);

   printf("=== Fit7 ===\n");
   PrintVector<Float_t>(Fit7);

   printf("=== Brho0 ===\n");
   PrintVector<Float_t>(Brho0);

   printf("=== Tof0 ===\n");
   PrintVector<Float_t>(Tof0);

   printf("=== Esi0 ===\n");
   PrintVector<Float_t>(Esi0);

   printf("=== Tof0_code1 ===\n");
   PrintVector<Float_t>(Tof0_code1);

   printf("=== Deltat_code1 ===\n");
   PrintVector<Float_t>(Deltat_code1);

   printf("=== Stat_Indra ===\n");
   PrintVector<Float_t>(Stat_Indra);

   printf("=== Offset_relativiste_chiosi ===\n");
   PrintVector<Float_t>(Offset_relativiste_chiosi);

   printf("=== P0_aq_straight ===\n");
   PrintVector<Float_t>(P0_aq_straight);

   printf("=== P1_aq_straight ===\n");
   PrintVector<Float_t>(P1_aq_straight);

   printf("=== P2_aq_straight ===\n");
   PrintVector<Float_t>(P2_aq_straight);

   printf("=== P0_aq_straight_chiosi ===\n");
   PrintVector<Float_t>(P0_aq_straight_chiosi);

   printf("=== P1_aq_straight_chiosi ===\n");
   PrintVector<Float_t>(P1_aq_straight_chiosi);

   printf("=== P2_aq_straight_chiosi ===\n");
   PrintVector<Float_t>(P2_aq_straight_chiosi);

}

template <typename Type>
void Identificationv::PrintVector(const std::vector<Type>* const v) const
{
   if (!v) return;

   Int_t num_elements = 0;
   Long64_t num_line = 0;

   std::cout << "Vector data (modulo " << print_modulo << ")" << std::endl;
   std::cout << "[" << num_line << "] : ";
   ++num_line;

   for (size_t i = 0; i < v->size(); ++i) {

      if (num_elements >= print_modulo) {
         std::cout << "\n[" << num_line << "] : ";
         num_elements = 0;
         ++num_line;
      }

      std::cout << v->at(i) << ", ";
      ++num_elements;
   }

   std::cout << std::endl;
}

template <typename Type>
void Identificationv::PrintArray(const Type* const a, const size_t& size) const
{
   if (!a) return;

   if (size > print_max_array_size) {
      Error("PrintArray", "Size of array above maximum allowed value");
      return;
   }

   Int_t num_elements = 0;
   Long64_t num_line = 0;

   std::cout << "Array data (modulo " << print_modulo << ")" << std::endl;
   std::cout << "[" << num_line << "] : ";
   ++num_line;

   for (size_t i = 0; i < size; ++i) {

      if (num_elements >= print_modulo) {
         std::cout << "\n[" << num_line << "] : ";
         num_elements = 0;
         ++num_line;
      }

      std::cout << a[i] << ", ";
      ++num_elements;
   }

   std::cout << std::endl;
}

template <typename Type>
void Identificationv::PrintArray(const Type* const* const a,
                                 const size_t& size_1,
                                 const size_t& size_2) const
{
   if (!a) return;

   if ((size_1 * size_2) > print_max_array_size) {
      Error("PrintArray", "Size of array above maximum allowed value");
      return;
   }

   Int_t num_elements = 0;
   Long64_t num_line = 0;

   std::cout << "Array data (modulo " << print_modulo << ")" << std::endl;
   std::cout << "[" << num_line << "] : ";
   ++num_line;

   for (size_t i = 0; i < size_1; ++i) {
      for (size_t j = 0; j < size_2; ++j) {

         if (num_elements >= print_modulo) {
            std::cout << "\n[" << num_line << "] : ";
            num_elements = 0;
            ++num_line;
         }

         std::cout << a[i][j] << ", ";
         ++num_elements;

      }
   }

   std::cout << std::endl;
}

template <typename Type>
void Identificationv::PrintArray(const Type* const* const* const a,
                                 const size_t& size_1,
                                 const size_t& size_2,
                                 const size_t& size_3) const
{
   if (!a) return;

   if ((size_1 * size_2) > print_max_array_size) {
      Error("PrintArray", "Size of array above maximum allowed value");
      return;
   }

   Int_t num_elements = 0;
   Long64_t num_line = 0;

   std::cout << "Array data (modulo " << print_modulo << ")" << std::endl;
   std::cout << "[" << num_line << "] : ";
   ++num_line;

   for (size_t i = 0; i < size_1; ++i) {
      for (size_t j = 0; j < size_2; ++j) {
         for (size_t k = 0; k < size_3; ++k) {

            if (num_elements >= print_modulo) {
               std::cout << "\n[" << num_line << "] : ";
               num_elements = 0;
               ++num_line;
            }

            std::cout << a[i][j][k] << ", ";
            ++num_elements;

         }
      }
   }

   std::cout << std::endl;
}

void Identificationv::PrintGridMisses() const
{
   Info("PrintGridMisses", "Grid Miss Data:");
   Info("PrintGridMisses", "SICSI:      %d", grid_misses->sicsi);
   Info("PrintGridMisses", "CHIOSI:     %d", grid_misses->chiosi);
   Info("PrintGridMisses", "SITOF:      %d", grid_misses->sitof);
   Info("PrintGridMisses", "CHIOV2:     %d", grid_misses->chiov2);
   Info("PrintGridMisses", "QAQ:        %d", grid_misses->qaq);
   Info("PrintGridMisses", "QAQ_CHIOSI: %d", grid_misses->qaq_chiosi);
   Info("PrintGridMisses", "CUTSCODE2:  %d", grid_misses->qaq_chiosi);
}

void Identificationv::PrintAssertionStatus() const
{
   std::cout << "<Identificationv::PrintAssertionStatus>: "
             << "Assertions are ";

#ifdef NDEBUG
   std::cout << "OFF" << std::endl;
#else
   std::cout << "ON" << std::endl;
#endif

}

