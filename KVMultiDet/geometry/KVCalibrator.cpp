/***************************************************************************
$Id: KVCalibrator.cpp,v 1.20 2007/02/27 11:57:58 franklan Exp $
                         kvcalibrator.cpp  -  description
                             -------------------
    begin                : mer sep 18 2002
    copyright            : (C) 2002 by Alexis Mignon
    email                : mignon@ganil.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVCalibrator.h"
#include "Riostream.h"

using namespace std;

ClassImp(KVCalibrator);
//////////////////////////////////////////////////////////////////////////////
//              ********    Base class for calibrating detectors.  ********
//
//Each KVCalibrator has a set of parameters (coefficients),
//possibly read from a file at initialisation time.
//The actual functional form of the calibration formula using these
//coefficients is defined in the child classes of KVCalibrator via the
//following methods :
//              Double_t Compute(Double_t x,....)
//           ou Double_t operator()(Double_t x,....)
//
//////////////////////////////////////////////////////////////////////////////

//
void KVCalibrator::init()
{
   //default initialisations

   fParamNumber = 0;
   fPar = 0;
   fReady = 0;
   fDetector = 0;
}

//___________________________________________________________________________
KVCalibrator::KVCalibrator(): KVBase("Calibrator", "KVCalibrator")
{
   init();
}

//___________________________________________________________________________
KVCalibrator::KVCalibrator(UShort_t pnum): KVBase("Calibrator",
         "KVCalibrator")
{
   //Create a generic calibration object with pnum coefficients in its formula
   init();
   fParamNumber = pnum;
   fPar = new Double_t[pnum];
   for (UShort_t i = 0; i < pnum; i++) {
      fPar[i] = 0;
   }
}

//___________________________________________________________________________
KVCalibrator::KVCalibrator(const Char_t* name, const Char_t* type,
                           UShort_t pnum): KVBase(name, type)
{
   //Create a generic calibration object with pnum coefficients in its formula
   init();
   fParamNumber = pnum;
   fPar = new Double_t[pnum];
   for (UShort_t i = 0; i < pnum; i++) {
      fPar[i] = 0;
   }
}

//___________________________________________________________________________
KVCalibrator::~KVCalibrator()
{
   if (fPar) {
      delete[]fPar;
      fPar = 0;
   }
}

//
KVCalibrator::KVCalibrator(const KVCalibrator& obj) : KVBase()
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVCalibrator&) obj).Copy(*this);
#endif
}

//___________________________________________________________________________
void KVCalibrator::Print(Option_t*) const
{
   //Print a description of the calibration object, including a list of its parameters
   cout << "_________________________________________________" << endl
        << "KVCalibrator :" << endl
        << "  Name : " << GetName() << endl
        << "  Type : " << GetType() << endl
        << "  Number of Parameters : " << GetNumberParams() << endl
        << "  Parameters :" << endl;
   for (UShort_t i = 0; i < GetNumberParams(); i++) {
      cout << "    " << GetParameter(i) << endl;
   }
   if (GetStatus())
      cout << "  Status : ready" << endl;
   else
      cout << "  Status : not ready" << endl;
}

//____________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVCalibrator::Copy(TObject& obj) const
#else
void KVCalibrator::Copy(TObject& obj)
#endif
{
   //
   //Copy this to obj
   //
   KVBase::Copy(obj);
   ((KVCalibrator&) obj).SetDetector(GetDetector());
   ((KVCalibrator&) obj).SetNumberParams(GetNumberParams());
   if (GetNumberParams()) {
      for (UShort_t i = 0; i < GetNumberParams(); i++) {
         ((KVCalibrator&) obj).SetParameter(i, GetParameter(i));
      }
   }
   ((KVCalibrator&) obj).SetStatus(GetStatus());
}

//____________________________________________________________________________
void KVCalibrator::SetNumberParams(Int_t npar)
{
   //
   //Set/reset number of parameters in calibration formula
   //
   fParamNumber = npar;
   if (fPar)
      delete[]fPar;
   fPar = new Double_t[fParamNumber];
   for (UShort_t i = 0; i < fParamNumber; i++)
      fPar[i] = 0;
}

//____________________________________________________________________________

void KVCalibrator::SetDetector(KVDetector* kvd)
{
   //The name of the calibration object is set to the full name of the detector,
   //i.e. TYPE_RINGMODULE
   fDetector = kvd;
   SetNameFromDetector();
}

//____________________________________________________________________________
void KVCalibrator::SetParameters(Double_t val, ...)
{
   //Set the fParamNumber parameters of the calibrator.
   //The number of arguments must be exactly fParamNumber
   //(which must previously have been set)
   //Once the parameters have been set with this method, the status of the
   //calibrator becomes 'OK' or 'Ready'

   va_list ap;
   va_start(ap, val);

   int arg_n = 0;
   SetParameter(arg_n++, val);
   while (arg_n < fParamNumber) {
      SetParameter(arg_n, (Double_t) va_arg(ap, Double_t));
      arg_n++;
   }

   va_end(ap);
   SetStatus(kTRUE);
}

//____________________________________________________________________________
void KVCalibrator::SetNameFromDetector()
{
   if (!GetDetector()) {
      Warning("SetNameFromDetector()", "No detector set !");
      return;
   }
   SetName(GetDetector()->GetName());
}

void KVCalibrator::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVCalibrator.
   //customised in case no parameters are initialised (fPar null pointer)

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v) {
      }
      KVBase::Streamer(R__b);
      R__b >> fParamNumber;
      if (fPar) {
         delete[]fPar;
         fPar = 0;
      }
      if (fParamNumber) {
         fPar = new Double_t[fParamNumber];
         R__b.ReadFastArray(fPar, fParamNumber);
      }
      R__b >> fDetector;
      R__b >> fReady;
      R__b.CheckByteCount(R__s, R__c, KVCalibrator::IsA());
   } else {
      R__c = R__b.WriteVersion(KVCalibrator::IsA(), kTRUE);
      KVBase::Streamer(R__b);
      R__b << fParamNumber;
      if (fParamNumber) {
         R__b.WriteFastArray(fPar, fParamNumber);
      }
      R__b << fDetector;
      R__b << fReady;
      R__b.SetByteCount(R__c, kTRUE);
   }
}

TGraph* KVCalibrator::MakeGraph(Double_t xmin, Double_t xmax,
                                Int_t npoints) const
{
   //Creates a TGraph with npoints points (default 50 points) showing the calibration
   //formula between input values xmin and xmax (npoints equally-spaced points).
   //User should delete the TGraph after use.
   TGraph* tmp = 0;
   if (GetStatus()) {           // check calibrator is ready
      if (GetNumberParams()) {  // check calibrator is ready
         if (npoints > 1) {
            Double_t dx = (xmax - xmin) / ((Double_t) npoints - 1.0);
            if (dx) {
               Double_t* xval = new Double_t[npoints];
               Double_t* yval = new Double_t[npoints];
               for (register int i = 0; i < npoints; i++) {
                  xval[i] = xmin + dx * ((Double_t) i);
                  yval[i] = Compute(xval[i]);
               }
               tmp = new TGraph(npoints, xval, yval);
               delete[]xval;
               delete[]yval;
            }
         }
      }
   }
   return tmp;
}
