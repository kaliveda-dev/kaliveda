//Created by KVClassFactory on Mon Sep  3 11:29:00 2012
//Author: Guilain ADEMARD

#include "KVDriftChamber.h"
#include "TH1F.h"

ClassImp(KVDriftChamber)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDriftChamber</h2>
<h4>Drift Chamber, used at the focal plan of VAMOS</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVString KVDriftChamber::fACQParamTypes("0:Q, 1:E, 2:T, 9:NO_TYPE");
KVString KVDriftChamber::fPositionTypes("0:X1, 1:X2, , 2:Y, 3:Z, 4:XY, 9:NO_TYPE");
Int_t    KVDriftChamber::fNstripsOK = 3;

void KVDriftChamber::init()
{
   // Initialise non-persistent pointers
   fQ  = new TH1F** [3];
   for (Int_t i = 0; i < 3; i++) { //[ raw, calibrated, clean ]
      fQ[i] = new TH1F*[2];
      for (Int_t j = 0; j < 2; j++) { //[ Chamber 1, Chamber 2 ]
         fQ[i][j]      = NULL;
      }
   }

   fRawPosX [0] = fRawPosX [1] = fRawPosX [2] = -500;
   fERawPosX[0] = fERawPosX[1] = fERawPosX[2] = -500;
   fRawPosY = fERawPosY = -500;

   fNstripsOK = (Int_t)gDataSet->GetDataSetEnv("KVDriftChamber.NumberOfStripsOK", 3.);
   fDriftV    = gDataSet->GetDataSetEnv("KVDriftChamber.DriftVelocity", 4.84);  // in cm/us
   fStripWidth = gDataSet->GetDataSetEnv("KVDriftChamber.StripWidth", 0.63);  // in cm

   fTfilPar = NULL;
   fTfilCal = NULL;

   fNmeasX = 2; // the 2 chambers of the Drift Chamber give to measurment
   // of the X position (X1 and X2)

   // Initial values of Z offsets for X1, X2 and Y position measurements
   // are given by the position in the detector of :
   //   - the first  line of pads of the cathode ( -2.5 cm ) for X1
   //   - the second line of pads of the cathode ( +2.5 cm ) for X2
   //   - the middle of the detector ( 0 cm ) for Y
   SetZOffsets();

   // The center of the detector is at the middle of the strip 32
   // of the range 2 and between strips 32 and 33 of the range 1 (offsets 0,0).
   SetXOffsets();

   // The Hyperbolic Secant Squared (SECHS) method is by default used for X position measurements
   SetSECHSReconstructionX(kFALSE);

   // To be modified ???
   // By default the raw positions in drift chambers are calculated from the Q histograms
   // (see GetRawPosition() method) i.e fkRawPosSetByHand=kFALSE.
   // This can be modified and set by hand (can be useful for tracking for example)
   // using the SetRawPosition() method and then fkRawPosSetByHand=kTRUE.
   // As long as fkRawPosSetByHand=kTRUE, GetRawPosition() will return the values set by hand, the only
   // way to go back to the raw position calculations from Q histograms is to use ResetRawPositions() method.
   fkRawPosSetByHand = kFALSE;

   //a KVDriftChamber can not be used in a ID telescope
   ResetBit(kOKforID);
}
//________________________________________________________________

KVDriftChamber::KVDriftChamber()
{
   // Default constructor
   init();
   SetType("DC");
   SetLabel("DC");
   SetName(GetArrayName());
}
//________________________________________________________________

KVDriftChamber::KVDriftChamber(const KVDriftChamber& obj)  : KVVAMOSDetector()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVDriftChamber::~KVDriftChamber()
{
   // Destructor
   for (Int_t i = 0; i < 3; i++) { // loop over [ raw, calib, clean ]
      for (Int_t j = 0; j < 2; j++) { // loop over both chambers
         SafeDelete(fQ[i][j]);
      }
      delete[] fQ[i];
   }
   delete[] fQ;
}
//________________________________________________________________

void KVDriftChamber::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVDriftChamber::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVVAMOSDetector::Copy(obj);
   //KVDriftChamber& CastedObj = (KVDriftChamber&)obj;
}
//________________________________________________________________

void  KVDriftChamber::Clear(Option_t* option)
{
   // Set the energy losses etc. to zero and reset the histograms used
   // to find the positions (Qraw and Q)

   KVVAMOSDetector::Clear(option);
   ResetCalculatedData();
}
//________________________________________________________________

