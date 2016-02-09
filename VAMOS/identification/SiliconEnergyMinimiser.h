///
/// @file SiliconEnergyMinimiser.h
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

#ifndef __SILICON_ENERGY_MINIMISER_H__
#define __SILICON_ENERGY_MINIMISER_H__

#include <cassert>
#include <vector>

#include "Rtypes.h"
#include "TString.h"

//
// To change the implementation, for example to use the threaded minimiser
// implementation, do the following:
//
// 1. Change the #include lines below to include the correct implementation
// header file.
//
// 2. Anywhere in this source file you see "MonoMinimiserImpl" replace with the
// "ThreadedMinimiserImpl" class name.
//
// 3. Recompile and you're done :)
//

//#include "ThreadedMinimiserImpl.h"
#include "MonoMinimiserImpl.h"

class SiliconEnergyMinimiser {

   /// @brief Minimiser implementation
#if __cplusplus < 201103L
   MonoMinimiserImpl* impl_;
#else
   std::unique_ptr<MonoMinimiserImpl> impl_;
#endif

public:

   /// @brief Default constructor
   SiliconEnergyMinimiser();
   /// @brief Default destructor
   virtual ~SiliconEnergyMinimiser();

   /// @brief Initialisation function (to be called post-construction)
   void Init();

   /// @brief Set the identification telescope
   /// @param telescope_name Name as returned by KVIDTelescope::GetName()
   Bool_t SetIDTelescope(const TString& telescope_name);

   /// @brief Estimate the A value of a nucleus by minimising the difference
   /// between measured and simulated silicon energies in VAMOS. Runs the
   /// underlying implementation (either MonoMinimiserImpl::Minimise or
   /// ThreadedMinimiserImpl::Minimise) which is compile-time dependent.
   /// MonoMinimiserImpl is the default.
   ///
   /// @param z_value Measured Z value
   /// @param si_energy Measured silicon energy (MeV)
   /// @param csi_light Measured raw caesium iodide light (Channel)
   ///
   /// @see MonoMinimiserImpl::Minimise
   /// @see ThreadedMinimiserImpl::Minimise
   Int_t Minimise(UInt_t z_value, Double_t si_energy, Double_t csi_light);

   /// @brief Set the maximum number of iterations allowed
   /// @param max_iterations Maximum number of iterations
   void SetMaximumIterations(UInt_t max_iterations);

   /// @brief Set the algorithm tolerance
   /// @param tolerance algorithm tolerance (if applicable)
   void SetTolerance(Double_t tolerance = 0.05);

private:

   /// @brief Copy constructor (Declared private to prevent copying as it is
   /// not worth implementing yet).
   SiliconEnergyMinimiser(const SiliconEnergyMinimiser&);
   /// @brief Assignment operator (Declared private to prevent copying as it is
   /// not worth implementing yet).
   SiliconEnergyMinimiser& operator=(const SiliconEnergyMinimiser&);

   /// @brief Initialisation status
   Bool_t kInitialised_;

   ClassDef(SiliconEnergyMinimiser, 1)
};

#endif
