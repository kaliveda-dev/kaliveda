//Created by KVClassFactory on Wed Jun  9 11:30:26 2010
//Author: John Frankland,,,

#include "KVIdentificationResult.h"

ClassImp(KVIdentificationResult)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIdentificationResult</h2>
<h4>Full result of one attempted particle identification</h4>
When we attempt to identify a reconstructed charged particle (KVReconstructedNucleus) using an identification
telescope (KVIDTelescope), we call the
KVIDTelescope::Identify() method of the telescope in question, and the results of the
identification attempt are stored in the KVIdentificationResult object.
<br><br>
The informations stored are:
<ul>
<li>Bool_t IDattempted : (=kTRUE if an identification attempt was made)</li>
<li>Bool_t IDOK : general status of identification (=kTRUE if identification attempt successful)</li>
<li>GetIDType() : returns string containing type of identification (corresponds to type of identification telescope)</li>
<li>GetGridName() : returns the name of the grid used for identification (VARY_VARX)
<li>Int_t IDcode : general identification quality code associated with this type of identification</li>
<li>Int_t IDquality : specific quality code returned by identification procedure</li>
<li>GetComment() : explanatory message regarding quality code returned by identification procedure</li>
<li>Double_t PID : the particle identifier (interpolated mass or charge)</li>
<li>Bool_t Zident : (=kTRUE if particle's Z was determined by the identification)</li>
<li>Bool_t Aident : (=kTRUE if particle's A was determined by the identification)</li>
<li>Int_t Z : the Z given by the identification attempt if Zident==kTRUE</li>
<li>Int_t A : the A given by the identification attempt if Aident==kTRUE</li>
<li>Int_t deltaEpedestal : tells if particle is in pedestal region of delta-E</li>
</ul>
Note that, apart from GetIDType() and GetComment() methods, all other informations are public member variables
which can be accessed directly.
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVIdentificationResult::Copy(TObject& obj) const
{
   // Copy this to obj
   KVBase::Copy(obj);
   KVIdentificationResult& id = (KVIdentificationResult&)obj;
   id.IDattempted = IDattempted;
   id.IDOK = IDOK;
   id.IDcode = IDcode;
   id.Zident = Zident;
   id.Aident = Aident;
   id.IDquality = IDquality;
   id.Z = Z;
   id.A = A;
   id.PID = PID;
   id.deltaEpedestal = deltaEpedestal;
}

void KVIdentificationResult::Clear(Option_t*)
{
   // Reset to initial values
   SetIDType("");
   SetComment("");
   SetGridName("");
   IDattempted = kFALSE;
   IDOK = kFALSE;
   IDcode = -1;
   Zident = kFALSE;
   Aident = kFALSE;
   IDquality = -1;
   Z = -1;
   A = -1;
   PID = -1.0;
   deltaEpedestal = deltaEpedestal_UNKNOWN;
}

void KVIdentificationResult::Print(Option_t*) const
{
   printf("Identification #%d  -  Type:%s ", GetNumber(), GetIDType());
   if (!IDattempted) {
      printf("   => not attempted\n\n");
      return;
   }
   if (IDOK) printf("   => SUCCESS\n");
   else printf("   => FAILURE\n");
   printf("  Grid used: %s\n", GetGridName());
   printf("  Quality code = %d (%s)\n", IDquality, GetLabel());
   if (Zident) printf("  Z identified = %d", Z);
   else printf("  Z returned = %d", Z);
   if (Aident) printf("    A identified = %d", A);
   else printf("  A returned = %d", A);
   if (Zident || Aident) printf("    PID = %f\n", PID);
   switch (deltaEpedestal) {

      case deltaEpedestal_NO:
         printf("  delta-E pedestal : NO\n");
         break;

      case deltaEpedestal_YES:
         printf("  delta-E pedestal : YES\n");
         break;

      default:
      case deltaEpedestal_UNKNOWN:
         break;
   }
}
