#ifndef __GRID_LOADER_H__
#define __GRID_LOADER_H__

/**
   WARNING: This class has been deprecated and will eventually be removed. Do
   not use!

   This class is only compiled if the deprecated code is enabled in the build
   configuration (e.g. cmake -DUSE_DEPRECATED_VAMOS=yes). If you enable the
   deprecated code then a large number of warnings will be printed to the
   terminal. To disable these warnings (not advised) compile VAMOS with
   -Wno-deprecated-declarations. Despite these warnings the code should compile
   just fine. The warnings are there to prevent the unwitting use of the
   deprecated code (which should be strongly discouraged).

   BY DEFAULT THIS CLASS IS NOT COMPILED.

   Deprecated by: Peter Wigg (peter.wigg.314159@gmail.com)
   Date:          Thu 17 Dec 17:24:38 GMT 2015
*/

// C
#include <cassert>

// KaliVeda (Standard)
#include "KVIDGraph.h"
#include "KVNumberList.h"
#include "KVIDGridManager.h"

// KaliVeda (VAMOS)
#include "Deprecation.h"

class GridLoader {
   KVList* global_grid_list_;
   KVList* run_grid_list_;

   Bool_t kInitialised_;

   // Prohibit Copy and Copy Assign
   GridLoader(const GridLoader&);
   GridLoader& operator=(const GridLoader&);

public:

   GridLoader();
   virtual ~GridLoader();

   Bool_t InitAnalysis();
   Bool_t InitRun(const UInt_t run_number);
   Bool_t IsInitialised()    const;
   void   PrintGlobalGrids() const;
   void   PrintRunGrids()    const;
   void   PrintAssertionStatus() const;

   const KVIDGraph* GetRunGrid(const TString& grid_name) const;

   ClassDef(GridLoader, 1);

};

#endif // __GRID_LOADER_H__ is not set

#ifdef __GRID_LOADER_H__
DEPRECATED_CLASS(GridLoader);
#endif

