//Created by KVClassFactory on Thu Jun 17 14:19:50 2010
//Author: John Frankland,,,

#include "KVClassMonitor.h"
#include "TObjectTable.h"
#include "TROOT.h"
#include "TClass.h"

ClassImp(KVClassMonitor)


KVClassMonitor* KVClassMonitor::fgClassMonitor = nullptr;
Bool_t KVClassMonitor::fDisableCheck = kFALSE;

KVClassMonitor::KVClassMonitor()
{
   // Default constructor
   // fill fClassStats with all current instance counts
   // Store current class instance statistics as 'initial' values
   // used for global comparison by CompareToInit()

   fgClassMonitor = this;
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

std::map<std::string, int>& KVClassMonitor::GetListOfChanges() const
{
   // Return list of classes whose count changed in last call to Check()
   // Each parameter (classname) has a value corresponding to the change (+ or -)

   return fChanges;
}

KVClassMonitor* KVClassMonitor::GetInstance()
{
   // Return pointer to unique instance of class monitor class
   if (!fgClassMonitor) new KVClassMonitor;
   return fgClassMonitor;
}

KVClassMonitor::~KVClassMonitor()
{
   // Destructor
   fgClassMonitor = nullptr;
}

void KVClassMonitor::Check()
{
   // update class instance statistics
   // print warning for every class whose number has increased
   Info("Check", "Checking class instance statistics");
   fChanges.clear();
   gObjectTable->UpdateInstCount();
   TIter next(gROOT->GetListOfClasses());
   TClass* cl;
   while ((cl = (TClass*) next())) {
      Int_t n = cl->GetInstanceCount();
      Int_t old = fClassStats.GetValue(cl->GetName(), 0);
      if (n != old) {
         if (n > old) Warning("Check", "%s +%d     (%d --> %d)", cl->GetName(), n - old, old, n);
         else Warning("Check", "%s -%d     (%d --> %d)", cl->GetName(), old - n, old, n);
         fChanges[cl->GetName()] = (n - old);
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