void KVDriftChamber::Initialize()
{
   // Initialize the data members. Called by KVVAMOS::Initialize().
   ResetCalculatedData();
}
//________________________________________________________________

Bool_t KVDriftChamber::PositionIsOK()
{
   // Returns true if the position can be given by this DriftChamber, i.e. the
   // position is calibrated, the raw X and Y positions are well
   // determined. In this case the position is given by the method
   // GetPosition(...).

   return IsPositionCalibrated() && (GetRawPosition('X') >= 0) && (GetRawPosition('Y') >= 0);
}
//________________________________________________________________

void KVDriftChamber::ResetCalculatedData()
{
   // Reset all position histograms (raw, calibrated, clean)
   // and raw positions for both chambers.
   // This method have to be called each time you read a new event (i.e.
   // when the ACQ parameter change) otherwise certain methods
   // (such as GetPosition(), GetQrawHisto() ...) will not return
   // what you expect.
   for (Int_t i = 0; i < 3; i++) {
      for (Int_t j = 0; j < 2; j++) {
         if (fQ[i][j]) fQ[i][j]->Reset();
      }
   }

   fRawPosX [0] = fRawPosX [1] = fRawPosX [2] = -500;
   fERawPosX[0] = fERawPosX[1] = fERawPosX[2] = -500;
   fRawPosY = fERawPosY = -500;
}
//________________________________________________________________

const Char_t* KVDriftChamber::GetArrayName()
{
   // Name of detector given in the form
   // DC1 DC2 ...
   //
   // The root of the name is the detector type.

   if (GetNumber() > 0) {
      fFName  = Form("%s%d", GetType(), GetNumber());
      SetLabel(fFName.Data());
   }
   else fFName = GetType();
   return fFName.Data();
}
//________________________________________________________________

const Char_t* KVDriftChamber::GetEBaseName() const
{
   // Base name of the energy used to be compatible
   // GANIL acquisition parameters
   //
   // The base name is "EFIL_<number>"

   return Form("EFIL_%d", GetNumber());
}
//________________________________________________________________

void KVDriftChamber::SetACQParams()
{
   // Setup the acquistion parameters of electron drift time, energy and position  to the drift chamber.
   // Electron drif time ACQ parameters have the type 'T' (for time).
   // Energy ACQ parameters have the type 'E' (for Energy).
   // Their name has the form: [param. type]FIL_[detector number]
   //
   // Position ACQ parameters have the type 'Q' (for charge).
   // Their name has the form: STR_[detector number][chamber number]_[64]
   // where 64 is the number of strips or the X measurment in each chamber.



   // Charge ACQ parameters
   TString name, title;
   KVEBYEDAT_ACQParam* par = NULL;
   Int_t Nstrips =  _DC_N_STRIPS_;
   const Char_t* extraname[]  = {    "raw", "calib",        "clean"};
   const Char_t* extratitle[] = {       "",     "", "without noise"};
   const Char_t* units[]      = {"channel", "Volt",         "Volt"};
   for (Int_t i = 0; i < 2; i++) { // loop over row
      Int_t c_num = i + 1;
      for (Int_t num = 1; num <= Nstrips; num++) { // loop over strips
         par = new KVEBYEDAT_ACQParam;
         name.Form("STR_%d%d_%02d", GetNumber(), c_num, num);
         par->SetName(name);
         par->SetType("Q");
         par->SetLabel(Form("X%d", c_num));
         par->SetNumber(num);
         par->SetUniqueID(CalculateUniqueID(par));
         AddACQParam(par);
      }

      // build  Q histograms for both rows
      for (Int_t j = 0; j < 3; j++) { // loop over {raw,calib,clean}
         // Qraw histogram
         if (fQ[j][i]) SafeDelete(fQ[j][i]);
         name.Form("Q%s_%s_C%d", extraname[j], GetArrayName(), c_num);
         title.Form("X position in chamber %d of %s %s; Strip number; Q%s (%s)", c_num, GetArrayName(), extratitle[j], extraname[j], units[j]);
         fQ[j][i] = new TH1F(name.Data(), title.Data(), Nstrips, .5, Nstrips + .5);
         fQ[j][i]->SetDirectory(0);
         fQ[j][i]->SetLineColor(j + 1);
      }
   }

   // Energy ACQ Parameters
   par = new KVEBYEDAT_ACQParam;
   par->SetName(GetEBaseName());
   par->SetType("E");
   par->SetNumber(GetNumber());
   par->SetUniqueID(CalculateUniqueID(par));
   AddACQParam(par);

   // Electron drift time ACQ Parameters
   par = new KVEBYEDAT_ACQParam;
   par->SetName(Form("TFIL_%d", GetNumber()));
   par->SetType("T_FIL");
   par->SetLabel("T_FIL");
   par->SetNumber(GetNumber());
   par->SetUniqueID(CalculateUniqueID(par));
   AddACQParam(par);
   fTfilPar = par;
}
//________________________________________________________________

