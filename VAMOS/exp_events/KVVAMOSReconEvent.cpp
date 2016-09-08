//Created by KVClassFactory on Thu Jun 13 08:24:29 2013
//Author: Guilain ADEMARD

#include "KVVAMOSReconEvent.h"
#include "KVVAMOSReconNuc.h"
#include "KVVAMOS.h"
#include "KVTarget.h"

ClassImp(KVVAMOSReconEvent)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSReconEvent</h2>
<h4>Event reconstructed from energy losses in VAMOS spectrometer</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//________________________________________________________________

void KVVAMOSReconEvent::init()
{
   //Default initialisations
   fCodeMask = NULL;
}
//________________________________________________________________

KVVAMOSReconEvent::KVVAMOSReconEvent(Int_t mult, const char* classname) : KVReconstructedEvent(mult, classname)
{
   init();
}
//________________________________________________________________

KVVAMOSReconEvent::~KVVAMOSReconEvent()
{
   // Destructor
   SafeDelete(fCodeMask);
}
//________________________________________________________________

void KVVAMOSReconEvent::Streamer(TBuffer& R__b)
{
   //Stream an object of class KVVAMOSReconEvent.
   //We loop over the newly-read nuclei in order to set their
   //IsOK() status by comparison with the event's code mask

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVVAMOSReconEvent::Class(), this);
      KVVAMOSReconNuc* nuc;
      while ((nuc = GetNextNucleus())) {
         if (CheckCodes(nuc->GetCodes()))
            nuc->SetIsOK();
         else
            nuc->SetIsOK(kFALSE);
      }
   } else {
      R__b.WriteClassBuffer(KVVAMOSReconEvent::Class(), this);
   }
}
//________________________________________________________________

void KVVAMOSReconEvent::AcceptECodes(UChar_t code)
{
   //Define the calibration codes for VAMOS's detectors that you want to include
   //in your analysis.
   //Example: to keep only ecodes 1 and 2, use
   //        event.AcceptECodes( kECode2 | kECode3 );
   //If the nucleus reconstructed in VAMOS have the correct E codes it will
   //have IsOK() = kTRUE.
   //If this nucleus does not have IsOK() = kTRUE then the method GetNextNucleus("ok")
   //will return a null pointer.
   //
   //To remove any previously defined acceptable calibration codes, use AcceptECodes(0).
   //
   //N.B. : this method is preferable to using directly the KVAMOSCodes pointer
   //of the nucleus as the 'IsOK' status of this nucleus of the current event
   //is automatically updated when the code mask is changed using this method.

   GetCodeMask()->SetEMask(code);
   KVVAMOSReconNuc* nuc = NULL;
   while ((nuc = GetNextNucleus())) {
      if (CheckCodes(nuc->GetCodes()))
         nuc->SetIsOK();
      else
         nuc->SetIsOK(kFALSE);
   }
}
//________________________________________________________________

void KVVAMOSReconEvent::AcceptFPCodes(UInt_t code)
{
   //Define the VAMOS Focal plane Position reconstruction (FP) codes that you want
   //to include in your analysis.
   //Example: to keep only codes 1 and 2, use
   //        event.AcceptFPCodes( kFPCode2 | kFPCode3 );
   //If the nucleus reconstructed in VAMOS have the correct FP codes it will
   //have IsOK() = kTRUE.
   //If this nucleus does not have IsOK() = kTRUE then the method GetNestNucleus("ok")
   //will return a null pointer.
   //
   //To remove any previously defined acceptable FP codes, use AcceptFPCodes(0).
   //
   //N.B. : this method is preferable to using directly the KVAMOSCodes pointer
   //of the nucleus as the 'IsOK' status of this nucleus of the current event
   //is automatically updated when the code mask is changed using this method.

   GetCodeMask()->SetFPMask(code);
   KVVAMOSReconNuc* nuc = NULL;
   while ((nuc = GetNextNucleus())) {
      if (CheckCodes(nuc->GetCodes()))
         nuc->SetIsOK();
      else
         nuc->SetIsOK(kFALSE);
   }
}
//________________________________________________________________

void KVVAMOSReconEvent::AcceptIDCodes(UShort_t code)
{
   //Define the Z-identification codes that you want to include
   //in your analysis.
   //Example: to keep only ID codes 1 and 2, use
   //        event.AcceptIDCodes( kIDCode1 | kIDCode2 );
   //If the nucleus reconstructed in VAMOS have the correct ID codes it will
   //have IsOK() = kTRUE.
   //If this nucleus does not have IsOK() = kTRUE then the method GetNextNucleus("ok")
   //will return a null pointer.
   //
   //To remove any previously defined acceptable identification codes, use AcceptIDCodes(0).
   //
   //N.B. : this method is preferable to using directly the KVAMOSCodes pointer
   //of the nucleus as the 'IsOK' status of this nucleus of the current event
   //is automatically updated when the code mask is changed using this method.

   GetCodeMask()->SetIDMask(code);
   KVVAMOSReconNuc* nuc = NULL;
   while ((nuc = GetNextNucleus())) {
      if (CheckCodes(nuc->GetCodes()))
         nuc->SetIsOK();
      else
         nuc->SetIsOK(kFALSE);
   }
}
//________________________________________________________________

