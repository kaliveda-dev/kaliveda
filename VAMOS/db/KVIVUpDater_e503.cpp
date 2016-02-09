
///
/// @file KVIVUpDater_e503.cpp
///
/// @section Description
///
/// KVIVUpDater class for the E503 experiment. This class inherits from that of
/// our sister experiment (E494s) and adds the functionality required to
/// implement the VAMOS identification corrections.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed Jun 11 15:09:35 CEST 2014
///

#include "KVIVUpDater_e503.h"

ClassImp(KVIVUpDater_e503)

KVIVUpDater_e503::KVIVUpDater_e503()
{

}

KVIVUpDater_e503::~KVIVUpDater_e503()
{

}

void KVIVUpDater_e503::SetParameters(UInt_t run)
{
   /// IMPORTANT!
   ///
   /// An exception can be thrown here (intentionally, as one can't pass a
   /// return code back up the stack). The reason for this is that the function
   /// signature must match that of KVINDRAUpDater::SetParameters (which has a
   /// void return type).
   ///
   /// The advantage of throwing an exception is that the error can be handled
   /// properly further up the stack in KVIVSelector::Notify.
   /// KVIVSelector::Notify will automatically end the current run if it
   /// catches this exception.
   ///
   /// The alternative is the unexpected termination of the parent process,
   /// which if you happen to be in an interactive ROOT session is bad news (it
   /// will kill that too).
   ///

   KVIVUpDater::SetParameters(run);

   // ----------------------------------
   // Load database records for this run
   // ----------------------------------

   KVDBRun* db_run_entry(gIndraDB->GetRun(run));
   assert(db_run_entry);

   Info("KVIVUpDater_e503::SetParameters",
        "Setting VAMOS ID correction parameters...");

   const TString keyname("VAMOSIDCorrectionParameters");

   Info("KVIVUpDater_e503::SetParameters",
        "Retrieving records using key \'%s\'...", keyname.Data());

   const KVRList* const records(db_run_entry->GetLinks(keyname));
   assert(records);

   if (!records) {

      Error("KVIVUpDater_e503::SetParameters",
            "Failed to retrieve records using key \"%s\", "
            "has the key name changed?", keyname.Data());


      throw std::runtime_error("\'records\' is a null pointer");
   }

   // ---------------------------------------------------
   // Silicon - Caesium Iodide Identification Corrections
   // ---------------------------------------------------

   Info("KVIVUpDater_e503::SetParameters",
        "Setting ID correction parameters for VAMOS.SI-CSI telescopes...");

   KVSeqCollection* telescopes(
      gVamos->GetIDTelescopesWithType("VAMOS.SI-CSI")
   );
   assert(telescopes);

   if (!telescopes) {

      Error("KVIVUpDater_e503::SetParameters",
            "Failed to retrieve the list of VAMOS.SI-CSI telescopes");

      throw std::runtime_error("\'telescopes\' is a null pointer!");
   }

   TIter next_sicsi(telescopes);

   Info("KVIVUpDater_e503::SetParameters", "%d telescopes of type VAMO.SI-CSI",
        telescopes->GetEntries());

   Long64_t num_calls(0);
   Bool_t status(kTRUE);

#if __cplusplus < 201103L
   KVIDHarpeeSiCsI_e503* si_csi(NULL);
#else
   KVIDHarpeeSiCsI_e503* si_csi(nullptr);
#endif

   while ((si_csi = dynamic_cast<KVIDHarpeeSiCsI_e503*>(next_sicsi()))) {
      status &= si_csi->SetIDCorrectionParameters(records);
      ++num_calls;
   }

   delete telescopes;
#if __cplusplus < 201103L
   telescopes = NULL;
#else
   telescopes = nullptr;
#endif

   if (status) {
      Info("KVIVUpDater_e503::SetParameters", "Si-CsI OK");
   } else {
      Error("KVIVUpDater_e503::SetParameters", "Si-CsI NOT OK");
   }

   // -------------------------------------------------------
   // Ionisation Chamber - Silicon Identification Corrections
   // -------------------------------------------------------

   Info("KVIVUpDater_e503::SetParameters",
        "Setting ID correction parameters for VAMOS.CHI-SI telescopes...");

   telescopes = gVamos->GetIDTelescopesWithType("VAMOS.CHI-SI");
   assert(telescopes);

   if (!telescopes) {
      Error("KVIVUpDater_e503::SetParameters",
            "Failed to retrieve the list of VAMOS.CHI-SI telescopes");

      throw std::runtime_error("\'telescopes\' is a null pointer!");
   }

   TIter next_chiosi(telescopes);

   Info("KVIVUpDater_e503::SetParameters",
        "%d telescopes of type VAMOS.CHI-SI", telescopes->GetEntries());

   num_calls = 0;

#if __cplusplus < 201103L
   KVIDHarpeeICSi_e503* chio_si(NULL);
#else
   KVIDHarpeeICSi_e503* chio_si(nullptr);
#endif

   while ((chio_si = dynamic_cast<KVIDHarpeeICSi_e503*>(next_chiosi()))) {
      status &= chio_si->SetIDCorrectionParameters(records);
      ++num_calls;
   }

   delete telescopes;
#if __cplusplus < 201103L
   telescopes = NULL;
#else
   telescopes = nullptr;
#endif

   if (status) {
      Info("KVIVUpDater_e503::SetParameters", "IC-Si OK");
   } else {
      Error("KVIVUpDater_e503::SetParameters", "IC-Si NOT OK");
   }

   // ------------------
   // Final status check
   // ------------------

   if (!status) {

      Error("KVIVUpDater_e503::SetParameters",
            "Error(s) encountered while setting VAMOS ID correction "
            "parameters.");

      throw std::runtime_error(
         "Bad status in KVIVUpDater_e503::SetParameters");
   }

   return;
}

