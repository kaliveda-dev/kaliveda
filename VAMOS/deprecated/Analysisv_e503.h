#ifndef __ANALYSISV_e503_H
#define __ANALYSISV_e503_H

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

#include <cstdlib>

#include "TList.h"
#include "TROOT.h"
#include "TTree.h"

#define DRIFT
#define IONCHAMBER
#define SI
#define CSI

#include "Analysisv.h"
#include "CsICalib.h"
#include "CsIv.h"
#include "Deprecation.h"
#include "DriftChamberv.h"
#include "Identificationv.h"
#include "IonisationChamberv.h"
#include "KVCsIVamos.h"
#include "KVDetector.h"
#include "KVMaterial.h"
#include "KVSiliconVamos.h"
#include "KVTelescope.h"
#include "Random.h"
#include "Reconstructionv.h"
#include "Sive503.h"

#include "KVMacros.h" // 'UNUSED' macro

class Analysisv_e503 : public Analysisv {
public:

   DriftChamberv* Dr;
   Reconstructionv* RC;
   Identificationv* Id;
   IonisationChamberv* Ic;
   Sive503* Si;
   CsIv* CsI;
   CsICalib* energytree;

   KVDetector* si;
   KVDetector* csi;
   KVDetector* gap;

   KVTarget* tgt;
   KVDetector* dcv1;
   KVMaterial* sed;
   KVDetector* dcv2;
   KVDetector* ic;
   KVMaterial* isogap1;
   KVMaterial* ssi;
   KVMaterial* isogap2;
   KVMaterial* ccsi;

   TTree* t;

   UShort_t T_Raw[10];

   Analysisv_e503(LogFile* Log);
   virtual ~Analysisv_e503(void);

   void inAttach(); //Attaching the variables
   void outAttach(); //Attaching the variables
   void Treat(); // Treating data
   void CreateHistograms();
   void FillHistograms();

   void SetTel1(KVDetector* si);
   void SetTel2(KVDetector* gap);
   void SetTel3(KVDetector* csi);

   /*TList *fcoup;
   TList *fcoup2;
   TList *fcoup3;
   */
   void SetFileCut(TList* list);
   void SetFileCutChioSi(TList* list2);
   void SetFileCutSiTof(TList* list3);

   void SetTarget(KVTarget* tgt);
   void SetDC1(KVDetector* dcv1);
   void SetSed(KVMaterial* sed);
   void SetDC2(KVDetector* dcv2);
   void SetIC(KVDetector* ic);
   void SetGap1(KVMaterial* isogap1);
   void SetSi(KVMaterial* ssi);
   void SetGap2(KVMaterial* isogap2);
   void SetCsI(KVMaterial* ccsi);

   virtual void SetAngleVamos(Float_t angle)
   {
      if (RC) RC->SetAngleVamos(angle);
   };
   virtual void SetBrhoRef(Float_t brho)
   {
      if (RC) RC->SetBrhoRef(brho);
      if (Id) Id->SetBrhoRef(brho);
   };

   virtual void SetCurrentRun(Int_t run)
   {
      if (Si) Si->SetOffsetRelativisteRun(run);
   }

   ClassDef(Analysisv_e503, 0) //VAMOS calibration for e503

};

#endif // __ANALYSISV_e503_H is not set

#ifdef __ANALYSISV_e503_H
DEPRECATED_CLASS(Analysisv_e503);
#endif

