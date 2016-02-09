#ifndef __ANALYSISV_E494S_H
#define __ANALYSISV_E494S_H

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

#define SED1
#define SED2
#define IONCHAMBER
#define SI

#ifdef SED1
#ifdef SED2
#define SED12
#endif
#endif

#include "Analysisv.h"
#include "Deprecation.h"
#include "IonisationChamberv.h"
#include "Random.h"
#include "ReconstructionSeDv.h"
#include "SeD12v.h"
#include "SeDv.h"
#include "Siv.h"
#include <cstdlib>

class Analysisv_e494s : public Analysisv {
public:

   SeDv* SeD1;
   SeDv* SeD2;
   SeD12v* SeD12;
   ReconstructionSeDv* RC;
   IonisationChamberv* Ic;
   Siv* Si;

   UShort_t T_Raw[10];

   Analysisv_e494s(LogFile* Log);
   virtual ~Analysisv_e494s(void);

   void inAttach(); //Attaching the variables
   void outAttach(); //Attaching the variables
   void Treat(); // Treating data
   void CreateHistograms();
   void FillHistograms();

   ClassDef(Analysisv_e494s, 0); //VAMOS calibration for E494S

};

#endif // __ANALYSISV_E494S_H is not set

#ifdef __ANALYSISV_E494S_H
DEPRECATED_CLASS(Analysisv_e494s);
#endif

