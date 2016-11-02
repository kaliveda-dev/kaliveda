#include "CsICalib.h"

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

ClassImp(CsICalib)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>CsICalib</h2>
<h4>>Best estimations of the total incident energy (=>energy loss in CsI) and particle mass</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

CsICalib::CsICalib(LogFile* Log, Sive503* SiD):
   grid_loader(NULL),
   kInitialised(kFALSE),
   bisector_iterations_(0)
{
   // Default constructor

   LightCsI = 0.;

   eEnergySi = 0.;
   eEnergyCsI = 0.;
   eEnergyGap = 0.;
   sEnergySi = 0.;
   sEnergyCsI = 0.;

   ePied = 0.;
   eZ = 0;
   eA = 0;

   esi1 = 0.;
   esi2 = 0.;
   ecsi1 = 0.;
   ecsi2 = 0.;

   diff1 = 0.;
   diff2 = 0.;
   diffsi = 0.;
   diffcsi = 0.;

   sA = 0;
   iA = 0.;
   right = 0;
   left = 0;
   sRefECsI = 0.;

   Einc = 0.;
   EEsi = 0.;

   L = Log;
   Si = SiD;

   gap = NULL;
   si = NULL;
   csi = NULL;
   kvt_icsi = NULL;
   kvt_sicsi = NULL;
   lum = NULL;

   frag = NULL;
   ttel = NULL;

   kvd_si = NULL;
   kvd_csi = NULL;
   kvd_gap = NULL;

   si_detector = NULL;
   gap_detector = NULL;
   csi_detector = NULL;

   CsI = NULL;

   kvid = NULL;
   kvid_chiosi = NULL;
   kvid_sitof = NULL;
   kvid_cutscode2 = NULL;
   kvid_chiov2 = NULL;
   kvid_qaq = NULL;
   kvid_qaq_chiosi = NULL;

}

CsICalib::~CsICalib()
{
   // Destructor
   if (grid_loader) {
      delete grid_loader;
      grid_loader = NULL;
   }

   if (kvt_sicsi) {
      delete kvt_sicsi;
      kvt_sicsi = NULL;
   }

   if (lum) {
      delete lum;
      lum = NULL;
   }
}

Bool_t CsICalib::Init()
{
   if (kInitialised) return kTRUE;

   grid_loader = new GridLoader();
   assert(grid_loader);

   if (grid_loader) {
      grid_loader->InitAnalysis();
      if (grid_loader->IsInitialised()) kInitialised = kTRUE;
   }

   kvt_sicsi = new KVTelescope();

   si = new KVDetector("Si", 530.*KVUnits::um);
   gap = new KVDetector("C4H10", 136.5 * KVUnits::mm);
   gap->SetPressure(40.*KVUnits::mbar);
   csi = new KVDetector("CsI", 1.*KVUnits::cm);

   kvt_sicsi->Add(si);
   kvt_sicsi->Add(gap);
   kvt_sicsi->Add(csi);

   lum = new KVLightEnergyCsIVamos();

   kInitialised = kTRUE;
   return kTRUE;
}

Bool_t CsICalib::InitRun(const UInt_t run)
{
   // Build a grid list comprising ONLY those grids which are valid for this
   // run. We perform this fairly expensive operation only once per run and it
   // reduces the number of grids that must be searched on an event-by-event
   // basis.

   if (!kInitialised) return kFALSE;

   assert(grid_loader);
   grid_loader->InitRun(run);

   return kTRUE;
}

Bool_t CsICalib::InitTelescope(Int_t num_si, Int_t num_csi)
{
   // Si input (0..17)
   assert((num_si >= 0) && (num_si < 18));

   // CsI input (0..79)
   assert((num_csi >= 0) && (num_csi < 80));

   kvid = NULL;

   TString telescope_name =
      Form("VID_SI_%02d_CSI%02d", num_si + 1, num_csi + 1);

   assert(grid_loader);
   kvid = static_cast<const KVIDGrid*>(
             grid_loader->GetRunGrid(telescope_name)
          );

   if (!kvid) return kFALSE;
   return kTRUE;
}

Bool_t CsICalib::InitTelescopeChioSi(Int_t num_chio, Int_t num_si)
{
   // ChIo input (0..6)
   assert((num_chio >= 0) && (num_chio < 7));

   // Si input (0..17)
   assert((num_si >= 0) && (num_si < 18));

   kvid_chiosi = NULL;

   TString telescope_name =
      Form("VID_CHI_%02d_SI_%02d", num_chio + 1, num_si + 1);

   assert(grid_loader);
   kvid_chiosi = static_cast<const KVIDGrid*>(
                    grid_loader->GetRunGrid(telescope_name)
                 );

   if (!kvid_chiosi) return kFALSE;
   return kTRUE;
}

