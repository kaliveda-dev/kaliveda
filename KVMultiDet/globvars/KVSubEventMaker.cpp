#include "KVSubEventMaker.h"
#include <random>
#include <algorithm>
ClassImp(KVSubEventMaker)

////////////////////////////////////////////////////////////////////////////////////
/// Copy state of 'this' object into the KVSubEventMaker object referenced by 'a'.
/// This is needed for the automatically generated copy constructor and copy
/// assignment operator to work correctly.

void KVSubEventMaker::Copy(TObject& a) const
{
   KVVarGlob::Copy(a);// copy attributes of KVVarGlob base object

   // Now copy any additional attributes specific to this class:
   // To copy a specific field, do as follows:
   //
   //     aglob.field=field;
   //
   // If setters and getters are available, you can also proceed as follows
   //
   //    aglob.SetField(GetField());
   //
}

/////////////////////////////////////////////////////////////////////////////////
/// Initialisation of internal variables, called once before beginning treatment

void KVSubEventMaker::Init(void)
{
}

///////////////////////////////////////////////////////////////////
/// Reset internal variables, called before treatment of each event

void KVSubEventMaker::Reset(void)
{
   particles.clear();
}

//////////////////////////////////////////////////////////////////////
/// Calculation of global variable value(s) after filling is finished

void KVSubEventMaker::Calculate(void)
{
   // Set group for required fraction of particles (starting from the first one)

   auto mult = particles.size();
   // perform a random shuffle of the particles in case there is a correlation
   // between position in the event and type of particle, where it was detected,
   // etc. etc.
   std::random_device rd;
   std::mt19937 g(rd());
   std::shuffle(particles.begin(), particles.end(), g);

   auto required = mult * GetParameter("FRACTION");
   // nothing guarantees that the required number of particles is a whole number
   auto req_int = std::floor(required);
   auto req_frac = required - req_int;
   // if so, we use req_int particles with probability 1-req_frac and req_int+1
   // particles with probability req_frac. The mean number of particles used
   // is exactly the value 'required'.
   if (req_frac > 1.e-10) {
      if (gRandom->Uniform() < req_frac) ++req_int;
   }
   int i = 0;
   for (auto p : particles) {
      if (i < req_int) p->AddGroup(GetName());
      ++i;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Protected method, called by GetValue(Int_t) and GetValue(const Char_t*)
/// If your variable calculates several different values, this method allows to access each value
/// based on a unique index number.
///
/// You should implement something like the following:
///
///   switch(index){
///         case 0:
///            return val0;
///            break;
///         case 1:
///            return val1;
///            break;
///   }
///
/// where 'val0' and 'val1' are the internal variables of your class corresponding to the
/// required values.
///
/// In order for GetValue(const Char_t*) to work, you need to associate each named value
/// with an index corresponding to the above 'switch' statement, e.g.
///
///     SetNameIndex("val0", 0);
///     SetNameIndex("val1", 1);
///
/// This should be done in the init() method.

Double_t KVSubEventMaker::getvalue_int(Int_t index) const
{

   return 0;
}

void KVSubEventMaker::fill(const KVNucleus* n)
{
   // add to internal list
   particles.push_back(const_cast<KVNucleus*>(n));
}

///////////////////////////////////////////////////////////////////////////////
/// Private initialisation method called by all constructors.
/// All member initialisations should be done here.
/// You should also (if your variable calculates several different quantities)
/// set up a correspondance between named values and index number
/// using method SetNameIndex(const Char_t*,Int_t)
/// in order for GetValue(const Char_t*) to work correctly.
/// The index numbers should be the same as in your getvalue_int(Int_t) method.

void KVSubEventMaker::init()
{
   // PRIVATE method
   fType = KVVarGlob::kOneBody; // this is a N-body variable
}

