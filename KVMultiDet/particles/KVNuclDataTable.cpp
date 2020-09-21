//Created by KVClassFactory on Wed Feb  9 11:06:42 2011
//Author: bonnet

#include "KVNuclDataTable.h"

using namespace NDT;

ClassImp(key)
ClassImp(value)

ClassImp(KVNuclDataTable)


KVNuclDataTable::KVNuclDataTable(): kclassname("KVNuclData")
{
   // Default constructor
   init();

}

//_____________________________________________
KVNuclDataTable::KVNuclDataTable(KVString classname): kclassname(classname)
{
   // Constructor with class name of the store objects
   init();


}

//_____________________________________________
KVNuclDataTable::~KVNuclDataTable()
{
   // Destructor
   if (nucMap) {
      nucMap->DeleteAll();
      delete nucMap;
   }
   if (tobj)   delete tobj;

}

//_____________________________________________
void KVNuclDataTable::init()
{
   nucMap = 0;
   tobj = 0;

   current_idx = 0;
   NbNuc = 0;
   kcomments = "";
   SetName("NuclDataTable");

}

//_____________________________________________
void KVNuclDataTable::GiveIndexToNucleus(Int_t zz, Int_t aa, Int_t ntot)
{

   //Add a new entry in the table
   nucMap->Add(new NDT::key(zz, aa), new NDT::value(ntot));
   NbNuc += 1;

}

//_____________________________________________
NDT::value* KVNuclDataTable::getNDTvalue(Int_t zz, Int_t aa) const
{
   // Return NDT::value object pointer stored at map position (Z,A).
   return (NDT::value*)nucMap->GetValue(Form("%d:%d", zz, aa));
}

//_____________________________________________
Bool_t KVNuclDataTable::IsInTable(Int_t zz, Int_t aa) const
{
   // Returns kTRUE if there is a couple (Z,A) in the table.

   return (getNDTvalue(zz, aa) != 0);
}

//_____________________________________________
KVNuclData* KVNuclDataTable::GetData(Int_t zz, Int_t aa) const
{
   // Returns the registered KVNuclData object associated to the couple (Z,A).
   // Don't need to test its presence
   //returns 0 if no such object is present

   NDT::value* val = getNDTvalue(zz, aa);
   if (val) return (KVNuclData*)tobj->At(val->Index());
   return 0;

}

//_____________________________________________
Double_t KVNuclDataTable::GetValue(Int_t zz, Int_t aa) const
{

   // Returns the value of the registered KVNuclData object associated to the couple (Z,A).
   // Don't need to test the presence of the object
   // returns -555 if no such object is present

   KVNuclData* nd = 0;
   if ((nd = GetData(zz, aa)))
      return nd->GetValue();
   else
      return -555;

}

void KVNuclDataTable::SetValue(Int_t zz, Int_t aa, Double_t val)
{
   KVNuclData* nd = 0;
   if ((nd = GetData(zz, aa))) {
      nd->SetValue(val);
   }
   else
      Error("SetValue", "No existing entry for this nucleus: Z=%d, A=%d", zz, aa);

}

//_____________________________________________
const Char_t*  KVNuclDataTable::GetUnit(Int_t zz, Int_t aa) const
{

   // Returns the unit of the registered KVNuclData object associated to the couple (Z,A).
   // Don't need to test the presence of the object
   // returns "NONE" if no such object is present

   KVNuclData* nd = 0;
   if ((nd = GetData(zz, aa)))
      return nd->GetUnit();
   else
      return "NONE";

}

//_____________________________________________
Bool_t KVNuclDataTable::IsMeasured(Int_t zz, Int_t aa) const
{

   // Returns bit of the registered KVNuclData object associated to the couple (Z,A).
   // indicating if the value are measured (kTRUE) or not is from systematic (kFALSE)
   // Don't need to test the presence of the object
   // returns "NONE" if no such object is present

   KVNuclData* nd = 0;
   if ((nd = GetData(zz, aa)))
      return nd->IsMeasured();
   else
      return kFALSE;

}

//_____________________________________________
Int_t KVNuclDataTable::GetNumberOfNuclei() const
{

   return NbNuc;

}

//_____________________________________________
const Char_t*  KVNuclDataTable::GetReadFileName() const
{

   return GetTitle();

}

//_____________________________________________
KVString KVNuclDataTable::GetCommentsFromFile() const
{

   return kcomments;

}

//_____________________________________________
void KVNuclDataTable::CreateTable(Int_t ntot)
{

   cl = TClass::GetClass(kclassname.Data());
   tobj = new TObjArray(ntot);

}

//_____________________________________________
void KVNuclDataTable::CreateElement(Int_t idx)
{

   current_idx = idx;
   tobj->Add((KVNuclData*)cl->New());

}

//_____________________________________________
void KVNuclDataTable::InfoOnMeasured() const
{
   TString line = "";
   line += "Information concerning value status (measured or deduced)";
   line += " IS NOT available, Methodes KVNuclDataTable::IsMeasured and";
   line += " KVNuclData::IsMeasured will always return kFALSE";

   Warning("InfoOnMeasured", "%s", line.Data());


}
