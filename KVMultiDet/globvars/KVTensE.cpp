/*
$Id: KVTensE.cpp,v 1.1 2008/04/11 09:58:58 franklan Exp $
$Revision: 1.1 $
$Date: 2008/04/11 09:58:58 $
*/

//Created by KVClassFactory on Fri Apr 11 11:34:23 2008
//Author: John Frankland

#include "KVTensE.h"

ClassImp(KVTensE)

////////////////////////////////////////////////////////////////////////////////
/*
BEGIN_HTML
<h2>KVTensE</h2>
<h4>Kinetic energy tensor</h4>
We calculate the tensor:
END_HTML
BEGIN_LATEX
Q_{ij} = \sum_{\nu} \frac{p^{i}_{\nu} p^{j}_{\nu}}{m_{\nu}(\gamma_{\nu}+1)}
END_LATEX
BEGIN_HTML
from particle momenta.
END_HTML
*/
////////////////////////////////////////////////////////////////////////////////


void KVTensE::Fill(KVNucleus* c)
{
   //Add the particle's contribution to the tensor.
   //
   //This particle will be rejected if:
   //(1) it has Z < Zmin
   //(2) it does not have the name/label defined with SetPartLabel**
   //
   // **if no label defined, only Z>=Zmin condition is used.
   //
   //By default the momentum vector in the default (laboratory) frame is used.
   //To calculate in another frame, use KVVarGlob::SetFrame() before
   //calculation begins

   // if method is called by FillWithCondition i.e. by KVGVList::CalculateGlobalVariables
   // (conditioned_fill==kTRUE), all selections have already been tested
   // if not i.e. Fill method called "by hand", we need to test here

   if (conditioned_fill) { // || fSelection.Test(c)) {
      Double_t poids = c->GetMass();
      if (poids) poids = 1. / (poids * (c->GetFrame(GetFrame(), kFALSE)->Gamma() + 1.));
      tenseurP->Fill(c->GetFrame(GetFrame(), kFALSE)->GetMomentum(), poids);
   }
}
