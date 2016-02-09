//Created by KVClassFactory on Tue Mar 12 15:08:25 2013
//Author: Guilain ADEMARD

#include "KVVAMOSCodes.h"
#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
using namespace std;

ClassImp(KVVAMOSCodes)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSCodes</h2>
<h4>Status for calibration, trajectory reconstruction, identification and time of flight in VAMOS</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVDataSet* KVVAMOSCodes::fDataSet        = NULL;
UChar_t    KVVAMOSCodes::fNFPdets        = 0;
UChar_t    KVVAMOSCodes::fNToF           = 0;
Char_t**   KVVAMOSCodes::fFPdetNames     = NULL;
Char_t**   KVVAMOSCodes::fToFNames       = NULL;
Char_t**   KVVAMOSCodes::fCodeGenFPRecon = NULL;
Char_t**   KVVAMOSCodes::fCodeGenToF     = NULL;

KVVAMOSCodes::KVVAMOSCodes()
{
   // Default constructor
   fFPMask = 0;
   fTMask  = 0;
   RefreshCodes();
}

//________________________________________________________________

KVVAMOSCodes::~KVVAMOSCodes()
{
   // Destructor
   fFPMask = 0;
   fTMask  = 0;
}
//________________________________________________________________

Bool_t KVVAMOSCodes::TestFPCode(UInt_t code)
{
   //check Focal plan Position reconstruction code against code mask
   //always kTRUE if no mask set (fFPMask==0)
   if (!fFPMask)
      return kTRUE;
   return (Bool_t)((fFPMask & code) != 0);
}
//________________________________________________________________

Bool_t KVVAMOSCodes::TestTCode(UShort_t code)
{
   //check Time of Flight code against code mask
   //always kTRUE if no mask set (fTMask==0)
   if (!fTMask)
      return kTRUE;
   return (Bool_t)((fTMask & code) != 0);
}
//________________________________________________________________

Bool_t KVVAMOSCodes::RefreshCodes()
{
   //Refresh the Focal Plan Position reconstruction codes and the
   //Time of Flight codes if the dataset has changed

   if (!gDataSet) {
      cout << "Error in <KVVAMOSCodes::RefreshCodes>: dataset not found" << endl;
      return kFALSE;
   }

   if (fDataSet == gDataSet) return kTRUE;

   fDataSet = gDataSet;
   return RefreshFPCodes() && RefreshTCodes();
}
//________________________________________________________________

Bool_t KVVAMOSCodes::RefreshFPCodes()
{
   //Refresh the Focal Plan Position reconstruction code. Te list of detectors
   //chosen for the reconstruction is read to build codes for each
   //combination of detectors. This list is given by the environment
   //variable KVVAMOSCodes.FocalPlanReconDetList. At least 2 detectors
   //(name or type) have to be set in this list with a maximum of
   //4 detectors. This list can be set for a specific dataset.
   //For example:
   //
   // INDRA_e494s.KVVAMOSCodes.FocalPlanReconDetList: SED1 SED2 SI
   //
   //The detectors have to be listed from the more appropriate for
   //position determination (Xf,Yf) to the less appropriate.
   //
   //See BuildFPReconCodes() method for more informations about
   //combinations.

   DeleteFPReconCodes();

   TString envname("KVVAMOSCodes.FocalPlanReconDetList");
   TString str(fDataSet->GetDataSetEnv(envname.Data()));
   TObjArray* tok = str.Tokenize(" ");
   fNFPdets = (tok->GetEntries() > 4 ? 4 : tok->GetEntries());
   if (fNFPdets < 2) {
      cout << "Error in <KVVAMOSCodes::RefreshFPCodes>: ";
      cout << "Set at least 2 detectors in the environment variable"
           << envname.Data() << "used in the reconstruction of the position at the focal plan (Xf,Yf)" << endl;
      fNFPdets = 0;
      return kFALSE;
   }

   fFPdetNames = new Char_t* [fNFPdets];
   TObjString* ostr = NULL;
   for (Int_t i = 0; i < fNFPdets; i++) {

      ostr = (TObjString*)tok->At(i);
      str  = ostr->GetString();
      Int_t size = str.Sizeof();
      fFPdetNames[i] = new Char_t[ size ];
      strcpy(fFPdetNames[i], str.Data());
   }
   delete tok;

   BuildFPReconCodes();

   return kTRUE;
}
//________________________________________________________________

