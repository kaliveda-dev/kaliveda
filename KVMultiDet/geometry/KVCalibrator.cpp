#include "KVCalibrator.h"
#include "Riostream.h"
using namespace std;

ClassImp(KVCalibrator)

void KVCalibrator::adjust_range_of_inverse_calibration()
{
   // For an inverse calibration, the limits [min,max] given in the options concern the Y-values,
   // i.e. the values Y=f(X) of the function which correspond to the input signal.
   // We need to find the corresponding values of X in order
   // to fix the range of the function, and avoid use of TF1::GetX() leading to many error messages.

   if (fInputMax < fInputMin) return; // no sense trying if this is true

   Double_t fXmin, fXmax;
   // Initial range corresponds to estimated output range given as [out_min,out_max] options
   fCalibFunc->GetRange(fXmin, fXmax);
   // check if function increases or decreases with X
   Bool_t fMonoIncreasing = true;
   Double_t fFuncMax = fCalibFunc->Eval(fXmax);
   Double_t fFuncMin = fCalibFunc->Eval(fXmin);
   if (fFuncMin > fFuncMax) {
      fMonoIncreasing = false;
      std::swap(fFuncMin, fFuncMax);
   }
   // For all to be well, fInputMin & fInputMax need to be within the range of values of the function
   if (!in_range(fInputMin, fFuncMin, fFuncMax) || !in_range(fInputMax, fFuncMin, fFuncMax)) {
      Double_t Range = fXmax - fXmin;
      if (!in_range(fInputMin, fFuncMin, fFuncMax)) {
         if (fInputMin < fFuncMin) {
            // need to reduce minimum value of function until it is less than fInputMin
            if (fMonoIncreasing) {
               // need to decrease fXmin until fInputMin>fFuncMin
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmin);
               }
               while (fInputMin < fFuncMin);
            }
            else {
               // need to increase fXmax until fInputMin>fFuncMin
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmax);
               }
               while (fInputMin < fFuncMin);
            }
         }
         else if (fInputMin > fFuncMax) {
            // need to increase maximum value of function until it is greater than fInputMin
            if (fMonoIncreasing) {
               // need to increase fXmax until fInputMin<fFuncMax
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmax);
               }
               while (fInputMin > fFuncMax);
            }
            else {
               // need to decrease fXmin until fInputMin<fFuncMax
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmin);
               }
               while (fInputMin > fFuncMax);
            }
         }
      }
      if (!in_range(fInputMax, fFuncMin, fFuncMax)) {
         if (fInputMax < fFuncMin) {
            // need to reduce minimum value of function until it is less than fInputMax
            if (fMonoIncreasing) {
               // need to decrease fXmin until fInputMax>fFuncMin
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmin);
               }
               while (fInputMax < fFuncMin);
            }
            else {
               // need to increase fXmax until fInputMax>fFuncMin
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmax);
               }
               while (fInputMax < fFuncMin);
            }
         }
         else if (fInputMax > fFuncMax) {
            // need to increase maximum value of function until it is greater than fInputMax
            if (fMonoIncreasing) {
               // need to increase fXmax until fInputMax<fFuncMax
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmax);
               }
               while (fInputMax > fFuncMax);
            }
            else {
               // need to decrease fXmin until fInputMax<fFuncMax
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmin);
               }
               while (fInputMax > fFuncMax);
            }
         }
      }
      // adjust range of function to new values
      fCalibFunc->SetRange(fXmin, fXmax);
   }
   // at this point fInputMin and fInputMax are certainly within the range of the function values
   if (fMonoIncreasing) {
      fXmin = fCalibFunc->GetX(fInputMin);
      fXmax = fCalibFunc->GetX(fInputMax);
   }
   else {
      fXmin = fCalibFunc->GetX(fInputMax);
      fXmax = fCalibFunc->GetX(fInputMin);
   }
   // set range of function so that [min,max] values correspond to those given in option
   fCalibFunc->SetRange(fXmin, fXmax);

   Info("adjust_range_of_inverse_function", "After adjustment: %s", GetName());
   Info("adjust_range_of_inverse_function", "After adjustment: %s INPUT-range = [%f, %f]", GetInputSignalType().Data(), fInputMin, fInputMax);
   Info("adjust_range_of_inverse_function", "After adjustment: %s OUTPUT-range = [%f, %f]", GetOutputSignalType().Data(), fXmin, fXmax);
}

