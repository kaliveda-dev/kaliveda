
///
/// @file MonoMinimiserImpl.cpp
///
/// @section Description
///
/// Standard implementation of the silicon energy minimiser. This class
/// implements the minimisation by using two while loops operating in different
/// directions (in terms of A value), both starting from the same initial ansatz
/// that A is most likely to be around to 2Z. It is essentially the single
/// threaded version of the ThreadedMassEstimator, operating to reduce the
/// difference between the observed and simulated silicon detector energies in
/// order to determine the most likely value for A.
///
/// The bisection method previously used has been replaced by this "brute force"
/// method. The reasoning behind this decision is that the bisection method
/// relies upon the function being well behaved and passing through delta = 0
/// with positive values on one side of the minimum and negative ones on the
/// other. However due to the large A range being used, this does not seem to be
/// guaranteed.  For example if one takes a value in A which is too large, the
/// energy calculation can result in something quite extreme and using the
/// bisection method you will lose 1/2 of the available solution space from this
/// single error.
///
/// On my desktop this algorithm processes at about 85 events/s compared with
/// 120 events/s for the threaded version (ThreadedMassEstimator).
///
/// @note: It is important that you keep the public interface (public member
/// functions) of the implementation classes in agreement with those in this
/// class, as all functions are simple passed off to the implementation class.
/// For example SiliconEnergyMinimiser::SetIDTelescope() simply calls
/// MonoMinimiserImpl::SetIDTelescope().
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Sun 17 Jan 20:43:44  2016
///

#include "MonoMinimiserImpl.h"

ClassImp(MonoMinimiserImpl)

MonoMinimiserImpl::MonoMinimiserImpl() :
   kInitialised_(kFALSE),
   kTelescopeSet_(kFALSE),
   max_iterations_(1000),
   tolerance_(0.05),
#if __cplusplus < 201103L
   sim_parameters_(NULL),
   sim_results_(NULL),
   stack_(NULL)
#else
   sim_parameters_(nullptr),
   sim_results_(nullptr),
   stack_(nullptr)
#endif
{

}

MonoMinimiserImpl::~MonoMinimiserImpl()
{
#if __cplusplus < 201103L
   if (stack_) {
      delete stack_;
      stack_ = NULL;
   }

   if (sim_results_) {
      delete sim_results_;
      sim_results_ = NULL;
   }

   if (sim_parameters_) {
      delete sim_parameters_;
      sim_parameters_ = NULL;
   }
#endif
}

void MonoMinimiserImpl::Init()
{
   if (kInitialised_) return;

#if __cplusplus < 201103L
   sim_parameters_ = new struct me::SimulationParameters();
   sim_results_ = new struct me::SimulationResult();
   stack_ = new MEDetectorStack();
#else
   sim_parameters_.reset(new struct me::SimulationParameters());
   sim_results_.reset(new struct me::SimulationResult());
   stack_.reset(new MEDetectorStack());
#endif

   stack_->Init();
   kInitialised_ = kTRUE;
}

Bool_t MonoMinimiserImpl::SetIDTelescope(const TString& telescope_name)
{
   if (!kInitialised_) {
      Init();
      return kFALSE;
   }

   kTelescopeSet_ = kFALSE;

   Bool_t status(stack_->SetIDTelescope(telescope_name));
   if (status) {
      kTelescopeSet_ = kTRUE;
   }

   return status;
}

