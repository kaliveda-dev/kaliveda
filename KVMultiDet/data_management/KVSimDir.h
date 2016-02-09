//Created by KVClassFactory on Tue Jul 17 08:46:41 2012
//Author: John Frankland,,,

#ifndef __KVSIMDIR_H
#define __KVSIMDIR_H

#include "KVBase.h"
#include "KVList.h"

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

   void ls(Option_t* opt = "") const;

   ClassDef(KVSimDir, 1) //Handle directory containing simulated and/or filtered simulated data
};

#endif
