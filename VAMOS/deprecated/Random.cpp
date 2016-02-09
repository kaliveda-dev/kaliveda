#include "Random.h"

//Author: Maurycy Rejmund

/**
   WARNING: This class has been deprecated and will eventually be removed. Do
   not use!

   This class is only compiled if the deprecated code is enabled in the build
   configuration (e.g. cmake -DUSE_DEPRECATED_VAMOS=yes). If you enable the
   deprecated code then a large number of warnings will be printed to the
   terminal. To disable these warnings (not advised) compile VAMOS with
   -Wno-deprecated-declarations. Despite these warnings the code should compile
   just fine. The warnings are there to prevent the unwitting use of the
   deprecated code (which should be strongly discouraged).

   BY DEFAULT THIS CLASS IS NOT COMPILED.

   Deprecated by: Peter Wigg (peter.wigg.314159@gmail.com)
   Date:          Thu 17 Dec 17:24:38 GMT 2015
*/

using namespace std;

ClassImp(Random)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Random</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Random::Random(void)
{
#ifdef DEBUG
   cout << "Random::Constructor" << endl;
#endif

   Array = Ptr = new Float_t[255];
   if (!Array) {
      cout << "Coud not allocate memory to hold Array !" << endl;
      exit(EXIT_FAILURE);
   }

   for (Int_t i = 0; i < 255; i++)
      *(Ptr++) = ((Float_t) i) / 254.0 - 0.5;

   Ptr = Array;
}
Random::~Random(void)
{
#ifdef DEBUG
   cout << "Random::Destructor" << endl;
#endif
   delete Array;
}

Float_t Random::Next(void)
{
#ifdef DEBUG
   //  cout << "Random::R()" << endl;
#endif
   if (Ptr < Array + 254)
      Ptr++;
   else
      Ptr = Array;

   return (*Ptr);
}
Float_t Random::Value(void)
{
#ifdef DEBUG
   //  cout << "Random::(Float_t)R()" << endl;
#endif
   return (*Ptr);
}


