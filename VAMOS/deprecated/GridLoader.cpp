#include "GridLoader.h"

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

ClassImp(GridLoader)

GridLoader::GridLoader() :
   global_grid_list_(NULL),
   run_grid_list_(NULL),
   kInitialised_(kFALSE)
{

}

GridLoader::~GridLoader()
{
   if (run_grid_list_) {
      delete run_grid_list_;
      run_grid_list_ = NULL;
   }
}

Bool_t GridLoader::InitAnalysis()
{
   // Load the global-scope grid set and allocate memory for the run-scope set.

   if (kInitialised_) return kTRUE;

   assert(gIDGridManager);
   global_grid_list_ = gIDGridManager->GetGrids();

   // Run list, doesn't own its objects
   run_grid_list_ = new KVList(kFALSE);

   if (global_grid_list_ && run_grid_list_) kInitialised_ = kTRUE;
   return kInitialised_;
}

Bool_t GridLoader::InitRun(const UInt_t run_number)
{
   // Build the run-scope grid set by iterating through the global-scope set
   // and appending only those grids that are valid for the current run. This
   // fairly expensive operation is performed once at the beginning of a run in
   // order to reduce the final size of the list that will be searched in the
   // event loops.

   if (!kInitialised_) return kFALSE;

   assert(global_grid_list_);
   assert(run_grid_list_);

   run_grid_list_->Clear();

   KVNumberList run_list;

   KVIDGraph* grid(NULL);
   TIter next(global_grid_list_);
   while ((grid = static_cast<KVIDGraph*>(next()))) {
      run_list = grid->GetRuns();
      if (run_list.Contains(run_number)) {
         run_grid_list_->Add(grid);
      }
   }

   return kTRUE;
}

Bool_t GridLoader::IsInitialised() const
{
   return kInitialised_;
}

void GridLoader::PrintGlobalGrids() const
{
   if (!kInitialised_) return;

   KVIDGraph* grid(NULL);
   TIter next(global_grid_list_);
   while ((grid = static_cast<KVIDGraph*>(next()))) {
      std::cout << grid->GetName() << std::endl;
   }

   std::cout << "Number of grids (global scope): "
             << global_grid_list_->GetEntries() << std::endl;

}

void GridLoader::PrintRunGrids() const
{
   if (!kInitialised_) return;

   KVIDGraph* grid(NULL);
   TIter next(run_grid_list_);
   while ((grid = static_cast<KVIDGraph*>(next()))) {
      std::cout << grid->GetName() << std::endl;
   }

   std::cout << "Number of grids (run scope): " << run_grid_list_->GetEntries()
             << std::endl;

}

const KVIDGraph* GridLoader::GetRunGrid(const TString& grid_name) const
{
   // Return the grid which matches the given name ONLY if it is in the list of
   // valid grids.

   if (!kInitialised_) return NULL;

   assert(run_grid_list_);

   KVIDGraph* graph(
      static_cast<KVIDGraph*>(
         run_grid_list_->FindObject(grid_name.Data())
      )
   );

   return graph;
}

void GridLoader::PrintAssertionStatus() const
{
   std::cout << "<GridLoader::PrintAssertionStatus>: "
             << "Assertions are ";

#ifdef NDEBUG
   std::cout << "OFF" << std::endl;
#else
   std::cout << "ON" << std::endl;
#endif

}

