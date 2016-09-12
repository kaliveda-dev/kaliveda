//Created by KVClassFactory on Mon May 27 10:38:29 2013
//Author: Guilain ADEMARD & Quentin FABLE

#include "KVVAMOSIDTelescope.h"
#include "KVIdentificationResult.h"
#include "KVVAMOSCodes.h"

ClassImp(KVVAMOSIDTelescope)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSIDTelescope</h2>
<h4>A deltaE-E identification telescope in VAMOS</h4>
<!-- */
// --> END_HTML
//Charged particle identification in VAMOS is handled by KVVAMOSIDTelescope and derived classes.
//A KVIDVAMOSTelescope is an association of one or more detectors which is capable of particle
//identification.
//Although initially conceived in terms of DE-E two-stage telescopes, the identification
//method can be quite different (see KVVAMOSIDCsI for example).
//
//KVVAMOSIDTelescope inherits from KVINDRAIDTelescope to keep the same phylosophy of ID codes and subcodes.
//
// A KVVAMOSIDTelescope has to be exclusively composed of detectors derived from
// KVVAMOSDetector.
////////////////////////////////////////////////////////////////////////////////

KVVAMOSIDTelescope::KVVAMOSIDTelescope() : fDEdet(NULL), fEdet(NULL), fGrid(NULL)
{
   // Default constructor

   // For now, the codes corresponding to correct identification/calibration
   // are code 1, they do not depend on the detectors in the IDTelescope.
   fIDCode  = kIDCode1;
   fECode   = kECode1;
   // Switch between Z only or both (Z,A) identification (when possible)
   fonlyZId = kFALSE;
}
//________________________________________________________________

KVVAMOSIDTelescope::~KVVAMOSIDTelescope()
{
   // Destructor
}
//________________________________________________________________

const Char_t* KVVAMOSIDTelescope::GetArrayName()
{
   // Name of identification telescope given in the form IDV_Det1_Det2.
   // Prefix VID for Vamos IDentification.
   // The detectors are signified by their names i.e. KVDetector::GetName

   if (GetSize()) {
      KVDetector* DEdet = GetDetector(1);
      KVDetector* Edet  = GetDetector(2);

      if (Edet) {
         SetName(Form("VID_%s_%s", DEdet->GetName(), Edet->GetName()));
         SetType(Form("%s_%s", DEdet->GetType(), Edet->GetType()));
      } else {
         SetName(Form("VID_%s", DEdet->GetName()));
         SetType(Form("%s", DEdet->GetType()));
      }
   } else SetName("VID_EMPTY");

   return fName.Data();
}
//________________________________________________________________

Bool_t KVVAMOSIDTelescope::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid KVIDGChIoSi

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   //identification
   Double_t de = (y < 0. ? GetIDMapY() : y);
   Double_t e  = (x < 0. ? GetIDMapX() : x);

   if (fGrid && fGrid->IsIdentifiable(e, de)) {
      fGrid->SetOnlyZId(fonlyZId);
      fGrid->Identify(e, de, IDR);
      IDR->IDquality = fGrid->GetQualityCode();
   }

   // set general ID code for correct identification
   IDR->IDcode = GetIDCode();
   //if point lies above Zmax line, we give Zmax as Z of particle (real Z is >= Zmax)
   //general ID code = kIDCode5 (Zmin)
   if (IDR->IDquality == KVIDZAGrid::kICODE7) {
      IDR->IDcode = GetZminCode();
   }

   //debug
//   std::cout << "KVVAMOSIDTelescope::Identify() tel=" << GetName()  << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() IDquality=" << IDR->IDquality << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() IDcode="    << IDR->IDcode << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() Zident?="   << IDR->Zident << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() PID="       << IDR->PID << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() Z="         << IDR->Z << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() Aident?="   << IDR->Aident << std::endl;
//   std::cout << "KVVAMOSIDTelescope::Identify() A="         << IDR->A << std::endl;

   //    //Identified particles with subcode kID_LeftOfBragg are given
   //    //general ID code kIDCode5 (Zmin).
   //    if (IDR->IDquality==KVIDGChIoSi::k_LeftOfBragg)
   //    {
   //        IDR->IDcode = GetZminCode();
   //    }
   //    //unidentifiable particles with subcode kID_BelowSeuilSi are given
   //    //general ID code kIDCode5 (Zmin) and we estimate Zmin from energy
   //    //loss in ChIo
   //    if (IDR->IDquality==KVIDGChIoSi::k_BelowSeuilSi)
   //    {
   //        IDR->IDcode = GetZminCode();
   //        IDR->Z = fDEdet->FindZmin();
   //        IDR->SetComment("point to identify left of Si threshold line (bruit/arret ChIo?)");
   //    }
   //    if(IDR->IDquality==KVIDGChIoSi::k_RightOfEmaxSi) IDR->SetComment("point to identify has E_Si > Emax_Si i.e. codeur is saturated. Unidentifiable");

   return kTRUE;
}
//________________________________________________________________

void KVVAMOSIDTelescope::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   fDEdet = (KVVAMOSDetector*)GetDetector(1);
   fEdet  = (KVVAMOSDetector*)GetDetector(2);
   fGrid  = (KVIDZAGrid*)GetIDGrid(1);
   if (fDEdet && fEdet && fGrid) {
      fGrid->Initialize();
      SetBit(kReadyForID);
   } else
      ResetBit(kReadyForID);
}
//________________________________________________________________

//const Char_t *KVVAMOSIDTelescope::GetIDSubCodeString(KVIDSubCode & concat) const{
//
// Warning("GetIDSubCodeString","TO BE IMPLEMENTED");
//}
