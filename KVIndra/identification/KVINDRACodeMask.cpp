/***************************************************************************
$Id: KVINDRACodeMask.cpp,v 1.2 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVINDRACodeMask.h"

ClassImp(KVINDRACodeMask)
////////////////////////////////////////////////////////////////////////////////////////////////////////
//KVINDRACodeMask
//
//Handles bit-masks used to describe identification & calibration codes for particles,
//and also used to define which particles are to be considered "ok" or "good" for analysis
//(i.e. which particles have IsOK()=kTRUE).
//
//For compatibility with old VEDA6 codes, a set of bit-mask constants are defined to
//describe ID and energy codes:
//  ID codes:  kIDCode0, kIDCode1, ..., kIDCode15
//  E  codes:  kECode0, kECode1, ..., kECode15
//
//_____________________________________________________________________________________
//Let us recall the meaning of the VEDA_6.INCL codes :
//
//                         code = 0 : gamma
//                         code = 1 : neutron (seulement couronnes 2 a 9)
//                         code = 2 : part. ident. dans CsI ou Phoswich (OK + 8Be)
//                         code = 3 : fragment identifie dans Si-CsI
//                                                ou fragment ident. dans Si75-SiLi ou SiLi-CsI
//                         code = 4 : fragment identifie dans ChIo-Si
//                                                ou fragment ou part. identifie dans ChIo-Si75
//                         code = 5 : fragment non-ident. (montee de Bragg)(Z min.)
//                                                                 ou stoppe dans Chio (Z min)
//                         code = 6 : frag. cree par la coherence CsI  (stoppe ds Si.)
//                         code = 7 : frag. cree par la coherence ChIo (stoppe ds ChIo)
//                         code = 8 : multiple comptage dans ChIo avec arret
//                                                                 - dans les siliciums (cour. 2-9)
//                                                                 - dans les CsI (cour. 10-17)
//                         code = 9 : ident. incomplete dans CsI ou Phoswich (Z.min)
//                         code =10 : ident. "entre les lignes" dans CsI
//                         code =11 : ident. Z=2 due a l'inversion CHIO (Argon)
//
//                         code =14 : Particule restee non identifiee (a rejeter)
//                         code =15 : frag. ou part. rejete
//
//    Nota: Pour le code 8 la contribution ChIo est partagee egalement entre
//               toutes les particules qui s'arretent dans les modules suivants.
//               Le Z individuel est donc surement faux mais cela permet
//               d'avoir une assez bonne estimation du Z.total.
//
//               Les codes 9 et 10 sont des codes qui ont ete ajoutes pour donner
//               au code 2 la notion d'identification sans ambiguites.
//____________________________________________________________________________________
//
//NEW ID CODE MASKS
//-----------------
//For greater ease of use, we also define new bit-mask constants which are equivalent
//to the previous ones, but have names which make their meaning more clear (and easier to
//remember) :
//
//              kIDCode_Gamma = kIDCode0,
//              kIDCode_Neutron = kIDCode1,
//              kIDCode_CsI = kIDCode2,
//              kIDCode_Phoswich = kIDCode2,
//              kIDCode_SiCsI = kIDCode3,
//              kIDCode_SiLiCsI = kIDCode3,
//              kIDCode_Si75SiLi = kIDCode3,
//              kIDCode_ChIoSi = kIDCode4,
//              kIDCode_ChIoCsI = kIDCode4,
//              kIDCode_ChIoSi75 = kIDCode4,
//              kIDCode_Bragg = kIDCode5,
//              kIDCode_Zmin = kIDCode5,
//              kIDCode_ZminCsI = kIDCode9,
//              kIDCode_ArretChIo = kIDCode5,
//              kIDCode_ArretSi = kIDCode5,
//              kIDCode_MultihitChIo = kIDCode8
//
//
//DEFINING CODE MASKS
//-------------------
//Code masks defining acceptable particles for events are defined using the predefined
//bit-masks and the bit-wise "OR" as follows:
//
//  SetIDMask( kIDCode2 | kIDCode3 | kIDCode 4 | kIDCode6 );   // ID codes 2-4 & 6 are accepted, all others rejected
//  SetEMask ( kECode1 | kECode2 );                            // accept calibration codes 1 & 2
//
//
//TESTING CODE MASKS
//------------------
//You can test the identification code of a particle using the TestIDCode method (see
//KVINDRAReconNuc on how to obtain the particle's code mask):
//
//      Bool_t idchiosi = TestIDCode( kIDCode_ChIoSi );
//
//In this example, idchiosi=kTRUE if the particle's ID code indicates detection in a
//ChIo-Si telescope.
//
//This method also works to test if a given code is accepted by the mask set for an
//event (see KVINDRAReconEvent).
//
Bool_t KVINDRACodeMask::TestIDCode(UShort_t code)
{
   //check identification code against code mask
   //always kTRUE if no mask set (fIDMask==0)
   if (!fIDMask)
      return kTRUE;
   return (Bool_t)((fIDMask & code) != 0);
}

Bool_t KVINDRACodeMask::TestECode(UChar_t code)
{
   //check calibration code against code mask
   //always kTRUE if no mask set (fEMask==0)
   if (!fEMask)
      return kTRUE;
   return (Bool_t)((fEMask & code) != 0);
}
