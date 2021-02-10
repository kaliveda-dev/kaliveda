#include "KVCalibrator.h"
#include "Riostream.h"
using namespace std;

ClassImp(KVCalibrator)

void KVCalibrator::adjust_range_of_inverse_calibration()
{
   // For an inverse calibration, the limits [min,max] given in the options concern the Y-values,
   // i.e. the values Y=f(X) of the function. We need to find the corresponding values of X in order
   // to fix the range of the function, and avoid use of TF1::GetX() leading to many error messages.

   Double_t fYmin, fYmax, fXmin, fXmax;
   // Initial range corresponds to values given as [min,max] options
   fCalibFunc->GetRange(fYmin, fYmax);
   fXmin = fYmin;
   fXmax = fYmax;
   // check if function increases or decreases with X
   Bool_t fMonoIncreasing = true;
   Double_t fFuncMax = fCalibFunc->Eval(fXmax);
   Double_t fFuncMin = fCalibFunc->Eval(fXmin);
   if (fFuncMin > fFuncMax) {
      fMonoIncreasing = false;
      std::swap(fFuncMin, fFuncMax);
   }
   // For all to be well, fYmin & fYmax need to be within the range of values of the function
   if (!in_range(fYmin, fFuncMin, fFuncMax) || !in_range(fYmax, fFuncMin, fFuncMax)) {
      Double_t Range = fXmax - fXmin;
      if (!in_range(fYmin, fFuncMin, fFuncMax)) {
         if (fYmin < fFuncMin) {
            // need to reduce minimum value of function until it is less than fYmin
            if (fMonoIncreasing) {
               // need to decrease fXmin until fYmin>fFuncMin
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmin);
               }
               while (fYmin < fFuncMin);
            }
            else {
               // need to increase fXmax until fYmin>fFuncMin
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmax);
               }
               while (fYmin < fFuncMin);
            }
         }
         else if (fYmin > fFuncMax) {
            // need to increase maximum value of function until it is greater than fYmin
            if (fMonoIncreasing) {
               // need to increase fXmax until fYmin<fFuncMax
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmax);
               }
               while (fYmin > fFuncMax);
            }
            else {
               // need to decrease fXmin until fYmin<fFuncMax
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmin);
               }
               while (fYmin > fFuncMax);
            }
         }
      }
      if (!in_range(fYmax, fFuncMin, fFuncMax)) {
         if (fYmax < fFuncMin) {
            // need to reduce minimum value of function until it is less than fYmax
            if (fMonoIncreasing) {
               // need to decrease fXmin until fYmax>fFuncMin
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmin);
               }
               while (fYmax < fFuncMin);
            }
            else {
               // need to increase fXmax until fYmax>fFuncMin
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMin = fCalibFunc->Eval(fXmax);
               }
               while (fYmax < fFuncMin);
            }
         }
         else if (fYmax > fFuncMax) {
            // need to increase maximum value of function until it is greater than fYmax
            if (fMonoIncreasing) {
               // need to increase fXmax until fYmax<fFuncMax
               do {
                  fXmax += Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmax);
               }
               while (fYmax > fFuncMax);
            }
            else {
               // need to decrease fXmin until fYmax<fFuncMax
               do {
                  fXmin -= Range;
                  Range = fXmax - fXmin;
                  fFuncMax = fCalibFunc->Eval(fXmin);
               }
               while (fYmax > fFuncMax);
            }
         }
      }
      // adjust range of function to new values
      fCalibFunc->SetRange(fXmin, fXmax);
   }
   // at this point fYmin and fYmax are certainly within the range of the function values
   if (fMonoIncreasing) {
      fXmin = fCalibFunc->GetX(fYmin);
      fXmax = fCalibFunc->GetX(fYmax);
   }
   else {
      fXmin = fCalibFunc->GetX(fYmax);
      fXmax = fCalibFunc->GetX(fYmin);
   }
   // set range of function so that [min,max] values correspond to those given in option
   fCalibFunc->SetRange(fXmin, fXmax);
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
   // CalibOptions:   func=[function],min=[minimum of X],max=[maximum of X]
   //~~~~~~~~~~~~~~
   //
   // An optional option is `inverse=true` which will call KVCalibrator::SetUseInverseFunction(true)
   //
   // In this case we need to calculate the range of the function in order to have [ymin,ymax]
   // corresponding to the values [min,max]. This will be done when SetStatus() is called with argument
   // 'true' (we suppose that this is only done once all parameters have been set and the calibrator
   // is ready for use). See adjust_range_of_inverse_function().

   fCalibFunc = new TF1("KVCalibrator::fCalibFunc", opt.GetStringValue("func"), opt.GetTStringValue("min").Atof(), opt.GetTStringValue("max").Atof());
   SetUseInverseFunction(opt.IsValue("inverse", "true"));
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
