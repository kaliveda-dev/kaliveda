
///
/// @file ThreadedMassEstimator.cpp
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

#include "ThreadedMassEstimator.h"

ClassImp(ThreadedMassEstimator)

const Int_t me::ThreadStatus::kThreadStop(0);
const Int_t me::ThreadStatus::kThreadContinue(-1);

#if __cplusplus < 201103L
MEDetectorStack* ThreadedMassEstimator::medetector_stack_(
   new MEDetectorStack()
);
#else
std::unique_ptr<MEDetectorStack> ThreadedMassEstimator::medetector_stack_(
   new MEDetectorStack()
);
#endif

pthread_mutex_t ThreadedMassEstimator::io_mutex_;
pthread_mutex_t ThreadedMassEstimator::medetector_mutex_;

ThreadedMassEstimator::ThreadedMassEstimator() :
   kInitialised_(kFALSE),
   load_balance_(2.25),
   max_iterations_(100)
{

}

ThreadedMassEstimator::~ThreadedMassEstimator()
{
   pthread_mutex_destroy(&io_mutex_);
   pthread_mutex_destroy(&medetector_mutex_);

#if __cplusplus < 201103L
   if (medetector_stack_) {
      delete medetector_stack_;
      medetector_stack_ = NULL;
   }
#endif

}

Bool_t ThreadedMassEstimator::Init()
{
   if (kInitialised_) {
      return kTRUE;
   }

   if (!medetector_stack_->Init()) {
      return kFALSE;
   }

#if __cplusplus < 201103L
   pthread_mutex_init(&io_mutex_, NULL);
   pthread_mutex_init(&medetector_mutex_, NULL);
#else
   pthread_mutex_init(&io_mutex_, nullptr);
   pthread_mutex_init(&medetector_mutex_, nullptr);
#endif

   kInitialised_ = kTRUE;

   return kTRUE;
}

Bool_t ThreadedMassEstimator::IsInitialised() const
{
   return kInitialised_;
}

Bool_t ThreadedMassEstimator::SetIDTelescope(const TString& name)
{
   if (!medetector_stack_->SetIDTelescope(name)) {
      return kFALSE;
   }

   return kTRUE;
}

void ThreadedMassEstimator::GetPossibleAValues(
   const Int_t z_value,
   std::vector<Int_t>* results
)
{
   assert(results);

   // Destroy all the objects currently in result
   results->clear();

   for (Int_t a_value = 1; a_value < 240; ++a_value) {
      KVNucleus nucleus(z_value, a_value);
      if (nucleus.GetLifeTime() > 0.) {
         results->push_back(a_value);
      }
   }
}

void* ThreadedMassEstimator::ThreadExec(void* thread_data)
{
   struct me::ThreadData* data(
      static_cast<struct me::ThreadData*>(thread_data)
   );

   assert(data);

   Bool_t process_enable(kFALSE);

   struct me::SimulationParameters parameters;
   parameters.z = data->z_value_expt;
   parameters.si_energy = data->si_energy_expt;
   parameters.csi_light = data->csi_light_expt;

   Int_t status(0);
   data->counter = 0;

   if (data->kForwardIterator == kTRUE) {

      // Iterate forwards through the list of possible A values.

      std::vector<Int_t>::iterator itr;
      for (itr = data->possible_a_values.begin();
            itr != data->possible_a_values.end();
            ++itr
          ) {

         if (process_enable) {

            parameters.a = *itr;
            data->current_a = *itr;

            // Process this point
            status = ThreadProcess(data, &parameters);

            // If we are maximising or something went wrong we return
            if (status == me::ThreadStatus::kThreadStop) {
               break;
            }

         }

         // Do nothing until we reach the pre-defined starting value
         if (*itr == data->minimum_a) {
            // Remove the inhibitor switch for the next loop
            process_enable = kTRUE;
         }

      }

   } else {

      // Iterate backwards through the list of possible A values.

      std::vector<Int_t>::reverse_iterator ritr;
      for (ritr = data->possible_a_values.rbegin();
            ritr != data->possible_a_values.rend();
            ++ritr
          ) {

         if (process_enable) {

            parameters.a = *ritr;
            data->current_a = *ritr;

            // Process this point
            status = ThreadProcess(data, &parameters);

            if (status == me::ThreadStatus::kThreadStop) {
               break;
            }

         }

         // Do nothing until we reach the pre-defined starting value
         if (*ritr == data->minimum_a) {
            // Remove the inhibitor switch for the next loop
            process_enable = kTRUE;
         }


      }

   }

#if __cplusplus < 201103L
   pthread_exit(NULL);
#else
   pthread_exit(nullptr);
#endif

}

