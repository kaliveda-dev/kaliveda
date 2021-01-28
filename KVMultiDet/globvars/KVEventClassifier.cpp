#include "KVEventClassifier.h"

#ifndef WITH_CPP11
struct test_var {
   double VAR;
   bool operator()(double x)
   {
      return x > VAR;
   }
   test_var(double v)
   {
      VAR = v;
   }
};
#endif

int KVEventClassifier::calc_where() const
{
   std::vector<double>::const_iterator result = std::find_if(fCuts.begin(), fCuts.end(),
#ifndef WITH_CPP11
         test_var(fVar->GetValue())
#else
   [&](double x) {
      return x > fVar->GetValue();
   }
#endif
                                                            );
   return std::distance(fCuts.begin(), result);
}

ClassImp(KVEventClassifier)