void KVVAMOSReconEvent::AcceptTCodes(UShort_t code)
{
   //Define the Time of Flight codes that you want to include
   //in your analysis.
   //Example: to keep only T codes 1 and 2, use
   //        event.AcceptTCodes( kTCode1 | kTCode2 );
   //If the nucleus reconstructed in VAMOS have the correct ID codes it will
   //have IsOK() = kTRUE.
   //If this nucleus does not have IsOK() = kTRUE then the method GetNextNucleus("ok")
   //will return a null pointer.
   //
   //To remove any previously defined acceptable Time of Flight codes, use AcceptTCodes(0).
   //
   //N.B. : this method is preferable to using directly the KVAMOSCodes pointer
   //of the nucleus as the 'IsOK' status of this nucleus of the current event
   //is automatically updated when the code mask is changed using this method.

   GetCodeMask()->SetTMask(code);
   KVVAMOSReconNuc* nuc = NULL;
   while ((nuc = GetNextNucleus())) {
      if (CheckCodes(nuc->GetCodes()))
         nuc->SetIsOK();
      else
         nuc->SetIsOK(kFALSE);
   }
}
//________________________________________________________________

KVVAMOSReconNuc* KVVAMOSReconEvent::GetNextNucleus(Option_t* opt)
{
   //Use this method to iterate over the list of nuclei in the event.
   //
   //If opt="ok" only the nuclei whose ID codes, FP codes, and E codes correspond to those set
   //using AcceptIDCodes, AccepFPCodes and AcceptECodes will be returned, in the order in which
   //they appear in the event.
   //
   //After the last nucleus in the event GetNextNucleus() returns a null pointer and
   //resets itself ready for a new iteration over the nucleus list.
   //Therefore, to loop over all nucleus in an event, use a structure like:
   //
   //      KVVAMOSReconNuc* n;
   //      while( (n = GetNextNucleus() ){
   //              ... your code here ...
   //      }

   return (KVVAMOSReconNuc*) KVEvent::GetNextParticle(opt);
}
//________________________________________________________________

KVVAMOSReconNuc* KVVAMOSReconEvent::GetNucleus(Int_t n_nuc) const
{
//
//Access to event member with index n_nuc (1<=n_nuc<=fMult)
//

   return (KVVAMOSReconNuc*)(KVReconstructedEvent::GetParticle(n_nuc));
}
//________________________________________________________________

void KVVAMOSReconEvent::IdentifyEvent_A()
{

   Warning("IdentifyEvent_A", "TO BE IMPLEMENTED");
}
//________________________________________________________________

