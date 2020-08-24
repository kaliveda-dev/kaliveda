//Created by KVClassFactory on Tue Jul 17 08:46:41 2012
//Author: John Frankland,,,

#ifndef __KVSIMDIR_H
#define __KVSIMDIR_H

#include "KVBase.h"
#include "KVList.h"

class KVSimFile;

/**
\class KVSimDir
\brief Handle directory containing simulated and/or filtered simulated data
\ingroup DM,Simulation

 This class will read the contents of a directory and automatically list the simulated and/or
 filtered simulated data it contains. To use:

      KVSimDir sim("MySimultations", "/path/to/my/simulations");
      sim.AnalyseDirectory();

 When method AnalyseDirectory() is called, each ROOT file in the directory given to the
 constructor will be opened and analysed in turn, by method AnalyseFile().
 If there is a TTree in the file, then we look at all of its branches until we find one
 containing objects which derive from KVEvent:

   -- if they inherit from KVSimEvent, we add the file to the list of simulated data:
            * a KVSimFile is created. The title of the TTree where data were found will
               be used as 'Information' on the nature of the simulation.
   -- if they inherit from KVReconstructedEvent, we add the file to the list of filtered data.
            * a KVSimFile is created. Informations on the filtered data are extracted from
              TNamed objects in the file with names 'Dataset', 'System', 'Run', 'Geometry'
              (type of geometry used, 'ROOT' or 'KV'), 'Origin' (i.e. the name of the simulation
              file which was filtered). These objects are automatically created when data is
              filtered using KVEventFiltering.

 Analysis of the file stops after the first TTree with a branch satisfying one of the
 two criteria is found (it is assumed that in each file there is only one TTree containing
 either simulated or filtered data).
*/

class KVSimDir : public KVBase {
protected:
   KVList fSimData;// list of simulated data files
   KVList fFiltData;// list of filtered simulated data files
   void init();

public:
   KVSimDir();
   KVSimDir(const Char_t* name, const Char_t* path);
   KVSimDir(const KVSimDir&) ;
   virtual ~KVSimDir();
   void Copy(TObject&) const;

   virtual void SetDirectory(const Char_t* d)
   {
      SetTitle(d);
   }
   virtual const Char_t* GetDirectory() const
   {
      return GetTitle();
   }

   virtual void AnalyseDirectory();
   virtual void AnalyseFile(const Char_t*);

   virtual const KVList* GetSimDataList() const
   {
      return &fSimData;
   }
   virtual const KVList* GetFiltDataList() const
   {
      return &fFiltData;
   }
   void AddSimData(KVSimFile*);
   void AddFiltData(KVSimFile*);

   void ls(Option_t* opt = "") const;

   ClassDef(KVSimDir, 1) //Handle directory containing simulated and/or filtered simulated data
};

#endif