Bool_t CsICalib::InitTelescopeSiTof(Int_t num_si)
{
   // Si input (0..17)
   assert((num_si >= 0) && (num_si < 18));

   kvid_sitof = NULL;

   TString telescope_name = Form("SIE_%02d_TOF", num_si + 1);

   assert(grid_loader);
   kvid_sitof = static_cast<const KVIDGrid*>(
                   grid_loader->GetRunGrid(telescope_name)
                );

   if (!kvid_sitof) return kFALSE;
   return kTRUE;
}

Bool_t CsICalib::InitCode2Cuts(Float_t brho0)
{
   kvid_cutscode2 = NULL;

   TString telescope_name =
      Form("SIE_TOF_%04d", static_cast<Int_t>(brho0 * 1000.));

   assert(grid_loader);
   kvid_cutscode2 = static_cast<const KVIDGrid*>(
                       grid_loader->GetRunGrid(telescope_name)
                    );

   if (!kvid_cutscode2) return kFALSE;
   return kTRUE;
}

Bool_t CsICalib::InitChioV2(Int_t num_chio)
{
   // ChIo input (0..6)
   assert((num_chio >= 0) && (num_chio < 7));

   kvid_chiov2 = NULL;

   TString telescope_name = Form("CI_V2_%02d", num_chio + 1);

   assert(grid_loader);
   kvid_chiov2 = static_cast<const KVIDGrid*>(
                    grid_loader->GetRunGrid(telescope_name)
                 );

   if (!kvid_chiov2) return kFALSE;
   return kTRUE;
}

Bool_t CsICalib::InitQStraight(Int_t num_csi)
{
   // CsI input (0..79)
   assert((num_csi >= 0) && (num_csi < 80));

   kvid_qaq = NULL;

   TString telescope_name = Form("Q_AQ_CSI%02d", num_csi + 1);

   assert(grid_loader);
   kvid_qaq = static_cast<const KVIDGrid*>(
                 grid_loader->GetRunGrid(telescope_name)
              );

   if (!kvid_qaq) return kFALSE;
   return kTRUE;
}

Bool_t CsICalib::InitQStraight_chiosi(Int_t num_chio)
{
   // ChIo input (0..6)
   assert((num_chio >= 0) && (num_chio < 7));

   kvid_qaq_chiosi = NULL;

   TString telescope_name = Form("Q_AQ_CI%02d", num_chio + 1);

   assert(grid_loader);
   kvid_qaq_chiosi = static_cast<const KVIDGrid*>(
                        grid_loader->GetRunGrid(telescope_name)
                     );

   if (!kvid_qaq_chiosi) return kFALSE;
   return kTRUE;
}

void CsICalib::InitSiCsI(Int_t number) // Si-CsI Telescope
{
   // Si input (0..17)
   assert((number >= 0) && (number < 18));

   /********************************************************************************
       TELESCOPE LAYOUT: Using custom built classes IonisationChamber and
   PlaneAbsorber

       beam >>  | Silicon | C4H10 (Gap) | CsI


   ********************************************************************************/

   KVDetector* silicon_layer(kvt_sicsi->GetDetector(1));
   assert(silicon_layer);
   silicon_layer->SetThickness(Si->si_thick[number + 1]*KVUnits::um);

   KVDetector* csi_layer(kvt_sicsi->GetDetector(3));
   assert(csi_layer);

   assert(lum);
   lum->SetDetector(csi_layer);

}


void CsICalib::SetCalibration(Sive503* Si, CsIv* CsI, Int_t sinum, Int_t csinum)
{
   // Si input (0..17)
   assert((sinum >= 0) && (sinum < 18));

   // CsI input (0..79)
   assert((csinum >= 0) && (csinum < 80));

   if (Si->EMSI > 0) {
      a = Si->ECoef[sinum][0];
      b = Si->ECoef[sinum][1];
      c = Si->ECoef[sinum][2];
      alpha = 1;
      //L->Log<<"parametri silicio a="<<a<<" b="<<b<<" c="<<c<<endl;
   } else {
      a = b = c = alpha = 0.0;
   }

   KVDetector* csi_layer(kvt_sicsi->GetDetector(3));
   assert(csi_layer);

   lum->SetDetector(csi_layer);

   if (CsI->EMCSI > 0) {
      ePied = CsI->Ped[csinum];
      a1 = CsI->ECoef[csinum][0];
      a2 = CsI->ECoef[csinum][1];
      a3 = CsI->ECoef[csinum][2];

      lum->SetNumberParams(3);
      lum->SetParameters(a1, a2, a3);
      //L->Log<<"parametri cesio a1="<<a1<<" a2="<<a2<<" a3="<<a3<<endl;
      //L->Log<<"pedestal csi=      "<<ePied<<endl;

   } else {
      ePied = 0.0;

      lum->SetNumberParams(3);
      lum->SetParameters(0., 0., 0.);
   }
}

