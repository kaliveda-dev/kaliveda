#ifndef __KVFLOWTENSOR_H
#define __KVFLOWTENSOR_H

#include "KVVarGlob.h"
#include "TMatrixDSym.h"
#include "TVectorT.h"

/**
\class KVFlowTensor
\brief Kinetic energy flow tensor of Gyulassy et al and associated shape variables
\ingroup GlobalVariables

 This global variable class implements the kinetic energy flow tensor of
 M. Gyulassy et al., _Phys. Lett._ **110B**(1982)185:
 \f[
 F_{ij}=\sum_{\nu=1}^{N}w_{\nu}p_{i}(\nu)p_{j}(\nu)
 \f]
 which is built from the components of the momenta of reaction products in each event.

### Available Options
Option Name  | Option type | Function
-------------|-------------|--------------------------------
"weight"     | string      | Determine weight \f$w_{\nu}\f$
"DOUBLE"     | any         | Double weight of all particles

### Weight options
 With the weight \f$w=1\f$, this expression gives the sphericity tensor. Such a
 tensor is not appropriate for analysing collisions in which composite
 fragments are produced.

To correctly weight composite fragments, Gyulassy et al. proposed to use the weight
 \f[
 w_{\nu}=(2m_{\nu})^{-1}
 \f]
 in which case \f$F\f$ becomes the (non-relativistic) kinetic energy flow tensor
 (\f$\mathrm{Tr} F\f$ is the total non-relativistic kinetic energy of the products).
 This is the default weight used.

 To change the weight, call method SetOption("weight","[your choice]")
 with one of the following choices:

Option          | Name                                 | Weight
----------------|--------------------------------------|---------------------------------------------------
"NRKE" [default]| non-relativistic KE tensor           | \f$w_{\nu}=(2m_{\nu})^{-1}\f$
"RKE"           | relativistic KE tensor               | \f$w_{\nu}={1 \over {m_{\nu}(\gamma_{\nu}+1)}}\f$
"ONE","1",""    | sphericity tensor                    | \f$w=1\f$

 To quote Gyulassy et al.:
   > "We emphasize that event shape analysis makes sense only in the
   > nucleus-nucleus center of mass system since the eigenvalues of \f$F\f$ specify
   > an ellipsoid centered at the origin."
Thus the default frame is "CM".

 We remind the user also that attempting shape analysis for events where a large proportion of reaction
 products have not been detected is meaningless: some kind of completeness criterion should be applied to
 the analysed events.

 ### Derived shape variables and related values
 Diagonalisation of the tensor gives three eignevectors \f$\vec{e_1},\vec{e_2},\vec{e_3}\f$ and three (normalized) eigenvalues
 \f$\lambda_1\geq\lambda_2\geq\lambda_3\f$. Variables describing the shape/orientation of the event can then
 be obtained either by name or by index as in the following table:

 Name           | Index                       | Definition
 ---------------|-----------------------------|---------------------------------------------
"FlowAngle"     |KVFlowTensor::kFlowAngle     |polar angle of \f$\vec{e_1}\f$
"Sphericity"    |KVFlowTensor::kSphericity    |\f$\frac{3}{2}(1-\lambda_1)\f$
"Coplanarity"   |KVFlowTensor::kCoplanarity   |\f$\frac{\sqrt{3}}{2}(\lambda_2-\lambda_3)\f$
"KinFlowRatio13"|KVFlowTensor::kKinFlowRatio13|\f$\sqrt{\frac{\lambda_1}{\lambda_3}}\f$
"KinFlowRatio23"|KVFlowTensor::kKinFlowRatio23|\f$\sqrt{\frac{\lambda_2}{\lambda_3}}\f$
"PhiReacPlane"  |KVFlowTensor::kPhiReacPlane  |\f$\phi\f$ angle of \f$\vec{e_1}\f$
"SqueezeAngle"  |KVFlowTensor::kSqueezeAngle  |see Gyulassy et al
"SqueezeRatio"  |KVFlowTensor::kSqueezeRatio  |see Gyulassy et al
"NumberParts"   |KVFlowTensor::kNumberParts   |number of particles used in tensor

\author John Frankland (GANIL)
\date Tue Jun 30 12:41:37 2015

*/

class KVFlowTensor: public KVVarGlob {

   TMatrixDSym fTensor;//! the tensor
   enum {
      kONE,
      kNRKE,
      kRKE
   } weight;
   mutable TVectorD fEVal;//! the 3 eigenvalues
   mutable TVector3 fEVec[3];//! the 3 eigenvectors
   Double_t f(int i) const
   {
      return fEVal[i - 1];
   }
   const TVector3& e(int i) const
   {
      return fEVec[i - 1];
   }
   TRotation fAziReacPlane;//!azimuthal rotation around beam axis to reaction plane
   TRotation fFlowReacPlane;//!rotate XZ to reaction plane, then align Z with flow axis
   Double_t fSqueezeAngle;//!Gutbrod squeeze angle
   Double_t fSqOutRatio;//!Gutbrod squeeze-out ratio
   Double_t fSphericity;//!
   Double_t fCoplanarity;//!
   Int_t fNParts;//! number of particles included in tensor
   Bool_t fCalculated;
   Double_t sum_val_prop;

protected:
   Double_t getvalue_int(Int_t) const;

public:
   enum {
      kFlowAngle,
      kSphericity,
      kCoplanarity,
      kKinFlowRatio13,
      kKinFlowRatio23,
      kPhiReacPlane,
      kSqueezeAngle,
      kSqueezeRatio,
      kNumberParts
   };

   KVFlowTensor();
   KVFlowTensor(const Char_t* nom);
   KVFlowTensor(const KVFlowTensor& a);

   virtual ~KVFlowTensor(void);

   virtual void Copy(TObject& obj) const;

   KVFlowTensor& operator=(const KVFlowTensor& a);

   virtual void Init();
   virtual void Reset();
   void Calculate();

private:
   void init_KVFlowTensor();
public:
   void fill(const KVNucleus* n);
   const TRotation& GetAziReacPlaneRotation();
   const TRotation& GetFlowReacPlaneRotation();

   KVFlowTensor& operator+=(const KVFlowTensor& other)
   {
      // Add all components of tensor in other to this one, and increase number of
      // particles included in tensor by the number in other

      fTensor += other.fTensor;
      fNParts += other.fNParts;
      return (*this);
   }
   Double_t GetNormalisedEigenValue(Int_t i)
   {
      // Return normalised eignevalues
      // \param[in] i index=1,2,3 (i=1 is largest eigenvalue)

      return f(i) / sum_val_prop;
   }

   void Print(Option_t* = "") const;

   Bool_t HasNaNElements() const
   {
      // check all elements of tensor are valid numbers
      for (int i = 0; i < 3; ++i) {
         for (int j = 0; j < 3; ++j) {
            if (TMath::IsNaN(fTensor(i, j))) return kTRUE;
         }
      }
      return kFALSE;
   }

   ClassDef(KVFlowTensor, 1) //Kinetic energy flow tensor of Gyulassy et al
};


#endif
