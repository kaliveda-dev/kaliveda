#include "KVCalibrator.h"
#include "Riostream.h"
using namespace std;

ClassImp(KVCalibrator)

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
