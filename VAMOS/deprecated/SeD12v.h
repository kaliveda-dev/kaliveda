#ifndef _SED12_CLASS
#define _SED12_CLASS

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

#include "Defines.h"
#include "Deprecation.h"
#include "KVDataSet.h"
#include "LogFile.h"
#include "Random.h"
#include "Rtypes.h"
#include "SeDv.h"
#include <cmath>
#include <cstdlib>

class SeD12v {

   Bool_t Ready;

public:

   SeD12v(LogFile* Log, SeDv* SeD1, SeDv* SeD2);
   virtual ~SeD12v(void);

   LogFile* L;
   SeDv* S1;
   SeDv* S2;

   bool Present; //true if focal coordinates determined

   void Init(void); //Init for every event, focal variables go to -500.
   void SetMatX(void);
   void SetMatY(void);
   void Calibrate(); //
   void Focal(); //
   void FocalX(); // X
   void FocalY(); // Y
   void Show(void);
   void Treat(void);
   void inAttach(TTree* inT);
   void outAttach(TTree* outT);
   void CreateHistograms();
   void FillHistograms();
   void PrintCounters(void);

   Random* Rnd;

   // Focal position Reference & Position
   Double_t XRef[2];
   Double_t YRef[2];
   Double_t FocalPos; //Focal Plane position
   Double_t MatX[2][2];
   Double_t MatY[2][2];
   Double_t AngleFocal[2];
   Double_t TanFocal[2];

   //Time Wire
   UShort_t T_Raw[1];

   // Calibration coeff
   Float_t TCoef[1][2];

   //Calibrated Time Wire
   Float_t T[1];

   //Velocity cm/ns
   Float_t V;

   //Focal position
   Float_t X[2]; //Subsequent X
   Float_t Y[2]; //Subsequent Y
   Float_t Xf;
   Float_t Yf;
   Float_t Tf;
   Float_t Pf;

   //Counters
   Int_t Counter[14];

   //Histograms

   ClassDef(SeD12v, 0)

};

#endif

#ifdef _SED12_CLASS
DEPRECATED_CLASS(SeD12v);
#endif

