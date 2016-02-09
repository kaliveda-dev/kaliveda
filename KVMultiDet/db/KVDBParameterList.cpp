//Created by KVClassFactory on Fri Sep 11 15:25:54 2015
//Author: ,,,

#include "KVDBParameterList.h"

ClassImp(KVDBParameterList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDBParameterList</h2>
<h4>To store in a database some parameters list</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//---------------------
KVDBParameterList::KVDBParameterList()
//---------------------
{
   // Default constructor
   fParameters = new KVNameValueList();
}


//---------------------
KVDBParameterList::KVDBParameterList(const Char_t* name, const Char_t* title) : KVDBRecord(name, title)
//---------------------
{
   // Write your code here
   fParameters = new KVNameValueList();
}

//---------------------
KVDBParameterList::~KVDBParameterList()
//---------------------
{
   // Destructor
   delete fParameters;
}

//---------------------
KVNameValueList* KVDBParameterList::GetParameters() const
//---------------------
{
   return fParameters;

}

//---------------------
void KVDBParameterList::Print(Option_t* option) const
//---------------------
{

   TString opt(option);
   opt.ToUpper();
   if (opt == "PAR") {
      GetParameters()->Print();
   } else if (opt == "ALL") {
      KVDBRecord::Print(option);
      GetParameters()->Print();
   } else {
      KVDBRecord::Print(option);
   }

}
