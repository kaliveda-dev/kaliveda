///
/// @file SiliconEnergyMinimiser.cpp
///
/// @section Description
///
/// Wrapper class for the silicon energy minimisation routines. This class
/// allows the underlying implementation to be changed easily and without
/// affecting any existing code.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Sun 17 Jan 20:35:37  2016
///

#include "SiliconEnergyMinimiser.h"

ClassImp(SiliconEnergyMinimiser)

SiliconEnergyMinimiser::SiliconEnergyMinimiser() :
#if __cplusplus < 201103L
   impl_(NULL),
#else
   impl_(nullptr),
#endif
   kInitialised_(kFALSE)
{

}

SiliconEnergyMinimiser::~SiliconEnergyMinimiser()
{
#if __cplusplus < 201103L
   if (impl_) {
      delete impl_;
      impl_ = NULL;
   }
#endif
}

void SiliconEnergyMinimiser::Init()
{
   if (kInitialised_) return;

#if __cplusplus < 201103L
   impl_ = new MonoMinimiserImpl();
#else
   impl_.reset(new MonoMinimiserImpl());
#endif

   impl_->Init();
   kInitialised_ = kTRUE;

   return;
}

Bool_t SiliconEnergyMinimiser::SetIDTelescope(const TString& telescope_name)
{
   if (!kInitialised_) {
      Error("SiliconEnergyMinimiser::SetIDTelescope",
            "You need to call SiliconEnergyMinimiser::Init()");
      return kFALSE;
   }

   return impl_->SetIDTelescope(telescope_name);
}

Int_t SiliconEnergyMinimiser::Minimise(
   UInt_t z_value, Double_t si_energy, Double_t csi_light
)
{
   return impl_->Minimise(z_value, si_energy, csi_light);
}

void SiliconEnergyMinimiser::SetMaximumIterations(UInt_t max_iterations)
{
   impl_->SetMaximumIterations(max_iterations);
}

void SiliconEnergyMinimiser::SetTolerance(Double_t tolerance)
{
   impl_->SetTolerance(tolerance);
}

