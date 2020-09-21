//Created by KVClassFactory on Wed Feb 23 10:21:20 2011
//Author: bonnet

#include "KVNuclData.h"
#include "Riostream.h"

using namespace std;

ClassImp(KVNuclData)

KVNuclData::KVNuclData(const Char_t* name, const Char_t* units): fName(name), fUnits(units)
{
   //Constructor with name & units
   init();

}

KVNuclData::KVNuclData(const KVNuclData& o) : TObject()
{
   // Copy constructor
   o.Copy(*this);
}

//_________________________________
KVNuclData::KVNuclData(): fName(), fUnits("NONE")
{
   // Default constructor
   init();
}

//_________________________________
KVNuclData::~KVNuclData()
{
   // Destructor
}

void KVNuclData::Copy(TObject& object) const
{
   // Copy this to object
   TObject::Copy(object);
   KVNuclData& nd = (KVNuclData&)object;
   nd.kValue = kValue;
   nd.fName = fName;
   nd.fUnits = fUnits;
   nd.SetMeasured(IsMeasured());
}

//_________________________________
void KVNuclData::init()
{

   SetMeasured(kFALSE);
   SetValue(0);
}


//_________________________________
const Char_t* KVNuclData::GetName() const
{

   return fName.Data();

}

//_________________________________
void KVNuclData::SetValue(Double_t val)
{

   kValue = val;

}

//_________________________________
Double_t KVNuclData::GetValue() const
{

   return kValue;

}

//_________________________________
void KVNuclData::Print(Option_t*) const
{

   TString post = " ";
   if (!IsMeasured()) post += "(SYST)";

   cout << ClassName() << " : " << GetName() << " " << GetValue() << " " << GetUnit() << " " << post.Data() << endl;
}

