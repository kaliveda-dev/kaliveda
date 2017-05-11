//Created by KVClassFactory on Wed Jan 18 14:44:36 2017
//Author: John Frankland,,,

#include "KVRotatedTensor.h"

ClassImp(KVRotatedTensor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVRotatedTensor</h2>
<h4>Tensor analysis using multiple rotations of events around beam axis</h4>
<!-- */
// --> END_HTML
// For each event, N random phi rotations around the beam axis are performed
// in the centre of mass frame "CM".
// The N*M particles in this pseudo-event are then used to fill the tensor.
//
// Set number of rotations with parameter "NROT" [default=10]
////////////////////////////////////////////////////////////////////////////////

KVRotatedTensor::KVRotatedTensor(void): KVTensPCM()
{
   // Default constructor
   init_KVRotatedTensor();
   SetName("KVRotatedTensor");
   SetTitle("A KVRotatedTensor");
}

//_________________________________________________________________
KVRotatedTensor::KVRotatedTensor(const Char_t* nom): KVTensPCM(nom)
{
   // Constructor with a name for the global variable
   init_KVRotatedTensor();
}

//_________________________________________________________________
KVRotatedTensor::KVRotatedTensor(const KVRotatedTensor& a): KVTensPCM()
{
   // Copy constructor
   init_KVRotatedTensor();
   a.Copy(*this);
}

//_________________________________________________________________
KVRotatedTensor::~KVRotatedTensor(void)
{
   // Destructor
}

//_________________________________________________________________
void KVRotatedTensor::Copy(TObject& a) const
{
   // Copy properties of 'this' object into the KVVarGlob object referenced by 'a'

   KVTensPCM::Copy(a);// copy attributes of KVVarGlob base object

   KVRotatedTensor& aglob = (KVRotatedTensor&)a;
   aglob.fNROT = fNROT;
}

//_________________________________________________________________
KVRotatedTensor& KVRotatedTensor::operator = (const KVRotatedTensor& a)
{
   // Operateur =

   if (&a != this) a.Copy(*this);
   return *this;
}

//_________________________________________________________________
void KVRotatedTensor::Init(void)
{
   // Initialisation of internal variables, called once before beginning treatment

   KVTensPCM::Init();
   if (IsParameterSet("NROT")) fNROT = GetParameter("NROT");
}

//________________________________________________________________
void KVRotatedTensor::init_KVRotatedTensor()
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
   fType = KVVarGlob::kNBody; // this is a N-body variable
   fNROT = 10;
   SetFrame("KVRotatedTensor_rotatedFrame");//calculate tensor in rotated frame
}

//____________________________________________________________________________//

void KVRotatedTensor::FillN(KVEvent* e)
{
   // Calculation of contribution to N-body global variable of particles in event e.

   for (int i = 0; i < fNROT; ++i) {

      // random phi rotation
      fRot.RotateZ(gRandom->Uniform(TMath::TwoPi()));

      // turn event
      e->SetFrame("KVRotatedTensor_rotatedFrame", "CM", fRot);

      e->ResetGetNextParticle();
      KVNucleus* nuc;
      while ((nuc = e->GetNextParticle("ok"))) {
         FillWithCondition(nuc);
      }

   }
}

//____________________________________________________________________________//
