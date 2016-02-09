
///
/// @file ThreadedMinimiserImpl.h
///
/// @section Description
///
/// Threaded implementation of the silicon energy minimiser. This class
/// implements the minimisation using the ThreadedMassEstimator.
///
/// @warning This implementation does not include a tolerance or a consistent
/// maximisation check (maximisation is declared as soon as delta increases,
/// i.e. we do not wait to see if delta is maximising consistently). It is
/// recommended that you use the standard implementation (MonoMinimiserImpl) if
/// you can as it does not require pthreads, is less complex and probably does
/// the job well enough. If not then you are free to implement the
/// aforementioned changes :)
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Sun 17 Jan 20:40:12  2016
///

#ifndef __THREADED_MINIMISER_IMPL_H__
#define __THREADED_MINIMISER_IMPL_H__

#if __cplusplus >= 201103L
#include <memory>
#endif

#include "ThreadedMassEstimator.h"

class ThreadedMinimiserImpl {

public:

   /// @brief Default constructor
   ThreadedMinimiserImpl();
   /// @brief Default destructor
   virtual ~ThreadedMinimiserImpl();

   /// @brief Initialisation function (to be called post-construction)
   void Init();

   /// @brief Multi threaded minimisation implementation
   ///
   /// We use two threads operating from the same starting point (A=2Z) but
   /// working in opposite directions. This is essentially the multi-threaded
   /// version of the standard MonoMinimiserImpl minimiser implementation.
   ///
   /// @param z_value Measured Z value
   /// @param si_energy Measured silicon energy (MeV)
   /// @param csi_light Measured raw caesium iodide light (Channel)
   ///
   /// @warning This implementation does not include a tolerance or a
   /// consistent maximisation check (maximisation is declared as soon as delta
   /// increases, i.e. we do not wait to see if delta is maximising
   /// consistently). It is recommended that you use the standard
   /// implementation (MonoMinimiserImpl) if you can as it does not require
   /// pthreads, is less complex and probably does the job well enough. If not
   /// then you are free to implement the aforementioned changes :)
   ///
   /// @see SiliconEnergyMinimiser::Minimise
   /// @see ThreadedMassEstimator
   Int_t Minimise(UInt_t z_value, Double_t si_energy,
                  Double_t csi_light);

   /// @brief Set the identification telescope
   /// @param telescope_name As returned by KVIDTelescope::GetName()
   Bool_t SetIDTelescope(const TString& telescope_name);

   /// @brief Set maximum number of iterations per thread
   void SetMaximumIterations(UInt_t max_iterations);

   /// @brief Print minimiser implementation information
   void Print() const;

private:

   /// @brief Copy constructor (Declared private to prevent copying as it is
   /// not worth implementing yet).
   ThreadedMinimiserImpl(const ThreadedMinimiserImpl& rhs);
   /// @brief Assignment operator (Declared private to prevent copying as it is
   /// not worth implementing yet).
   ThreadedMinimiserImpl& operator=(const ThreadedMinimiserImpl& rhs);

   /// @brief Initialisation status
   Bool_t kInitialised_;

   /// @brief Minimisation routine error codes
   enum ThreadErrorCodes {
      kNotInitialised = -1,
      kGetPossibleAFailed = -2,
      kEstimateAFailed = -3,
      kEstimateAErrors = -4
   };

#if __cplusplus < 201103L
   /// @brief struct to contain the ThreadedMassEstimator input data
   struct me::EstimatorInput* estimator_input_;
   /// @brief vector to contain list of possible A values
   std::vector<Int_t>* possible_a_values_;

   /// @brief struct to retrieve the ThreadedMassEstimator output data
   struct me::EstimatorResult* estimator_result_;
   /// @brief ThreadedMassEstimator (Minimiser)
   ThreadedMassEstimator* mass_estimator_;
#else
   /// @brief struct to contain the ThreadedMassEstimator input data
   std::unique_ptr<struct me::EstimatorInput> estimator_input_;
   /// @brief vector to contain list of possible A values
   std::unique_ptr<std::vector<Int_t> > possible_a_values_;
   /// @brief struct to retrieve the ThreadedMassEstimator output data
   std::unique_ptr<struct me::EstimatorResult> estimator_result_;
   /// @brief ThreadedMassEstimator (Minimiser)
   std::unique_ptr<ThreadedMassEstimator> mass_estimator_;
#endif

   ClassDef(ThreadedMinimiserImpl, 1)
};

#endif
