//Created by KVClassFactory on Thu Jun 17 14:19:50 2010
//Author: John Frankland,,,

#include "KVClassMonitor.h"
#include "TObjectTable.h"
#include "TROOT.h"
#include "TClass.h"

ClassImp(KVClassMonitor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVClassMonitor</h2>
<h4>track potential memory leaks</h4>
<p>
Simple tool to track which classes instance count increases over time.
Every time the Check() method is called we compare the instance count of every
class derived from TObject to the previous Check().
We print a warning message for every class whose instance count has increased.
</p>
<p>
YOU MUST ACTIVATE ROOT OBJECT TRACKING FOR THIS TO WORK!
In your .rootrc file, add/change this variable:<br>
Root.ObjectStat:   1
</p>
<p>
Note that, as the instance counts are stored in a TEnv, any time new classes are
instanciated, some class instances increase due to the TEnv itself changing.
Typical output in this case is:
</p>
<pre>
Info in <KVClassMonitor::Check>: Checking class instance statistics
Warning in <KVClassMonitor::Check>: TList increase from 4060 --> 4065
Warning in <KVClassMonitor::Check>: TEnvRec increase from 3032 --> 3038
Warning in <KVClassMonitor::Check>: TClass increase from 70 --> 71
Warning in <KVClassMonitor::Check>: TObjArray increase from 90 --> 92
</pre>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVClassMonitor::KVClassMonitor()
{
   // Default constructor
   // fill fClassStats with all current instance counts
   // Store current class instance statistics as 'initial' values
   // used for global comparison by CompareToInit()
   SetInitStatistics();
   TIter next(gROOT->GetListOfClasses());
   TClass* cl;
   while ((cl = (TClass*) next())) {
      Int_t n = cl->GetInstanceCount();
      fClassStats.SetValue(cl->GetName(), n);
   }
}

void KVClassMonitor::SetInitStatistics()
{
   // Store current class instance statistics as 'initial' values
   // used for global comparison by CompareToInit()

   gObjectTable->UpdateInstCount();
   TIter next(gROOT->GetListOfClasses());
   TClass* cl;
   while ((cl = (TClass*) next())) {
      Int_t n = cl->GetInstanceCount();
      fClassStatsInit.SetValue(cl->GetName(), n);
   }
}

KVClassMonitor::~KVClassMonitor()
{
   // Destructor
}

void KVClassMonitor::Check()
{
   // update class instance statistics
   // print warning for every class whose number has increased
   Info("Check", "Checking class instance statistics");
   gObjectTable->UpdateInstCount();
   TIter next(gROOT->GetListOfClasses());
   TClass* cl;
   while ((cl = (TClass*) next())) {
      Int_t n = cl->GetInstanceCount();
      Int_t old = fClassStats.GetValue(cl->GetName(), 0);
      if (n != old) {
         if (n > old) Warning("Check", "%s +%d     (%d --> %d)", cl->GetName(), n - old, old, n);
         else Warning("Check", "%s -%d     (%d --> %d)", cl->GetName(), old - n, old, n);
      }
      fClassStats.SetValue(cl->GetName(), n);
   }
}

void KVClassMonitor::CompareToInit()
{
   // compare class instance statistics to initial values
   // print warning for every class whose number has increased
   Info("CompareToInit", "Checking class instance statistics");
   gObjectTable->UpdateInstCount();
   TIter next(gROOT->GetListOfClasses());
   TClass* cl;
   while ((cl = (TClass*) next())) {
      Int_t n = cl->GetInstanceCount();
      Int_t old = fClassStatsInit.GetValue(cl->GetName(), 0);
      if (n != old) {
         if (n > old) Warning("CompareToInit", "%s +%d     (%d --> %d)", cl->GetName(), n - old, old, n);
         else Warning("CompareToInit", "%s -%d     (%d --> %d)", cl->GetName(), old - n, old, n);
      }
   }
}
