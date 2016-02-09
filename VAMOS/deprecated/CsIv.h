//Created by KVClassFactory on Thu Aug 27 10:54:29 2009
//Author: marini

#ifndef __CSIV_H
#define __CSIV_H

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

#include "Deprecation.h"
#include "KVDataSet.h"
#include "LogFile.h"
#include "Random.h"
#include "TRandom3.h"
#include "TTree.h"
#include <Riostream.h>
#include <TString.h>
#include <cmath>

//const int MAX_CSI=80;

class CsIv {

public:
   CsIv(LogFile* Log);
   virtual ~CsIv();

   LogFile* L;

   Int_t NbCsI;

   void Init(void);
   void InitSavedQuantities(void);
   void Treat(void);
   void inAttach(TTree* inT);
   void outAttach(TTree* outT);
   void CreateHistograms();
   void FillHistograms();

   Float_t*  Ped;       //CsI pedestal
   Float_t**  ECoef;  //CsI calibration coefficients

   TRandom3* Rnd;

   //energy Raw
   UShort_t* E_Raw;
   Int_t* CsIERaw;
   UShort_t* E_Raw_Nr;
   Int_t E_RawM;

   //energy time Calibrated
   Int_t   EMCSI;
   Int_t* DetCsI;

   ClassDef(CsIv, 2) //VAMOS CsI

};

#endif // __CSIV_H is not set

#ifdef __CSIV_H
DEPRECATED_CLASS(CsIv);
#endif

