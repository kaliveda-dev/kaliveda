//Created by KVClassFactory on Mon Feb 13 18:44:49 2017
//Author: Quentin Fable,,,

#include "KVVAMOSDataCorrection.h"
#include "TPluginManager.h"

ClassImp(KVVAMOSDataCorrection)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSDataCorrection</h2>
<h4>Base class to use for VAMOS data corrections</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVVAMOSDataCorrection::KVVAMOSDataCorrection(Int_t run_number) : KVBase("VAMOSDataCorrection", "Correction of VAMOS data")
{
   fRecords   = NULL;
   fRunNumber = run_number;
}

//____________________________________________________________________________//
KVVAMOSDataCorrection::~KVVAMOSDataCorrection()
{
   // Destructor
#if __cplusplus < 201103L
   if (fRecords) {
      delete fRecords;
      fRecords = NULL;
   }
#endif
}


//____________________________________________________________________________//
void KVVAMOSDataCorrection::Init()
{
   //Generic (empty) initialisation method.
   //Override in child classes to apply specific corrections.
   //Do nothing

   return;
}

//____________________________________________________________________________//
KVVAMOSDataCorrection* KVVAMOSDataCorrection::MakeDataCorrection(const Char_t* uri, Int_t run_number)
{
   //Looks for plugin (see $KVROOT/KVFiles/.kvrootrc) with name 'uri'(=name of dataset),
   //loads plugin library, creates object and returns pointer.
   //If no plugin defined for dataset, instanciates a KVVAMOSDataCorrection (default)

   //debug
   std::cout << "<KVVAMOSDataCorrection::MakeDataCorrection> ... creating data correction ..." << std::endl;

   //check and load plugin library
   TPluginHandler* ph;
   KVVAMOSDataCorrection* dc = 0;
   if (!(ph = KVBase::LoadPlugin("KVVAMOSDataCorrection", uri))) {
      dc = new KVVAMOSDataCorrection(run_number);
   } else {
      dc = (KVVAMOSDataCorrection*) ph->ExecPlugin(1, run_number);
   }

   dc->fDataSet   = uri;
   dc->Init();

   std::cout << "<KVVAMOSDataCorrection::MakeDataCorrection> ... printing infos about created object ..." << std::endl;
   dc->Print();

   return dc;
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection::SetIDCorrectionParameters(const KVRList* const records)
{
   if (!records) {
      Error("SetIDCorrectionParameters",
            "Supplied record list is a null pointer");
      return kFALSE;
   }

   if (fRecords) {
      delete fRecords;
      fRecords = NULL;
   }

   fRecords = new KVList(kFALSE);
   fRecords->AddAll(records);

   return kTRUE;
}

//____________________________________________________________________________//
const KVList* KVVAMOSDataCorrection::GetIDCorrectionParameters() const
{
   return fRecords;
}
//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection::ApplyCorrections(KVVAMOSReconNuc*)
{
   // Generic (empty) method. Override in child classes to apply specific corrections.
   return kFALSE;
}
