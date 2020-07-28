//Created by KVClassFactory on Wed Jun  4 17:27:05 2014
//Author: get_indra

#include "KVColouredBase.h"

ClassImp(KVColouredBase)

KVColouredBase::KVColouredBase() : fColour("white")
{
   // Default constructor
}

//________________________________________________________________

KVColouredBase::KVColouredBase(const Char_t* name, const Char_t* title, const Char_t* colour)
   : KVBase(name, title), fColour(colour)
{
   // Write your code here
}

KVColouredBase::~KVColouredBase()
{
   // Destructor
}