void KVVAMOSCodes::DeleteFPReconCodes()
{
   // Deletes codes, status, detector names used for the Focal plan
   // Position reconstruction. Called by RefreshFPCodes() method.

   //Remove previous codes
   if (fCodeGenFPRecon) {
      Int_t Ncodes = GetNFPCodes();
      for (Int_t i = 0; i < Ncodes; i++)
         delete[] fCodeGenFPRecon[i];
      delete[] fCodeGenFPRecon;
      fCodeGenFPRecon = NULL;
   }

   //Remove array of detectors used for focal
   //plan position reconstruction
   if (fNFPdets) {
      for (Int_t i = 0; i < fNFPdets; i++)
         delete[] fFPdetNames[i];
      delete[] fFPdetNames;
      fFPdetNames = NULL;
      fNFPdets    = 0;
   }
}
//________________________________________________________________

void KVVAMOSCodes::BuildFPReconCodes()
{
   //Builds the codes and corresponding status for the Focal plan
   //Position reconstruction from the list of detectors given
   //in this case the list of detectors  by the environment
   //variable KVVAMOSCodes.FocalPlanReconDetList. At least 2 detectors
   //(name or type) have to be set in this list with a maximum of
   //4 detectors. This list can be set for a specific dataset.
   //For example:
   //
   // INDRA_e494s.KVVAMOSCodes.FocalPlanReconDetList: SED1 SED2 SI
   //
   //The detectors have to be listed from the more appropriate for
   //position determination (Xf,Yf) to the less appropriate.
   //
   //The built status are obtain from combinations of detectors which
   //measured positions. For example, if the list of detectors is
   //
   //KVVAMOSCodes.FocalPlanReconDetList: D1 D2 D3 D4
   //
   //then the available codes are the following:
   //
   // idx                FP-Status FocalPosCodes
   //   0         no position (XY)      kFPCode0
   //   1            D1(XY)-D2(XY)      kFPCode1
   //   2       D1(XY)-D2(X)-D3(Y)      kFPCode2
   //   3       D1(XY)-D2(Y)-D3(X)      kFPCode3
   //   4       D1(XY)-D2(X)-D4(Y)      kFPCode4
   //   5       D1(XY)-D2(Y)-D4(X)      kFPCode5
   //   6            D1(XY)-D3(XY)      kFPCode6
   //   7       D1(XY)-D3(X)-D4(Y)      kFPCode7
   //   8       D1(XY)-D3(Y)-D4(X)      kFPCode8
   //   9            D1(XY)-D4(XY)      kFPCode9
   //  10       D2(XY)-D1(X)-D3(Y)     kFPCode10
   //  11       D2(XY)-D1(Y)-D3(X)     kFPCode11
   //  12       D2(XY)-D1(X)-D4(Y)     kFPCode12
   //  13       D2(XY)-D1(Y)-D4(X)     kFPCode13
   //  14            D2(XY)-D3(XY)     kFPCode14
   //  15       D2(XY)-D3(X)-D4(Y)     kFPCode15
   //  16       D2(XY)-D3(Y)-D4(X)     kFPCode16
   //  17            D2(XY)-D4(XY)     kFPCode17
   //  18       D3(XY)-D1(X)-D2(Y)     kFPCode18
   //  19       D3(XY)-D1(Y)-D2(X)     kFPCode19
   //  20       D3(XY)-D1(X)-D4(Y)     kFPCode20
   //  21       D3(XY)-D1(Y)-D4(X)     kFPCode21
   //  22       D3(XY)-D2(X)-D4(Y)     kFPCode22
   //  23       D3(XY)-D2(Y)-D4(X)     kFPCode23
   //  24            D3(XY)-D4(XY)     kFPCode24
   //  25       D4(XY)-D1(X)-D2(Y)     kFPCode25
   //  26       D4(XY)-D1(Y)-D2(X)     kFPCode26
   //  27       D4(XY)-D1(X)-D3(Y)     kFPCode27
   //  28       D4(XY)-D1(Y)-D3(X)     kFPCode28
   //  29       D4(XY)-D2(X)-D3(Y)     kFPCode29
   //  30       D4(XY)-D2(Y)-D3(X)     kFPCode30
   //
   //where 'idx' is the index of the code. The second column gives
   //the status, and the third column gives the FocalPosCodes enumerate
   //element corresponding to each code.
   //Code 0 corresponds to the cas where Focal plan position
   //reconstruction is impossible.
   //From the code 1, the codes are sorted from the more
   //appropriate case for position determination (Xf,Yf) to
   //the less appropriate.
   //In the status of these codes, D1(XY) means that, in the Focal
   //plan Position reconstruction, the detector D1 gives both
   //coordinates X and Y of a position. D2(X) means the detector
   //D2 gives only the coordinate X of a position.


   Char_t* det_c1;        // 1st det. with complet position (XY)
   Char_t* det_c2;        // 2nd det. with complet position (XY)
   Char_t* det_i1;        // 1st det. with incomplet position
   Char_t* det_i2;        // 2nd det. with incomplet position
   Char_t  XorY_det_i1;   // the coordinate given by det_i1
   Char_t  XorY_det_i2;   // the coordinate given by det_i2

   //Build new code for each combination
   fCodeGenFPRecon = new Char_t* [ GetNFPCodes() ];
   TString str;
   Int_t size, idx = 0;

   //Code 0 for no position measurment in all the detectors
   str = "no position (XY)";
   size = str.Sizeof();
   fCodeGenFPRecon[idx] = new Char_t[ size ];
   strcpy(fCodeGenFPRecon[idx], str.Data());
   idx++;

   //The other codes for each combinations of detectors
   for (Int_t i = 0; i < fNFPdets; i++) {
      det_c1 = fFPdetNames[i];
      for (Int_t j = 0; j < i; j++) {
         det_i1 = fFPdetNames[j];
         for (Int_t k = j + 1; k < fNFPdets; k++) {
            if (k == i) continue;
            det_i2 = fFPdetNames[k];
            for (Int_t l = 0; l <= 1; l++) {
               XorY_det_i1 = 'X' + l;
               XorY_det_i2 = 'Y' - l;
               str.Form("%s(XY)-%s(%c)-%s(%c)", det_c1, det_i1, XorY_det_i1, det_i2, XorY_det_i2);
               size = str.Sizeof();
               fCodeGenFPRecon[idx] = new Char_t[ size ];
               strcpy(fCodeGenFPRecon[idx], str.Data());
               idx++;
            }
         }
      }
      for (Int_t j = i + 1; j < fNFPdets; j++) {
         det_c2 = fFPdetNames[j];
         str.Form("%s(XY)-%s(XY)", det_c1, det_c2);
         size = str.Sizeof();
         fCodeGenFPRecon[idx] = new Char_t[ size ];
         strcpy(fCodeGenFPRecon[idx], str.Data());
         idx++;
         det_i1 = fFPdetNames[j];
         for (Int_t k = j + 1; k < fNFPdets; k++) {
            det_i2 = fFPdetNames[k];
            for (Int_t l = 0; l <= 1; l++) {
               XorY_det_i1 = 'X' + l;
               XorY_det_i2 = 'Y' - l;
               str.Form("%s(XY)-%s(%c)-%s(%c)", det_c1, det_i1, XorY_det_i1, det_i2, XorY_det_i2);
               size = str.Sizeof();
               fCodeGenFPRecon[idx] = new Char_t[ size ];
               strcpy(fCodeGenFPRecon[idx], str.Data());
               idx++;
            }
         }
      }
   }
}
//________________________________________________________________

