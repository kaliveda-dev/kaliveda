//$Id: KVTensPCM.cpp,v 1.1 2007/11/27 08:05:30 franklan Exp $
//Author: John Frankland
//1/3/2004
//Calculate tensor in reaction CM Frame
//

#include "KVTensPCM.h"

ClassImp(KVTensPCM)
//////////////////////////////////////////////////////////////////////////////////
//Calculates tensor in reaction CM frame
//This is the same as KVTensP after using SetFrame("CM")
//_________________________________________________________________
KVTensPCM::KVTensPCM(void)
{
//
// Createur par default
//
   SetFrame("CM");
}

//_________________________________________________________________
KVTensPCM::KVTensPCM(const Char_t* nom): KVTensP(nom, "CM")
{
//
// Constructeur avec un nom
//
}

//_________________________________________________________________
KVTensPCM::~KVTensPCM(void)
{
//
// Destructeur
//
}
