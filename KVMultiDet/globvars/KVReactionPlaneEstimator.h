#ifndef __KVREACTIONPLANEESTIMATOR_H
#define __KVREACTIONPLANEESTIMATOR_H

#include "KVVGVectorSum.h"

/**
 \class KVReactionPlaneEstimator
 \brief Estimate of reaction plane orientation using transverse momentum method of Danielewicz & Odyniec
 \ingroup GlobalVariables

 This global variable calculates a vector which estimates the direction of the impact parameter
 (and hence the orientation of the reaction plane) for each event.

 If projectile (target) fragments are deflected away from the target (projectile), the vector obtained by
 summing all the transverse momenta of the particles produced in the projectile (target) rapidity region
 is parallel (antiparallel) to the impact parameter, \f$\vec{b}\f$.  More generally, one constructs a
 vector \f$\vec{Q}\f$
 \f[
 \vec{Q} = \sum_{k=1}^{N}w_k\vec{p}^t_k
 \f]
 from the transverse momenta \f$\vec{p}^t_k\f$ of all particles of the event with weights \f$w_k\f$ which
 are positive at projectile rapidities, negative at target rapidities.

 The default weighting used is
 \f[
 w_{k}=+1 \quad y_k/y_{P}>0.3\\
w_{k}=-1 \quad y_k/y_{P}<-0.3\\
w_{k}=0 \quad |y_k/y_{P}|<0.3
\f]
where \f$y_k\f$ and \f$y_P\f$ are respectively the centre of mass rapidity of the particle and the projectile.

Before use, the projectile c.m. rapidity for the data must be set using SetNormalization().
Note that this quantity can be retrieved from reaction kinematics described by KV2Body (such as returned
by KVDBSystem::GetKinematics() or KVDataAnalyser::GetKinematics()) like so:
~~~~{.cpp}
KV2Body* kinema; // pointer to kinematics
kinema->GetNucleus(1)->GetFrame("cm")->Rapidity();
~~~~

After calculation, the \f$\vec{Q}\f$ vector can be retrieved using either GetQ() or GetSumObject() (method of
KVVGVectorSum base class). This vector is the total transverse momentum transfer for the event, supposed
to be parallel to the impact parameter of the event, \f$\vec{b}\f$.

When used in a list of variables with automatic TTree branch creation (see KVGVList::CreateBranches()),
the following branches will be added to the tree by default:

   // Name  |  Index  |  Meaning
   // ------|---------|------------------------------------------
   // Q     |    0    | Magnitude of \f$\vec{Q}\f$
   // Phi   |    1    | Azimuthal angle of \f$\vec{Q}\f$
   // X     |    2    | \f$x\f$-component of \f$\vec{Q}\f$
   // Y     |    3    | \f$y\f$-component of \f$\vec{Q}\f$

 \author John Frankland
 \date Fri Jun 25 12:08:14 2021
*/

class KVReactionPlaneEstimator : public KVVGVectorSum {
   double fProjCMRapidity{0};// rapidity of projectile in CM

   void init();
   Double_t getvalue_int(Int_t) const;
   std::function<double(const KVNucleus*)> weight_function = [&](const KVNucleus* n)
   {
      // Default weight function from Ollitrault/Danielewicz.
      //
      // for particle CM normalised rapidity y>0.3 w=1, y<-0.3 w=-1, -0.3<y<0.3 w=0
      auto rapidity = n->Rapidity() / fProjCMRapidity;
      if (rapidity > 0.3) return 1.0;
      else if (rapidity < -0.3) return -1.0;
      return 0.0;
   };
   void fill(const KVNucleus* n)
   {
      Add(weight_function(n) * n->GetTransverseMomentum());
   }
public:
   KVReactionPlaneEstimator() : KVVGVectorSum()
   {
      init();
   }
   KVReactionPlaneEstimator(const Char_t* name) : KVVGVectorSum(name)
   {
      init();
   }

   virtual ~KVReactionPlaneEstimator() {}

   void SetNormalization(Double_t r)
   {
      // Use to set value of projectile CM rapidity for normalization of particle rapidities
      fProjCMRapidity = r;
   }
   TVector3 GetQ() const
   {
      // \returns the total transverse momentum transfer, \f$\vec{Q}\f$
      return KVVGVectorSum::GetSumObject();
   }
   TVector3 GetQForParticle(const KVNucleus* n);
   static TF1* GetDeltaPhiRFitFunction();

   void Init()
   {
      // Make sure that projectile CM rapidity has been set before use

      if (fProjCMRapidity == 0)
         Error("Init", "Projectile CM rapidity not set for this variable: %s. Use method SetNormalization() before calculating.",
               GetName());
   }

   ClassDef(KVReactionPlaneEstimator, 1) //Calculates the transverse momentum Q vector of Danielewicz & Odyniec
};

#endif
