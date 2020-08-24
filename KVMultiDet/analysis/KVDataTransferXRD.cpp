//Created by KVClassFactory on Wed Apr 28 12:29:38 2010
//Author: John Frankland,,,

#include "KVDataTransferXRD.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "KVDataSetManager.h"

using namespace std;

ClassImp(KVDataTransferXRD)


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
   GetRunList().Begin();
   while (!GetRunList().End()) {

      Int_t irun = GetRunList().Next();

      // source file full path
      TString src_full = GetDataSet()->GetFullPathToRunfile(GetDataType(), irun);

      //target repository dataset pointer
      KVDataSet* targ_ds = fTargetRep->GetDataSetManager()->GetDataSet(GetDataSet()->GetName());
      TString targ_fn = targ_ds->GetRunfileName(GetDataType(), irun);
      //we have to replace illegal characters like ":" in the target file name
      //to avoid problems due to the meaning of the ":" character for some
      //systems (MacOsX, Windows, some Linux distributions)
      targ_fn.ReplaceAll(":", "_");
      TString dest_full = fTargetRep->GetFullPathToTransferFile(targ_ds, GetDataType(), targ_fn.Data());

      cout << "TFile::Cp(\"" << src_full.Data() << "\", \"" << dest_full.Data() << "\")" << endl;

      // copy file
      TFile::Cp(src_full.Data(), dest_full.Data());
   }
}
