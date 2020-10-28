#include "KVDirectivity.h"

ClassImp(KVDirectivity)

////////////////////////////////////////////////////////////////////////////////////
/// Copy state of 'this' object into the KVDirectivity object referenced by 'a'.
/// This is needed for the automatically generated copy constructor and copy
/// assignment operator to work correctly.

void KVDirectivity::Copy(TObject& a) const
{

   KVVarGlob1::Copy(a);// copy attributes of KVVarGlob1 base object

   KVDirectivity& aglob = (KVDirectivity&)a;
   aglob.total_mod_trans = total_mod_trans;
   aglob.total_trans = total_trans;
}

//////////////////////////////////////////////////////////////////////
/// Calculation of global variable value(s) after filling is finished

void KVDirectivity::Calculate(void)
{
   double mod_tot_trans = total_trans.Mag();
   SetValue(mod_tot_trans / total_mod_trans);
}

/////////////////////////////////////////////////////////////
/// Private initialisation method called by all constructors.
/// All member initialisations should be done here.

void KVDirectivity::init()
{
   fType = KVVarGlob::kOneBody; // this is a 1-body variable
#ifdef USING_ROOT6
   SetSelection({"V>Vcm", [](const KVNucleus * n)
   {
      return n->GetVpar() > 0;
   }
                });
#else
   SetSelection("_NUC_->GetVpar()>0");
#endif
}

////////////////////////////////////////////////////////////////////////////////
/// Calculation of contribution to 1-body global variable of nucleus n
/// Use the FillVar(v) method to increment the quantity of the global variable.
/// The value, v, is to be evaluated from the properties of the
/// KVNucleus passed as argument. For example, to evaluate the sum of the charge
/// of all fragments, you may proceed as follows:
/// FillVar(n->GetZ());

void KVDirectivity::fill(const KVNucleus* n)
{
   TVector3 trans_mom = n->GetTransverseMomentum();
   total_trans += trans_mom;
   total_mod_trans += trans_mom.Mag();
}

