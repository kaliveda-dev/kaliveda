/***************************************************************************
                          kvacqparam.cpp  -  description
                             -------------------
    begin                : Wed Nov 20 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVACQParam.cpp,v 1.23 2007/12/06 15:12:54 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Riostream.h"
#include "KVEBYEDAT_ACQParam.h"
#include "TRandom.h"

using namespace std;

ClassImp(KVEBYEDAT_ACQParam)

void KVEBYEDAT_ACQParam::init()
{
//Default initialisations
   fChannel = nullptr;
   fFired = nullptr;
   fData = 0;
   fNbBits = 16;
}

//_________________________________________________________________________
KVEBYEDAT_ACQParam::KVEBYEDAT_ACQParam()
   : KVBase()
{
   //default constructor
   init();
}

//_________________________________________________________________________
KVEBYEDAT_ACQParam::KVEBYEDAT_ACQParam(const TString& name, const TString& type)
   : KVBase(name, type)
{
   // Make acquisition parameter with given name & type.
   init();
}

//
KVEBYEDAT_ACQParam::KVEBYEDAT_ACQParam(const KVEBYEDAT_ACQParam& obj) : KVBase()
{
   //Copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVEBYEDAT_ACQParam&) obj).Copy(*this);
#endif
}

////////////////////////////////////////////////////////////////////////////
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVEBYEDAT_ACQParam::Copy(TObject& obj) const
#else
void KVEBYEDAT_ACQParam::Copy(TObject& obj)
#endif
{
   //
   //Copy this to obj
   //
   KVBase::Copy(obj);
   ((KVEBYEDAT_ACQParam&) obj).SetData(GetData());
   ((KVEBYEDAT_ACQParam&) obj).SetNbBits(GetNbBits());
}

void KVEBYEDAT_ACQParam::Print(Option_t*) const
{
   cout << "_________________________________________" << endl;
   cout << " KVACQParam: " << GetName() << " " << GetType() << endl;
   cout << " Data = " << GetData() << endl;
   cout << "_________________________________________" << endl;
}

void KVEBYEDAT_ACQParam::ls(Option_t*) const
{
   //Dump name of parameter, raw coder value, and "randomised" value
   cout << ClassName() << " : " << GetName() << " raw=" << GetData() << endl;
}
