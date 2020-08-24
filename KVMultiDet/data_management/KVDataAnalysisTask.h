/*
$Id: KVDataAnalysisTask.h,v 1.7 2009/01/14 16:01:38 franklan Exp $
$Revision: 1.7 $
$Date: 2009/01/14 16:01:38 $
$Author: franklan $
*/

#ifndef __KVDATAANALYSISTASK_H
#define __KVDATAANALYSISTASK_H

#include "KVBase.h"
#include "KVList.h"
#include "KVString.h"

/**
\class KVDataAnalysisTask
\brief Define and manage data analysis tasks
\ingroup Analysis
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
*/

class KVDataAnalysisTask: public KVBase {

   KVString fPrereq;           //prerequisite data directory i.e. data on which analysis task is performed
   KVString fAnalyser;         //name of KVDataAnalyser class used to perform analysis
   KVString fBaseClass;        //base class for user analysis
   Bool_t   fUserClass;        //true if analysis task requires user-supplied class (derived from fBaseClass)

   Bool_t   fBaseIsPlugin;     //true if base class for user analysis is in a plugin library
   KVString fPluginURI;        //uri of the plugin library containing user base class
   KVString fPluginBase;       //known base class extended by plugin library
   KVString fExtraAClicIncludes;  //to be added to AClic include paths before compilation
   Long64_t    fStatusUpdateInterval;//interval (number of events) after which batch job progress and status are updated

public:

   KVDataAnalysisTask();
   KVDataAnalysisTask(const KVDataAnalysisTask&);
   ROOT_COPY_ASSIGN_OP(KVDataAnalysisTask)
   virtual ~ KVDataAnalysisTask();

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   virtual void SetPrereq(const Char_t* p)
   {
      fPrereq = p;
   };
   virtual const Char_t* GetPrereq() const
   {
      return fPrereq.Data();
   };

   virtual void ls(Option_t* opt = "") const;
   virtual void Print(Option_t* opt = "") const
   {
      ls(opt);
   };

   virtual void SetDataAnalyser(const Char_t* d)
   {
      fAnalyser = d;
   };
   virtual const Char_t* GetDataAnalyser() const
   {
      return fAnalyser;
   };
   virtual void SetUserBaseClass(const Char_t* d);
   virtual const Char_t* GetUserBaseClass() const
   {
      return fBaseClass;
   };
   virtual void SetWithUserClass(Bool_t w = kTRUE)
   {
      fUserClass = w;
   };
   virtual Bool_t WithUserClass() const
   {
      return fUserClass;
   };
   virtual void SetStatusUpdateInterval(Long64_t n)
   {
      fStatusUpdateInterval = n;
   }
   virtual Long64_t GetStatusUpdateInterval() const
   {
      return fStatusUpdateInterval;
   }

   virtual Bool_t CheckUserBaseClassIsLoaded();
   void SetExtraAClicIncludes(const KVString& list)
   {
      fExtraAClicIncludes = list;
   }

   ClassDef(KVDataAnalysisTask, 0)      //A data analysis task
};

#endif
