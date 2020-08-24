//Created by KVClassFactory on Fri Sep 11 15:25:54 2015
//Author: ,,,

#include "KVDBParameterList.h"

ClassImp(KVDBParameterList)

void KVDBParameterList::Print(Option_t* option) const
//---------------------
{

   TString opt(option);
   opt.ToUpper();
   if (opt == "PAR") {
      GetParameters()->Print();
   }
   else if (opt == "ALL") {
      KVDBRecord::Print(option);
      GetParameters()->Print();
   }
   else {
      KVDBRecord::Print(option);
   }

}
