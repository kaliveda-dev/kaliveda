/***************************************************************************************
$Id: KVTGIDFunctions.h,v 1.6 2009/03/03 14:27:15 franklan Exp $
                        KVTGIDFunctions.h

         Contains Tassan-Got functionals for mass and charge
         identification used for Si-CsI, Si150-CsI and ChIo-CsI, and ChIo-Si matrices.
***************************************************************************************/

#ifndef __KVTGIDFUNCTIONS_H
#define __KVTGIDFUNCTIONS_H

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

/**
\namespace KVTGIDFunctions
\ingroup Identification

  Contains Tassan-Got functionals for mass and charge
  identification used for Si-CsI, Si150-CsI and ChIo-CsI, and ChIo-Si matrices.

<p>
For xxx-CsI telescopes:<br>
tassangot_A and tassangot_Z are the original 10 parameter functionals<br>
(corresponds to Eq.10 with Eq.12 (E vs. h) from <a href="http://arxiv.org/abs/nucl-ex/0103004">Nucl.Instrum.Meth. B194 (2002) 503-512</a>).<br>
pawlowski_A and pawlowski_Z are modified functionals with 11 parameters.
</p>

In all cases here is the correspondance between par[] and the formulae<br>
given in <a href="http://arxiv.org/abs/nucl-ex/0103004">Nucl.Instrum.Meth. B194 (2002) 503-512</a> (with initial values and limits used when fitting<br>
parameters from same publication) :<br>
</p>


       par[0] : lambda
       par[1] : alpha (1: 0.5 - 1.5)
       par[2] : beta (0.5: 0.2 - 1)
       par[3] : mu (1: 0.2 - 1.5)
       par[4] : nu (1: 0.1 - 4)
       par[5] : xi (>0)
       par[6] : g
       par[7] : pedestal of x-coordinate
       par[8] : pedestal of y-coordinate

<br>
  In case of xxx-CsI telescopes, the relationship between E and h introduces
  an additional parameter:


       par[9] : eta (Eq.11)


  lambda and g are determined from starting points (E=0) and initial slopes
  of lines, respectively, and their ranges are given as between 0.25 and 4 times
  these values.
  If no A identification is given, beta is fixed at beta=0.5
<br>
  With the Pawlowski modification, we have also:

       par[10] : Pawlowski correction

*/
namespace KVTGIDFunctions {
   Double_t fede(Double_t* x, Double_t* par);
   Double_t chiosi_Z(Double_t* x, Double_t* par);
   Double_t tassangot_Z(Double_t* x, Double_t* par);
   Double_t pichon_Z(Double_t* x, Double_t* par);
   Double_t tassangot_A(Double_t* x, Double_t* par);
   Double_t pawlowski_Z(Double_t* x, Double_t* par);
   Double_t pawlowski_A(Double_t* x, Double_t* par);
   Double_t starting_points_Z(Double_t* x, Double_t* par);
   Double_t starting_points_A(Double_t* x, Double_t* par);
}
#endif
