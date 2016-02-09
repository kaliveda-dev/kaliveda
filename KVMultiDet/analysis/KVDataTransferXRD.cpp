//Created by KVClassFactory on Wed Apr 28 12:29:38 2010
//Author: John Frankland,,,

#include "KVDataTransferXRD.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "KVDataSetManager.h"

using namespace std;

ClassImp(KVDataTransferXRD)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataTransferXRD</h2>
<h4>Transfer data from remote repository using xrootd</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVDataTransferXRD::KVDataTransferXRD()
{
   // Default constructor
}

KVDataTransferXRD::~KVDataTransferXRD()
{
   // Destructor
}

void KVDataTransferXRD::ExecuteCommand()
{
   // Transfer all requested runs using TFile::Cp

   //check connection to remote repository : i.e. open ssh tunnel if needed
   fSourceRep->IsConnected();

   // loop over runs
   fRunList.Begin();
   while (!fRunList.End()) {

      Int_t irun = fRunList.Next();

      // source file full path
      TString src_full = fDataSet->GetFullPathToRunfile(fDataType.Data(), irun);

      //target repository dataset pointer
      KVDataSet* targ_ds = fTargetRep->GetDataSetManager()->GetDataSet(fDataSet->GetName());
      TString targ_fn = targ_ds->GetRunfileName(fDataType.Data(), irun);
      //we have to replace illegal characters like ":" in the target file name
      //to avoid problems due to the meaning of the ":" character for some
      //systems (MacOsX, Windows, some Linux distributions)
      targ_fn.ReplaceAll(":", "_");
      TString dest_full = fTargetRep->GetFullPathToTransferFile(targ_ds, fDataType.Data(), targ_fn.Data());

      cout << "TFile::Cp(\"" << src_full.Data() << "\", \"" << dest_full.Data() << "\")" << endl;

      // copy file
      TFile::Cp(src_full.Data(), dest_full.Data());
   }
}