Bool_t KVVAMOSCodes::RefreshTCodes()
{
   //Refresh the Time of Flight codes. The list of ACQ parameters
   //chosen for the Time of Flight calculation of the nuclei reconsturction
   //in VAMOS is read to build codes. This list is given by the environment
   //variable KVVAMOSCodes.ACQParamListForToF. At least 1 ACQ parameter
   //(name) has to be set in this list with a maximum of 15.
   //This list can be set for a specific dataset.
   //For example:
   //
   // INDRA_e494s.KVVAMOSCodes.ACQParamListForToF: TSED2_HF TSED1_HF
   //
   //The ACQ parameters have to be listed from the more appropriate (with
   // the best resolution) for a ToF measurement.

   DeleteTimeOfFlightCodes();

   TString envname("KVVAMOSCodes.ACQParamListForToF");
   TString str(fDataSet->GetDataSetEnv(envname.Data()));
   TObjArray* tok = str.Tokenize(" ");
   fNToF = (tok->GetEntries() > 15 ? 15 : tok->GetEntries());
   if (fNToF < 1) {
      cout << "Error in <KVVAMOSCodes::RefreshTCodes>: ";
      cout << "Set at least 1 ACQ parameter in the environment variable"
           << envname.Data() << "to use for the Time of Flight of the nuclei reconstructed in VAMOS(Xf,Yf)" << endl;
      fNToF = 0;
      return kFALSE;
   }

   fToFNames = new Char_t* [fNToF];
   TObjString* ostr = NULL;
   for (Int_t i = 0; i < fNToF; i++) {

      ostr = (TObjString*)tok->At(i);
      str  = ostr->GetString();
      Int_t size = str.Sizeof();
      fToFNames[i] = new Char_t[ size ];
      strcpy(fToFNames[i], str.Data());
   }
   delete tok;

   BuildTimeOfFlightCodes();

   return kTRUE;
}
//________________________________________________________________

