/***************************************************************************
$Id: KVINDRACodes.cpp,v 1.3 2008/02/14 10:30:18 franklan Exp $
 ***************************************************************************/

#include "KVINDRACodes.h"


ClassImp(KVINDRACodes)


Char_t KVINDRACodes::fCodeGenIdent[14][120] = {
   "gamma",
   "neutron (seulement couronnes 2 a 9)",
   "part. ident. dans CsI ou Phoswich (OK + 8Be)",
   "fragment identifie dans Si-CsI ou fragment ident. dans Si75-SiLi ou SiLi-CsI",
   "fragment identifie dans ChIo-Si/ChIo-CsI ou fragment ou part. identifie dans ChIo-Si75",
   "fragment non-ident. (montee de Bragg)(Z min.) ou stoppe dans Chio (Z min)",
   "frag. cree par la coherence CsI  (stoppe ds Si.)",
   "frag. cree par la coherence ChIo (stoppe ds ChIo)",
   "multiple comptage dans ChIo avec arret: - dans les siliciums (cour. 2-9); - dans les CsI (cour. 10-17)",
   "ident. incomplete dans CsI ou Phoswich (Z.min)",
   "ident. 'entre les lignes' dans CsI",
   "ident. Z=2 due a l'inversion CHIO (Argon)",
   "Particule restee non identifiee (a rejeter)",
   "frag. ou part. rejete"
};

UChar_t KVINDRACodes::fIDCodes[14] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15 };

UShort_t KVINDRACodes::fIDBits[16] = {
   kIDCode0,
   kIDCode1,
   kIDCode2,
   kIDCode3,
   kIDCode4,
   kIDCode5,
   kIDCode6,
   kIDCode7,
   kIDCode8,
   kIDCode9,
   kIDCode10,
   kIDCode11,
   0,
   0,
   kIDCode14,
   kIDCode15
};

Char_t KVINDRACodes::fCodeGenCalib[8][120] = {
   "Aucun appel a une routine de calibration effectuee",
   "Calibration sans probleme (toute les routines ont retourne le code 1)",
   "Warning : Une des routines a retourne un code different de 1",
   "Warning specifique au CSI : E_CsI > E_Max du CsI",
   "Detecteur en panne (CHIO ou SI): perte d'energie dans ce detecteur calcule par Veda",
   "Rejet : Une des routines a retourne un code de rejet.",
   "One or more detectors hit by more than one particle, energy loss calculated for this detector",
   "Coherency test of energy losses reveals presence of other particles"
};

UChar_t KVINDRACodes::fECodes[8] = { 0, 1, 2, 3, 11, 15, 4, 5 };

UChar_t KVINDRACodes::fEBits[16] = {
   kECode0,
   kECode1,
   kECode2,
   kECode3,
   kECode4,
   kECode5,
   0,
   0,
   0,
   0,
   0,
   kECode11,
   0,
   0,
   0,
   kECode15
};

/////////////////////////////////////////////////////////////////////////////////////////
KVINDRACodes::KVINDRACodes()
{
}


const Char_t* KVINDRACodes::GetIDStatus()
{
   //Give an explanation for the ID code

   return fCodeGenIdent[GetCodeIndex(GetIDMask())];
}

const Char_t* KVINDRACodes::GetEStatus()
{
   //Give an explanation for the calibration code

   return fCodeGenCalib[GetCodeIndex(GetEMask())];
}

void KVINDRACodes::SetVedaIDCode(UChar_t icod)
{
   //Set ID code - the argument given is the old VEDA6 code number

   SetIDMask(fIDBits[icod]);
}

void KVINDRACodes::SetVedaECode(UChar_t icod)
{
   //Set E code - the argument given is the old VEDA6 code number

   SetEMask(fEBits[icod]);
}

void KVINDRACodes::SetIDCode(UShort_t icod)
{
   //Set ID code - the argument given is one of the EVedaCodes or EIDCodes bitmasks

   SetIDMask(icod);
}

void KVINDRACodes::SetECode(UChar_t icod)
{
   //Set E code - the argument given is one of the EVedaCodes or EIDCodes bitmasks

   SetEMask(icod);
}

UChar_t KVINDRACodes::GetCodeIndex(UShort_t mask)
{
   //Argument is a bit-mask representing particle ID or E code
   //Returns the number of the bit which is set to 1 (right-most bit is 0).
   //Result is corresponding index in arrays fCodeGenIdent, fIDCodes, etc.
   int i = 0;
   if (!mask)
      return 0;
   do {
      mask = (mask >> 1);
      i++;
   }
   while (mask);
   return i - 1;
}

UChar_t KVINDRACodes::GetVedaIDCode()
{
   //Returns VEDA6 identification code

   return fIDCodes[GetCodeIndex(GetIDMask())];
}

UChar_t KVINDRACodes::GetVedaECode()
{
   //Returns VEDA6 energy calibration code

   return fECodes[GetCodeIndex(GetEMask())];
}

UShort_t KVINDRACodes::GetIDCode()
{
   //Returns identification code in the form of EVedaCodes or EIDCodes bitmask

   return GetIDMask();
}

UChar_t KVINDRACodes::GetECode()
{
   //Returns energy calibration code in the form of EVedaCodes or EIDCodes bitmask

   return GetEMask();
}

UShort_t KVINDRACodes::VedaIDCodeToBitmask(UChar_t veda_id_code)
{
   //Static method
   //Returns bitmask equivalent for old VEDA ID code
   return fIDBits[veda_id_code];
}

UChar_t KVINDRACodes::VedaECodeToBitmask(UChar_t veda_e_code)
{
   //Static method
   //Returns bitmask equivalent for old VEDA ID code
   return fEBits[veda_e_code];
}

void KVINDRACodes::Clear(Option_t*)
{
   //resets all id subcodes.
   KVINDRACodeMask::Clear();
}