Int_t MonoMinimiserImpl::Minimise(UInt_t z_value, Double_t si_energy,
                                  Double_t csi_light)
{

   if (!kTelescopeSet_) {
      Error("MonoMinimiserImpl::Minimise",
            "You must call MonoMinimiserImpl::Init() "
            "and MonoMinimiserImpl::SetIDTelescope() before calling this "
            "function!");
      return -1;
   }

   sim_parameters_->z = z_value;
   sim_parameters_->si_energy = si_energy;
   sim_parameters_->csi_light = csi_light;

   UInt_t starting_a_value(2 * z_value);

   UInt_t a_value(starting_a_value);
   Double_t delta(100000.);
   Double_t delta_last(100000.);

   Double_t best_forward_delta(100000.);
   Double_t best_backward_delta(100000.);
   UInt_t best_forward_a_value(0);
   UInt_t best_backward_a_value(0);

   // Records the number of steps for which we have been consistently
   // maximising.
   UInt_t num_maximising(0);

   // Same starting point (A = 2Z) for both directions (while loops)
   sim_parameters_->a = a_value;
#if __cplusplus < 201103L
   if (stack_->Simulate(sim_parameters_, sim_results_)) {
#else
   if (stack_->Simulate(sim_parameters_.get(), sim_results_.get())) {
#endif
      delta_last = sim_results_->delta;
   } else {
      delta_last = 100000.;
      // TODO: Return failed?
   }

   best_forward_delta = delta_last;
   best_backward_delta = delta_last;
   best_forward_a_value = a_value;
   best_backward_a_value = a_value;

   Bool_t forward_status(kTRUE);
   Bool_t backward_status(kTRUE);

   // -------------------------------------
   // Iterate Forwards (Increasing A value)
   // -------------------------------------

   a_value = starting_a_value + 1;
   UInt_t n(1);
   while (n < max_iterations_) {

      // Go forward from 2Z

      if (a_value > 299) {
         forward_status = kFALSE;
         break;
      }

      sim_parameters_->a = a_value;
#if __cplusplus < 201103L
      if (stack_->Simulate(sim_parameters_, sim_results_)) {
#else
      if (stack_->Simulate(sim_parameters_.get(), sim_results_.get())) {
#endif
         delta = sim_results_->delta;
      } else {
         delta = 100000.;
      }

      if (delta > (delta_last + (tolerance_ * delta_last))) {
         // The value for delta in this iteration is greater than the last by
         // the tolerance setting (default 5%)

         if (num_maximising >= 2) {
            // We have been maximising at greater than the tolerance setting
            // (default 5%) for at least 2 iterations, it is perhaps safe to
            // assume that this trend will continue and so we stop
            // processing

            break;
         }
         ++num_maximising;
      } else {
         num_maximising = 0;
      }

      if (delta < best_forward_delta) {
         best_forward_delta = delta;
         best_forward_a_value = a_value;
      }

      delta_last = delta;
      ++n;
      ++a_value;
   }

   if (n >= max_iterations_) {
      Error("MonoMinimiserImpl::Minimise",
            "Forward loop exceeded the maximum number of iterations");
      return -1;
   }

   // --------------------------------------
   // Iterate Backwards (Decreasing A value)
   // --------------------------------------

   num_maximising = 0;
   a_value = starting_a_value - 1;
   n = 1;
   while (n < max_iterations_) {

      // Go backward from 2Z

      if (a_value < 1) {
         backward_status = kFALSE;
         break;
      }

      sim_parameters_->a = a_value;
#if __cplusplus < 201103L
      if (stack_->Simulate(sim_parameters_, sim_results_)) {
#else
      if (stack_->Simulate(sim_parameters_.get(), sim_results_.get())) {
#endif
         delta = sim_results_->delta;
      } else {
         delta = 100000.;
      }

      if (delta > (delta_last + (tolerance_ * delta_last))) {
         // The value for delta in this iteration is greater than the last by
         // the tolerance setting (default 5%)

         if (num_maximising >= 2) {
            // We have been maximising at greater than the tolerance setting
            // (default 5%) for at least 2 iterations, it is perhaps safe to
            // assume that this trend will continue and so we stop
            // processing

            break;
         }
         ++num_maximising;
      } else {
         num_maximising = 0;
      }

      if (delta < best_backward_delta) {
         best_backward_delta = delta;
         best_backward_a_value = a_value;
      }

      delta_last = delta;
      ++n;
      --a_value;

   }

   if (n >= max_iterations_) {
      Error("MonoMinimiserImpl::Minimise",
            "Backward loop exceeded the maximum number of iterations");
      return -1;
   }

   // ----------------------------------
   // Select and return the best A value
   // ----------------------------------

   if (forward_status && !backward_status) {
      return best_forward_a_value;
   } else if (backward_status && !forward_status) {
      return best_backward_delta;
   } else if (!backward_status && !forward_status) {
      return -1;
   } else {
      if (best_backward_delta < best_forward_delta) {
         return best_backward_a_value;
      } else {
         return best_forward_a_value;
      }
   }
}

void MonoMinimiserImpl::SetMaximumIterations(UInt_t max_iterations)
{
   max_iterations_ = max_iterations;
}

void MonoMinimiserImpl::SetTolerance(Double_t tolerance)
{
   tolerance_ = tolerance;
}

void MonoMinimiserImpl::Print() const
{
   Info("MonoMinimiserImpl::Print", "Bisection minimiser");
}