Int_t ThreadedMassEstimator::ThreadProcess(
   struct me::ThreadData* const data,
   const struct me::SimulationParameters* const parameters)
{

   ++data->counter;
   if (data->counter > data->max_iterations) {
      // We've exceeded the number of allowed iterations, stop processing and
      // signal an error.
      data->current_delta = -1.;
      return me::ThreadStatus::kThreadStop;
   }

   // We have to lock the detector stack with a mutex when performing the
   // calculation as the energy loss functions are not thread safe - there are
   // several static pointers used.
   pthread_mutex_lock(&medetector_mutex_);
   if (!medetector_stack_->Simulate(parameters, &(data->sim_result))) {
      data->current_delta = -1.;
      pthread_mutex_unlock(&medetector_mutex_);
      return me::ThreadStatus::kThreadStop;

   }

   data->current_delta = data->sim_result.delta;
   pthread_mutex_unlock(&medetector_mutex_);

   if (data->current_delta < 0.) {

      // This should be impossible but we might as well check

      pthread_mutex_lock(&io_mutex_);
      Error("ThreadedMassEstimator::ThreadProcess",
            "Negative value for current delta!");
      pthread_mutex_unlock(&io_mutex_);

      return me::ThreadStatus::kThreadStop;
   }

   if (data->minimum_delta < 0.) {

      // This should be impossible but we might as well check

      pthread_mutex_lock(&io_mutex_);
      Error("ThreadedMassEstimator::ThreadProcess",
            "Negative value for minimum delta!");
      pthread_mutex_unlock(&io_mutex_);

      return me::ThreadStatus::kThreadStop;
   }

   if (data->current_delta < data->minimum_delta) {
      // The current point (A value, Delta) yields a better result than the
      // stored point (a lower value for delta). We therefore overwrite the
      // stored data with the current values;

      data->minimum_delta = data->current_delta;
      data->minimum_a = data->current_a;

      return me::ThreadStatus::kThreadContinue;

   } else if (data->current_delta > data->minimum_delta) {
      // We are maximising delta which means we are moving away from the
      // minimum point we are searching for so we stop the processing.

      return me::ThreadStatus::kThreadStop;

   } else {
      // Deltas are equal, this means we cannot be sure we have the correct A
      // value (the current minimum value for A will be used)

      return me::ThreadStatus::kThreadStop;
   }

}