void CsICalib::SetSimCalibration(Sive503* Si, CsIv* CsI, Int_t sinum, Int_t csinum)
{
   // Test method, the calibration parameters are read by CsIv::CsIv(). This
   // method is identical to SetCalibration except I make no checks on the
   // multiplicity and set the pedestals to zero.

   // Si input (0..17)
   assert((sinum >= 0) && (sinum < 18));

   // CsI input (0..79)
   assert((csinum >= 0) && (csinum < 80));

   a = Si->ECoef[sinum][0];
   b = Si->ECoef[sinum][1];
   c = Si->ECoef[sinum][2];
   alpha = 1;

   KVDetector* csi_layer(kvt_sicsi->GetDetector(3));
   assert(csi_layer);

   lum->SetDetector(csi_layer);

   ePied = 0.;
   a1 = CsI->ECoef[csinum][0];
   a2 = CsI->ECoef[csinum][1];
   a3 = CsI->ECoef[csinum][2];

   lum->SetNumberParams(3);
   lum->SetParameters(a1, a2, a3);
}

//fragment Z
void CsICalib::SetFragmentZ(Int_t Zin)
{
   eZ = Zin;
   lum->SetZ(eZ);
}

//set fragment A
void CsICalib::SetFragmentA(Int_t Ain)
{
   lum->SetA(Ain);
   sA = Ain;
   //L->Log<<"SetFragment A : "<<sA<<endl;
}

//Complete procedure to get the best estimate of the residual energy
Double_t CsICalib::GetResidualEnergyCsI(Double_t esi, Double_t chcsi)
{
   assert((esi >= 0.) && (esi < 3000.));
   assert(chcsi >= 0.);

   Int_t A;
   A = 2 * eZ;
   eEnergySi = esi;

   LightCsI = chcsi - ePied;

   bisector_iterations_ = 0;
   Bisection(A, static_cast<Double_t>(LightCsI)); //Call the bisection method
   Interpolate(); //Interpolation of ECsI and A values

   return sRefECsI;
}

//Si calibration function
void CsICalib::CalculateESi(Double_t chan)
{
   assert(chan >= 0.);

   Double_t fact = 1.;
   eEnergySi = fact * alpha * (a + chan * b + chan * chan * c);
   assert((eEnergySi >= 0.) && (eEnergySi < 3000.));
}

//Complete procedure for to get the energy losses from simulation
void  CsICalib::CompleteSimulation()
{
   ++bisector_iterations_;

   sEnergySi = 0.0;
   sEnergyCsI = 0.0;
   eEnergyCsI = 0.0;
   eEnergyGap = 0.0;

   eEnergyCsI = lum->Compute(LightCsI); //Get the ECsI from the known (Z,A) and the calibration

   Double_t einc_isogap2 = 0.0;

   KVDetector* silicon_layer(kvt_sicsi->GetDetector(1));
   KVDetector* gap_layer(kvt_sicsi->GetDetector(2));
   KVDetector* csi_layer(kvt_sicsi->GetDetector(3));

   assert(silicon_layer);
   assert(gap_layer);
   assert(csi_layer);

   einc_isogap2 = silicon_layer->GetEResFromDeltaE(eZ, sA, eEnergySi);
   eEnergyGap = gap_layer->GetDeltaE(eZ, sA, einc_isogap2);
   Einc = eEnergySi + eEnergyGap + eEnergyCsI;

   part.Clear();
   part.SetZAandE(eZ, sA, Einc);

   kvt_sicsi->DetectParticle(&part); //Simulate the fragment through the telescope

   //simulated ESi obtained from the Einc (ESi(calibration) + ECsI(Z,A known and calibration)):
   sEnergySi = silicon_layer->GetEnergy();
   sEnergyCsI = csi_layer->GetEnergy();

   if (sEnergyCsI >= 3000.) {
      std::cout << "Error Report: " << std::endl;
      std::cout << "eEnergySi: " << eEnergySi << std::endl;
      std::cout << "eZ: " << eZ << std::endl;
      std::cout << "sA: " << sA << std::endl;
      std::cout << "Einc: " << Einc << std::endl;
      std::cout << "sEnergyCsI: " << sEnergyCsI << std::endl;
      std::cout << "sEnergyGap: " << gap_layer->GetEnergy() << std::endl;
      std::cout << "sEnergySi: " << sEnergySi << std::endl;
   }

   assert((sEnergyCsI >= 0.) && (sEnergyCsI < 3000.));

   part.Clear();
   kvt_sicsi->ResetDetectors();

}