void KVVAMOSReconEvent::IdentifyEvent_Z()
{
   //If the nuclei measured in VAMOS have not been previously Z-identified
   //it will be Z-identified.

   KVReconstructedEvent::IdentifyEvent();
}
//________________________________________________________________
void KVVAMOSReconEvent::IdentAndCalibEvent()
{

   //If the nuclei measured in VAMOS have not been previously Z-identified
   //it will be Z-identified.

   // In order to make sure that target energy loss corrections are correctly
   // calculated, we first set the state of the target in the current multidetector

   KVTarget* t = gMultiDetArray->GetTarget();
   if (t) {
      t->SetIncoming(kFALSE);
      t->SetOutgoing(kTRUE);
   }


   //debug
   std::cout << "#############KVVAMOSReconEvent -> Identifying new event !##############" << std::endl;

   KVVAMOSReconNuc* d;
   while ((d = GetNextNucleus())) {

      //debug
      std::cout << "==>KVVAMOSReconEvent::IdentAndCalibEvent -> new particle in the event! GetStatus()=" << d->GetStatus() << "<==" << std::endl;


      //-----------------------
      // Z-identification
      //-----------------------
      if (!d->IsZidentified()) {
         if (d->GetStatus() == KVReconstructedNucleus::kStatusOK) {
            // identifiable particles
            d->IdentifyZ();
         } else if (d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
            // particles stopped in first member of a telescope
            // estimation of Z (minimum) from energy loss (if detector is calibrated)
            UInt_t zmin = d->GetStoppingDetector()->FindZmin(-1., d->GetMassFormula());
            if (zmin) {
               d->SetZ(zmin);
               d->SetIsZidentified();
               // "Identifying" telescope is taken from list of ID telescopes
               // to which stopping detector belongs
               d->SetIdentifyingTelescope((KVIDTelescope*)d->GetStoppingDetector()->GetIDTelescopes()->At(0));
               d->SetIDCode(kIDCode5); // Zmin
            }
         }
      }

      //debug
      std::cout << "#KVVAMOSReconEvent::IdentAndCalib() -> End of Z identification for the particle... results follow:" << std::endl;
      std::cout << "IDcode=" << d->GetIDCode() << std::endl;
      //std::cout << "Zident=" <<
      std::cout << "Z="      << d->GetZ()      << std::endl;
      std::cout << "RealZ="  << d->GetRealZ()  << std::endl;
      std::cout << "A="      << d->GetA()      << std::endl;
      std::cout << "RealA="  << d->GetRealA()  << std::endl;
      std::cout << "RealQ="  << d->GetRealQ()  << std::endl;
      std::cout << "RealAoQ=" << d->GetRealAoverQ() << std::endl;

      //-----------------------
      // Calibration
      //-----------------------
      if (d->IsZidentified() && !d->IsCalibrated()) d->Calibrate();

      //--------------------------------------------------------------------------
      //All particles which have previously Z-identified (IsZidentified=kTRUE) and
      // calibrated (IsCalibrated=kTRUE) will be Q and A identified.
      //--------------------------------------------------------------------------
      if (d->IsZidentified() && d->IsCalibrated() && !d->IsQandAidentified()) {
         d->IdentifyQandA();

         //debug
         std::cout << "#KVVAMOSReconEvent::IdentAndCalib() -> End of A identification for the particle... results follow:" << std::endl;
         std::cout << "IDcode=" << d->GetIDCode() << std::endl;
         std::cout << "Z="      << d->GetZ()      << std::endl;
         std::cout << "RealZ="  << d->GetRealZ()  << std::endl;
         std::cout << "A="      << d->GetA()      << std::endl;
         std::cout << "RealA="  << d->GetRealA()  << std::endl;
         std::cout << "RealQ="  << d->GetRealQ()  << std::endl;
         std::cout << "RealAoQ=" << d->GetRealAoverQ() << std::endl;
         std::cout << "Energy_ChIo=" << d->GetEnergy("CHI") << std::endl;
         std::cout << "Energy_Si="  << d->GetEnergy("SI")  << std::endl;
         std::cout << "Energy_CsI=" << d->GetEnergy("CSI") << std::endl;
      }
   }
}
//________________________________________________________________

Bool_t KVVAMOSReconEvent::IsOK()
{
   //Returns kTRUE if event is acceptable for analysis.
   //This means that the multiplicity of nuclei with IsOK()=kTRUE (having good ID codes etc.)
   //is at least equal to the multiplicity trigger of the (experimental) event.

   if (gVamos) {
      if (gVamos->GetTrigger())
         return (GetMult("ok") >= gVamos->GetTrigger());
      else
         return (GetMult("ok") > 0);
   } else {
      return (KVEvent::IsOK());
   }
   return kTRUE;
}
//________________________________________________________________

void KVVAMOSReconEvent::Print(Option_t* option) const
{
   //Print out list of nuclei in the event.
   //If option="ok" only nuclei with IsOK=kTRUE are included.

   cout << GetTitle() << endl;  //system
   cout << GetName() << endl;   //run
   cout << "Event number: " << GetNumber() << endl << endl;
   cout << "MULTIPLICITY = " << ((KVVAMOSReconEvent*) this)->
        GetMult(option) << endl << endl;

   KVVAMOSReconNuc* nuc = NULL;
   while ((nuc = ((KVVAMOSReconEvent*) this)->GetNextNucleus(option))) {
      nuc->Print();
   }
}
//________________________________________________________________

void KVVAMOSReconEvent::ReconstructEvent(KVMultiDetArray* mda, KVDetectorEvent* kvde)
{
   //
   // Reconstruction of detected nuclei
   //
   // - loop over last stage of group telescopes
   // - if any detector is hit, construct "nucleus" from energy losses in all detectors
   //   directly in front of it.
   // - loop over next to last stage...if any detector hit NOT ALREADY IN A "NUCLEUS"
   //   construct "nucleus" etc. etc.
   //
   //   Then the trajectory (theta, phi, Brho, path) of each reconstructed nucleus is reconstructed
   //   from positions (Xf, Yf, Theta_f, Phi_f) mesured at the focal plane.

   mda->ReconstructEvent(this, kvde);

   KVVAMOSReconNuc* nuc = NULL;
// UChar_t Nnuc = 0;
   while ((nuc = GetNextNucleus())) {
//    if( Nnuc > 1 ){
//       Warning("ReconstrucEvent","Ambiguous trajectory reconstruction,  more than one nucleus in event %d", GetNumber());
//       break;
//    }
      nuc->ReconstructTrajectory();
//    Nnuc++;
   }
}
