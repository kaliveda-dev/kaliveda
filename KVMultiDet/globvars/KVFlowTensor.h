//Created by KVClassFactory on Tue Jun 30 12:41:37 2015
//Author: John Frankland,,,

#ifndef __KVFLOWTENSOR_H
#define __KVFLOWTENSOR_H

#include "KVVarGlob.h"
#include "TMatrixTSym.h"
#include "TVectorT.h"

class KVFlowTensor: public KVVarGlob {

   TMatrixTSym<double> fTensor;//! the tensor
   enum {
      kONE,
      kNRKE,
      kRKE
   } weight;
   TVectorD fEVal;//! the 3 eigenvalues
   TVector3 fEVec[3];//! the 3 eigenvectors
   Double_t f(int i) const
   {
      return fEVal[i - 1];
   }
   const TVector3& e(int i) const
   {
      return fEVec[i - 1];
   }
   Bool_t fCalculated;
   enum {
      kFlowAngle,
      kKinFlowRatio13,
      kKinFlowRatio23,
      kPhiReacPlane,
      kSqueezeAngle,
      kSqueezeRatio,
      kNumberParts
   };
   TRotation fAziReacPlane;//!azimuthal rotation around beam axis to reaction plane
   TRotation fFlowReacPlane;//!rotate XZ to reaction plane, then align Z with flow axis
   Double_t fSqueezeAngle;//!Gutbrod squeeze angle
   Double_t fSqOutRatio;//!Gutbrod squeeze-out ratio
   Int_t fNParts;//! number of particles included in tensor

protected:
   Double_t getvalue_void() const;
   Double_t getvalue_int(Int_t);
   void Calculate();

public:
   KVFlowTensor(void);
   KVFlowTensor(Char_t* nom);
   KVFlowTensor(const KVFlowTensor& a);

   virtual ~KVFlowTensor(void);

   virtual void Copy(TObject& obj) const;

   KVFlowTensor& operator = (const KVFlowTensor& a);

   virtual void Init(void);
   virtual void Reset(void);

   virtual Double_t* GetValuePtr(void);
   virtual TObject* GetObject(void) const;
private:
   void init_KVFlowTensor();
public:
   void Fill(KVNucleus* n);
   const TRotation& GetAziReacPlaneRotation();
   const TRotation& GetFlowReacPlaneRotation();

   ClassDef(KVFlowTensor, 1) //Kinetic energy flow tensor of Gyulassy et al
};


#endif