void CsICalib::Bisection(Int_t A, Double_t chan)   //UShort_t chan
{
   IGNORE_UNUSED(chan);

   Int_t middle, it = 0;
   good_bisection = kTRUE;

   if ((A - 5) > 0) {
      left = A - 5;
   } else {
      left = 1;
   }

   right = A + 10;

   Int_t factor = 1;
   Double_t difference = 0.;
   Double_t diffright = 0.;

   SetFragmentA(left);
   CompleteSimulation();
   difference = eEnergySi - sEnergySi;
   if (difference > 0) {
      factor = -1;
   }
   difference *= factor;

   SetFragmentA(right);
   CompleteSimulation();
   difference = eEnergySi - sEnergySi;
   if (difference > 0)  good_bisection = kFALSE;
   difference *= factor;
   diffright = difference;

   if (difference < 0) {
      while (diffright < 0) {

         if ((left - 1) > 0) left = left - 1;
         else left = 1;

         SetFragmentA(left);
         CompleteSimulation();
         difference = eEnergySi - sEnergySi;
         if (difference > 0) {
            factor = -1;
         }
         diffright *= factor;

         if (left == 1) {
            cout << "Bisection error : Lower value of A not found" << endl;
            good_bisection = kFALSE;

            break;
         }
      }

   }

   //loop: bisection
   while ((right - left) > 1) {

      middle = (left + right + 1) / 2;

      SetFragmentA(middle);
      CompleteSimulation();
      esi2 = sEnergySi;
      ecsi2 = sEnergyCsI;

      difference = factor * (eEnergySi - sEnergySi);
      if (difference < 0) {
         left = middle;
      } else
         right = middle;

      it++;
   }

   //simulated energies for two values of A closest to the point of zero
   //difference between the simulated energy for Si and the one from calibration

   SetFragmentA(left);
   CompleteSimulation();
   esi1 = sEnergySi;
   ecsi1 = sEnergyCsI;

   SetFragmentA(right);
   CompleteSimulation();
   esi2 = sEnergySi;
   ecsi2 = sEnergyCsI;
}

Double_t CsICalib::BisectionLight(Double_t Z, Double_t A, Double_t ECsI)
{
   Double_t middle = 0.;
   Int_t it = 0;
   Double_t leftA, rightA = 0.;
   //L->Log<<"------------"<<endl;
   //L->Log<<"Valeur de A : "<<A<<endl;
   //L->Log<<"Light Reference :  "<<LightCsI<<endl;

   if (A - 0.5 > 0)
      leftA = A - 0.5; //left and right are integer!! A is a double
   else leftA = 0.;
   rightA = A + 0.5;

   //L->Log<<"-----------------------------------BISECTION-LIGHT------------------"<<endl;//paola

   //new bisection method
   //=== Test

   Double_t calcul_light = lum->Invert(Z, leftA, ECsI);

   Int_t factor = 1;
   Double_t difference = 0.;
   difference = LightCsI - calcul_light;
   if (difference > 0) {
      factor = -1;
   }
   difference *= factor;
   //L->Log<<"A left : "<<leftA<<" Measured Light : "<<LightCsI<<" Calculated
   //Light : "<<calcul_light<<" difference : "<<difference<<" factor :
   //"<<factor<<endl;

   calcul_light = lum->Invert(Z, rightA, ECsI);
   difference = LightCsI - calcul_light;
   difference *= factor;
   //L->Log<<"A right : "<<rightA<<" Measured Light : "<<LightCsI<<" Calculated
   //Light : "<<calcul_light<<" difference : "<<difference<<" factor :
   //"<<factor<<endl;

   if (difference < 0) {
      //L->Log<<"Bisection error"<<endl;
   }

   // === End test

   //loop: bisection
   while (rightA - leftA > 0.1) {

      //L->Log<<"-------------------loop bisection------------"<<endl;
      //L->Log<<"LightCsI  : "<<LightCsI<<endl;
      //cout<<"LightCsI : "<<LightCsI<<endl;
      middle = (leftA + rightA) / 2;

      //simulated energies stored for middle A
      calcul_light = lum->Invert(Z, middle, ECsI);

      //new bisection method
      //=== Test
      difference = factor * (LightCsI - calcul_light);
      //L->Log<<"difference : "<<difference<<endl;
      if (difference < 0) {
         leftA = middle;
      } else
         rightA = middle;
      // === End test

      /*if(esi2>eEnergySi){
         right=middle;
      }
      else
      left=middle;*/

      //L->Log<<"A2 (middle) :   "<<middle<<"   Measured Light =
      //"<<LightCsI<<"  Calculated Light =   "<<calcul_light<<endl; //paola

      it++;
      //L->Log<<"----------------iteration= "<<it<<endl;
   } //end of bisection loop

   //simulated energies for two values of A closest to the point of zero
   //difference between the simulated energy for Si and the one from calibration

   Double_t calcul_light_left = lum->Invert(Z, leftA, ECsI);
   Double_t calcul_light_right = lum->Invert(Z, rightA, ECsI);
   if (TMath::Abs(LightCsI - calcul_light_left) > TMath::Abs(LightCsI - calcul_light_right)) {
      return rightA;
   } else
      return leftA;

}

