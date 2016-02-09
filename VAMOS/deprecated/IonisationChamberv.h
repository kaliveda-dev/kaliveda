#ifndef _IONCHAMBER_CLASS
#define _IONCHAMBER_CLASS

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
#include "Rtypes.h"
#include "TTree.h"
#include <cmath>

//const int MAXIC=7;

class IonisationChamberv {
   Bool_t Ready;

public:
   IonisationChamberv(LogFile* LOg);
   virtual ~IonisationChamberv(void);

   LogFile* L;

   Int_t NbChio;
   //bool Present;

   void Init(void);
   void InitSavedQuantities(void);
   void Calibrate(void);
   void Show_Raw(void);
   void Show(void);
   void Treat(void);
   void inAttach(TTree* inT);
   void outAttach(TTree* outT);
   void CreateHistograms();
   void FillHistograms();
   void PrintCounters(void);

   Random* Rnd;

   //energy Raw
   UShort_t* E_Raw;
   UShort_t* E_Raw_Nr;
   Int_t* IcRaw;
   Int_t E_RawM;

   Int_t* DetChio;

   //Calibration coeff
   Float_t* a;
   Float_t* b;
   Float_t* Vnorm;

   //energy Calibrated
   Float_t* E;
   Int_t EMIC;

   Float_t eloss;

   //Counters
   Int_t Counter[5];

   ClassDef(IonisationChamberv, 0)
};

#endif // _IONCHAMBER_CLASS is not set

#ifdef _IONCHAMBER_CLASS
DEPRECATED_CLASS(IonisationChamberv);
#endif

