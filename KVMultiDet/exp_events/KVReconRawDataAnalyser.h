#ifndef KVRECONRAWDATAANALYSER_H
#define KVRECONRAWDATAANALYSER_H

#include "KVRawDataAnalyser.h"
#include "KVEventReconstructor.h"

/**
  \class KVReconRawDataAnalyser
  \ingroup Analysis
  \brief Base class for user analysis of raw data with event reconstruction

  In this analysis, the event reconstruction is performed just before the user's analysis
  method is called . This can give access to informations which are not stored in the final
  reconstructed data files when data is reconstructed. Note that in the present case the
  reconstructed events are not stored to disk, unless the user chooses to do so.

  As event reconstruction is a CPU-time-consuming process, this analysis task should only
  be used if there are indeed informations which can only be accessed in this way; for analysis
  of reconstructed events it is far more efficient to generate the reconstructed event files
  with the appropriate analysis task and then analyse the reconstructed events subsequently.
 */
class KVReconRawDataAnalyser : public KVRawDataAnalyser {
protected:
   unique_ptr<KVEventReconstructor> fEvRecon;
   KVReconstructedEvent* fRecev;

public:
   KVReconRawDataAnalyser() {}
   virtual ~KVReconRawDataAnalyser() {}

   void preInitAnalysis();
   void preInitRun();
   void preAnalysis();
   void postAnalysis();
   void postEndRun();

   KVReconstructedEvent* GetReconstructedEvent() const
   {
      return fEvRecon->GetEvent();
   }

   static void Make(const Char_t* kvsname);

   ClassDef(KVReconRawDataAnalyser, 0) // Analysis of raw data with reconstruction of events
};

#endif // KVRECONRAWDATAANALYSER_H
