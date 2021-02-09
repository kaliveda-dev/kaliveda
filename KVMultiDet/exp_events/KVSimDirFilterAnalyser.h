//Created by KVClassFactory on Thu Mar 23 14:02:40 2017
//Author: John Frankland,,,

#ifndef __KVSIMDIRFILTERANALYSER_H
#define __KVSIMDIRFILTERANALYSER_H

#include "KVSimDirAnalyser.h"
#include <KV2Body.h>
#include <KVDBRun.h>
#include <KVDBSystem.h>
#include <KVEventSelector.h>

/**
\class KVSimDirFilterAnalyser
\brief Manage user analysis of filtered simulation data
\ingroup Analysis
*/

class KVSimDirFilterAnalyser : public KVSimDirAnalyser {
protected:
   KVEventSelector* fAnalysisClass;   // user analysis class
   KV2Body*         fKinematics;      // kinematics of reaction
   KVDBSystem*      fSystem;          // currently analysed system
   KVDBRun*         fRun;             // currently analysed run

public:
   KVSimDirFilterAnalyser();
   virtual ~KVSimDirFilterAnalyser();

   void RegisterUserClass(TObject* c)
   {
      fAnalysisClass = dynamic_cast<KVEventSelector*>(c);
   }

   const KV2Body* GetKinematics() const
   {
      return fKinematics;
   }

   void preInitAnalysis();
   void preInitRun();
   void preAnalysis();

   static void Make(const Char_t* kvsname = "MyFilteredAnalysis");

   KVDBSystem* GetAnalysedSystem() const
   {
      return fSystem;
   }
   KVDBRun* GetAnalysedRun() const
   {
      return fRun;
   }
#ifdef USING_ROOT6
   void SetTriggerConditionsForRun(int run);
#endif
 ClassDef(KVSimDirFilterAnalyser, 1) //Analysis of filtered simulations
};

#endif
