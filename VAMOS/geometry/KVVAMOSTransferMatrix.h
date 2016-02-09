//Created by KVClassFactory on Fri Mar 29 08:56:52 2013
//Author: Guilain ADEMARD

#ifndef __KVVAMOSTRANSFERMATRIX_H
#define __KVVAMOSTRANSFERMATRIX_H

#include "Riostream.h"
#include "KVBase.h"
#include "KVVAMOSReconTrajectory.h"
#include "KVMacros.h" // 'UNUSED' macro

using namespace std;

class KVVAMOSTransferMatrix : public KVBase {
public:

   enum EReconParam {
      kBrho = 0,
      kTheta,
      kPhi,
      kPath
   };

protected:

   UChar_t   fOrder;   // order of the matrix reconstruction
   UShort_t  fNCoef[4];//[4] Number of coef for each parameter (Brho, theta, phi, path)
   Double_t* fCoef [4];//![4][fNCoef[4]] coefficients for each parameter
   UShort_t* f_ijkl[4];//![4][fNCoef[4]] indexes i,j,k,l corresponding to each parameters (4 bits for each index)

   enum {
      kIsReady      = BIT(14),//flag set when the coef. have been set up
      kIsCompressed = BIT(15) //flag when the arrays are compressed
   };

   void NewMatrix();
   void DeleteMatrix();
   void ReadCoefInDataSet();
   void ReadCoefficients(EReconParam par, ifstream& file);
   void RecursiveReadOfCoef(EReconParam par, ifstream& file, UChar_t order, UShort_t* nijkl, UShort_t& NtoRead, UChar_t prev_i = 0);


   inline UShort_t Code_ijkl(UShort_t i, UShort_t j, UShort_t k, UShort_t l)
   {
      UShort_t val = i + (j << 4) + (k << 8) + (l << 12);
      return val;
   }

   inline void Decode_ijkl(UShort_t code, UShort_t& i, UShort_t& j, UShort_t& k, UShort_t& l) const
   {
      i = code       & 15;
      j = (code >> 4)  & 15;
      k = (code >> 8)  & 15;
      l = (code >> 12) & 15;
   }

public:

   KVVAMOSTransferMatrix(Bool_t scan_ds = kFALSE);
   KVVAMOSTransferMatrix(const KVVAMOSTransferMatrix&) ;
   virtual ~KVVAMOSTransferMatrix();
   virtual void Copy(TObject&) const;
   void init();

   void CompressArrays();
   virtual void Print(Option_t* option = "") const;
   Bool_t ReconstructFPtoLab(KVVAMOSReconTrajectory* traj);
   Bool_t ReconstructFPtoLab(Double_t x_f, Double_t y_f, Double_t theta_f, Double_t phi_f, Double_t& brho, Double_t& path, Double_t& theta_v, Double_t& phi_v);
   Bool_t ReconstructLabToFP(KVVAMOSReconTrajectory* traj);

   void SetCoef(EReconParam par, UShort_t i, UShort_t j, UShort_t k, UShort_t l, Double_t coef);
   Bool_t SetOrder(UChar_t order);

   //------------- inline methods ------------------------------//

   inline UChar_t GetOrder() const
   {
      return fOrder;
   }


   inline Bool_t IsReady() const
   {
      return TestBit(kIsReady);
   }
   inline void SetIsReady(Bool_t ok = kTRUE)
   {
      SetBit(kIsReady, ok);
   }

   inline UShort_t GetMaxNcoef() const
   {
      //returns the maximal number of coefficients for one parameter
      //from the value of the order.

      UInt_t num = 1, denum = 1;
      for (Int_t i = 1; i <= 4; i++) {
         num *= (fOrder + i);
         denum *= i;
      }
      return num / denum;
   }

   ClassDef(KVVAMOSTransferMatrix, 1) //VAMOS's transformation matrix to map the measured coordinates at the focal plane back to the target
};

#endif