void KVVAMOSCodes::DeleteTimeOfFlightCodes()
{
   // Deletes codes, status, names used for the Time of Flight of the
   // nuclei reconstructed in VAMOS. Called by RefreshTCodes() method.

   //Remove previous codes
   if (fCodeGenToF) {
      Int_t Ncodes = GetNTCodes();
      for (Int_t i = 0; i < Ncodes; i++)
         delete[] fCodeGenToF[i];
      delete[] fCodeGenToF;
      fCodeGenToF = NULL;
   }

   //Remove array of ToF names
   if (fNToF) {
      for (Int_t i = 0; i < fNToF; i++)
         delete[] fToFNames[i];
      delete[] fToFNames;
      fToFNames = NULL;
      fNToF     = 0;
   }
}
//________________________________________________________________

void KVVAMOSCodes::BuildTimeOfFlightCodes()
{
   //Builds the codes and corresponding status for the Time of Flight
   //used in the reconstructed nuclei in VAMOS from the list of acq. parameter
   //given by the environment variable KVVAMOSCodes.ACQParamListForToF.
   //At least 1 ACQ parameter(name) has to be set in this list with a maximum of 15.
   //This list can be set for a specific dataset.
   //For example:
   //
   // INDRA_e494s.KVVAMOSCodes.ACQParamListForToF: TSED2_HF TSED1_HF
   //
   //The ACQ parameters have to be listed from the more appropriate (with
   // the best resolution) for a ToF measurement.
   //
   //For example, for the following liste
   //
   //KVVAMOSCodes.ACQParamListForToF: TSED1_HF TSED1_HF TSI_HF TSED1_SED2
   //
   //then the available codes are the following:
   //
   // idx               ToF-Status   TimeOfFlightCodes
   //   0        NO Time of Flight             kTCode0
   //   1    ToF given by TSED1_HF             kTCode1
   //   2    ToF given by TSED2_HF             kTCode2
   //   3      ToF given by TSI_HF             kTCode3
   //   4  ToF given by TSED1_SED2             kTCode4
   //
   //where 'idx' is the index of the code. The second column gives
   //the status, and the third column gives the TimeOfFlightCodes enumerate
   //element corresponding to each code.
   //Code 0 corresponds to the cas where no Time Of Flight can be set to the
   //nucleus.
   //From the code 1, the codes are sorted from the more
   //appropriate case to the less appropriate.

   //Build new code for each combination
   fCodeGenToF = new Char_t* [ GetNTCodes() ];
   TString str;
   Int_t size, idx = 0;

   //Code 0 for no ToF set to the nucleus
   str = "NO Time of Flight";
   size = str.Sizeof();
   fCodeGenToF[idx] = new Char_t[ size ];
   strcpy(fCodeGenToF[idx], str.Data());
   idx++;

   //The other codes for each combinations of detectors
   for (Int_t i = 0; i < fNToF; i++) {
      str.Form("ToF given by %s", fToFNames[i]);
      size = str.Sizeof();
      fCodeGenToF[idx] = new Char_t[ size ];
      strcpy(fCodeGenToF[idx], str.Data());
      idx++;
   }
}
//________________________________________________________________

