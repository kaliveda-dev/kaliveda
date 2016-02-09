#ifndef _RECONSTRUCTIONSED_CLASS
#define _RECONSTRUCTIONSED_CLASS

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
#include "SeD12v.h"
#include "TVector3.h"

class ReconstructionSeDv {
   Bool_t Ready;
public:
   ReconstructionSeDv(LogFile* Log, SeD12v* SeD12);
   virtual ~ReconstructionSeDv(void);

   LogFile* L;
   SeD12v* S12;

   bool Present; //true if coordinates determined



   void Init(void); //Init for every event,  variables go to -500.
   void Calculate(); // Calulate  Initial coordinates
   void Show(void);
   void Treat(void);
   void outAttach(TTree* outT);
   void CreateHistograms();
   void FillHistograms();
   void PrintCounters(void);

   Random* Rnd;

   Float_t BrhoRef;
   Double_t Coef[4][330]; //D,T,P seventh order reconst in x,y,t,p

   Float_t Theta;
   Float_t Phi;
   Float_t Brho;
   Float_t Path;
   Float_t PathOffset;
   Float_t ThetaL;
   Float_t PhiL;


   //Counters
   Int_t Counter[6];

   ClassDef(ReconstructionSeDv, 0)

};

#endif // _RECONSTRUCTIONSED_CLASS is not set

#ifdef _RECONSTRUCTIONSED_CLASS
DEPRECATED_CLASS(ReconstructionSeDv);
#endif

