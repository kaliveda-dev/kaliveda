/*
$Id: KVINDRAUpDater_e416a.cpp,v 1.1 2007/02/14 14:12:00 franklan Exp $
$Revision: 1.1 $
$Date: 2007/02/14 14:12:00 $
*/

//Created by KVClassFactory on Wed Feb 14 12:54:25 2007
//Author: franklan

#include "KVINDRAUpDater_e416a.h"
#include "KVRList.h"
#include "KVDBRun.h"
#include "KVDB_BIC_Pressures.h"
#include "Riostream.h"
#include "KVINDRA.h"
#include "KVINDRADB.h"

using namespace std;

ClassImp(KVINDRAUpDater_e416a)

KVINDRAUpDater_e416a::KVINDRAUpDater_e416a()
{
   //Default constructor
}

KVINDRAUpDater_e416a::~KVINDRAUpDater_e416a()
{
   //Destructor
}

void KVINDRAUpDater_e416a::SetChIoPressures(KVDBRun* kvrun)
{
   //Update ChIo and BIC pressures for this run with values in database

   KVINDRAUpDater::SetChIoPressures(kvrun);

   //set BIC pressures
   KVRList* param_list = kvrun->GetLinks("BIC Pressures");
   if (!param_list) {
      return;
   }
   if (param_list->GetSize() > 1) {
      Warning("SetChIoPressures",
              "There are %d sets of BIC pressures for this run. Check file %s",
              param_list->GetSize(), gIndraDB->GetCalibFileName("BICPressures"));
   }
   KVDB_BIC_Pressures* kvps = (KVDB_BIC_Pressures*)param_list->At(0);
   cout << "--> Setting BIC pressures" << endl;
   cout << "       BIC_1: " << kvps->
        GetPressure(BIC_1) << " Torr" << endl;
   cout << "       BIC_2: " << kvps->
        GetPressure(BIC_2) << " Torr" << endl;
   cout << "       BIC_3: " << kvps->
        GetPressure(BIC_3) << " Torr" << endl;
   gIndra->GetDetector("BIC_1")->SetPressure(kvps->GetPressure(BIC_1));
   gIndra->GetDetector("BIC_2")->SetPressure(kvps->GetPressure(BIC_2));
   gIndra->GetDetector("BIC_3")->SetPressure(kvps->GetPressure(BIC_3));
}

