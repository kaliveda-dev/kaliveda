//Created by KVClassFactory on Thu Dec  3 15:59:09 2015
//Author: bonnet,,,

#include "KVFAZIACalibrator.h"

ClassImp(KVFAZIACalibrator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIACalibrator</h2>
<h4>Basic class for FAZIA calibrators</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIACalibrator::KVFAZIACalibrator()
{
   // Default constructor
}

KVFAZIACalibrator::KVFAZIACalibrator(const Char_t* name, const Char_t* type, UShort_t pnum) : KVCalibrator(name, type, pnum)
{
   // Default constructor
}

//___________________________________________________________________________
KVFAZIACalibrator::~KVFAZIACalibrator()
{
   // Destructor
}

//___________________________________________________________________________
Double_t KVFAZIACalibrator::Compute(Double_t chan) const
{
   //Calculate the calibrated energy.
   //Returns -99 if calibrator is not ready (parameters not set)

   if (fReady) {
      if (GetParameter(0) != 0)
         return (chan / GetParameter(0));
   }
   return -99.;
}


//___________________________________________________________________________
Double_t KVFAZIACalibrator::operator()(Double_t chan)
{
   //Overloading of "()" to allow syntax such as:
   //
   //        KVFAZIACalibrator calibrator;
   //           ....
   //        Float_t calibrated_energy = calibrator(channel);
   //
   //equivalently to:
   //
   //        Float_t calibrated_energy = calibrator.Compute(channel);
   return Compute(chan);
}

//___________________________________________________________________________
Double_t KVFAZIACalibrator::Invert(Double_t energy)
{
   //Given the calibrated (or simulated) energy in MeV,
   //calculate the corresponding channel number according to the
   //calibration parameters (useful for filtering simulations).

   Int_t channel = 0;

   if (fReady) {
      channel = (Int_t)energy * GetParameter(0);
   }
   return (Double_t) channel;
}
