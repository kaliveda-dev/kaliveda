//Created by KVClassFactory on Sun Jan 15 23:10:34 2017
//Author: Quentin Fable,,,

#include "KVVAMOSDataCorrection.h"
#include "TPluginManager.h"


ClassImp(KVVAMOSDataCorrection)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSDataCorrection</h2>
<h4>A class to implement corrections on VAMOS events</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVVAMOSDataCorrection::KVVAMOSDataCorrection() : KVBase("VAMOSDataCorrection", "Correction of VAMOS data")
{
   // Default constructor
   fRecords = new KVList(kFALSE);
}

//____________________________________________________________________________//
KVVAMOSDataCorrection::~KVVAMOSDataCorrection()
{
   // Destructor
   if (fRecords) {
      delete fRecords;
      fRecords = NULL;
   }
}

//____________________________________________________________________________//
KVVAMOSDataCorrection* KVVAMOSDataCorrection::MakeDataCorrection(const Char_t* uri)
{
   //Looks for plugin (see $KVROOT/KVFiles/.kvrootrc) with name 'uri'(=name of dataset),
   //loads plugin library, creates object and returns pointer.
   //If no plugin defined for dataset, instanciates a KVVAMOSDataCorrection (default)

   //check and load plugin library
   TPluginHandler* ph;
   KVVAMOSDataCorrection* dc = 0;
   if (!(ph = KVBase::LoadPlugin("KVVAMOSDataCorrection", uri))) {
      dc = new KVVAMOSDataCorrection;
   } else {
      dc = (KVVAMOSDataCorrection*) ph->ExecPlugin(0);
   }

   dc->fDataSet = uri;
   return dc;
}

//____________________________________________________________________________//
Bool_t KVVAMOSDataCorrection::SetIDCorrectionParameters(const KVRList* const records)
{
   if (!records) {
      Error("SetIDCorrectionParameters", "Supplied record list is a null pointer");
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
void KVVAMOSDataCorrection::ApplyCorrections(KVVAMOSReconNuc*)
{
   // Generic (empty) method. Override in child classes to apply specific corrections.
}
