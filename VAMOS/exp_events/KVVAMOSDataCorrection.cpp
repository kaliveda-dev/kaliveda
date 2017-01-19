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
#if __cplusplus < 201103L
   fRecords = NULL;
#else
   fRecords(nullptr);
#endif
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
Bool_t KVVAMOSDataCorrection::Init()
{
   //Generic (empty) initialisation method.
   //Override in child classes to apply specific corrections.
   //Returns kFALSE.

   return kFALSE;
}

//____________________________________________________________________________//
KVVAMOSDataCorrection* KVVAMOSDataCorrection::MakeDataCorrection(const Char_t* uri)
{
   //Looks for plugin (see $KVROOT/KVFiles/.kvrootrc) with name 'uri'(=name of dataset),
   //loads plugin library, creates object and returns pointer.
   //If no plugin defined for dataset, instanciates a KVVAMOSDataCorrection (default)

   //debug
   std::cout << "KVVAMOSDataCorrection::MakeDataCorrection, ... creating data correction ..." << std::endl;

   //check and load plugin library
   TPluginHandler* ph;
   KVVAMOSDataCorrection* dc = 0;
   if (!(ph = KVBase::LoadPlugin("KVVAMOSDataCorrection", uri))) {
      dc = new KVVAMOSDataCorrection;
   } else {
      dc = (KVVAMOSDataCorrection*) ph->ExecPlugin(0);
   }

   dc->fDataSet = uri;

   std::cout << "KVVAMOSDataCorrection::MakeDataCorrection, ... printing info on created object ..." << std::endl;
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

#if __cplusplus < 201103L
   if (fRecords) {
      delete fRecords;
      fRecords = NULL;
   }

   fRecords = new KVList(kFALSE);
#else
   fRecords.reset(new KVList(kFALSE));
#endif

   fRecords->AddAll(records);
   return kTRUE;
}

//____________________________________________________________________________//
const KVList* KVVAMOSDataCorrection::GetIDCorrectionParameters() const
{
#if __cplusplus < 201103L
   return fRecords;
#else
   return fRecords.get();
#endif
}
//____________________________________________________________________________//
void KVVAMOSDataCorrection::ApplyCorrections(KVVAMOSReconNuc*)
{
   // Generic (empty) method. Override in child classes to apply specific corrections.
}