void KVDriftChamber::SetPressure(Double_t p /* mbar */)
{
   // Set the same  pressure for each gas layer (in mbar)

   if (!IsGas()) return;

   KVMaterial* abs = NULL;
   TIter next(GetListOfAbsorbers());
   while ((abs = (KVMaterial*)next())) {
      if (abs->IsGas()) abs->SetPressure(p * KVUnits::mbar);
   }

   KVDetector::SetPressure(p * KVUnits::mbar);
}
//________________________________________________________________

void KVDriftChamber::SetTemperature(Double_t t)
{
   // Set the same temperature for each gaz layer.
   // The units are: degrees celsius

   if (!IsGas()) return;

   KVMaterial* abs = NULL;
   TIter next(GetListOfAbsorbers());
   while ((abs = (KVMaterial*)next())) {
      if (abs->IsGas()) abs->SetTemperature(t);
   }
}
//________________________________________________________________

TH1F* KVDriftChamber::GetQrawHisto(Int_t c_num)
{
   // Retruns the X position histogram with raw charges (Q) for
   // a given chamber. c_num=[1,2] is the number of this chamber.
   // If the histogram is empty then no acquisition parameter
   // was fired.

   Int_t i = c_num - 1;
   if ((i < 0) || (i > 1)) return NULL;
   if (!fQ[0][i]) return NULL;
   if (fQ[0][i]->GetEntries()) return fQ[0][i];

   fQ[0][i]->Reset();
   TIter next(GetACQParamList());

   KVEBYEDAT_ACQParam* par  = NULL;
   while ((par = (KVEBYEDAT_ACQParam*)next())) {

      if ((par->GetLabel()[1] != Char_t('1' + i)) || !par->Fired("P")) continue;

      Int_t num = (c_num % 2 ? par->GetNumber() : _DC_N_STRIPS_ + 1 - par->GetNumber());
      fQ[0][i]->SetBinContent(num, (Double_t)par->GetCoderData());
   }

   return fQ[0][i];
}
//________________________________________________________________

TH1F* KVDriftChamber::GetQHisto(Int_t c_num)
{
   // Retruns the position histogram with calibrated charges (Q)
   // for a given chamber. c_num=[1,2] is the number of this chamber.

   Int_t i = c_num - 1;
   if ((i < 0) || (i > 1)) return NULL;
   if (!fQ[1][i]) return NULL;
   if (fQ[1][i]->GetEntries()) return fQ[1][i];

   fQ[1][i]->Reset();
   TIter next(GetListOfCalibrators());

   KVCalibrator* cal  = NULL;
   KVEBYEDAT_ACQParam*   par  = NULL;

   Int_t NcalOK = 0;
   while ((cal = (KVCalibrator*)next())) {


      if (!cal->GetStatus() || (GetPositionTypeIdxFromID(cal->GetUniqueID()) != i)) continue;
      NcalOK++;

      KVFunctionCal* calf = (KVFunctionCal*)cal;
      par = calf->GetACQParam();
      if (!par->Fired("P")) continue;

      Int_t num = (c_num % 2 ? calf->GetNumber() : _DC_N_STRIPS_ + 1 - calf->GetNumber());
      fQ[1][i]->SetBinContent(num, calf->Compute((Double_t)calf->GetACQParam()->GetCoderData()));
   }

   if (NcalOK < 4) Warning("GetQHisto", "Impossible to calibrate correctly the charges for X position in the chamber %d of %s, %d calibrators are OK", c_num, GetName(), NcalOK);

   return fQ[1][i];
}
//________________________________________________________________