void KVVAMOSCodes::ShowAvailableFPCodes()
{
   //Static method which shows the available codes for Focal plan
   //position reconstruction built for the current dataset
   //(gDataSet) by the method BuildFPReconCodes().

   if (!RefreshCodes()) return;
   Int_t Ncodes = GetNFPCodes();
   cout << setw(4) << "idx" << setw(25) << "FP-Status" << setw(14) << "FocalPosCodes" << endl;
   for (Int_t i = 0; i < Ncodes; i++)
      cout << setw(4) << i << setw(25) << fCodeGenFPRecon[i] << setw(14) << Form("kFPCode%d", i) << endl;
}
//________________________________________________________________

void KVVAMOSCodes::ShowAvailableTCodes()
{
   //Static method which shows the available codes for the
   //Time of Flight to be used for the nuclei reconstructed in VAMOS
   //for the current dataset (gDataSet).

   if (!RefreshCodes()) return;
   Int_t Ncodes = GetNTCodes();
   cout << setw(4) << "idx" << setw(25) << "ToF-Status" << setw(20) << "TimeOfFlightCodes" << endl;
   for (Int_t i = 0; i < Ncodes; i++)
      cout << setw(4) << i << setw(25) << fCodeGenToF[i] << setw(20) << Form("kTCode%d", i) << endl;
}
//________________________________________________________________

const Char_t* KVVAMOSCodes::GetFPStatus()
{
   //Give an explanation for the Focal Plan position reconstruction code.
   if (!RefreshCodes()) return NULL;

   UChar_t idx = GetFPCodeIndex();
   if (idx < GetNFPCodes())
      return fCodeGenFPRecon[ idx ];

   Error("GetFPStatus", "The FP-code index is out of the number of available codes");
   return NULL;
}
//________________________________________________________________

const Char_t* KVVAMOSCodes::GetTStatus()
{
   //Give an explanation for the current Time of Flight code.
   if (!RefreshCodes()) return NULL;

   UChar_t idx = GetTCodeIndex();
   if (idx < GetNTCodes())
      return fCodeGenToF[ idx ];

   Error("GetTStatus", "The T-code index is out of the number of available codes");
   return NULL;
}
//________________________________________________________________

UInt_t KVVAMOSCodes::GetFPCode()
{
   //Returns Focal Plan position reconstruction code in the form
   //of FocalPlanCodes bitmask.

   return GetFPMask();
}
//________________________________________________________________

UShort_t KVVAMOSCodes::GetTCode()
{
   //Returns code corresponding to the Time Of Flight used in the form
   //of TimeOfFlightCodes bitmask.

   return GetTMask();
}
//________________________________________________________________

UShort_t KVVAMOSCodes::GetTCode(const Char_t* parnam)
{
   //This static method allows to get the T-code from the name of the acquisition
   //parameter used for the time of flight of the nucleus reconstructed in VAMOS.
   //
   //for example:
   //   GetTCode( "TSED1_HF" );

   if (parnam) {
      for (Int_t i = 0; i < fNToF; i++) {
         if (!strcmp(parnam, fToFNames[i])) return (1 << (i + 1));
      }
   }
   return 0;
}
//________________________________________________________________

void KVVAMOSCodes::SetFPCode(UInt_t mask)
{
   //Set Focal plan Position reconstruction code - the argument
   //given is one of the FocalPlanCodes bitmasks.

   SetFPMask(mask);
}
//________________________________________________________________

void KVVAMOSCodes::SetTCode(UShort_t mask)
{
   //Set Time of Flight code - the argument
   //given is one of the TimeOfFlightCodes bitmasks.

   SetTMask(mask);
}
//________________________________________________________________

void KVVAMOSCodes::SetTCode(const Char_t* parnam)
{
   //this method allows to set the T-code from the name of the acquisition
   //parameter used for the time of flight of the nucleus reconstructed in VAMOS.
   //
   //for example:
   //   vamos_code->SetTCode( "TSED1_HF" );

   SetTCode(GetTCode(parnam));
}
//________________________________________________________________

