//Created by KVClassFactory on Wed Feb 13 10:18:35 2013
//Author: Guilain ADEMARD

#include "KVSeDPositionCal.h"

ClassImp(KVSeDPositionCal)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSeDPositionCal</h2>
<h4>Position calibration for SeD detectors of VAMOS</h4>
<!-- */
// --> END_HTML
//
// Calibration object dedicated to handle the position calibration
// (conversion position in channel -> cm) and to correct the deviation of
// the position in SeD detectors of VAMOS. For this purpose, an aluminium
// mask with calibrated holes of 5mm diameter is put in front of the emissive
// foil. The positions are corrected with a bidimensional polynomial of
// order 4 (method of P. Napolitani and S. Leblond) in two rounds. Let X and Y
// be the raw values (in channel) of positions. The calibration process is the
// following:
// - First round
// Begin_Latex X_{1} = #sum_{i=0}^{4} #sum_{j=0}^{4} C_{X1;i,j} X^{i}Y^{j} End_Latex Begin_Latex  Y_{1} = #sum_{i=0}^{4} #sum_{j=0}^{4} C_{Y1;i,j} X_{1}^{j}Y^{i} End_Latex
// - Second round
// Begin_Latex X_{2} = #sum_{i=0}^{4} #sum_{j=0}^{4} C_{X2;i,j} X_{1}^{i}Y_{1}^{j} End_Latex Begin_Latex Y_{2} = #sum_{i=0}^{4} #sum_{j=0}^{4} C_{Y2;i,j} X_{2}^{j}Y_{1}^{i} End_Latex
// where Begin_Latex X_{2} End_Latex and Begin_Latex Y_{2} End_Latex are the calibrated positions obtained at the end
// of the process. This calibrator has then 100 parameters:
// - parameters 0 to 24 correspond to  Begin_Latex C_{X1;i,j} End_Latex
// - parameters 25 to 49 correspond to Begin_Latex C_{Y1;i,j} End_Latex
// - parameters 50 to 74 correspond to Begin_Latex C_{X2;i,j} End_Latex
// - parameters 75 to 99 correspond to Begin_Latex C_{Y2;i,j} End_Latex
//
// The type of the calibrator is "position->cm Detector_name" if the detector
// is set in the constructor or "position->cm" if not.
//
// Calibrator used for the INDRA_VAMOS experiment e494s.
////////////////////////////////////////////////////////////////////////////////

KVSeDPositionCal::KVSeDPositionCal() : KVCalibrator(NPAR_SEDPOSCAL)
{
   // Default constructor
   SetType("position->cm");
   SetLabel("XY");
}
//________________________________________________________________

KVSeDPositionCal::KVSeDPositionCal(KVDetector* det) : KVCalibrator(NPAR_SEDPOSCAL)
{
   //Create a position calibration object for a specific detector (*det).
   //The calibrator takes the name of the detector.
   SetDetector(det);
   SetName(det->GetName());
   SetType(Form("position->cm %s", GetName()));
   SetLabel("XY");
}
//________________________________________________________________

KVSeDPositionCal::~KVSeDPositionCal()
{
   // Destructor
}
//________________________________________________________________

Double_t KVSeDPositionCal::Compute(Double_t chan) const
{
   // Obsolete method, use Compute(Double_t X, Double_t Y, Double_t &Xcal, Double_t &Ycal).
   IGNORE_UNUSED(chan);
   Warning("Compute(Double_t chan)", "Obsolete method, use Compute(Double_t X, Double_t Y, Double_t &Xcal, Double_t &Ycal)");
   return -1;
}
//________________________________________________________________

Double_t KVSeDPositionCal::operator()(Double_t chan)
{
   // Obsolete method, use operator()(Double_t X, Double_t Y, Double_t &Xcal, Double_t &Ycal).
   IGNORE_UNUSED(chan);
   Warning("operator()(Double_t chan)", "Obsolete method, use operator()(Double_t X, Double_t Y, Double_t &Xcal, Double_t &Ycal)");
   return -1;
}
//________________________________________________________________

Double_t KVSeDPositionCal::Invert(Double_t volts)
{
   //Obsolete method, too much difficult to invert this calibrator!
   IGNORE_UNUSED(volts);
   Warning("Invert(Double_t volts)", "Obsolete method, too much difficult to invert this calibrator!");
   return -1;
}
//________________________________________________________________

Bool_t KVSeDPositionCal::Compute(Double_t X, Double_t Y, Double_t& Xcal, Double_t& Ycal)
{
   //Calculate calibrated X and Y position (Xcal, Ycal) in cm for the given raw X and Y values.

   Double_t Xtmp[2];
   Xtmp[0] = X;
   Xtmp[1] = Y;

   Xcal = Ycal = -666;
   if (Xtmp[0] <= 0 || Xtmp[1] <= 0) return kFALSE;

   UShort_t num = 0;
   for (Int_t h = 0; h < 2; h++) { // calibration process performed two times
      for (Int_t i = 0; i < 2; i++) { // Correction in X and in Y

         // calibration on the SeD# plane in cm
         Double_t cor   = Xtmp[i];
         Double_t other = Xtmp[ i ? 0 : 1 ];
         Xtmp[i] = 0.;
         for (Int_t j = 0; j <= SEDPOSCAL_FIT_ORDER; j++) {
            for (Int_t k = 0; k <= SEDPOSCAL_FIT_ORDER; k++) {
               if (GetParameter(num) != 0)
                  Xtmp[i] += GetParameter(num) * TMath::Power(cor, j) * TMath::Power(other, k);
               num++;
            }
         }
      }
   }

   Xcal = Xtmp[0];
   Ycal = Xtmp[1];

   return kTRUE;
}
//________________________________________________________________

Bool_t KVSeDPositionCal::operator()(Double_t X, Double_t Y, Double_t& Xcal, Double_t& Ycal)
{
   // Overloading of "()" to allow syntax such as:
   //
   // KVSeDPositionCal calibrator;
   // Double_t Xcal, Ycal;
   // Bool_t isOK = calibrator(X,Y,Xcal,Ycal);
   //
   // equivalently to:
   //
   // Bool_t isOK = calibrator.Compute(X,Y,Xcal,Ycal);
   return Compute(X, Y, Xcal, Ycal);
}