TH1F* KVDriftChamber::GetCleanQHisto(Int_t c_num)
{
   // Retruns the X position histogram with calibrated charges (Q)
   // after the substration of noise for a given chamber. c_num=[1,2] is the number of this chamber.

   Int_t i = c_num - 1;
   if ((i < 0) || (i > 1)) return NULL;
   if (!fQ[2][i]) return NULL;
   if (fQ[2][i]->GetEntries()) return fQ[2][i];

   TH1F* hQcal = GetQHisto(c_num);
   if (!hQcal) return NULL;

   Float_t threshold = CalculateQThreshold(c_num);

   for (Int_t j = 1; j <= hQcal->GetNbinsX(); j++) { //loop over strips
      Double_t QTmp = hQcal->GetBinContent(j);
      if (QTmp > threshold) {
         fQ[2][i]->SetBinContent(j, QTmp - threshold);
      }
   }

   return fQ[2][i];
}
//________________________________________________________________

Float_t KVDriftChamber::CalculateQThreshold(Int_t c_num)
{
   // Calculate the noise in order to remove it on strips for X
   // position of a given chamber. c_num=[1,2] is the number of this
   // chamber.

   //Study the noise and the maximum value for the charge.
   //At this stage the quantities noise_*** contain also the signal!
   Float_t QTmp, noise_mean, noise_var, noise_stdev, noise_sum;
   Float_t Q_max;
   Q_max = noise_mean = noise_var = noise_sum = noise_stdev = 0.;

   UShort_t Q_max_Nr;
   Q_max_Nr = 1000 ;

   TH1F* hQcal = GetQHisto(c_num);
   if (!hQcal) return 0;

   for (Int_t j = 1; j <= hQcal->GetNbinsX(); j++) { //loop over strips
      QTmp = hQcal->GetBinContent(j);
      if (QTmp > 0) {
         noise_mean += QTmp;
         noise_var  += QTmp * QTmp;
         noise_sum  += 1.;
      }
      if (QTmp > Q_max) {
         Q_max    = QTmp;
         Q_max_Nr = j;
      }
   }

   //Study the surrounding of the maximum charge
   Float_t max_mean, max_var, max_sum;
   max_mean = max_var = max_sum = 0.;
   for (Int_t j = -Border; j <= Border; j++) {
      Int_t k = Q_max_Nr + j;
      if ((0 < k) && (k <= hQcal->GetNbinsX())) {
         QTmp = hQcal->GetBinContent(k);
         if (QTmp > 0) {
            max_mean  += QTmp;
            max_var   += QTmp * QTmp;
            max_sum   += 1.;
         }
      }
   }
   //The signal of the maximum charge is removed from the quantities noise_***
   noise_mean -= max_mean;
   noise_var  -= max_var;
   noise_sum  -= max_sum;
   //Calculate statistical quntities related to the noise and fix the threshold
   //as the mean of the noise + 1 time its standart deviation
   if (noise_sum < 2. || noise_var == 0) {
      noise_mean = noise_var = 0;
   }
   else {
      noise_mean /= noise_sum;
      noise_var   = noise_var / noise_sum - noise_mean * noise_mean;
      noise_stdev = TMath::Sqrt(noise_var);
   }
   Float_t threshold = noise_mean + noise_stdev;

   return threshold;
}
//________________________________________________________________

void KVDriftChamber::ShowCleanQHisto(Int_t c_num, Option_t* opt)
{
   TH1F* hh = GetCleanQHisto(c_num);
   if (hh) hh->Draw(opt);
}
//________________________________________________________________

void KVDriftChamber::ShowQrawHisto(Int_t c_num, Option_t* opt)
{
   TH1F* hh = GetQrawHisto(c_num);
   if (hh) hh->Draw(opt);
}
//________________________________________________________________

void KVDriftChamber::ShowQHisto(Int_t c_num, Option_t* opt)
{
   TH1F* hh = GetQHisto(c_num);
   if (hh) hh->Draw(opt);
}
//________________________________________________________________


