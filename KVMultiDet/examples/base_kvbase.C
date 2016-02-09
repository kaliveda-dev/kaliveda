//# Example of use of the KVBase class
//
// This is the base class of most classes in the KaliVeda framework.
// It provides information on the version & installation layout of
// KaliVeda, plus various general "utility" methods.
//
// To execute this function, either do
//
// $ kaliveda
// root[0] .L base_kvbase.C+
// root[1] kvbase_example()
//
// or
//
// $ root
// root[0] KVBase::InitEnvironment()
// root[1] .L base_kvbase.C+
// root[2] kvbase_example()

#include "KVBase.h"
#include "TList.h"
#include <iostream>

void kvbase_example()
{
   KVBase base_obj;
   base_obj.SetLabel("KVBase object");
   TNamed tobject;

   // list of mixed KVBase- & TObject-derived objects
   TList list_obj;
   list_obj.Add(&base_obj);
   list_obj.Add(&tobject);

   TIter next_obj(&list_obj);
   TObject* obj_ptr;
   while ((obj_ptr = next_obj())) {

      // test if object is KVBase-based
      if (obj_ptr->TestBit(KVBase::kIsKaliVedaObject)) {
         // call a KVBase-only method
         std::cout << "KVBase label = " <<
                   dynamic_cast<KVBase*>(obj_ptr)->GetLabel()
                   << std::endl;
      }

   }

   // look for location of an executable on the system
   TString path_to_kaliveda = "kaliveda-config";
   if (KVBase::FindExecutable(path_to_kaliveda)) {
      // found executable
      std::cout << "kaliveda-config executable located at "
                << path_to_kaliveda << std::endl;
   }
}
