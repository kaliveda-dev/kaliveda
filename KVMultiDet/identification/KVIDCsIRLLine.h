/***************************************************************************
                          KVIDCsIRLLine.h  -  description
                             -------------------
    begin                : Nov 10 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDCsIRLLine.h,v 1.6 2009/03/03 13:36:00 franklan Exp $
***************************************************************************/

#ifndef KVIDCsIRLLine_H
#define KVIDCsIRLLine_H

#include "KVIDZALine.h"

/**
   \class KVIDCsIRLLine
\brief Base class for CsI R-L identification lines (A and Z identification).
\ingroup Identification

The particularity of such lines is the order in which they appear on the grid
(in order of increasing R):
p, d, t, 3He, 4He, 6He, *** 8Be ***, 6Li, 7Li, 8Li, 9Li, 7Be, 9Be, ...
*/

class KVIDCsIRLLine: public KVIDZALine {


protected:

   virtual void SetAsymWidth(Double_t d_l, Double_t d_r);

public:

   KVIDCsIRLLine();
   KVIDCsIRLLine(const KVIDCsIRLLine&);

   virtual ~ KVIDCsIRLLine();
   Int_t Compare(const TObject*) const;

   void Print(Option_t* opt = "") const;

   ClassDef(KVIDCsIRLLine, 3)   //Base class for CsI R-L identification lines
};

#endif
