//Created by KVClassFactory on Fri Dec 13 11:56:00 2013
//Author: Guilain ADEMARD

#include "KVIDHarpeeICSi_e503.h"

ClassImp(KVIDHarpeeICSi_e503)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDHarpeeICSi_e503</h2>
<h4>DeltaE-E identification with Harpee's IC-Si detectors for e503
experiment</h4>
<!-- */
// --> END_HTML
//
// This telescope has to be exclusively composed of detectors derived from
// KVVAMOSDetector.
//
// The code corresponding to correct identification by this type of telescope
// is kIDCode4 ( the same as for INDRA's telescope_CSI )
//
// This KVIDHarpeeICSi_e503 class uses, as Y coordinate for identification, the
// calibrated energy of the fired acquisition parameter of KVHarpeeIC i.e.
// the energy loss in the crossed gas segment (see doc. of GetIDMapY for more
// details). So it is very important to indicate the acq. parameters
// (CHI_01, CHI_02, ..., CHI_07 )  to be used as Y variable with each ID grid.
// You have to separate each name by a comma if several parameters is
// associated to the grid.
// For example, if you want to associate the parameter 2 and 3 to a same
// grid then:
//   grid->GetVarY() have to return  "CHI_02,CHI_03"
//
////////////////////////////////////////////////////////////////////////////////

KVIDHarpeeICSi_e503::KVIDHarpeeICSi_e503() :
#if __cplusplus < 201103L
   records_(NULL),
#else
   records_(nullptr),
#endif
   kInitialised_(kFALSE)
{
   // Constructor (Public Interface Method)
   //
   // It is good practice to keep the constructor clear of initialisation code
   // as there is no easy way for constructors to handle errors. Instead you
   // should call Init() immediately after you construct this object.

   fIDCode = kIDCode4;
}

Bool_t KVIDHarpeeICSi_e503::Init()
{
   // Init (Public Interface Method)
   //
   // You should call this method immediately after you have constructed the
   // KVIDHarpeeICSi_e503 object. If the telescope has already been
   // initialised this method will do nothing. (DOES NOTHING AT THE MOMENT!)
   //

   if (kInitialised_) return kTRUE;

   kInitialised_ = kTRUE;
   return kTRUE;

}

//________________________________________________________________

KVIDHarpeeICSi_e503::~KVIDHarpeeICSi_e503()
{
   // Destructor (Public Interface Method)

#if __cplusplus < 201103L
   if (records_) {
      delete records_;
      records_ = NULL;
   }
#endif

}
//________________________________________________________________

Double_t KVIDHarpeeICSi_e503::GetIDMapY(Option_t* opt)
{
   // GetIDMapY (Public Interface Method)
   //
   // Calculates the current Y coordinate for identification.  It is the
   // calibrated energy of the fired acquisition parameter of HarpeeIC (i.e.
   // the energy loss in the crossed segment) for which we found an ID grid. If
   // more than one fired acq. parameter is found with their associated ID
   // grid then zero is returned. The found ID grid is loaded in the pointer
   // fGrid to be used in the method Identify.

   UNUSED(opt);

   fGrid = NULL;
   Double_t E   = 0;
   TIter next(fDEdet->GetListOfCalibrators());
   KVFunctionCal* cal = NULL;
   while ((cal = (KVFunctionCal*)next())) {

      //Energy only coded in ACQParam with label 'A'
      if (cal->GetLabel()[0] != 'A') continue;

      // look at only the ACQParam having its value greater than its pedestal
      if (cal->GetACQParam()->Fired("P")) {

         // find if a grid exists in the list for the fired acq. parameter
         // of the IC
         KVIDZAGrid* tmp = FindGridForSegment(cal->GetNumber());

         // more than one grid correspond to fired acq. parameters
         if (tmp && fGrid) {
            fGrid = NULL;
            return 0.;
         } else if (tmp) {
            // Load the grid associated to the fired ACQ Parameter
            // for identification
            fGrid = tmp;

            // returns the calibrated ACQ parameter i.e. energy measured
            // in the fired section of HarpeeIC
            if (cal->GetStatus()) E = cal->Compute();
         }
      }
   }

   return E;
}
//________________________________________________________________

KVIDZAGrid* KVIDHarpeeICSi_e503::FindGridForSegment(Int_t num)
{
   // Find the grid in the list fIDGrids for which its Y variable (
   // grid->GetVarY() ) contains the name corresponding to the HarpeeIC segment
   // ( i.e. the energy acq. parameter ) with number 'num'. We assume that each
   // name in Y variable is separated by a comma and that the segment number is
   // given by the last charater of the name, for example:
   // VarY="CHI_01,CHI_2,IC3" means the the grid is associate to the segment 1,
   // 2 and 3

   TIter next(fIDGrids);
   KVIDZAGrid* grid = NULL;
   while ((grid = (KVIDZAGrid*)next())) {
      static KVString vars;
      vars  = grid->GetVarY();
      vars.Begin(",");
      while (!vars.End()) {
         static KVString var;
         var   = vars.Next(kTRUE);
         if (var.Remove(0, var.Length() - 1).Atoi() == num) return grid;
      }
   }
   return NULL;
}

Bool_t KVIDHarpeeICSi_e503::Identify(
   KVIdentificationResult* idr,
   Double_t x,
   Double_t y
)
{
   // At the moment this class does nothing more than the basic KVIDHarpeeICSi
   // identification with some additional logic checks.

   assert(idr);

   //IMPORTANT !!! For the moment we need to reset the kInitialised flag
   //each time the Identify method is called.
   //Else each time the same KVIDHarpeeICSi_e503 is called (for ex: VID_CHI_SI_05) more
   //than once (typically in a KVIVSelector), it will be considered as already initialised and
   //the base_id_result_ will keep the information of the former identification !!!
   kInitialised_ = kFALSE;
   Init();

   idr->IDOK = kFALSE;
   idr->IDattempted = kTRUE;

   KVHarpeeIC* ic = static_cast<KVHarpeeIC*>(fDEdet);
   assert(ic);

   if (ic->GetFiredSegNumber() == 0) {
      // Mutliple segments (or none at all) have been fired in the ionisation
      // chamber and so we abandon this identification attempt.
      idr->IDquality = kSegmentMisfire;
      return kFALSE;
   }

   // Enforce ic number boundary
   assert((ic->GetFiredSegNumber() > 0) && (ic->GetFiredSegNumber() < 8));

   // Base class performs the preliminary identification routine. We do this
   // before the initialisation check as that is only applicable to the derived
   // class.  This way we will store the data from the base class
   // identification even if the derived class initialisation fails.

   SetOnlyZId(kFALSE);

   Bool_t status = KVIDHarpeeICSi::Identify(idr, x, y);

   // Set the idcode and type for this telescope
   idr->IDcode = fIDCode;
   idr->SetIDType(GetType());

   return status;
}

Bool_t KVIDHarpeeICSi_e503::SetIDCorrectionParameters(
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

const KVList* KVIDHarpeeICSi_e503::GetIDCorrectionParameters() const
{
#if __cplusplus < 201103L
   return records_;
#else
   return records_.get();
#endif
}

