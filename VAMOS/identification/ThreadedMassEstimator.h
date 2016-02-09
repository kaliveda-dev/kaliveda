
///
/// @file ThreadedMassEstimator.h
///
/// @section Description
///
/// This class estimates the A value of a fragment detected in the
/// Si:Isobutane:CsI detector stack, given the experimentally measured data
/// (Z, Silicon Energy, CsI Light).
///
/// It achieves this by minimising the absolute magnitude of the difference
/// between the experimentally measured silicon energy and the silicon energy
/// simulated in a set of test nuclei (with fixed Z) to determine which nucleus
/// (A value) provides the closest match.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 15:19:33  2016
///

#ifndef __THREADED_MASS_ESTIMATOR__
#define __THREADED_MASS_ESTIMATOR__

// C
#include <cassert>

// CINT can't parse pthread.h
#ifndef __CINT__
#include "pthread.h"
#else
struct pthread_mutex_t;
#endif

// KaliVeda (VAMOS)
#include "MEDetectorStack.h"

/// @brief Mass estimator namespace (me) to prevent polluting the global
/// namespace with our generic naming conventions.
namespace me {

/// @brief Struct used for passing in the estimator inputs as a single
/// argument.
   struct EstimatorInput {
      Int_t z;            ///< Measured Z value
      Double_t si_energy; ///< Measured silicon energy
      Double_t csi_light; ///< Measured caesium iodide light
   };

/// @brief Struct used to store the thread status codes
   struct ThreadStatus {
      static const Int_t kThreadStop;     ///< Stop processing this thread
      static const Int_t kThreadContinue; ///< Continue processing this thread
   };

/// @brief Struct used to pass in data to each thread
///
/// Having a local copy of the possible A values removes the need for high
/// frequency thread locking (mutexes) as we need to access this vector every
/// iteration of each thread. We also get a speed increase as the iterators can
/// simply increment/decrement without jumping around  (as we iterate in
/// different directions in each thread)
   struct ThreadData {

      /// @brief A list of possible A values for the given Z value
      std::vector<Int_t> possible_a_values;

      /// @brief Specifies the iteration direction for the thread. Either
      /// increasing or decreasing A values.
      Bool_t kForwardIterator;

      // Experimental data
      Int_t    z_value_expt;   ///< Measured Z value
      Double_t si_energy_expt; ///< Mesured silicon energy
      Double_t csi_light_expt; ///< Measured caesium iodide light

      // Thread Storage
      Int_t    current_a;      ///< Current estimate for A
      Double_t current_delta;  ///< Current value for delta
      Int_t    minimum_a;      ///< Best estimate for A
      Double_t minimum_delta;  ///< Best value for delta

      /// @brief Struct used to store the results of simulations.
      /// @see MEDetectorStack::Simulate
      /// @see me::SimulationResult
      struct me::SimulationResult sim_result;

      UInt_t counter;        ///< Iteration counter
      UInt_t max_iterations; ///< Maximum number of iterations allowed
   };

/// @brief Struct used to store the results of the minimisation process.
   struct EstimatorResult {
      Int_t    z_value;          ///< Z value
      Int_t    a_value;          ///< Best value determined for A
      Double_t delta;            ///< Best value determined for delta
      UInt_t    forward_counter;  ///< Forward thread iterations
      UInt_t    backward_counter; ///< Backward thread iterations
      Int_t    status;           ///< Status flag to signify errors
   };

} // namespace me

class ThreadedMassEstimator {

public:

   /// @brief Default constructor.
   ThreadedMassEstimator();

   /// @brief Default destructor.
   virtual ~ThreadedMassEstimator();

   /// @brief Initialise the estimator.
   /// @note To be called immediately post-construction.
   Bool_t Init();

   /// @brief Public accessor used to evaluate the initialisation status.
   Bool_t IsInitialised() const;

   /// @brief Set the identification telescope to imitate.
   ///
   /// Simply a pass-through function to call MEDetectorStack::SetIDTelescope
   /// on the private detector stack.
   ///
   /// @param name The ID telescope name as it is returned by
   /// KVIDTelescope::GetName.
   ///
   /// @note Not thread-safe (If using threads you need to wrap each call with
   /// a mutex lock)
   ///
   /// @see MEDetectorStack::SetIDTelescope
   ///
   /// @return Boolean status indicating whether or not the telescope has been
   /// set correctly.
   Bool_t SetIDTelescope(const TString& name);