void KVVAMOSCodes::SetFPCode(Int_t nc1, Int_t nc2, Int_t ni1, Int_t ni2, Bool_t ni1x)
{
   //This method allows to set the code from the index in the list
   //of FP detectors (given in the environment variable
   //KVVAMOSCodes.FocalPlanReconDetList):
   // nc1  - index of the 1st det. with complet position (XY)
   // nc2  - index of the 2nd det. with complet position (XY)
   // ni1  - index of the 1st det. with incomplet position
   // ni2  - index of the 2nd det. with incomplet position
   // ni1x - boolean = true if the coordinate given by the det.
   //        with incomplet position (ni1) is X.
   //
   //Conditions that arguments have to verify:
   //    nc1 must always be given;
   //    nc1<nc2 if nc2 is given (nc2>-1);
   //    nc2 = -1 if ni1 and ni2 are given;
   //    ni1 and ni2 have to be given both at the same time;
   //    ni1<ni2 and ni1 != nc1
   //
   //if any of these conditions are verified the code 0 is set
   //i.e. "no position (XY)".

   Int_t idx = 0;
   if (RefreshCodes() && (-1 < nc1) && (nc1 < fNFPdets)) {

      idx = ((nc1 + 1) * (2 * (fNFPdets - 1) * (fNFPdets - 1) - nc1)) / 2;

      if ((nc1 < nc2) && (nc2 < fNFPdets)) {
         idx -= fNFPdets * fNFPdets + nc2 * (nc2 - 2 * fNFPdets) - 1;
      } else if ((ni1 != nc1) && (-1 < ni1) && (ni1 < ni2) && (ni2 < fNFPdets)) {
         if (ni1 > nc1) {
            idx -= fNFPdets * fNFPdets + ni1 * (ni1 - 2 * (fNFPdets - 1)) - 2 * ni2 + ni1x - 1;
         } else {
            idx -= fNFPdets * (fNFPdets - 2) - nc1 + ni1 * (ni1 - 2 * fNFPdets + 5) - 2 * ni2 + ni1x + 3 - (ni2 < nc1 ? 2 : 0);
         }
      } else idx = 0;
   }
   SetFPCodeFromIndex(idx);
}
//________________________________________________________________

UChar_t KVVAMOSCodes::GetFPCodeIndex(UInt_t mask)
{
   //Argument is a 32-bit-mask representing particle FP code.
   //Returns the number of the bit whick is set to 1 (right-most
   //bit is 0).

   Int_t i = 0;
   if (!mask) return 0;
   do {
      mask = (mask >> 1);
      i++;
   } while (mask);
   return i - 1;
}
//________________________________________________________________

UChar_t KVVAMOSCodes::GetTCodeIndex(UShort_t mask)
{
   //Argument is a 16-bit-mask representing particle T code.
   //Returns the number of the bit whick is set to 1 (right-most
   //bit is 0).

   Int_t i = 0;
   if (!mask) return 0;
   do {
      mask = (mask >> 1);
      i++;
   } while (mask);
   return i - 1;
}
//________________________________________________________________

UChar_t KVVAMOSCodes::GetFPCodeIndex()
{
   //Returns index of the Focal plan Position reconstruction code.

   return GetFPCodeIndex(GetFPMask());
}
//________________________________________________________________

UChar_t KVVAMOSCodes::GetTCodeIndex()
{
   //Returns index of the Time of Flight code.

   return GetTCodeIndex(GetTMask());
}
//________________________________________________________________

void KVVAMOSCodes::SetFPCodeFromIndex(Int_t idx)
{
   //Set Focal plan Position reconstruction code - the argument
   //given is the index of the codes. Call the static method
   //KVVAMOSCodes::ShowAvailbleFPCodes() to have the correspondence
   //between index and code.

   if (idx < 0) SetFPMask(0);
   else SetFPMask(1 << idx);
}
//________________________________________________________________

void KVVAMOSCodes::SetTCodeFromIndex(Int_t idx)
{
   //Set Time of Flight code - the argument
   //given is the index of the codes. Call the static method
   //KVVAMOSCodes::ShowAvailbleTCodes() to have the correspondence
   //between index and code.

   if (idx < 0) SetTMask(0);
   else SetTMask(1 << idx);
}
