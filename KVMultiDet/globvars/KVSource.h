#ifndef __KVSource_H
#define __KVSource_H

#include "KVVGObjectSum.h"
#include "KVNucleus.h"

/**
  \class KVSource
  \ingroup GlobalVariables
  \brief Simple source reconstruction global variable

 It takes all nuclei selected with SetSelection(KVParticleCondition&) and adds them
 together (charge, mass, momentum, energy) in order to calculate
 a 'source' nucleus, for which we can obtain various quantities (see below).
 The reference frame from which particle momenta are taken can be set with SetFrame("..").
 Each nucleus which is included in the source definition is added
 to a group with the same name as this variable. This can be used for further analysis
 of the particles (by testing KVParticle::BelongsToGroup("...") method).

 Index     Name       Meaning
-----------------------------------------------------
   0        Ex        Excitation Energy
   1        Z         Z source
   2        A         A source
   3        Vz        z component of velocity
   4        Vx        x component of velocity
   5        Vy        y component of velocity
   6        Theta     polar angle relative to the beam
   7        Mult      number of nuclei added to source

 These quantities can be obtained either by name:

      Double_t estar = GetValue("Ex");

 or by index:

      Double_t estar = GetValue(0);

 or in an array:

     Double_t *vals = GetValuePtr();
     Double_t estar = vals[0]

 */

class KVSource: public KVVGObjectSum<KVNucleus> {
   Int_t fMult;//! source multiplicity

protected:
   void init();
   Double_t getvalue_int(Int_t i) const;
   void fill(const KVNucleus* n)
   {
      Add(*n);
      ++fMult;
   }

public:
   KVSource() : KVVGObjectSum<KVNucleus>("KVSource")
   {
      init();
   }
   KVSource(const Char_t* name) : KVVGObjectSum<KVNucleus>(name)
   {
      init();
   }
   virtual ~KVSource(void) {}

   void Copy(TObject& a) const
   {
      KVVGObjectSum::Copy(a);
      dynamic_cast<KVSource&>(a).fMult = fMult;
   }
   void Init() {}
   void Reset()
   {
      KVVGObjectSum::Reset();
      fMult = 0;
   }
   Char_t GetValueType(Int_t) const;

   ClassDef(KVSource, 1) //Simple source reconstruction global variable

};
#endif