   /// @brief Populate a vector with all possible A values for a given Z.
   ///
   /// The only stipulation for a nucleus to be considered "possible" is that
   /// it has a positive lifetime when it is constructed by KaliVeda.
   ///
   /// @param z_value Z value of the nucleus
   /// @param results Vector in which to store the results (it will be cleared
   /// and then grown as needed)
   ///
   /// @note Not thread-safe (If using threads you need to wrap each call with
   /// a mutex lock)
   ///
   void GetPossibleAValues(const Int_t z_value, std::vector<Int_t>* results);

   /// @brief Estimate the A value of a nucleus given Z, SIE and CSIL.
   ///
   /// This function calculates an estimate of the A value of a nucleus from
   /// its Z, silicon energy, and caesium iodide light.
   ///
   /// @param input Struct containing the input data required for the
   /// estimator.
   ///
   /// @param possible_a_values Vector of possible A values for the supplied Z
   /// value.
   ///
   /// @param result Struct into which we will write the results.
   ///
   /// @see me::EstimatorInput
   /// @see me::EstimatorResult
   /// @see GetPossibleAValues
   ///
   /// @return Boolean value indicating success or failure of the estimation.
   ///
   Bool_t EstimateA(
      const struct me::EstimatorInput* const input,
      const std::vector<Int_t>* const possible_a_values,
      struct me::EstimatorResult* const result
   );

   /// @brief Public accessor to retrieve the load balance value
   Float_t get_load_balance() const;

   /// @brief Set the load balance value
   void set_load_balance(Float_t load_balance);

   /// @brief Set the maximum number of iterations per thread
   void set_max_iterations(UInt_t iterations);

   /// @brief Enum containing the minimisation status codes.
   ///
   /// Negative values indicate error states, positive values provide
   /// information, and 0 should never occur (used as an initialisation value).
   enum ResultStatus {
      kCalibrationRangeError  = -2,
      kNoResultFound          = -1,
      kEqualDeltas            =  1,
      kThreadOneResult        =  2,
      kThreadZeroResult       =  3
   };

private:

   /// @brief Si-Isobutane-CsI detector stack used to simulate particle
   /// passage.
   ///
   /// @see MEDetectorStack
#if __cplusplus < 201103L
   static MEDetectorStack* medetector_stack_;
#else
   static std::unique_ptr<MEDetectorStack> medetector_stack_;
#endif

   /// @brief Mutex used for locking the console (stdout)
   static pthread_mutex_t io_mutex_;
   /// @brief Mutex used for locking the detector stack.
   static pthread_mutex_t medetector_mutex_;

   /// @brief Thread execution function (Called when each thread is run)
   ///
   /// @param data Thread input data
   ///
   /// @see me::ThreadData
   static void* ThreadExec(void* data);

   /// @brief Thread Processing Method (Called from ThreadExec).
   ///
   /// This function takes care of calculating the delta values and determining
   /// the state of the minimisation for each thread.  The return value
   /// determines whether the current thread will stop or continue processing.
   ///
   /// @see me::ThreadStatus
   ///
   /// @return Status code used to continue or halt processing in the current
   /// thread
   static Int_t ThreadProcess(
      struct me::ThreadData* const data,
      const struct me::SimulationParameters* const parameters
   );

   Bool_t kInitialised_;  ///< Initialisation status
   Float_t load_balance_; ///< Load balance factor
   Int_t max_iterations_; ///< Maximum number of iterations allowed per thread

   /// @brief Copy constructor (Declared private to prevent copying as it is
   /// not worth implementing yet).
   ThreadedMassEstimator(const ThreadedMassEstimator&);
   /// @brief Assignment operator (Declared private to prevent copying as it is
   /// not worth implementing yet).
   ThreadedMassEstimator& operator=(const ThreadedMassEstimator&);

   ClassDef(ThreadedMassEstimator, 1); // ThreadedMassEstimator

};

#endif

