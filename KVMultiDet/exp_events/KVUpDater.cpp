/*
$Id: KVUpDater.cpp,v 1.4 2006/10/19 14:32:43 franklan Exp $
$Revision: 1.4 $
$Date: 2006/10/19 14:32:43 $
$Author: franklan $
*/

#include "KVUpDater.h"
#include "TPluginManager.h"
#include "KVBase.h"
#include "Riostream.h"
#include "KVMultiDetArray.h"
#include "KVIDTelescope.h"
#include "KVIDGridManager.h"
#include "KVDetector.h"
#include "KVCalibrator.h"
#include "KVExpDB.h"

using namespace std;

ClassImp(KVUpDater)
KVUpDater::KVUpDater() : fArray(nullptr)
{
   //Default ctor for KVUpDater object.
}

//_______________________________________________________________//

KVUpDater::~KVUpDater()
{
   //Destructor.
}

void KVUpDater::SetArray(KVMultiDetArray* a)
{
   fArray = a;
}

KVUpDater* KVUpDater::MakeUpDater(const Char_t* uri, KVMultiDetArray* a)
{
   //Looks for plugin (see $KVROOT/KVFiles/.kvrootrc) with name 'uri'(=name of dataset),
   //loads plugin library, creates object and returns pointer.
   //If no plugin defined for dataset, instanciates a KVUpDater (default)

   //check and load plugin library
   TPluginHandler* ph;
   KVUpDater* upd = 0;
   if (!(ph = KVBase::LoadPlugin("KVUpDater", uri))) {
      upd = new KVUpDater;
   }
   else {
      upd = (KVUpDater*) ph->ExecPlugin(0);
   }
   upd->fDataSet = uri;
   upd->SetArray(a);
   return upd;
}

void KVUpDater::SetParameters(UInt_t run)
{
   // Set parameters of multidetector for this run
   //This will:
   //      set the target corresponding to the run
   //      set calibration parameters for the run
   //      set identification parameters for the run

   if (!gExpDB) return;
   cout << "Setting parameters of multidetector array for run " << run << ":" <<
        endl;
   KVDBRun* kvrun = gExpDB->GetDBRun(run);
   if (!kvrun) {
      Error("SetParameters(UInt_t)", "Run %u not found in database!", run);
      return;
   }
   SetTarget(kvrun);
   SetCalibrationParameters(run);
   SetIdentificationParameters(run);
}

//_______________________________________________________________//

void KVUpDater::SetTarget(KVDBRun* kvrun)
{
   //Set target used during this run
   //If no target is found in the database, the existing target (if there is one)
   //will be removed, and the multidetector array will have no target defined.

   cout << "--> Setting Target:" << endl;

   //remove existing target
   fArray->SetTarget(0);
   if (!kvrun->GetSystem() || !kvrun->GetSystem()->GetTarget()) {
      cout << "      No target defined for run." << endl;
      return;
   }

   fArray->SetTarget((KVTarget*) kvrun->GetSystem()->GetTarget());

   cout << "      " << fArray->GetTarget()->
        GetName() << " Total Thickness: " << fArray->GetTarget()->
        GetTotalThickness()
        << " mg/cm2" << endl;
}


void KVUpDater::SetIdentificationParameters(UInt_t run)
{
   // Set identification parameters for this run.
   // Will call gMultiDetArray->InitializeIDTelescopes() in order to initialize
   // identifications & set status IsReadyForID() of each ID telescope for run.

   cout << "Setting identification parameters of multidetector array for run " << run << ":" <<
        endl;
   SetIDGrids(run);
   fArray->InitializeIDTelescopes();
}

//________________________________________________________________________________________________

void KVUpDater::SetIDGrids(UInt_t run)
{
   // Use global ID grid manager gIDGridManager to set identification grids for all
   // ID telescopes for this run. First, any previously set grids are removed.
   // Then all grids for current run are set in the associated ID telescopes.

   cout << "--> Setting Identification Grids" << endl;
   TIter next_idt(fArray->GetListOfIDTelescopes());
   KVIDTelescope* idt;
   while ((idt = (KVIDTelescope*) next_idt())) {
      idt->RemoveGrids();
   }
   fArray->SetGridsInTelescopes(run);
}

//_______________________________________________________________//

void KVUpDater::SetCalibrationParameters(UInt_t run)
{
   //Set calibration parameters for this run.

   cout << "Setting calibration parameters of multidetector array for run " << run << ":" <<
        endl;
   KVDBRun* kvrun = gExpDB->GetDBRun(run);
   if (!kvrun) {
      Error("SetParameters(UInt_t)", "Run %u not found in database!", run);
      return;
   }
   SetCalibParameters(kvrun);
}

void KVUpDater::SetCalibParameters(KVDBRun* r)
{
   // Override in child classes to set specific calibrations.
   fArray->SetCalibratorParameters(r, fArray->GetName());
}
