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

class KVDataSet;

class KVDataAnalysisTask: public KVBase {


   friend class KVDataSet;

   KVString fPrereq;           //prerequisite data directory i.e. data on which analysis task is performed
   KVString fAnalyser;         //name of KVDataAnalyser class used to perform analysis
   KVString fBaseClass;        //base class for user analysis
   Bool_t   fUserClass;        //true if analysis task requires user-supplied class (derived from fBaseClass)

   Bool_t   fBaseIsPlugin;     //true if base class for user analysis is in a plugin library
   KVString fPluginURI;        //uri of the plugin library containing user base class
   KVString fPluginBase;       //known base class extended by plugin library
   KVString fExtraAClicIncludes;  //to be added to AClic include paths before compilation
   Int_t    fStatusUpdateInterval;//interval (number of events) after which batch job progress and status are updated

   virtual void SetParametersForDataSet(KVDataSet*);

public:

   KVDataAnalysisTask();
   KVDataAnalysisTask(const KVDataAnalysisTask&);
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
   virtual void SetStatusUpdateInterval(Int_t n)
   {
      fStatusUpdateInterval = n;
   }
   virtual Int_t GetStatusUpdateInterval() const
   {
      return fStatusUpdateInterval;
   }

   virtual Bool_t CheckUserBaseClassIsLoaded();

   ClassDef(KVDataAnalysisTask, 0)      //A data analysis task
};

#endif
