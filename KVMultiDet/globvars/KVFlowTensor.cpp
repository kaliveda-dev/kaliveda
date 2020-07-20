//Created by KVClassFactory on Tue Jun 30 12:41:37 2015
//Author: John Frankland,,,

#include "KVFlowTensor.h"
#include "TMatrixDUtils.h"

ClassImp(KVFlowTensor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFlowTensor</h2>
<h4>Kinetic energy flow tensor of Gyulassy et al</h4>
<!-- */
// --> END_HTML
// This global variable class implements the kinetic energy flow tensor of
// M. Gyulassy et al., Phys. Lett. 110B(1982)185:
// BEGIN_LATEX
// F_{ij}=\sum_{\nu=1}^{N}w_{\nu}p_{i}(\nu)p_{j}(\nu)
// END_LATEX
// which is built from the components of the momenta of all (or a selection of)
// reaction products in each event.
// With the weight w=1, this expression gives the sphericity tensor. Such a
// tensor is not appropriate for analysing collisions in which composite
// fragments are produced. To correctly weight composite fragments, Gyulassy
// et al. proposed to use the weight
// BEGIN_LATEX
// w_{\nu}=(2m_{\nu})^{-1}
// END_LATEX
// in which case F becomes the (non-relativistic) kinetic energy flow tensor
// (the trace of F is the total non-relativistic kinetic energy of the products).
// This is the default weight used.
// To change the weight, call method SetOption("weight","[your choice]")
// with one of the following choices:
//
//     "RKE" : relativistic kinetic energy tensor
//     i.e. with
// BEGIN_LATEX
// w_{\nu}={1 \over {m_{\nu}(\gamma_{\nu}+1)}}
// END_LATEX
//
//     "ONE","1", or "" : weight w=1
//
//     "NRKE" : non-relativistic KE tensor (default)
//
// Gyulassy et al.: "We emphasize that event shape analysis makes sense only in the
// nucleus-nucleus center of mass system since the eigenvalues of F specify
// an ellipsoid centered at the origin."
//    => default frame is "CM"
////////////////////////////////////////////////////////////////////////////////

KVFlowTensor::KVFlowTensor(void): KVVarGlob(), fTensor(3)
{
   // Default constructor
   init_KVFlowTensor();
   SetName("KVFlowTensor");
   SetTitle("Kinetic energy flow tensor of Gyulassy et al.");
}

//_________________________________________________________________
KVFlowTensor::KVFlowTensor(const Char_t* nom): KVVarGlob(nom), fTensor(3)
{
   // Constructor with a name for the global variable
   init_KVFlowTensor();
}

//_________________________________________________________________
KVFlowTensor::KVFlowTensor(const KVFlowTensor& a): KVVarGlob(), fTensor(3)
{
   // Copy constructor
   init_KVFlowTensor();
   a.Copy(*this);
}

//_________________________________________________________________
KVFlowTensor::~KVFlowTensor(void)
{
   // Destructor
}

//_________________________________________________________________
void KVFlowTensor::Copy(TObject& a) const
{
   // Copy properties of 'this' object into the KVVarGlob object referenced by 'a'

   KVVarGlob::Copy(a);// copy attributes of KVVarGlob base object

   //KVFlowTensor& aglob = (KVFlowTensor&)a;
   // Now copy any additional attributes specific to this class:
   // To copy a specific field, do as follows:
   //
   //     aglob.field=field;
   //
   // If setters and getters are available, proceed as follows
   //
   //    aglob.SetField(GetField());
   //
}

//_________________________________________________________________
KVFlowTensor& KVFlowTensor::operator = (const KVFlowTensor& a)
{
   // Operateur =

   a.Copy(*this);
   return *this;
}

//_________________________________________________________________
void KVFlowTensor::Init(void)
{
   // Initialisation of internal variables, called once before beginning treatment
   Reset();
   TString wgt = GetOptionString("weight").Data();
   if (wgt == "RKE") weight = kRKE;
   else if (wgt == "ONE" || wgt == "" || wgt == "1") weight = kONE;
   else weight = kNRKE;
}

//_________________________________________________________________
void KVFlowTensor::Reset(void)
{
   // Reset internal variables, called before treatment of each event
   fTensor.Zero();
   fCalculated = kFALSE;
   fNParts = 0;
}

//_________________________________________________________________
TObject* KVFlowTensor::GetObject(void) const
{
   // You can use this method to return the address of an object associated with your global variable.
   // This may be a list of particles, an intermediate object used to compute values, etc.

   return NULL;
}

Double_t KVFlowTensor::getvalue_void() const
{
   // Protected method, called by GetValue()
   // This method should return the 'principal' or 'default' value associated with this global variable.
   //
   // For historical reasons, this method is 'const', which means that you cannot change the values
   // of any internal variables in this method (trying to do so will lead to your class not compiling).
   // If you need to get around this restriction, then you can use the following workaround:
   //
   //    const_cast<KVFlowTensor*>(this)->MemberVariable = NewValue;
   //
   // The restriction also applies to calling other methods of the class which are not 'const'.
   // If you need to do this, you can use the same workaround:
   //
   //   const_cast<KVFlowTensor*>(this)->NotAConstMethod(someArguments);

   return const_cast<KVFlowTensor*>(this)->getvalue_int(kFlowAngle);
}

Double_t KVFlowTensor::getvalue_int(Int_t index) const
{
   // Protected method, called by GetValue(Int_t) and GetValue(const Char_t*)
   // If your variable calculates several different values, this method allows to access each value
   // based on a unique index number.
   //
   // You should implement something like the following:
   //
   //   switch(index){
   //         case 0:
   //            return val0;
   //            break;
   //         case 1:
   //            return val1;
   //            break;
   //   }
   //
   // where 'val0' and 'val1' are the internal variables of your class corresponding to the
   // required values.
   //
   // In order for GetValue(const Char_t*) to work, you need to associate each named value
   // with an index corresponding to the above 'switch' statement, e.g.
   //
   //     SetNameIndex("val0", 0);
   //     SetNameIndex("val1", 1);
   //
   // This should be done in the init_KVFlowTensor() method.

   switch (index) {
      case kFlowAngle:
         return TMath::RadToDeg() * e(1).Theta();
         break;

      case kSphericity:
         return fSphericity;
         break;

      case kKinFlowRatio13: {
            if (f(3) <= 0.) return -1.;
            if (f(1) <= 0.) return -1.;
            return TMath::Min(1e+03, pow(f(1) / f(3), 0.5));
         }
         break;

      case kKinFlowRatio23: {
            if (f(3) <= 0.) return -1.;
            if (f(2) <= 0.) return -1.;
            return TMath::Min(1e+03, pow(f(2) / f(3), 0.5));
         }
         break;

      case kPhiReacPlane: {
            Double_t phi = TMath::RadToDeg() * e(1).Phi();
            return (phi < 0 ? 360 + phi : phi);
         }
         break;

      case kSqueezeAngle:
         return fSqueezeAngle;
         break;

      case kSqueezeRatio:
         return fSqOutRatio;
         break;

      case kNumberParts:
         return fNParts;
         break;

      default:
         break;
   }

   return 0;
}

void KVFlowTensor::Calculate()
{
   // Calculate eigenvalues & eigenvectors of tensor

   TMatrixT<double> evectors = fTensor.EigenVectors(fEVal);

   for (int i = 0; i < 3; i++) {
      TVectorD col = TMatrixDColumn(evectors, i);
      fEVec[i].SetXYZ(col[0], col[1], col[2]);
   }

   // check orientation of flow axis
   // by symmetry/convention, we allow FlowAngle between 0 & 90° i.e. flow vector always
   // points in the forward beam direction.
   if (fEVec[0].Theta() > TMath::PiOver2()) {
      fEVec[0] = -fEVec[0];
   }

   // check we have an orthonormal basis
   // we choose evec[0]=>'z' evec[1]=>'y' evec[2]=>'x'
   // therefore we should have evec[2].Cross(evec[1])=evec[0]
   // if not we change the sign of evec[2]
   if (fEVec[2].Cross(fEVec[1]) != fEVec[0]) fEVec[2] = -fEVec[2];

   // set up azimuthal rotation of CM axes around beam axis in order to put new 'X'-axis in reaction plane
   // we use the phi of the major axis (largest eigenvector)
   fAziReacPlane.SetToIdentity();
   fAziReacPlane.RotateZ(e(1).Phi());

   fFlowReacPlane.SetToIdentity();
   fFlowReacPlane.RotateY(e(1).Theta());
   fFlowReacPlane.RotateZ(e(1).Phi());

   // calculate Gutbrod squeeze angle (PRC42(1990)640
   // defined as the angle by which the middle eigenvector e2
   // needs to be rotated around the e1 flow axis
   // in order to be brought in to the reaction plane
   TVector3 normReac = e(1).Cross(TVector3(0, 0, 1)); //normal to reaction plane: e1 x z
   Double_t angle = TMath::RadToDeg() * e(2).Angle(normReac);
   // on the contrary to Gutbrod et al, we define this angle between 0 and 90 degrees.
   // see Fig. 3 of paper: only 0-90 angles can be defined
   fSqueezeAngle = (angle < 90 ? 90 - angle : angle - 90);

   // now calculate the in-plane and out-of-plane flow according to Fig. 5 of Gutbrod et al
   Double_t a = pow(f(2), 0.5); //semi-axes of ellipsoid given by square roots of eigenvalues
   Double_t b = pow(f(3), 0.5);
   Double_t tan_t = -a / b * TMath::Tan(fSqueezeAngle * TMath::DegToRad());
   Double_t t0 = TMath::ATan(tan_t);
   Double_t inPlane = a * cos(t0) * cos(fSqueezeAngle * TMath::DegToRad()) - b * sin(t0) * sin(fSqueezeAngle * TMath::DegToRad());
   tan_t = a / (b * tan(fSqueezeAngle * TMath::DegToRad()));
   t0 = atan(tan_t);
   Double_t outOfPlane = a * cos(t0) * sin(fSqueezeAngle * TMath::DegToRad()) + b * sin(t0) * cos(fSqueezeAngle * TMath::DegToRad());
   if (inPlane <= 0.) fSqOutRatio = -1.;
   else fSqOutRatio = TMath::Min(1.e+03, outOfPlane / inPlane);

   // calculate sphericity
   Double_t sum_val_prop = f(1) + f(2) + f(3);
   if (sum_val_prop > .1) {
      fSphericity = 3. * (1. - f(1) / sum_val_prop) / 2.;
   }
   else
      fSphericity = -1;

   fCalculated = kTRUE;
}


//________________________________________________________________
void KVFlowTensor::init_KVFlowTensor()
{
   // PRIVATE method
   // Private initialisation method called by all constructors.
   // All member initialisations should be done here.
   //
   // You should also (if your variable calculates several different quantities)
   // set up a correspondance between named values and index number
   // using method SetNameIndex(const Char_t*,Int_t)
   // in order for GetValue(const Char_t*) to work correctly.
   // The index numbers should be the same as in your getvalue_int(Int_t) method.

   fType = KVVarGlob::kOneBody; // this is a 1-body variable

   // reference frame = "CM"
   // weight = NRKE [non-relativistic kinetic energy] (1/2m)

   SetFrame("CM");
   SetOption("weight", "NRKE");

   fTensor.Zero();

   SetNameIndex("FlowAngle", kFlowAngle);
   SetNameIndex("Sphericity", kSphericity);
   SetNameIndex("KinFlowRatio13", kKinFlowRatio13);
   SetNameIndex("KinFlowRatio23", kKinFlowRatio23);
   SetNameIndex("PhiReacPlane", kPhiReacPlane);
   SetNameIndex("SqueezeAngle", kSqueezeAngle);
   SetNameIndex("SqueezeRatio", kSqueezeRatio);
   SetNameIndex("NumberParts", kNumberParts);
}

//________________________________________________________________
void KVFlowTensor::Fill(KVNucleus* n)
{
   // Fill tensor components with nucleus' momentum components in required
   // frame, using the required weight
   // If option "DOUBLE" is set, the chosen weight will be doubled
   // (this is for the case where e.g. only forward hemisphere particles are included)

   Double_t W;
   const KVParticle* frame = n->GetFrame(GetFrame(), kFALSE);
   switch (weight) {
      case kONE:
         W = 1;
         break;
      case kRKE:
         W = 1. / (n->GetMass() * (1 + frame->Gamma()));
         break;
      default:
      case kNRKE:
         W = 1. / (2.*n->GetMass());
         break;
   }
   if (IsOptionGiven("DOUBLE")) W *= 2.;
   for (int i = 0; i < 3; i++) {
      for (int j = i; j < 3; j++) {
         Double_t xx = W * frame->GetMomentum()[i] * frame->GetMomentum()[j];
         fTensor(i, j) += xx;
         if (i != j) fTensor(j, i) += xx;
      }
   }
   ++fNParts;
}

const TRotation& KVFlowTensor::GetAziReacPlaneRotation()
{
   // Returns the azimuthal rotation around the beam axis required
   // to put the X-axis in the reaction plane defined by the beam axis
   // and the major axis (largest eignevalue) of the ellipsoid.
   // The azimuthal angle of the rotation is that of the major axis
   // in the forward direction.

   if (!fCalculated) Calculate();
   return fAziReacPlane;
}

const TRotation& KVFlowTensor::GetFlowReacPlaneRotation()
{
   // Returns composition of two rotations:
   //   - around Z-axis to put X-axis in reaction plane (see GetAziReacPlaneRotation)
   //   - around Y-axis to align Z-axis with flow (major) axis
   // In this rotated frame, theta is polar angle with respect to flow axis
   // and phi is azimuthal angle around flow axis (phi=0,180 => in-plane)

   if (!fCalculated) Calculate();
   return fFlowReacPlane;
}
