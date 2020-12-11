//Created by KVClassFactory on Tue Jun 30 12:41:37 2015
//Author: John Frankland,,,

#include "KVFlowTensor.h"
#include "TMatrixDUtils.h"
#include "TMatrixDSymEigen.h"

ClassImp(KVFlowTensor)


KVFlowTensor::KVFlowTensor(void): KVVarGlob(), fTensor(3), fEVal(3)
{
   // Default constructor
   init_KVFlowTensor();
   SetName("KVFlowTensor");
   SetTitle("Kinetic energy flow tensor of Gyulassy et al.");
}

//_________________________________________________________________
KVFlowTensor::KVFlowTensor(const Char_t* nom): KVVarGlob(nom), fTensor(3), fEVal(3)
{
   // Constructor with a name for the global variable
   init_KVFlowTensor();
}

//_________________________________________________________________
KVFlowTensor::KVFlowTensor(const KVFlowTensor& a): KVVarGlob(), fTensor(3), fEVal(3)
{
   // Copy constructor
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

   KVFlowTensor& aglob = (KVFlowTensor&)a;
   KVVarGlob::Copy(a);// copy attributes of KVVarGlob base object
   aglob.Init();
   aglob.fTensor = fTensor;
   aglob.fNParts = fNParts;
   if (fCalculated) aglob.Calculate();
}

//_________________________________________________________________
KVFlowTensor& KVFlowTensor::operator = (const KVFlowTensor& a)
{
   // Operateur =

   if (&a != this) a.Copy(*this);
   return *this;
}

//_________________________________________________________________
void KVFlowTensor::Init(void)
{
   // Initialisation of internal variables, called once before beginning treatment

   Reset();
   TString wgt = GetOptionString("weight");
   if (wgt == "RKE") weight = kRKE;
   else if (wgt == "ONE" || wgt == "" || wgt == "1") weight = kONE;
   else weight = kNRKE;
   SetFrame("CM");
}

//_________________________________________________________________
void KVFlowTensor::Reset(void)
{
   // Reset internal variables, called before treatment of each event
   fTensor.Zero();
   fNParts = 0;
   fCalculated = false;
}

