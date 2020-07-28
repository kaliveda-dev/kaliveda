//Created by KVClassFactory on Thu Jun 17 14:19:50 2010
//Author: John Frankland,,,

#ifndef __KVCLASSMONITOR_H
#define __KVCLASSMONITOR_H

#include "KVBase.h"
#include <map>
#include <string>
#include "TEnv.h"

/**
  \class KVClassMonitor
\brief Simple tool to track which classes instance count increases over time.
\ingroup Core

Every time the Check() method is called we compare the instance count of every
class derived from TObject to the previous Check().
We print a warning message for every class whose instance count has increased.

__YOU MUST ACTIVATE ROOT OBJECT TRACKING FOR THIS TO WORK!__

In your .rootrc file, add/change this variable:
~~~~~~
    Root.ObjectStat:   1
~~~~~~
Note that, as the instance counts are stored in a TEnv, any time new classes are
instanciated, some class instances increase due to the TEnv itself changing.

Typical output in this case is:
~~~~~~
    Info in <KVClassMonitor::Check>: Checking class instance statistics
    Warning in <KVClassMonitor::Check>: TList increase from 4060 --> 4065
    Warning in <KVClassMonitor::Check>: TEnvRec increase from 3032 --> 3038
    Warning in <KVClassMonitor::Check>: TClass increase from 70 --> 71
    Warning in <KVClassMonitor::Check>: TObjArray increase from 90 --> 92
~~~~~~
#### Usage
This is a singleton class: only one instance can exist at a time. A pointer to
the current instance can be retrieved with static method KVClassMonitor::GetInstance().

Typical use:
~~~~~{.cpp}
    KVClassMonitor* cmon = KVClassMonitor::GetInstance();

    while( [some treatment loop] ){

       [lots of memory-leaking activity]

       cmon->Check();-//- print infos on memory usage
    }
~~~~~
*/

class KVClassMonitor : public KVBase {

   static KVClassMonitor* fgClassMonitor;//! singleton
   TEnv fClassStats;//! store class instance statistics
   TEnv fClassStatsInit;//! intitial class instance statistics
   static Bool_t fDisableCheck;//! disable static DoCheck() method
   mutable std::map<std::string, int> fChanges; //! list of classes whose number changed in last Check()

   KVClassMonitor();
public:
   virtual ~KVClassMonitor();

   void Check();
   void CompareToInit();
   void SetInitStatistics();
   std::map<std::string, int>& GetListOfChanges() const;

   static KVClassMonitor* GetInstance();

   // Print statistics of global singleton if it exists
   static void DoCheck(const TString& method, const TString& message)
   {
      if (!fDisableCheck && fgClassMonitor) {
         printf("<%s> : %s\n", method.Data(), message.Data());
         fgClassMonitor->Check();
      }
   }

   // Disable static DoCheck method
   static void DisableChecking()
   {
      fDisableCheck = kTRUE;
   }
   // Re-enable static DoCheck method
   static void EnableChecking()
   {
      fDisableCheck = kFALSE;
   }

   ClassDef(KVClassMonitor, 0) //Track potential memory leaks
};

#endif
