#ifndef __ANALYSISV_H__
#define __ANALYSISV_H__

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

#include "Deprecation.h"
#include "KVBase.h"
#include "KVCsIVamos.h"
#include "KVDetector.h"
#include "KVMaterial.h"
#include "KVSiliconVamos.h"
#include "KVTarget.h"
#include "KVTelescope.h"
#include "LogFile.h"
#include "Rtypes.h"
#include "TFile.h"
#include "TList.h"
#include "TPluginManager.h"
#include "TTree.h"

class Analysisv {

public:

   LogFile* L;

   TFile* inTree;
   TTree* inT;
   Int_t innEntries;
   Int_t totalEntries;
   TFile* outTree;
   TTree* outT;

   Analysisv();
   Analysisv(LogFile* Log);
   virtual ~Analysisv();

   virtual void OpenInputTree(TTree*);
   virtual void CloseInputTree(const char* inTreeName);
   virtual void OpenOutputTree(TTree*);
   virtual void CloseOutputTree(const char* outTreeName);
   static Analysisv* NewAnalyser(const Char_t* dataset, LogFile* Log);

   virtual void inAttach() = 0; //Attaching the variables
   virtual void outAttach() = 0; //Attaching the variables
   virtual void Treat() = 0; // Treating data
   virtual void CreateHistograms() = 0;
   virtual void FillHistograms() = 0;

   virtual void SetTel1(KVDetector* si) = 0;
   virtual void SetTel2(KVDetector* gap) = 0;
   virtual void SetTel3(KVDetector* csi) = 0;

   virtual void SetTarget(KVTarget* tgt) = 0;
   virtual void SetDC1(KVDetector* dcv1) = 0;
   virtual void SetSed(KVMaterial* sed) = 0;
   virtual void SetDC2(KVDetector* dcv2) = 0;
   virtual void SetIC(KVDetector* ic) = 0;
   virtual void SetGap1(KVMaterial* isogap1) = 0;
   virtual void SetSi(KVMaterial* si) = 0;
   virtual void SetGap2(KVMaterial* isogap2) = 0;
   virtual void SetCsI(KVMaterial* ccsi) = 0;

   virtual void SetFileCut(TList* list) = 0;
   virtual void SetFileCutChioSi(TList* list2) = 0;
   virtual void SetFileCutSiTof(TList* list3) = 0;

   virtual void SetAngleVamos(Float_t angle) = 0;
   virtual void SetBrhoRef(Float_t brho) = 0;
   virtual void SetCurrentRun(Int_t run) = 0;

   ClassDef(Analysisv, 0)

};

#endif // __ANALYSISV_H__ is not set

#ifdef __ANALYSISV_H__
DEPRECATED_CLASS(Analysisv);
#endif