void KVCalibrator::Print(Option_t*) const
{
   //Print a description of the calibration object, including a list of its parameters
   cout << "_________________________________________________" << endl
        << ClassName() << " :" << endl
        << "  Name : " << GetName() << endl
        << "  Type : " << GetType() << endl
        << "  Number of Parameters : " << GetNumberParams() << endl
        << "  Parameters :" << endl;
   for (int i = 0; i < GetNumberParams(); i++) {
      cout << "    " << GetParameter(i) << endl;
   }
   if (GetStatus())
      cout << "  Status : ready" << endl;
   else
      cout << "  Status : not ready" << endl;
}

KVCalibrator* KVCalibrator::MakeCalibrator(const Char_t* type)
{
   // Create a new KVCalibrator object with class given by the plugin of given type
   //
   // If type=="" or type is unknown, returns a new base calibrator KVCalibrator

   TPluginHandler* ph = LoadPlugin("KVCalibrator", type);
   if (!ph) {
      return new KVCalibrator;
   }
   // execute default constructor
   KVCalibrator* c = (KVCalibrator*)ph->ExecPlugin(0);
   return c;
}

void KVCalibrator::SetOptions(const KVNameValueList& opt)
{
   // Used to set up a function calibrator from infos in a calibration parameter file.
   // Use an option string like this:
   //
   //~~~~~~~~~~~~~~
   // CalibOptions:   func=[function],min=[minimum input],max=[maximum output]
   //~~~~~~~~~~~~~~
   //
   // This is the standard case where the calibration function is fitted to output signal vs. input signal.
   // In this case `[min,max]` are the expected (allowed) range of input values.
   //
   // If the calibration function is fitted to input signal vs. output signal then we have what we
   // call an 'inverse calibration function' and in this case you should use options like:
   //
   //~~~~~~~~~~~~~~
   // CalibOptions:   func=[function],inverse=true,min=[minimum input],max=[maximum input],out_min=[estimate minimum output],out_max=[estimate maximum output]
   //~~~~~~~~~~~~~~
   //
   // In this case we need to calculate the range of output values we can give to the inverse function
   // in order to have the expected (allowed) range of input values given by `[min,max]`.
   // In order to do this, it is necessary to give a reasonable
   // range for the expected output values, `[out_min,out_max]`. If these parameters are not given,
   // the calibrator will not be valid.
   //
   // Throws a std::invalid_argument exception if any of the required options are missing

   if (!opt.HasStringParameter("func") && !opt.HasNumericParameter("min") && !opt.HasNumericParameter("max")) {
      throw (std::invalid_argument("missing calibrator options: must give func, min, and max"));
   }
   SetUseInverseFunction(opt.IsValue("inverse", "true"));
   if (IsUseInverseFunction()) {
      if (!opt.HasNumericParameter("out_min") || !opt.HasNumericParameter("out_max")) {
         throw (std::invalid_argument("missing inverse calibrator options: must give out_min and out_max"));
      }
      fCalibFunc = new TF1("KVCalibrator::fCalibFunc", opt.GetStringValue("func"), opt.GetDoubleValue("out_min"), opt.GetDoubleValue("out_max"));
      fInputMin = opt.GetDoubleValue("min");
      fInputMax = opt.GetDoubleValue("max");
   }
   else
      fCalibFunc = new TF1("KVCalibrator::fCalibFunc", opt.GetStringValue("func"), opt.GetDoubleValue("min"), opt.GetDoubleValue("max"));
}

//TGraph* KVCalibrator::MakeGraph(Double_t xmin, Double_t xmax,
//                                Int_t npoints) const
//{
//   //Creates a TGraph with npoints points (default 50 points) showing the calibration
//   //formula between input values xmin and xmax (npoints equally-spaced points).
//   //User should delete the TGraph after use.
//   TGraph* tmp = 0;
//   if (GetStatus()) {           // check calibrator is ready
//      if (GetNumberParams()) {  // check calibrator is ready
//         if (npoints > 1) {
//            Double_t dx = (xmax - xmin) / ((Double_t) npoints - 1.0);
//            if (dx) {
//               Double_t* xval = new Double_t[npoints];
//               Double_t* yval = new Double_t[npoints];
//               for (int i = 0; i < npoints; i++) {
//                  xval[i] = xmin + dx * ((Double_t) i);
//                  yval[i] = Compute(xval[i]);
//               }
//               tmp = new TGraph(npoints, xval, yval);
//               delete[]xval;
//               delete[]yval;
//            }
//         }
//      }
//   }
//   return tmp;
//}
