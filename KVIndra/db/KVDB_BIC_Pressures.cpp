/*
$Id: KVDB_BIC_Pressures.cpp,v 1.3 2007/02/14 14:13:05 franklan Exp $
$Revision: 1.3 $
$Date: 2007/02/14 14:13:05 $
*/

//Created by KVClassFactory on Tue Feb 13 18:37:09 2007
//Author: franklan

#include "KVDB_BIC_Pressures.h"

using namespace std;

ClassImp(KVDB_BIC_Pressures)


KVDB_BIC_Pressures::KVDB_BIC_Pressures()
{
   //Default constructor
   init();
}

KVDB_BIC_Pressures::KVDB_BIC_Pressures(Float_t* p)
{
   //Default constructor
   init();
   SetPressures(p);
}

void KVDB_BIC_Pressures::init()
{
   TString name = GetName();
   name.ReplaceAll("ChIo", "BIC");
   SetName(name.Data());
   SetTitle("BIC Pressures set");
}

KVDB_BIC_Pressures::~KVDB_BIC_Pressures()
{
   //Destructor
}

void KVDB_BIC_Pressures::Print(Option_t*) const
{
   cout << "_____________________________________________________________"
        << endl << "BIC Pressures :" << endl << GetName() << " " <<
        GetTitle() << endl << "BIC_1  : " << fPressures[BIC_1] << endl
        << "BIC_2  : " << fPressures[BIC_2] << endl << "BIC_3  : "
        << fPressures[BIC_3] << endl <<
        "______________________________________________________________" <<
        endl;
}
