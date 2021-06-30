#ifndef __KVSUBEVENTMAKER_H
#define __KVSUBEVENTMAKER_H

#include "KVVarGlob.h"

#include "KVEvent.h"

/**
 \class KVSubEventMaker
 \brief Global variable for defining random subevents
  \ingroup GlobalVariables

 This variable will create a group for a given fraction \f$0<f<1\f$ of all particles in an event, chosen at random.
 The name of the variable is used as the name for the group. The fraction is given as a parameter.

 Example of use:
 ~~~~{.cpp}
 KVSubEventMaker S("SubEvent1");
 S.SetParameter("FRACTION", 0.5);
 ~~~~

 For each event, a random subset of half of the particles will be added to the group `"SubEvent1"`.

 If the required fraction does not correspond to a whole number of particles, the number of particles
 included in the subevent will be drawn at random so that the mean number of particles in all subevents
 for a given total multiplicity \f$N\f$ is equal to \f$fN\f$. In other words, the probability distribution
 of the number of particles \f$n\f$ in each subevent will be
 \f[
 P(n) = \delta(n-n_0)(1-x)+\delta(n-(n_0+1))x
 \f]
 with
 \f[
 n_0=\lfloor fN\rfloor, x=fN-n_0
 \f]

\author John Frankland
 \date Fri Jun 25 17:36:45 2021
*/

class KVSubEventMaker: public KVVarGlob {

   std::vector<KVNucleus*> particles;

protected:
   Double_t getvalue_int(Int_t) const;

   void fill(const KVNucleus*);

public:
   KVSubEventMaker()
      : KVVarGlob()
   {
      init();
   }
   KVSubEventMaker(const Char_t* nom)
      : KVVarGlob(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVSubEventMaker, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVSubEventMaker)

   virtual ~KVSubEventMaker() {}

   void Copy(TObject& obj) const;

   void Init();
   void Reset();
   void Calculate();

private:
   void init();

   ClassDef(KVSubEventMaker, 1) //Global variable for defining subevents
};

#endif