//interpolation usage for to find the right residual energy and right A, using
//the final interval returned by bisection
void CsICalib::Interpolate()
{
   sRefECsI = GetInterpolationD(esi1, ecsi1, esi2, ecsi2, eEnergySi);

   if ((sRefECsI < 0.) || (sRefECsI >= 3000.)) {
      std::cerr << "CsICalib::Interpolate Error Report:" << std::endl;
      std::cerr << "sRefECsI: " << sRefECsI << std::endl;
      std::cerr << "esi1:  " << esi1 << std::endl;
      std::cerr << "ecsi1: " << ecsi1 << std::endl;
      std::cerr << "esi2:  " << esi2 << std::endl;
      std::cerr << "ecsi2: " << ecsi2 << std::endl;
      std::cerr << "eEnergySi: " << eEnergySi << std::endl;
   }

   assert((sRefECsI >= 0.) && (sRefECsI < 3000.));

   iA = GetInterpolationD(esi1, (Double_t)left, esi2, (Double_t)right, eEnergySi);
}

// Interpolation procedure: finding y corresponding to x, under assumption that
//these variables behave linearly within the given ranges x1-x2, and y1-y2.
Double_t CsICalib::GetInterpolationD(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Double_t x)
{
   Double_t slope((y1 - y2) / (x1 - x2));
   Double_t y(slope * (x - x2) + y2);

   return y;
}

//--------------------------Retrieve values------------------------//
//Getter for energy loss in silicon
Double_t CsICalib::RetrieveEnergySi() const
{
   return eEnergySi;
}

//getter for the best estimate of A
Double_t CsICalib::RetrieveA() const
{
   return iA;
}

//getter for light in CsI
Double_t CsICalib::RetrieveLight() const
{
   return LightCsI;
}

Double_t CsICalib::RetrieveEnergyCsI() const
{
   return sRefECsI; //was eEnergyCsI
}

void CsICalib::PrintAssertionStatus() const
{
   std::cout << "<CsICalib::PrintAssertionStatus>: "
             << "Assertions are ";

#ifdef NDEBUG
   std::cout << "OFF" << std::endl;
#else
   std::cout << "ON" << std::endl;
#endif

}

Bool_t CsICalib::get_good_bisection() const
{
   return good_bisection;
};

Double_t CsICalib::get_eEnergyGap() const
{
   return eEnergyGap;
};

const KVIDGrid* CsICalib::get_kvid() const
{
   return kvid;
};

const KVIDGrid* CsICalib::get_kvid_chiosi() const
{
   return kvid_chiosi;
};

const KVIDGrid* CsICalib::get_kvid_sitof() const
{
   return kvid_sitof;
};

const KVIDGrid* CsICalib::get_kvid_cutscode2() const
{
   return kvid_cutscode2;
};

const KVIDGrid* CsICalib::get_kvid_chiov2() const
{
   return kvid_chiov2;
};

const KVIDGrid* CsICalib::get_kvid_qaq() const
{
   return kvid_qaq;
};

const KVIDGrid* CsICalib::get_kvid_qaq_chiosi() const
{
   return kvid_qaq_chiosi;
};

Int_t CsICalib::get_bisector_iterations() const
{
   return bisector_iterations_;
}

void CsICalib::set_si_detector(const KVDetector* const detector)
{
   si_detector = detector;
}

void CsICalib::set_gap_detector(const KVDetector* const detector)
{
   gap_detector = detector;
}

void CsICalib::set_csi_detector(const KVDetector* const detector)
{
   csi_detector = detector;
}