Bool_t ThreadedMassEstimator::EstimateA(
   const struct me::EstimatorInput* const input,
   const std::vector<Int_t>* const possible_a_values,
   struct me::EstimatorResult* const result
)
{
   assert(input);
   assert((input->si_energy > 0.) && (input->si_energy < 3000.));

   assert(possible_a_values);
   assert(result);

   if (!input) {
      Error("ThreadedMassEstimator::EstimateA", "Input data is nullptr");
      return kFALSE;
   }

   // You should never see this initialisation value in the output, it should
   // always be set to either a positive or a negative number below:
   result->status = 0;

   Bool_t calibration_range_error(kFALSE);

   if (input->si_energy > 1000.) {

      // IMPORTANT:
      //
      // Currently the CsI light to energy calibration function is only valid
      // up to around A=45 and we must warn the user that they are trying to
      // analyse a point which is likely outside of that range. Unfortunately,
      // there are only 3 variables available to us at this time: silicon
      // energy, caesium iodide light output and Z. I have chosen the silicon
      // energy and decided to specifiy the cut-off as 1000 MeV (48-Ca should
      // be around 800-900 MeV). If the CsI is ever re-calibrated to extend
      // this range, then this cut-off value should be increased accordingly.
      //
      // We process this event as normal but the results are extrememly
      // unreliable outside of the CsI calibration range. The
      // calibration_range_error flag is used to set the correct return status
      // after the event has been processed. This way we let the mass
      // estimator do its job but inform the user to be wary of the result.
      //

      calibration_range_error = kTRUE;
   }

   static Long64_t event_number(0);
   ++event_number;

   struct me::SimulationParameters parameters;
   parameters.z = input->z;

   // Initial simulation for the starting A value (common to both threads)
   //
   // IMPORTANT: The initial A value must be set at approximately the right
   // value (approximately 2*Z) in order to reduce the time taken for its
   // determination. However an additional linear scaling is needed to maintain
   // the balance of processing power between the "forward" thread and the
   // "backward" thread. If this balance is not maintained then all of the
   // processing is done in one thread and you lose the benefit of using two.
   // Empirically, the default scaling factor of 2.25 is sufficient over the
   // range of z values we expect in the E503 experiment.

   parameters.a = static_cast<Int_t>(load_balance_ * input->z);
   parameters.si_energy = input->si_energy;
   parameters.csi_light = input->csi_light;

   // Get the "delta" - the magnitude difference between the real silicon
   // energy and that simulated for our A = load_balance_*Z nucleus.
   // Single thread at this point, no need to lock with mutex
   Double_t delta_init(-1.);

   struct me::SimulationResult res;
   if (!medetector_stack_->Simulate(&parameters, &res)) {
      return kFALSE;
   }

   delta_init = res.delta;

   // Pack the input data into structs
   struct me::ThreadData data[2];

   for (Int_t i = 0; i < 2; ++i) {
      data[i].z_value_expt = input->z;
      data[i].si_energy_expt = input->si_energy;
      data[i].csi_light_expt = input->csi_light;

      // Give each thread a local copy of the possible A values
      data[i].possible_a_values = *possible_a_values;

      // Set the (A, delta) as the starting point for both threads
      data[i].minimum_a = parameters.a;
      data[i].minimum_delta = delta_init;
      data[i].max_iterations = max_iterations_;

   }

   // Iterating forwards (Increasing A from initial value)
   data[0].kForwardIterator = kTRUE;

   // Iterating backwards (Decreasing A from initial value)
   data[1].kForwardIterator = kFALSE;

   // Create the threads (one for each direction - increasing/decreasing A)

   pthread_t thr[2];
   Int_t status(0);

   //////////////////
   // Create Thread 0
   //////////////////

#if __cplusplus < 201103L
   status = pthread_create(
               &thr[0],
               NULL,
               ThreadExec,
               static_cast<void*>(&data[0])
            );
#else
   status = pthread_create(
               &thr[0],
               nullptr,
               ThreadExec,
               static_cast<void*>(&data[0])
            );
#endif

   if (status != 0) {
      Error("ThreadedMassEstimator::EstimateA", "Failed to create thread 0");
      return kFALSE;
   }

   //////////////////
   // Create Thread 1
   //////////////////

#if __cplusplus < 201103L
   status = pthread_create(
               &thr[1],
               NULL,
               ThreadExec,
               static_cast<void*>(&data[1])
            );
#else
   status = pthread_create(
               &thr[1],
               nullptr,
               ThreadExec,
               static_cast<void*>(&data[1])
            );
#endif

   if (status != 0) {
      Error("ThreadedMassEstimator::EstimateA", "Failed to create thread 1");
      return kFALSE;
   }

   ////////////////
   // Join Thread 0
   ////////////////

   Int_t thread_return(0);

   status = pthread_join(
               thr[0],
               reinterpret_cast<void**>(&thread_return)
            );

   if (status != 0) {
      Error(
         "ThreadedMassEstimator::EstimateA",
         "Failed to join Thread 0 (status = %d)", thread_return
      );
      return kFALSE;
   }

   ////////////////
   // Join Thread 1
   ////////////////

   status = pthread_join(
               thr[1],
               reinterpret_cast<void**>(&thread_return)
            );

   if (status != 0) {
      Error(
         "ThreadedMassEstimator::EstimateA",
         "Failed to join Thread 1 (status = %d)", thread_return
      );
      return kFALSE;
   }

   // We now compare the results of the two threads to see which one has the
   // best value for a_min

   Int_t a_min(-1);
   Double_t delta(-1.);
   Int_t result_status(0);

   Bool_t thread_ok[2];
   for (Int_t i = 0; i < 2; ++i) thread_ok[i] = kTRUE;

   if (data[0].minimum_delta < 0.) thread_ok[0] = kFALSE;
   if (data[1].minimum_delta < 0.) thread_ok[1] = kFALSE;

   if (thread_ok[0] && thread_ok[1]) {

      if (data[0].minimum_delta < data[1].minimum_delta) {
         a_min = data[0].minimum_a;
         delta = data[0].minimum_delta;
         result_status = kThreadZeroResult;

      } else if (data[1].minimum_delta < data[0].minimum_delta) {
         a_min = data[1].minimum_a;
         delta = data[1].minimum_delta;
         result_status = kThreadOneResult;

      } else {
         // Equal Deltas: This usually happens when the initial A value
         // provided the minimum delta. This is because both threads store
         // this point as their initial minimum reference)

         a_min = data[0].minimum_a;
         delta = data[0].minimum_delta;
         result_status = kEqualDeltas;
      }

   } else if (thread_ok[0] && !thread_ok[1]) {
      a_min = data[0].minimum_a;
      delta = data[0].minimum_delta;
      result_status = kThreadZeroResult;

   } else if (!thread_ok[0] && thread_ok[1]) {
      a_min = data[1].minimum_a;
      delta = data[1].minimum_delta;
      result_status = kThreadOneResult;

   } else {

      // IMPORTANT: Both threads failed to return a sensible result, you need
      // to handle this in your analysis class!

      result_status = kNoResultFound;

      result->z_value = input->z;
      result->a_value = -1;
      result->delta = -1.;
      result->forward_counter = data[0].counter;
      result->backward_counter = data[1].counter;
      result->status = result_status;

      return kFALSE;
   }

   if (calibration_range_error) {
      result_status = kCalibrationRangeError;
   }

   result->z_value = input->z;
   result->a_value = a_min;
   result->delta = delta;
   result->forward_counter = data[0].counter;
   result->backward_counter = data[1].counter;
   result->status = result_status;

   return kTRUE;
}

Float_t ThreadedMassEstimator::get_load_balance() const
{
   return load_balance_;
}

void ThreadedMassEstimator::set_load_balance(Float_t load_balance)
{
   load_balance_ = load_balance;
}

void ThreadedMassEstimator::set_max_iterations(UInt_t iterations)
{
   max_iterations_ = iterations;
}

