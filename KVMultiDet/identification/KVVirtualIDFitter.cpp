/*
$Id: KVVirtualIDFitter.cpp,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Apr 21 09:25:57 2008
//Author: franklan

#include "KVVirtualIDFitter.h"
#include "TEnv.h"
#include "TPluginManager.h"

ClassImp(KVVirtualIDFitter)

KVVirtualIDFitter* KVVirtualIDFitter::gDefaultFitter = 0;
KVVirtualIDFitter* KVVirtualIDFitter::GetDefaultFitter()
{
   // Static method. Creates (if necessary) and returns pointer to
   // static instanciation of default ID fitter object. The class of this
   // object is defined by the variable
   //
   // KVVirtualIDFitter.DefaultFitter:    xxxxxxxxxxx
   //
   // which can be customised in the user's .kvrootrc. The class must be defined
   // as a KVVirtualIDFitter plugin and have a default ctor with no arguments:
   //
   // +Plugin.KVVirtualIDFitter: MyFitter MyFitter MyFitter.cpp+ "MyFitter()"

   if (!gDefaultFitter) {
      TString df = gEnv->GetValue("KVVirtualIDFitter.DefaultFitter", "");
      if (df == "") {
         ::Error("KVVirtualIDFitter::GetDefaultFitter",
                 "No default fitter defined. Use variable KVVirtualIDFitter.DefaultFitter");
         return 0;
      }
      TPluginHandler* ph;
      if (!(ph = LoadPlugin("KVVirtualIDFitter", df.Data()))) return 0;
      gDefaultFitter = (KVVirtualIDFitter*) ph->ExecPlugin(0);
   }
   return gDefaultFitter;
}

KVVirtualIDFitter::KVVirtualIDFitter()
{
   // Default constructor
   fGrid = 0;
   fPad = 0;
}

KVVirtualIDFitter::~KVVirtualIDFitter()
{
   // Destructor
}

