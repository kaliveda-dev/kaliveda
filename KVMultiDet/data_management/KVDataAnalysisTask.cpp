/*
$Id: KVDataAnalysisTask.cpp,v 1.10 2009/01/14 16:01:38 franklan Exp $
$Revision: 1.10 $
$Date: 2009/01/14 16:01:38 $
$Author: franklan $
*/

#include "KVDataAnalysisTask.h"
#include "TSystem.h"
#include "Riostream.h"
#include "TObjString.h"
#include "TClass.h"
#include "TPluginManager.h"

using namespace std;

ClassImp(KVDataAnalysisTask)

KVDataAnalysisTask::KVDataAnalysisTask()
{
   //Default ctor.
   fUserClass = kFALSE;
   fBaseIsPlugin = kFALSE;
   fStatusUpdateInterval = 1000;
}

KVDataAnalysisTask::KVDataAnalysisTask(const KVDataAnalysisTask& obj) : KVBase()
{
   //Copy ctor
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVDataAnalysisTask&) obj).Copy(*this);
#endif
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVDataAnalysisTask::Copy(TObject& obj) const
#else
void KVDataAnalysisTask::Copy(TObject& obj)
#endif
{
   //Copy this to obj
   //Redefinition of KVBase::Copy

   KVBase::Copy(obj);
   ((KVDataAnalysisTask&) obj).fPrereq = fPrereq;
   ((KVDataAnalysisTask&) obj).fAnalyser = fAnalyser;
   ((KVDataAnalysisTask&) obj).fBaseClass = fBaseClass;
   ((KVDataAnalysisTask&) obj).fUserClass = fUserClass;
   ((KVDataAnalysisTask&) obj).fBaseIsPlugin = fBaseIsPlugin;
   ((KVDataAnalysisTask&) obj).fPluginBase = fPluginBase;
   ((KVDataAnalysisTask&) obj).fPluginURI = fPluginURI;
   ((KVDataAnalysisTask&) obj).fExtraAClicIncludes = fExtraAClicIncludes;
   ((KVDataAnalysisTask&) obj).fStatusUpdateInterval = fStatusUpdateInterval;
}

KVDataAnalysisTask::~KVDataAnalysisTask()
{
}

void KVDataAnalysisTask::ls(Option_t*) const
{
   //Print analysis task information
   cout << "KVDataAnalysisTask : ";
   cout << GetName() << " : ";
   cout << GetTitle();
   cout << "  (pre-req = " << GetPrereq();
   cout << ",  analyser = " << GetDataAnalyser();
   if (fUserClass) cout << ", base class for user analysis = " << GetUserBaseClass();
   else if (fBaseClass != "") cout << ", default class for analysis = " << GetUserBaseClass();
   cout << ")" << endl;
}

void KVDataAnalysisTask::SetUserBaseClass(const Char_t* base_name)
{
   // Set the class from which user's analysis class must derive.
   // This is normally read in from the following environment variable:
   //
   // TaskName.DataAnalysisTask.UserClass.Base:      [base_name]
   //
   // If base_name is not the name of a known (i.e. currently contained in the ROOT
   // dictionary class table) class, it is assumed to be the uri of a plugin class.
   // By default, we assume that the plugin extends the KVDataAnalyser class, but
   // this can be changed using the following syntax:
   //
   // TaskName.DataAnalysisTask.UserClass.Base:      [uri]/[base class]
   //
   // Example 1:
   // The user analysis class for task "RawAnalysis" must derive from class
   // KVIVRawDataAnalyser which is defined in library libVAMOS.so (not loaded by default):
   //
   // RawAnalysis.DataAnalysisTask.UserClass.Base:    IVRawDataAnalyser
   // +Plugin.KVDataAnalyser:   IVRawDataAnalyser   KVIVRawDataAnalyser VAMOS "KVIVRawDataAnalyser()"
   //
   // Example 2:
   // The user analysis class for task "ReconIdent" must derive from class
   // KVIVReconIdent (itself derived from TSelector) which is defined in library libVAMOS.so (not loaded by default):
   //
   // ReconIdent.DataAnalysisTask.UserClass.Base:     IVReconIdent/TSelector
   // +Plugin.TSelector:    IVReconIdent    KVIVReconIdent     VAMOS    "KVIVReconIdent()"

   fBaseClass = base_name;
   if (fBaseClass != "" &&  !TClass::GetClass(base_name)) {
      // Base class is not in dictionary. Assume that base_name is a plugin.
      fBaseIsPlugin = kTRUE;
      KVString _base(base_name);
      TObjArray* toks = _base.Tokenize("/");
      if (toks->GetEntries() > 1) {
         // base_name = uri/base
         fPluginURI = ((TObjString*)toks->At(0))->String();
         fPluginBase = ((TObjString*)toks->At(1))->String();
      }
      else {
         // base_name = uri. default plugin base class is KVDataAnalyser
         fPluginURI = base_name;
         fPluginBase = "KVDataAnalyser";
      }
      delete toks;
      // check plugin exists
      TPluginHandler* ph = gPluginMgr->FindHandler(fPluginBase.Data(), fPluginURI.Data());
      if (!ph) {
         Error("SetUserBaseClass", "Analysis task=%s :\nNo plugin found with URI=%s for base class %s",
               GetName(), fPluginURI.Data(), fPluginBase.Data());
         return;
      }
      // get name of plugin base class that user's analysis must derive from
      fBaseClass = ph->GetClass();
      if (gDebug > 1) Info("SetUserBaseClass", "Analysis task=%s :\nUser analysis class derives from %s, extension of %s in plugin %s",
                              GetName(), fBaseClass.Data(), fPluginBase.Data(), fPluginURI.Data());
   }
}

Bool_t KVDataAnalysisTask::CheckUserBaseClassIsLoaded()
{
   // This method checks that the base class for the user's analysis class is loaded.
   // If this base class requires a plugin library to be loaded, it will be loaded.
   // First we add any required extra ACliC include paths (taken from DataAnalysisTask.UserClass.ExtraACliCIncludes)
   // If all is OK, returns kTRUE.
   // Returns kFALSE if plugin cannot be loaded or class is simply unknown

   if (fExtraAClicIncludes != "") {
      gSystem->AddIncludePath(fExtraAClicIncludes.Data());
      Info("CheckUserBaseClassIsLoaded", "Added ACliC include path: %s", fExtraAClicIncludes.Data());
   }
   TClass* cl = TClass::GetClass(fBaseClass.Data()); // class in dictionary already ?
   if (cl) return kTRUE;
   if (fBaseIsPlugin) {
      // base class is in a plugin library. load it now.
      TPluginHandler* ph = LoadPlugin(fPluginBase.Data(), fPluginURI.Data());
      if (ph) return kTRUE;
      Error("CheckUserBaseClassIsLoaded", "Analysis task=%s : Failed to load plugin class=%s base=%s uri=%s",
            GetName(), fBaseClass.Data(), fPluginBase.Data(), fPluginURI.Data());
   }
   else {
      Error("CheckUserBaseClassIsLoaded", "Analysis task=%s : User analysis base class %s is unknown",
            GetName(), fBaseClass.Data());
   }
   return kFALSE;
}
