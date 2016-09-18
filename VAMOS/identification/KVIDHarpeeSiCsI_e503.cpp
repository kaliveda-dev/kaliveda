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
   //the base_id_result_ will keep the information of the former identification !!!
   kInitialised_ = kFALSE;
   Init();

   if (x < 0.) x = GetIDMapX();
   if (y < 0.) y = GetIDMapY();

   if (x == 0.) return kFALSE;
   if (y == 0.) return kFALSE;

   idr->IDOK = kFALSE;
   idr->IDattempted = kTRUE;

   // Base class performs the preliminary identification routine. We do this
   // after the initialisation check as we require base_id_result_.
   //
   // Only Z identification is required for Si-CsI identification.
   // This is important for the e503 experiment, in order to avoid the
   // loss of the PID for Z when a (Z,A) identification is called.
   // Furthermore for e503 the minimizer is used to estimate A,
   // thus IDR->Aident will often be kTRUE, and the further call
   // to KVReconstructedNucleus::SetIdentification() will not set the RealZ value
   // but only the RealA value.
   assert(base_id_result_);
   SetOnlyZId(kFALSE);

#if __cplusplus < 201103L
   KVIDHarpeeSiCsI::Identify(base_id_result_, x, y);
#else
   KVIDHarpeeSiCsI::Identify(base_id_result_.get(), x, y);
#endif

   ICode_ = base_id_result_->IDquality;

   // Set the idcode and type for this telescope
   idr = base_id_result_; //copy infos from base identification
   idr->IDcode = fIDCode;
   idr->SetIDType(GetType());

   if (!base_id_result_->Zident) {
      MCode_ = kMCode5;
      idr->IDquality = kMCode5;
      return kFALSE;
   }

   assert((idr->Z > 0) && (idr->Z < 120));
   assert(idr->PID > 0.);

   // At this point Z should be well identified by the base class
   minimiser_->SetIDTelescope(GetName());
   Amin_ = minimiser_->Minimise(idr->Z, y, x);

   //Consider all cases for identification
   //As long as a mass value is OK (either from KVIDZAGrid or Minimiser),
   //we consider the identification went well
   if ((Amin_ > 0) && (base_id_result_->Aident)) { //Minimisation is OK and (Z,A) found with KVIDZAGrid
      if (((Amin_ > 0) == (idr->A))) { //A from minimisation = A from KVIDZAGrid
         MCode_ = kMCode0;
         idr->IDquality = kMCode0;
         idr->IDOK = kTRUE;
      } else { //A from minimisation != A from KVIDZAGrid
         MCode_ = kMCode1;
         idr->IDquality = kMCode1;
         idr->IDOK = kTRUE;
      }
   }

   else {
      if ((Amin_ > 0)) { //Minimisation is OK, Z found but not A from KVIDZAGrid
         MCode_ = kMCode2;
         idr->IDquality = kMCode2;
         idr->IDOK = kTRUE;
      }

      else {
         if ((base_id_result_->Aident)) {//Minimisation not OK, (Z,A) found with KVIDZAGrid
            MCode_ = kMCode3;
            idr->IDquality = kMCode3;
            idr->IDOK = kTRUE;
         }

         else { //Minimisation not OK, Z but not A found from KVIDZAGrid
            MCode_ = kMCode4;
            idr->IDquality = kMCode4;
            idr->IDOK = kFALSE;

            return kFALSE;
         }
      }
   }

   std::cout

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

