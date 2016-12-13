#include "KVIDHarpeeSiCsI_e503.h"

ClassImp(KVIDHarpeeSiCsI_e503)

KVIDHarpeeSiCsI_e503::KVIDHarpeeSiCsI_e503() :
#if __cplusplus < 201103L
   records_(NULL),
   base_id_result_(NULL),
   minimiser_(NULL),
#else
   records_(nullptr),
   base_id_result_(nullptr),
   minimiser_(nullptr),
#endif
   kInitialised_(kFALSE)
{

   fIDCode = kIDCode3;
   Amin_   = 0;
   ICode_  = -1;
   MCode_  = -1;
}

KVIDHarpeeSiCsI_e503::~KVIDHarpeeSiCsI_e503()
{
#if __cplusplus < 201103L
   if (minimiser_) {
      delete minimiser_;
      minimiser_ = NULL;
   }

   if (base_id_result_) {
      delete base_id_result_;
      base_id_result_ = NULL;
   }

   if (records_) {
      delete records_;
      records_ = NULL;
   }
#endif
}

void KVIDHarpeeSiCsI_e503::Copy(TObject& obj) const
{
   // Call parent class copy method (takes care of the inherited stuff)
   KVIDHarpeeSiCsI::Copy(obj);

   // Then copy any desired member variables (not copied by the parent method)
   //KVIDHarpeeSiCsI_e503 *casted_obj = static_cast<KVIDHarpeeSiCsI_e503*>(&obj);
}

Bool_t KVIDHarpeeSiCsI_e503::Init()
{
   if (kInitialised_) return kTRUE;

#if __cplusplus < 201103L
   base_id_result_ = new KVIdentificationResult();
   minimiser_ = new SiliconEnergyMinimiser();
#else
   base_id_result_.reset(new KVIdentificationResult());
   minimiser_.reset(new SiliconEnergyMinimiser());
#endif

   minimiser_->Init();
   kInitialised_ = kTRUE;

   SetBit(kReadyForID);

   return kTRUE;
}

Bool_t KVIDHarpeeSiCsI_e503::Identify(KVIdentificationResult* idr, Double_t x,
                                      Double_t y)
{
   assert(idr);

   //IMPORTANT !!! For the moment we need to reset the kInitialised flag
   //each time the Identify method is called.
   //Else each time the same KVIDHarpeeSiCsI_e503 is called (for ex: VID_SI_06_CSI36) more
   //than once (typically in a KVIVSelector), it will be considered as already initialised and
   //the base_id_result_ will keep the information of the first identification !!!
   kInitialised_ = kFALSE;
   Init();

   if (x < 0.) x = GetIDMapX();
   if (y < 0.) y = GetIDMapY();

   if (x == 0.) return kFALSE;
   if (y == 0.) return kFALSE;

   idr->IDOK = kFALSE;
   idr->IDattempted = kTRUE;

   //Base class performs the preliminary identification routine. We do this
   //after the initialisation check as we require base_id_result_.
   //
   //For e503 Si-CsI identification, the charge Z is estimated using KVIDZAGrid. As the mass A is mandatory
   //in order to calibrate the CsI energy, it is estimated using the minimiser (by minimising the difference
   //between simulated and real silicon energies, see iliconEnergyMinimiser class).
   //
   //Update: should maybe try to use directly the computed CsI energy in MeV instead of setting the mass
   //found by minimisation process to the KVVAMOSReconNuc and compute again energy.

   assert(base_id_result_);

#if __cplusplus < 201103L
   KVIDHarpeeSiCsI::Identify(base_id_result_, x, y);
#else
   KVIDHarpeeSiCsI::Identify(base_id_result_.get(), x, y);
#endif

   ICode_ = base_id_result_->IDquality;

   // Set the idcode and type for this telescope
   idr->SetIDType(base_id_result_->GetIDType());
   idr->IDcode = fIDCode;
   idr->Zident = base_id_result_->Zident;
   idr->Z      = base_id_result_->Z;
   idr->Aident = base_id_result_->Aident;
   idr->A      = base_id_result_->A;
   idr->PID    = base_id_result_->PID; //from Z ident

   //Z not found
   if (!base_id_result_->Zident) {
      MCode_ = kMCode1;
      idr->IDquality = kMCode1;
      idr->IDOK = kFALSE;
      idr->SetComment("Z not found from grid (base_id_result_::Zident=kFALSE)");
      return kFALSE;
   }

   assert((idr->Z > 0) && (idr->Z < 120));
   assert(idr->PID > 0.);

   // At this point Z should be well identified by the base class
   minimiser_->SetIDTelescope(GetName());
   Amin_ = minimiser_->Minimise(idr->Z, y, x);

   if ((idr->A > 0)) {  //Z from grid OK, A from minimiser OK
      //We take the A from minimiser as the identified mass.
      //Save id results in standard way (like the A was identified from grid),
      //so the usual KVReconstructedNucleus::SetIdentification() method
      //can be used to save the results in KVVAMOSReconNuc::IdentifyZ().
      //This means that idr::Aident=kTRUE and idr::PID=Amin_
      MCode_ = kMCode0;
      idr->IDquality = kMCode0;
      idr->IDOK   = kTRUE;
      idr->Aident = kTRUE;
      idr->A      = Amin_;
      idr->PID    = Amin_;
      idr->SetComment("Z found with grid, A from minimiser found");

   }

   else { //Z from grid OK, A not found from minimiser
      MCode_ = kMCode2;
      idr->IDquality = kMCode2;
      idr->IDOK = kFALSE;
      idr->SetComment("Z found from grid, A from minimiser not found");
      return kFALSE;
   }

//   //debug
//   Info("Identify","ident successful, idr info follow...");
//   idr->Print();
//   std::cout << "idr::IDOK=" << idr->IDOK << std::endl;
//   std::cout << "idr::IDquality=" << idr->IDquality << std::endl;
//   std::cout << "idr::IDcode=" << idr->IDcode << std::endl;
//   std::cout << "idr::Zident=" << idr->Zident << std::endl;
//   std::cout << "idr::Z=" << idr->Z << std::endl;
//   std::cout << "idr::Aident=" << idr->Aident << std::endl;
//   std::cout << "idr::A=" << idr->A << std::endl;
//   std::cout << "idr::PID=" << idr->PID << std::endl;

   return kTRUE;
}

Bool_t KVIDHarpeeSiCsI_e503::SetIDCorrectionParameters(
   const KVRList* const records
)
{
   if (!records) {
      Error("SetIDCorrectionParameters",
            "Supplied record list is a null pointer");
      return kFALSE;
   }

#if __cplusplus < 201103L
   if (records_) {
      delete records_;
      records_ = NULL;
   }

   records_ = new KVList(kFALSE);
#else
   records_.reset(new KVList(kFALSE));
#endif

   records_->AddAll(records);
   return kTRUE;
}

const KVList* KVIDHarpeeSiCsI_e503::GetIDCorrectionParameters() const
{
#if __cplusplus < 201103L
   return records_;
#else
   return records_.get();
#endif
}

