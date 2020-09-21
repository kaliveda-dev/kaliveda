/*
$Id: KVDP2toCsIGridConvertor.h,v 1.1 2008/10/13 14:25:08 franklan Exp $
$Revision: 1.1 $
$Date: 2008/10/13 14:25:08 $
*/

//Created by KVClassFactory on Thu Oct  9 17:23:14 2008
//Author: franklan

#ifndef __KVDP2TOCSIGRIDCONVERTOR_H
#define __KVDP2TOCSIGRIDCONVERTOR_H

#include "KVDP2toIDGridConvertor.h"

/**
\class KVDP2toCsIGridConvertor
\brief Converts dp2-format CsI R-L grids to KVIDGrids
\ingroup Identification
*/

class KVDP2toCsIGridConvertor : public KVDP2toIDGridConvertor {
protected:
   virtual void ReadGammaFile(const Char_t* gammafile);

public:
   KVDP2toCsIGridConvertor();
   virtual ~KVDP2toCsIGridConvertor();

   void Convert(const Char_t* id_and_imf_file, const Char_t* gamma_file);

   ClassDef(KVDP2toCsIGridConvertor, 1) //Converts dp2-format CsI R-L grids to KVIDGrids
};

#endif
