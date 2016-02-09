#include "Analysisv.h"

//Author: Maurycy Rejmund

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

using namespace std;
ClassImp(Analysisv)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Analysisv</h2>
<h4>Reconstruction of trajectories, identification & calibration of VAMOS data</h4>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Analysisv::Analysisv() :
   L(NULL),
   inTree(NULL),
   inT(NULL),
   totalEntries(0),
   outTree(NULL),
   outT(NULL)
{

}

Analysisv::Analysisv(LogFile* Log) :
   L(Log),
   inTree(NULL),
   inT(NULL),
   totalEntries(0),
   outTree(NULL),
   outT(NULL)
{

}

Analysisv::~Analysisv()
{
   if (L) {
      delete L;
      L = NULL;
   }
}

void Analysisv::OpenInputTree(TTree* readtree)
{

   cout << "Allocating Raw_Data from " << readtree->GetTitle();
   L->Log << "Allocating Raw_Data from " << readtree->GetTitle();

   inT = readtree;

   cout << " O.K." << endl;
   L->Log << " O.K." << endl;
   innEntries = (Int_t) inT->GetEntries();
   cout << "nEntries " << innEntries << endl ;
   L->Log << "nEntries " << innEntries << endl ;


#ifdef ACTIVEBRANCHES
   inT->SetBranchStatus("*", 0);
   cout << "Analysisv::OpenInputTree Disabling All Branches" << endl;
   L->Log << "Analysisv::OpenInputTree Disabling All Branches" << endl;
#endif

}

void Analysisv::CloseInputTree(const char* inTreeName)
{
#ifdef DEBUG
   cout << "Analysisv::CloseInputTree " << inTreeName << endl;
#endif

   if (inTree->IsOpen()) {
      cout << "Deleting inTree : " << endl;
      L->Log << "Deleting inTree : " << endl;
      delete inT;
      cout << "Closing file : " << inTreeName << endl;
      L->Log << "Closing file : " << inTreeName << endl;
      inTree-> Close();
      delete inTree;
   } else {
      cout << "File was not opened " << inTreeName << endl;
      exit(EXIT_FAILURE);
   }

}

void Analysisv::OpenOutputTree(TTree* treewrite)
{

   outT = treewrite;
   cout << "Connecting output Tree : " << outT->GetTitle() << endl;
   L->Log << "Connecting output Tree : " << outT->GetTitle() << endl;
   cout << " O.K." << endl;
   L->Log << " O.K." << endl;

}


void Analysisv::CloseOutputTree(const char* outTreeName)
{
#ifdef DEBUG
   cout << "Analysisv::CloseOutputTree " << outTreeName << endl;
#endif

   outTree->ls();
   cout << "Writing outTree " << outTreeName << endl;
   L->Log << "Writing outTree " << outTreeName << endl;
   outTree->Write();

   if (outTree->IsOpen()) {
      cout << "Deleting outTree : " << endl;
      L->Log << "Deleting outTree : " << endl;
      delete outT;
      cout << "Closing file : " << outTreeName << endl;
      L->Log << "Closing file : " << outTreeName << endl;
      outTree-> Close();
      delete outTree;
   } else {
      cout << "File was not opened " << outTreeName << endl;
      exit(EXIT_FAILURE);
   }

}


Analysisv* Analysisv::NewAnalyser(const Char_t* dataset, LogFile* Log)
{
   //Static function which will create and 'Build' the Analysisv object for the datatset.
   //These are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   //
   //Plugin.Analysisv:   INDRA_e503     Analysisv_e503   VAMOS   "Analysisv_e503(LogFile*)"
   //+Plugin.Analysisv:   INDRA_e494s     Analysisv_e494s   VAMOS   "Analysisv_e494s(LogFile*)"

   //does plugin exist for given dataset ?
   TPluginHandler* ph;
   if (!(ph = KVBase::LoadPlugin("Analysisv", dataset))) {
      cout << "Plugin not loaded " << endl;
      return 0;
   }
   //execute constructor/macro for object
   Analysisv* mda = (Analysisv*) ph->ExecPlugin(1, Log);
   return mda;
}