void KVDriftChamber::SetRawPosition(Double_t* X1_X2_Y_Pad1_Pad2)
{
   // This method allows to set by hand the raw positions of the drift chamber.
   // X1X2Yf is an array of 3 elements: X1 is the raw X position for the first row
   //                                   X2 is the raw X position for the second row
   //                                   Y is the raw Y position for the drift chamber
   //                                   Pad1 is the pad with the maximum Q for row 1 (used for SECHS method)
   //                                   Pad2 is the pad with the maximum Q for row 2 (used for SECHS method)
   //
   // If the raw position is set successfully then fkRawPosSet==kTRUE and the GetRawPosition()
   // method will now return the values set by the user (in stead of calculating them
   // using the Q histograms), and this as long as fkRawPosSet==kTRUE.
   // To reset the raw position, the user should use the method ResetRawPosition().
   //
   // Once you have set the raw positions, the getter method to access to these positions
   // will return these new values i.e. the acquisition parameters will
   // not be read anymore for calculating the raw positions until the next
   // call to Initialize().
   //
   // It is the user responsability to verify what method is used for the raw position
   // measurement (either SECHS method or Mean/RMS method) (see SetSECHSReconstruction()).

   if (X1_X2_Y_Pad1_Pad2) {
      fRawPosX[0] = 0.5 * (X1_X2_Y_Pad1_Pad2[0] + X1_X2_Y_Pad1_Pad2[1]); //to be modified if we want to include statistcs...
      fRawPosX[1] = X1_X2_Y_Pad1_Pad2[0];
      fRawPosX[2] = X1_X2_Y_Pad1_Pad2[1];
      fRawPosY   = X1_X2_Y_Pad1_Pad2[2];
      fPadMax[0] = X1_X2_Y_Pad1_Pad2[3];
      fPadMax[1] = X1_X2_Y_Pad1_Pad2[4];
      fkRawPosSetByHand = kTRUE;
   }
}
//________________________________________________________________

