/*********************************************************************************
$Id: KVIDSubCodeManager.cpp,v 1.2 2006/10/19 14:32:43 franklan Exp $
*********************************************************************************/

#include "KVIDSubCodeManager.h"
#include "TString.h"

using namespace std;

ClassImp(KVIDSubCodeManager)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Stores concatenations of "subcodes" returned by different identification routines
//inside a KVIDSubCode (16-bit binary number).
//
//For a given type of identification routine/telescope, the subcode is stored
//in fNbits bits from bit number fMSB to bit (fMSB-fNbits+1) (fMSB is the bit-
//number of the most significant bit used to store the subcode - bit 0 is the
//Least Significant Bit).
//
KVIDSubCodeManager::KVIDSubCodeManager(UChar_t nbits, UChar_t msb)
{
   //Ctor with number of bits and position of Most Significant Bit.
   //By default, we use all the bits of a KVIDSubCode.
   SetNbits(nbits);
   SetMSB(msb);
}

const Char_t* KVIDSubCodeManager::GetIDSubCodeString(KVIDSubCode& concat) const
{
   //Should be redefined in child-classes (KVIDTelescopes) so that an appropriate
   //message corresponding to the subcode is returned.
   //If not, we return the result of "SubCodeNotSet()" if GetIDSubCode=-1, otherwise
   //we give the value of GetIDSubCode() as a string, "ID sub code = x"
   Int_t code = GetIDSubCode(concat);
   if (code == -1)
      return SubCodeNotSet();
   return Form("ID sub code = %d", code);
}
