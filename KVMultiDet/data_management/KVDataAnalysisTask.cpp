/*
$Id: KVDataAnalysisTask.cpp,v 1.10 2009/01/14 16:01:38 franklan Exp $
$Revision: 1.10 $
$Date: 2009/01/14 16:01:38 $
$Author: franklan $
*/

#include "KVDataAnalysisTask.h"
#include "KVDataSet.h"
#include "TSystem.h"
#include "Riostream.h"
#include "TObjString.h"
#include "TClass.h"
#include "TPluginManager.h"

using namespace std;

ClassImp(KVDataAnalysisTask)
/////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataAnalysisTask</h2>
<h4>Defines a data analysis task</h4>
<p>
Data analysis tasks are defined in the files $KVROOT/KVFiles/.kvrootrc and the user's
$HOME/.kvrootrc, as in the following examples:
</p>
<pre>
DataAnalysisTask:     Reconstruction
Reconstruction.DataAnalysisTask.Title:     Event reconstruction from raw data (raw->recon)
Reconstruction.DataAnalysisTask.Prereq:     raw
Reconstruction.DataAnalysisTask.Analyser:     INDRARawDataRecon
</pre>
<p>
Each task has a name ("Reconstruction"), a descriptive title ("Event reconstruction from raw data (raw->recon)"),
a prerequisite type of data on which the analysis is performed ("raw"), and the name of a
<a href="KVDataAnalyser.html">KVDataAnalyser</a> plugin which is used to perform the analysis ("INDRARawDataRecon").
For a given <a href="KVDataSet.html">dataset</a>, only analysis tasks whose prerequisite data type is available can be
performed.
</p>
<pre>
+DataAnalysisTask:     ReconAnalysis
ReconAnalysis.DataAnalysisTask.Title:     Analysis of reconstructed events (recon)
ReconAnalysis.DataAnalysisTask.Prereq:     recon
ReconAnalysis.DataAnalysisTask.Analyser:     INDRAReconData
ReconAnalysis.DataAnalysisTask.UserClass:     yes
ReconAnalysis.DataAnalysisTask.UserClass.Base:     INDRASelector/TSelector
+Plugin.TSelector:  INDRASelector    KVSelector      KVIndra    "KVSelector()"
</pre>
<p>
This analysis task requires the user to supply an analysis class ("...UserClass: yes"); the class must
be derived from a plugin base class, <a href="KVSelector.html">KVSelector</a>, which is not defined in
the basic KaliVeda library (see <a href="#KVDataAnalysisTask:SetUserBaseClass">SetUserBaseClass()</a> for details).
It will be automatically loaded when required.
</p>
<pre>
+DataAnalysisTask:     RawAnalysis
RawAnalysis.DataAnalysisTask.Title:     Analysis of raw data
RawAnalysis.DataAnalysisTask.Prereq:     raw
RawAnalysis.DataAnalysisTask.Analyser:     UserClass
RawAnalysis.DataAnalysisTask.UserClass:     yes
RawAnalysis.DataAnalysisTask.UserClass.Base:     INDRARawDataAnalyser
+Plugin.KVDataAnalyser:   INDRARawDataAnalyser   KVINDRARawDataAnalyser       KVIndra "KVINDRARawDataAnalyser()"
</pre>
<p>
This analysis task also requires the user to supply an analysis class ("...UserClass: yes"), but as the base class
for the user's class is itself derived from KVDataAnalyser (via a plugin: see <a href="#KVDataAnalysisTask:SetUserBaseClass">SetUserBaseClass()</a> for details)
it is the user's class which will be used to perform the analysis ("...Analyser: UserClass").
</p>
<pre>
+DataAnalysisTask:     ReconIdent
ReconIdent.DataAnalysisTask.Title:     Identification of reconstructed events (recon->ident)
ReconIdent.DataAnalysisTask.Prereq:     recon
ReconIdent.DataAnalysisTask.Analyser:     INDRAReconData
ReconIdent.DataAnalysisTask.UserClass:     no
ReconIdent.DataAnalysisTask.UserClass.Base:     INDRAReconIdent/TSelector
+Plugin.TSelector:   INDRAReconIdent        KVReconIdent                 KVIndra   "KVReconIdent()"
</pre>
<p>
This analysis task does not require a user-supplied analysis class ("...UserClass: no") as it supplies a default
analysis class to be used by the analyser (via a plugin: <a href="KVReconIdent.html">KVReconIdent</a>.
See <a href="#KVDataAnalysisTask:SetUserBaseClass">SetUserBaseClass()</a> for details).
</p>
<h4>Dataset-dependent analysis tasks</h4>
<p>
Certain characteristics (analyser plugin, base class for user analysis class)
of analysis tasks can be "tuned" for different datasets. For example, to change the
KVDataAnalyser plugin to use for the "Reconstruction" task for the dataset "INDRA_e494s":
</p>
<pre>
INDRA_e494s.Reconstruction.DataAnalysisTask.Analyser:     IVRawDataRecon_e494s
</pre>
<p>
where the plugin is defined as:
</p>
<pre>
+Plugin.KVDataAnalyser:   IVRawDataRecon_e494s   KVIVRawDataReconstructor VAMOS "KVIVRawDataReconstructor()"
</pre>
<!-- */
// --> END_HTML
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
      } else {
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

void KVDataAnalysisTask::SetParametersForDataSet(KVDataSet* dataset)
{
   //PRIVATE METHOD
   //Called by KVDataSet::SetAnalysisTasks
   //Check environment variables (i.e. .kvrootrc) to see if the task parameters
   //have been "tweaked" for the dataset.

   KVString envar = dataset->GetDataSetEnv(Form("%s.DataAnalysisTask.Analyser", GetName()));
   if (envar != "") fAnalyser = envar;
   envar = dataset->GetDataSetEnv(Form("%s.DataAnalysisTask.UserClass.Base", GetName()));
   if (envar != "") SetUserBaseClass(envar);
   envar = dataset->GetDataSetEnv(Form("%s.DataAnalysisTask.Prereq", GetName()));
   if (envar != "") SetPrereq(envar);
   envar = dataset->GetDataSetEnv(Form("%s.DataAnalysisTask.UserClass.ExtraACliCIncludes", GetName()));
   if (envar != "") fExtraAClicIncludes = envar;
   Int_t nev = (Int_t)dataset->GetDataSetEnv(Form("%s.DataAnalysisTask.StatusUpdateInterval", GetName()), 0.0);
   if (nev > 0) fStatusUpdateInterval = nev;
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
   } else {
      Error("CheckUserBaseClassIsLoaded", "Analysis task=%s : User analysis base class %s is unknown",
            GetName(), fBaseClass.Data());
   }
   return kFALSE;
}