Double_t KVDriftChamber::GetRawPosition(Char_t dir, Int_t num)
{
   // Returns the raw position in X/Y with respect to the center of the detector. The value is deduced from
   // the histogram representing the calibrated charge versus strip number for X direction
   // or returns the electron drift time for Y position.
   // Argument 'num' is used to specified the X position to be returned:
   //  dir = 'X' and num = 0  --> return the mean value between X1 and X2
   //  dir = 'X' and num = 1  --> return X1
   //  dir = 'X' and num = 2  --> return X2
   //
   //
   // Two methods can be used for X raw position: the Hyperblic Secant Squared (SECHS) method (used by default)
   // or the Mean and RMS estimation method. The method used for the X position measurement can be changed
   // by calling SetSECHSReconstructionX(Bool_t kSECHS)
   //
   // SECHS method ( see T.Roger et al. NIMA 638 (2001) and Lau et al. NIMA 366 (1995) ):
   // We compute here the charge profile's centroid, seen on a strip of width w=StripWidth()/2.
   // This means the avalanche centroid's position evolves in [0.,w/2.], without considering for the moment
   // the number of the strip.
   // To evaluate the centroid, we only consider the charge of the most significant strip and the charges of the
   // two neighbooring ones (as the spatial resolution is expected to be degraded by the use of more than 3 strips).
   //
   // Mean and RMS method:
   // We evaluate here the Mean and RMS values of the histogram representing the calibrated charge versus strip number.
   // The two cathode plans are offset by half a strip to reduce the non linearity of the position measurement
   // in between the strips. The center of the detector is by default at the middle of the strip 32 of the
   // range 2 and between strips 32 and 33 of the range 1.
   //
   //
   // You can apply an X offset to change the strip position with respect to
   // the center of the detector by calling SetXOffsets.
   //
   //
   // For Y raw position we evaluate the electron-drift-time canal.
   //
   //
   // In any case, if the raw position in not calculable, the value -666 will be returned.

   Int_t idx = IDX(dir);

   switch (idx) {

      case 0: { // for X direction

            //If X raw position was not set by hand
            if (!IsRawPosSetByHand()) {
               if (num < 0 || num > 2) num = 0;
               if (fERawPosX[ num ] > -500) return fRawPosX[ num ];

               fRawPosX [ 0 ] = 0;
               fERawPosX[ 0 ] = 0.;
               Double_t sum   = 0.;

               for (Int_t c = 1; c < 3; c++) { //loop over both rows of the chamber
                  fRawPosX [ c ]   = -666;
                  fERawPosX[ c ]   = -1;
                  fPadMax[ c - 1 ] = -666;

                  TH1F* hh = GetCleanQHisto(c);
                  if (!hh) continue;

                  if (!hh->GetEntries()) continue;

                  ///////////////////////////////////////////////////

                  Int_t binMax = hh->GetMaximumBin();
                  Int_t min;
                  for (min = binMax; min > 1; min--) {
                     if (hh->GetBinContent(min) <= 0.) {
                        min++;
                        break;
                     }
                     Double_t deltaQ = hh->GetBinContent(min) - hh->GetBinContent(min - 1);
                     if (deltaQ < 0) break;
                  }
                  Int_t max;
                  for (max = binMax; max < hh->GetNbinsX(); max++) {
                     if (hh->GetBinContent(max) <= 0.) {
                        max--;
                        break;
                     }
                     Double_t deltaQ = hh->GetBinContent(max) - hh->GetBinContent(max + 1);
                     if (deltaQ < 0) break;
                  }

                  if ((max - min + 1) < fNstripsOK) continue;

                  if (IsSECHSReconstructionX()) {
                     // Hyperbolic secant squared (SECHS) method:
                     // by convention Q0 is the charge of the most significant strip
                     // and Q+ and Q- are the charges of the left and right neighboring pads,
                     // and ww is the strip(pad) witdh.
                     fPadMax[c - 1] = hh->GetBinCenter(binMax); //the bin center is the number of the pad
                     Float_t* QQ  = new Float_t[fNstripsOK];
                     QQ[0] = hh->GetBinContent(binMax - 1); //Q-
                     QQ[1] = hh->GetBinContent(binMax);     //Q0
                     QQ[2] = hh->GetBinContent(binMax + 1); //Q+
                     Double_t ww = GetStripWidth();         //width of a pad in cm

                     Double_t a2 = 0.5 * (TMath::Sqrt(QQ[1] / QQ[2]) + TMath::Sqrt(QQ[1] / QQ[0]));
                     Double_t a1 = (TMath::Sqrt(QQ[1] / QQ[2]) - TMath::Sqrt(QQ[1] / QQ[0])) / (2.*TMath::SinH(a2));
                     Double_t delta = ww / 2. * TMath::Log((1 + a1) / (1 - a1)) / (TMath::Log(a2 + TMath::Sqrt(a2 * a2 - 1)));

                     fRawPosX [ c ]  = delta;
                     fERawPosX[ c ]  = 0.; //to be modified

                     delete[] QQ;
                  }
                  else {   //Meand and RMS method
                     // The two cathode plans are offset by half a strip to
                     // reduce the non linearity of the position measurement
                     // in between the strips. The center of the detector
                     // is at the middle of the strip 32 of the range 2
                     // and between strips 32 and 33 of the range 1.
                     hh->GetXaxis()->SetRange(min, max);

                     Double_t intgl  = hh->Integral();
                     sum            += intgl;

                     fPadMax[ c - 1 ]  = hh->GetBinCenter(binMax);
                     fRawPosX [ c ]  = hh->GetMean() - (c % 2 ? 32.5 + fOffsetX[0] : 32 + fOffsetX[1]);
                     fERawPosX[ c ]  = hh->GetMeanError();
                     fRawPosX [ 0 ] += intgl * fRawPosX [ c ];
                     fERawPosX[ 0 ] += intgl * fERawPosX[ c ];

                     hh->GetXaxis()->SetRange();

                     if (sum) {
                        fRawPosX [ 0 ] /= sum;
                        fERawPosX[ 0 ] /= sum;
                     }
                     else {
                        fRawPosX [ 0 ] = -666;
                        fERawPosX[ 0 ] = -1;
                     }
                  }
               }
               return fRawPosX[ num ];
            }

            //If X raw position set by hand
            else return fRawPosX[ num ];

         }//end case 0


      case 1: { // for Y direction returns the acq parameter of TFIL

            //If raw Y position was not set by hand
            if (!IsRawPosSetByHand()) {
               fRawPosY  = (fTfilPar && fTfilPar->Fired("P") ? fTfilPar->GetData() : -666);
               fERawPosY = 0.;

               return fRawPosY;
            }

            //If raw Y position was set by hand
            else return fRawPosY;

         }//end case 1
   }

   return -1;
}
//________________________________________________________________