Double_t KVFlowTensor::getvalue_int(Int_t index) const
{
   // \param index one of the following values:
   //
   //   Name           | Index                       | Definition
   //   ---------------|-----------------------------|---------------------------------------------
   //  "FlowAngle"     |KVFlowTensor::kFlowAngle     |polar angle of \f$\vec{e_1}\f$
   //  "Sphericity"    |KVFlowTensor::kSphericity    |\f$\frac{3}{2}(1-\lambda_1)\f$
   //  "Coplanarity"   |KVFlowTensor::kCoplanarity   |\f$\frac{\sqrt{3}}{2}(\lambda_2-\lambda_3)\f$
   //  "KinFlowRatio13"|KVFlowTensor::kKinFlowRatio13|\f$\sqrt{\frac{\lambda_1}{\lambda_3}}\f$
   //  "KinFlowRatio23"|KVFlowTensor::kKinFlowRatio23|\f$\sqrt{\frac{\lambda_2}{\lambda_3}}\f$
   //  "PhiReacPlane"  |KVFlowTensor::kPhiReacPlane  |\f$\phi\f$ angle of \f$\vec{e_1}\f$
   //  "SqueezeAngle"  |KVFlowTensor::kSqueezeAngle  |see Gyulassy et al
   //  "SqueezeRatio"  |KVFlowTensor::kSqueezeRatio  |see Gyulassy et al
   //  "NumberParts"   |KVFlowTensor::kNumberParts   |number of particles used in tensor
   //
   // \return value associated with given index
   switch (index) {
      case kFlowAngle:
         return TMath::RadToDeg() * e(1).Theta();
         break;

      case kSphericity:
         return fSphericity;
         break;

      case kCoplanarity:
         return fCoplanarity;
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

   TMatrixDSymEigen diagonalize(fTensor);
   TMatrixD evectors = diagonalize.GetEigenVectors();
   fEVal = diagonalize.GetEigenValues();

   for (int i = 0; i < 3; i++) {
      TMatrixDColumn col(evectors, i);
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

   // calculate sphericity & coplanarity
   sum_val_prop = f(1) + f(2) + f(3);
   if (sum_val_prop > .1) {
      fSphericity = 1.5 * (1. - f(1) / sum_val_prop);
      fCoplanarity = sqrt(3.) * (f(2) - f(3)) / sum_val_prop / 2.;
   }
   else {
      fSphericity = fCoplanarity = -1;
   }
   fCalculated = kTRUE;
}


//________________________________________________________________
void KVFlowTensor::init_KVFlowTensor()
{
   //Common initialisation for all constructors
   //
   // Default frame is set to "CM".
   //
   // Default weight is set to NRKE if none was given
   //  by calling method SetOption("weight","[your choice]")
   //   with one of the following choices:
   //  Option      | Name                                 | Weight
   //  ------------|--------------------------------------|---------------------------------------------------
   //  "NRKE"      | non-relativistic KE tensor (default) | \f$w_{\nu}=(2m_{\nu})^{-1}\f$
   //  "RKE"       | relativistic KE tensor               | \f$w_{\nu}={1 \over {m_{\nu}(\gamma_{\nu}+1)}}\f$
   //  "ONE","1",""| sphericity tensor                    | \f$w=1\f$
   //
   // Default maximum number of branches for TTree is 3: FlowAngle, Sphericity, and Coplanarity

   fType = KVVarGlob::kOneBody; // this is a 1-body variable

   SetFrame("CM");
   SetOption("weight", "NRKE");

   fTensor.Zero();

   SetNameIndex("FlowAngle", kFlowAngle);
   SetNameIndex("Sphericity", kSphericity);
   SetNameIndex("Coplanarity", kCoplanarity);
   SetNameIndex("KinFlowRatio13", kKinFlowRatio13);
   SetNameIndex("KinFlowRatio23", kKinFlowRatio23);
   SetNameIndex("PhiReacPlane", kPhiReacPlane);
   SetNameIndex("SqueezeAngle", kSqueezeAngle);
   SetNameIndex("SqueezeRatio", kSqueezeRatio);
   SetNameIndex("NumberParts", kNumberParts);

   SetMaxNumBranches(3);

   fNParts = 0;
}

void KVFlowTensor::fill(const KVNucleus* n)
{
   // Fill tensor components with nucleus' momentum components using the required weight
   //
   // If option "DOUBLE" is set, the chosen weight will be doubled
   // (this is for the case where e.g. only forward hemisphere particles are included)

   Double_t W;
   switch (weight) {
      case kONE:
         W = 1;
         break;
      case kRKE:
         W = 1. / (n->GetMass() * (1 + n->Gamma()));
         break;
      default:
      case kNRKE:
         W = 1. / (2.*n->GetMass());
         break;
   }
   if (IsOptionGiven("DOUBLE")) W *= 2.;
   for (int i = 0; i < 3; i++) {
      for (int j = i; j < 3; j++) {
         Double_t xx = W * n->GetMomentum()[i] * n->GetMomentum()[j];
         fTensor(i, j) += xx;
         if (i != j) fTensor(j, i) += xx;
      }
   }
   ++fNParts;
}

const TRotation& KVFlowTensor::GetAziReacPlaneRotation() const
{
   // Returns the azimuthal rotation around the beam axis required
   // to put the \f$x\f$-axis in the reaction plane defined by the beam axis
   // and the major axis (largest eigenvalue) of the ellipsoid.
   //
   // The azimuthal angle of the rotation is that of the major axis
   // in the forward direction.
   //
   // Note that this rotation applies to whatever frame was used to calculate
   // the tensor (default: "CM")

   return fAziReacPlane;
}

const TRotation& KVFlowTensor::GetFlowReacPlaneRotation() const
{
   // Returns composition of two rotations:
   //   - around \f$z\f$-axis to put \f$x\f$-axis in reaction plane (see GetAziReacPlaneRotation())
   //   - around \f$y\f$-axis to align \f$z\f$-axis with flow (major) axis
   //
   // In this rotated frame, \f$\theta\f$ is polar angle with respect to flow axis
   // and \f$\phi\f$ is azimuthal angle around flow axis (\f$\phi\f$=0,180 => in-plane)
   //
   // Note that this rotation applies to whatever frame was used to calculate
   // the tensor (default: "CM")

   return fFlowReacPlane;
}

void KVFlowTensor::Print(Option_t* opt) const
{
   // if opt="tensor", just print contents of tensor

   if (strcmp(opt, "tensor")) KVVarGlob::Print();
   std::cout << "Number of particles = " << fNParts << std::endl;
   fTensor.Print();
}