Double_t KVDriftChamber::GetRawPositionError(Char_t dir, Int_t num)
{
   // Returns the error on the position (strip for X and channel for Y) returned by GetRawPosition( dir, num ).

   Int_t idx = IDX(dir);

   switch (idx) {
      case 0: // X

         if (num < 0 || num > 2) num = 0;
         if (fERawPosX[ num ] > -500) return fERawPosX[ num ];
         GetRawPosition(dir, num);
         return fERawPosX[ num ];

      case 1: // Y

         if (fERawPosY > -500) return fERawPosY;
         GetRawPosition(dir, num);
         return fERawPosY;

   }

   return -1;
}
//________________________________________________________________

UChar_t KVDriftChamber::GetPosition(Double_t* XYZf, Char_t dir, Int_t num)
{
   // Get calibrated and deviation-corrected positions Xf, Yf and Zf (in cm)
   // in the focal plane reference frame from the raw positions in channel
   // obtained with GetRawPosition(...). The argument 'XYZf' has to be an
   // array of size 3 i.e. XYZf[3].
   // The 3 first bits of the returned UChar_t value give an information about
   // the coordinates well determined, calibrated and corrected. For example is the Y
   // coordinate, with the indice 1, is good then the bit 1 is set to 1.
   //
   // The two cathode plans are offset by half a strip to reduce the non linearity of the position measurement
   // in between the strips. The center of the detector is by default at the middle of the strip 32 of the
   // range 2 and between strips 32 and 33 of the range 1.
   //
   // The way X position is calculated depends on the method used for raw position.
   //
   // If the Hyperblic Secant Squared (SECHS) method was used (by default), then
   // the bin with the maximum charge, the strip width and the raw position
   // (in cm = disparity with respect to the pad center) are used to fine the position.
   // The offsets are taken into account.
   //
   // Else if the Mean and RMS method was used then we compute the real distance using
   // the strip width and the found raw position.

   // Nothing is done if there is no calibrator for the position
   if (!IsPositionCalibrated()) return 0;

   // num will contain info about dir and num
   //  X1 --> num=1
   //  X2 --> num=2
   //  else num = 0
   num = (dir == 'X' && (num == 1 || num == 2) ? num : 0);

   UChar_t rvalue = 0;       // returned value;

   Double_t Xraw = GetRawPosition('X', num);
   Double_t Yraw = GetRawPosition('Y');

   if (Xraw >= -500) { // Calibrate X
      if (IsSECHSReconstructionX()) {
         Int_t pad = GetPadMax(num);
         if (num == 1) XYZf[0] = (pad - 32.5) * GetStripWidth() + Xraw + fOffsetX[0];
         if (num == 2) XYZf[0] = (pad - 32.) * GetStripWidth()  + Xraw + fOffsetX[1];
      }
      else {
         XYZf[0] = Xraw * GetStripWidth();
      }
      rvalue += 1;
   }
   if ((Yraw >= -500) && GetDriftTimeCalibrator()) { //Calibrate Y
      XYZf[1] = GetDriftTimeCalibrator()->Compute(Yraw) * GetDriftVelocity() / 1000.; // the velocity is in cm/um
      rvalue += 2;
   }

   // Set the Z offset
   XYZf[2] = fOffsetZ[ num ];

   // Set the coordinate in the focal plane frame of reference
   if (rvalue &&  ActiveVolumeToFocal(XYZf, XYZf)) {
      if (!(rvalue & 1)) XYZf[0] = -666;
      else if (!(rvalue & 2)) XYZf[1] = -666;
      else rvalue += 4;
      return rvalue;
   }

   // No coordinates are OK
   XYZf[0] = XYZf[1] = XYZf[2] = -666;
   return 0;
}
//________________________________________________________________

void KVDriftChamber::GetDeltaXYZf(Double_t* DXYZf, Char_t /* dir */, Int_t num)
{
   // Returns in the DXYZf array the errors (in cm) of each coordinate of the position returned by
   // GetPosition(...) in the focal-plane frame of reference.
   //
   // The Y error is fixed to 1mm (resolution given in NIM A 593 (2008) 343).
   // For the drift chamber the error of Z is unkown and fixed to 0.

   if (num < 0 || num > 2) num = 0;

   UChar_t res = GetRawPositionError(DXYZf);
   DXYZf[0] = (res & 1 ?  fERawPosX[num] * GetStripWidth() : -1);
   DXYZf[1] = 0.1;
   DXYZf[2] = 0;
}
