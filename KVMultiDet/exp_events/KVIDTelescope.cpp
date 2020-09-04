/***************************************************************************
$Id: KVIDTelescope.cpp,v 1.52 2009/05/05 15:54:04 franklan Exp $
Author : $Author: franklan $
                          KVIDTelescope.cpp  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by J.D Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "TROOT.h"
#include "KVIDTelescope.h"
#include "KVTelescope.h"
#include "KVGroup.h"
#include "KVNucleus.h"
#include "KVReconstructedNucleus.h"
#include "KVIDGraph.h"
#include "KVIDGrid.h"
#include "Riostream.h"
#include "TPluginManager.h"
#include "KVMultiDetArray.h"
#include "KVDataSet.h"
#include "KVIDGridManager.h"
#include "KVIDZALine.h"
#include "KVIDCutLine.h"
#include "KVIdentificationResult.h"
#include "TMath.h"
#include "TClass.h"
#include "TH2.h"
#include "KVParticleCondition.h"

#include <KVDetectorSignalExpression.h>
#include <KVIDZAGrid.h>

using namespace std;

ClassImp(KVIDTelescope)
/////////////////////////////////////////////////////////////////////////////////////////
//KVIDTelescope
//
//Charged particle identification is handled by KVIDTelescope and derived classes.
//A KVIDTelescope is an association of one or more detectors which is capable of particle
//identification.
//Although initially conceived in terms of DE-E two-stage telescopes, the identification
//method can be quite different (see KVIDCsI for example).
//
//Visualising Identification (ID) Grids & Maps
//--------------------------------------------
//Identification in such 'telescopes' is often associated with a 'grid' or set of
//graphical cuts permitting to associate the informations measured in the 'telescope'
//with certain types of particles. For any class derived from KVIDTelescope, the
//identification grid (KVIDGraph), if one exists, can be obtained and visualised using :
//
//      idtelescope->GetIDGrid()->Draw();
//
//Note that if no grid has been defined for the telescope, GetIDGrid() returns a null
//pointer, so you should only use the previous method if you are absolutely sure that
//the ID telescope in question has a grid.
//
//Visualising identifications based on fitted functionals
//--------------------------------------------------------
//The identification functionals devised by L. Tassan-Got and defined in KVTGIDFunctions
//can be visualised as identification grids for classes having KVTGIDManager as a base
//class. In this case a grid can be created and drawn using :
//
//      KVIDGrid* g = idtelescope->GetTGIDGrid("name_of_functional", max_X_coord_of_lines);
//      g->Draw();
//
//You should keep a pointer to the grid because it is the user's responsibility to delete it after
//use.
//
//Particle Identification and Calibration/Coherency
//-------------------------------------------------
//Particles reconstructed from data measured in the detectors of a multidetector array
//(KVReconstructedNucleus) are identified using the KVReconstructedNucleus::Identify()
//method. This takes the list of ID telescopes through whose detectors the particle passed,
//beginning with the telescope in which the particle stopped, and calls the Identify()
//method of each ID telescope in turn. This continues until identification is achieved.
//Thus the identification method, specific to each type of ID telescope, can be defined
//by overriding the Identify() method in child classes.
//
//Similarly, calibration of identified particles is achieved by KVReconstructedNucleus::
//Calibrate() which uses the Calibrate() method of the 'identifying telescope' (i.e. the
//KVIDTelescope in which identification was achieved). Again, this may be redefined in
//child classes (see KVIDCsI for example), but the default procedure defined here is to
//calculate the energy losses of the particle in all the detectors preceding the ID
//telescope and compare them to the measured energy losses in these detectors, in order
//to check for double-hits. The results of this procedure may then be used by some
//coherency algorithm.
//
//Setting identification parameters
//---------------------------------------------
//The method SetIdentificationParameters(const KVMultiDetArray*) will be used by the
//KVMultiDetArray to which this telescope belongs in order to set the parameters of
//ALL telescopes of this type in the array.
//By default, this method looks for the file with name given by the environment variable
//[dataset name].IdentificationParameterFile.[telescope label]:       [filename]
//which is assumed to contain identification grids. The file will be read in by gIDGridManager
//and the grids added to its list.
/////////////////////////////////////////////////////////////////////////////////////////

TEnv* KVIDTelescope::fgIdentificationBilan = nullptr;

KVIDTelescope::KVIDTelescope()
   : fDetectors(kFALSE), fGroup(nullptr), fIDGrids(kFALSE)
{
   init();
}

void KVIDTelescope::init()
{
   //default init
   fDetectors.SetCleanup(kTRUE);
   fIDGrids.SetCleanup(kTRUE);
   fMassIDValidity = nullptr;
}

void KVIDTelescope::Initialize(void)
{
   // Default initialisation for ID telescopes.
   // If telescope has at least 1 grid then it is ready to identify
   // particles after initialising the grid(s) (kReadyForID=true);
   // otherwise kReadyForID is set to kFALSE, unless the current dataset (if defined)
   // has been declared to have no associated identification/calibration parameters,
   // in which case kReadyForID is by default set to kTRUE (for filtering simulations).
   //
   // In order to enable mass identification for certain telescopes without a dedicated
   // implementation (e.g. for simulating array response), put the following lines
   // in your .kvrootrc:
   //
   //   [dataset].[telescope label].MassID:   yes
   //
   // If you want to limit mass identification to certain values of Z and/or A,
   // add the following line:
   //
   //   [dataset].[telescope label].MassID.Validity:    [expression]
   //
   // where [expression] is some valid C++ boolean expression involving Z and/or A,
   // for example
   //
   //   [dataset].[telescope label].MassID.Validity:  (Z>3)&&(A<20)
   //
   //For identifications using more than one grid, the default behaviour is to try identification
   //with each grid in turn until a successful identification is obtained. The order in which
   //the grids should be tried should be specified by a variable with the following format:
   //
   //~~~~~~~~~~~~~~~~
   //[Dataset].[telescope label].GridOrder:  [Grid1],[Grid2],...
   //~~~~~~~~~~~~~~~~

   ResetBit(kReadyForID);
   if (GetIDGrid()) {
      KVIDGraph* gr;
      TIter it(GetListOfIDGrids());
      bool ok = kTRUE;
      KVUniqueNameList tmp_list;// for re-ordering grids
      bool mass_id = false;
      while ((gr = (KVIDGraph*)it())) {
         tmp_list.Add(gr);
         if (gr->HasMassIDCapability()) mass_id = true;
         gr->Initialize();
         // make sure both x & y axes' signals are well set up
         if (!fGraphCoords[gr].fVarX || !fGraphCoords[gr].fVarY) {
            ok = kFALSE;
            Warning("Initialize",
                    "ID tel. %s: grid %s has undefined VarX(%s:%p) or VarY(%s:%p) - WILL NOT USE",
                    GetName(), gr->GetName(), gr->GetVarX(), fGraphCoords[gr].fVarX, gr->GetVarY(), fGraphCoords[gr].fVarY);
         }
      }
      // set to true if at least one grid can provide mass identification
      SetHasMassID(mass_id);
      // if more than one grid, need to re-order them according to [Dataset].[telescope label].GridOrder
      if (GetListOfIDGrids()->GetEntries() > 1 && gDataSet) {
         KVString grid_list = gDataSet->GetDataSetEnv(Form("%s.GridOrder", GetLabel()));
         ok = kFALSE;
         if (grid_list == "")
            Warning("Initialize", "ID telescope %s has %d grids but no %s variable defined",
                    GetName(), GetListOfIDGrids()->GetEntries(), Form("%s.GridOrder", GetLabel()));
         else if (grid_list.GetNValues(",") != GetListOfIDGrids()->GetEntries())
            Warning("Initialize", "ID telescope %s has %d grids but %d grids appear in variable %s",
                    GetName(), GetListOfIDGrids()->GetEntries(), grid_list.GetNValues(","), Form("%s.GridOrder", GetLabel()));
         else {
            fIDGrids.Clear();
            grid_list.Begin(",");
            while (!grid_list.End()) fIDGrids.Add(tmp_list.FindObject(grid_list.Next()));
            ok = kTRUE;
         }
      }
      if (ok) SetBit(kReadyForID);
   }
   else if (gDataSet && !gDataSet->HasCalibIdentInfos()) SetBit(kReadyForID);
   else ResetBit(kReadyForID);

   if (gDataSet) {
      SetHasMassID(gDataSet->GetDataSetEnv(Form("%s.MassID", GetLabel()), kFALSE));
      TString valid;
      if ((valid = gDataSet->GetDataSetEnv(Form("%s.MassID.Validity", GetLabel()), "")) != "") {
         valid.ReplaceAll("Z", "_NUC_->GetZ()");
         valid.ReplaceAll("A", "_NUC_->GetA()");
         SafeDelete(fMassIDValidity);
         fMassIDValidity = new KVParticleCondition(valid);
      }
   }
}

KVIDTelescope::~KVIDTelescope()
{
   //delete this ID telescope
   SafeDelete(fMassIDValidity);
}

//___________________________________________________________________________________________
void KVIDTelescope::AddDetector(KVDetector* d)
{
   //Add a detector to the telescope.
   //The first detector added is the "DeltaE" member, the second the "Eresidual" member.
   //Update name of telescope to "ID_[name of DE]_[name of E]"

   if (d) {
      fDetectors.Add(d);
      d->AddIDTelescope(this);
      if (GetSize() > 1)
         SetName(Form("ID_%s_%s", GetDetector(1)->GetName(), GetDetector(2)->GetName()));
      else SetName(Form("ID_%s", GetDetector(1)->GetName()));
   }
   else {
      Warning("AddDetector", "Called with null pointer");
   }
}

//___________________________________________________________________________________________
void KVIDTelescope::Print(Option_t* opt) const
{
   // print out telescope structure
   //if opt="fired" only fired detectors are printed

   TIter next(GetDetectors());
   KVDetector* obj;

   if (!strcmp(opt, "fired")) {
      while ((obj = (KVDetector*) next())) {

         if (obj->Fired() || obj->GetEnergy())
            obj->Print("data");
      }
   }
   else {
      cout << "\n" << opt << "Structure of KVIDTelescope object: " <<
           GetName() << " " << GetType() << endl;
      cout << opt <<
           "--------------------------------------------------------" <<
           endl;
      while ((obj = (KVDetector*) next())) {
         cout << opt << "Detector: " << obj->GetName() << endl;
      }
   }
}

//____________________________________________________________________________________

KVDetector* KVIDTelescope::GetDetector(const Char_t* name) const
{
   // Return a pointer to the detector in the telescope with the name "name".

   KVDetector* tmp = (KVDetector*) GetDetectors()->FindObject(name);
   if (!tmp)
      Warning("GetDetector(const Char_t *name)",
              "Detector %s not found in telescope %s", name, GetName());
   return tmp;
}

//____________________________________________________________________________________

KVGroup* KVIDTelescope::GetGroup() const
{
   return fGroup;
}

//____________________________________________________________________________________

void KVIDTelescope::SetGroup(KVGroup* kvg)
{
   fGroup = kvg;
}

//____________________________________________________________________________________
UInt_t KVIDTelescope::GetGroupNumber()
{
   return (GetGroup() ? GetGroup()->GetNumber() : 0);
}

//____________________________________________________________________________________

TGraph* KVIDTelescope::MakeIDLine(KVNucleus* nuc, Double_t Emin,
                                  Double_t Emax, Double_t Estep)
{
   //For a given nucleus, generate a TGraph representing the line in the deltaE-E
   //plane of the telescope which can be associated with nuclei of this (A,Z) with total
   //incident energies between the two limits.
   //NOTE: if there are other absorbers/detectors placed before this telescope,
   //the energy losses of the particle in these will be taken into account.
   //If the step in energy is not given, it is taken equal to 100 equal steps between min and max.
   //The TGraph should be deleted by the user after use.


   if (!Estep)
      Estep = (Emax - Emin) / 100.;

   Int_t nsteps = 1 + (Int_t)((Emax - Emin) / Estep);

   if (nsteps < 1)
      return 0;

   Double_t* y = new Double_t[nsteps];
   Double_t* x = new Double_t[nsteps];
   Int_t step = 0;

   //get list of all detectors through which particle must pass in order to reach
   //2nd member of ID Telescope
   TList* detectors =
      GetDetector(2)->GetAlignedDetectors(KVGroup::kForwards);
   //detectors->ls();
   TIter next_det(detectors);
   //cout << "nsteps =" << nsteps << endl;

   for (Double_t E = Emin; E <= Emax; E += Estep) {
      //Set energy of nucleus
      nuc->SetEnergy(E);
      //cout << "Einc=" << E << endl;

      //Calculate energy loss in each member and stock in arrays x & y
      //first member
      KVDetector* det = 0;
      x[step] = y[step] = -1;
      while ((det = (KVDetector*) next_det())) {
         //det->Print();
         Double_t eloss = det->GetELostByParticle(nuc);
         if (det == GetDetector(1))
            y[step] = eloss;
         else if (det == GetDetector(2))
            x[step] = eloss;
         Double_t E1 = nuc->GetEnergy() - eloss;
         nuc->SetEnergy(E1);
         //cout << "Eloss=" << eloss << endl;
         //cout << "Enuc=" << nuc->GetEnergy() << endl;
         if (E1 < 1.e-3) break;
      }

      //cout << "step = " << step << " x = " << x[step] << " y = " << y[step] << endl;

      //make sure that some energy is lost in each member
      //otherwise miss a step and reduce number of points in graph
      if (x[step] > 0 && y[step] > 0) {
         step++;
      }
      else {
         nsteps--;
      }

      //cout << "nsteps =" << nsteps << endl;
      //reset iterator ready for next loop on detectors
      next_det.Reset();
   }
   TGraph* tmp = 0;
   if (nsteps > 1)
      tmp = new TGraph(nsteps, x, y);
   delete[]x;
   delete[]y;
   return tmp;
}

//____________________________________________________________________________________

Bool_t KVIDTelescope::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   //Default identification method.
   //
   //Works for ID telescopes for which one or more identification grids are defined, each
   //with VARX/VARY parameters corresponding to a KVDetectorSignal or KVDetectorSignalExpression
   //associated with one or other of the detectors constituting the telescope.
   //
   //For identifications using more than one grid, the default behaviour is to try identification
   //with each grid in turn until a successful identification is obtained. The order in which
   //the grids should be tried should be specified by a variable with the following format:
   //
   //~~~~~~~~~~~~~~~~
   //[Dataset].[Array Name].[ID type].GridOrder:  [Grid1],[Grid2],...
   //~~~~~~~~~~~~~~~~
   //
   //where the name of each grid is given as "VARY_VARX". If no variable defining the order is found,
   //the grids will be tried in the order they were found in the file containing the grids for this
   //telescope.

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   KVIDGraph* grid;
   TIter it(GetListOfIDGrids());
   while ((grid = (KVIDGraph*)it())) { //loop over grids in order given by [Dataset].[Array Name].[ID type].GridOrder:
      Double_t de, e;
      GetIDGridCoords(e, de, grid, x, y);
      idr->SetGridName(grid->GetName());
      if (grid->IsIdentifiable(e, de)) {
         grid->Identify(e, de, idr);
         if (idr->IDOK) break; // stop on first successful identification
      }
      else {
         idr->IDOK = kFALSE;
         idr->IDquality = KVIDZAGrid::kICODE8;
      }
   }
   idr->IDcode = GetIDCode();

   return kTRUE;
}

//____________________________________________________________________________________

void KVIDTelescope::SetIDGrid(KVIDGraph* grid)
{
   // Add an identification grid to the list of grids used by this telescope.
   //
   // If the grid's VARX and VARY parameters are set and contain the names of valid
   // detector signals (see formatting rules below) they will be used by
   // GetIDGridXCoord() and GetIDGridYCoord() to return the coordinates
   // needed to perform particle identification using the grid.
   //
   // The name of the grid is set to "VARY_VARX" (just the signal names, not the detector
   // label part - see below). This value will be stored in the
   // KVIdentificationResult corresponding to an attempted identification of a
   // KVReconstructedNucleus by this grid.
   //
   // VARX/VARY Formatting
   //
   // To be valid, grid VARX/Y parameters should be set as follows:
   //
   //~~~~~~~~~~~~~~~~~~
   //      [signal name]
   //  or  [det_label]::[signal name]
   //~~~~~~~~~~~~~~~~~~
   //
   // where
   //
   //~~~~~~~~~~~~~~~~~~
   //    [det_label] (optional)  = detector label i.e. string returned by KVDetector::GetLabel()
   //                    method for detector. By default, VARX is assumed to be the Eres detector
   //                    or second detector and VARY the DE detector or first detector
   //    [signal_name] = name of a signal defined for the detector, possibly depending
   //                    on availability of calibration
   //
   //    To see all available signals for a detector, use
   //
   //         KVDetector::GetListOfDetectorSignals()
   //~~~~~~~~~~~~~~~~~~

   if (grid) {
      fIDGrids.Add(grid);
      KVDetectorSignal* xx = GetSignalFromGridVar(grid->GetVarX(), "X");
      KVDetectorSignal* yy = GetSignalFromGridVar(grid->GetVarY(), "Y");
      GraphCoords gc;
      gc.fVarX = xx;
      gc.fVarY = yy;
      fGraphCoords[grid] = gc;
      TString grid_name;
      if (xx && yy) {
         grid_name.Form("%s_%s", yy->GetName(), xx->GetName());
         grid->SetName(grid_name);
      }
   }
}

KVDetectorSignal* KVIDTelescope::GetSignalFromGridVar(const KVString& var, const KVString& axe)
{
   // Deduce & return pointer to detector signal from grid VARX/VARY parameter
   //
   // To be valid, grid VARX/Y parameters should be set in one of two ways:
   //
   //~~~~~~~~~~~~~~~~~~
   //      [signal name]
   //      [det_label]::[signal name]
   //~~~~~~~~~~~~~~~~~~
   //
   // where
   //
   //~~~~~~~~~~~~~~~~~~
   //    [signal_name] = name of a signal or a mathematical expression using
   //                    any of the signals defined for the detector
   //    [det_label]   = optional detector label i.e. string returned by
   //                    KVDetector::GetLabel() method for detector
   //~~~~~~~~~~~~~~~~~~
   //
   // If `[det_label]` is not given, we assume for `VARX` the second (E) detector,
   // while for `VARY` we assume the first (dE) detector. If this telescope has only
   // one detector, we use it for both variables.
   //
   //    To see all available signals for a detector, use
   //
   //~~~~~~~~~~~~~~~~~~
   //         KVDetector::GetListOfDetectorSignals()
   //~~~~~~~~~~~~~~~~~~

   if (var == "") {
      Warning("GetSignalFromGridVar",
              "No VAR%s defined for grid for telescope %s. KVIDTelescope-derived class handling identification must override GetIDMapX/GetIDMapY",
              axe.Data(), GetName());
      return nullptr;
   }
   KVDetector* det = nullptr;
   KVDetectorSignal* ds(nullptr);
   KVString sig_type;
   if (var.GetNValues("::") == 2) {
      // VARX/Y = [det_label]::[signal name]
      var.Begin("::");
      KVString det_label = var.Next();
      sig_type = var.Next();
      det = (KVDetector*)GetDetectors()->FindObjectByLabel(det_label);
      if (!det) {
         Error("GetSignalFromGridVar",
               "Problem initialising ID-grid %s coordinate for telescope %s. Request for unknown detector label %s. Check definition of VAR%s for grid (=%s)",
               axe.Data(), GetName(), det_label.Data(), axe.Data(), var.Data());
         return nullptr;
      }
   }
   else {
      // VARX/Y = [signal name]
      if (axe == "Y" || GetSize() == 1) det = GetDetector(1);
      else det = GetDetector(2);
      sig_type = var;
   }
   ds = det->GetDetectorSignal(sig_type);
   if (!ds) {
      // sig_type is not the name of a known signal: assume it is an expression using known signal names
      if (!det->AddDetectorSignalExpression(Form("%s-VAR%s", GetName(), axe.Data()), sig_type)) {
         Error("GetSignalFromGridVar",
               "Problem initialising ID-grid %s coordinate for telescope %s. Request for unknown signal %s for detector %s. Check definition of VAR%s for grid (=%s)",
               axe.Data(), GetName(), sig_type.Data(), det->GetName(), axe.Data(), var.Data());
         ds = nullptr;
      }
      else
         ds = det->GetDetectorSignal(Form("%s-VAR%s", GetName(), axe.Data()));
   }
   return ds;
}

//____________________________________________________________________________________

KVIDGraph* KVIDTelescope::GetIDGrid()
{
   //Return the first in the list of identification grids used by this telescope
   //(this is for backwards compatibility with ID telescopes which had only one grid).
   return (KVIDGraph*)GetListOfIDGrids()->First();
}

//____________________________________________________________________________________

KVIDGraph* KVIDTelescope::GetIDGrid(Int_t index)
{
   //Return pointer to grid using position in list. First grid has index = 1.
   return (KVIDGraph*)GetListOfIDGrids()->At(index - 1);
}

//____________________________________________________________________________________

KVIDGraph* KVIDTelescope::GetIDGrid(const Char_t* label)
{
   //Return pointer to grid using "label" to search in list of grids associated
   //to this telescope.
   return (KVIDGraph*)GetListOfIDGrids()->FindObjectByLabel(label);
}

//____________________________________________________________________________________

Double_t KVIDTelescope::GetIDMapX(Option_t*)
{
   AbstractMethod("GetIDMapX");
   return -1.;
}

Double_t KVIDTelescope::GetPedestalX(Option_t*)
{
   // Returns the pedestal associated with the 2nd detector of the telescope,
   // optionally depending on the given option string.
   // By default this returns 0, and should be overridden in specific implementations.

   return 0.;
}

Double_t KVIDTelescope::GetPedestalY(Option_t*)
{
   // Returns the pedestal associated with the 1st detector of the telescope,
   // optionally depending on the given option string.
   // By default this returns 0, and should be overridden in specific implementations.

   return 0.;
}

Double_t KVIDTelescope::GetIDGridXCoord(KVIDGraph* g) const
{
   // Return value of X coordinate to be used with the given ID grid
   // This corresponds to whatever was given as parameter "VARX" for the grid

   KVDetectorSignal* ds = fGraphCoords[g].fVarX;
   if (ds) return ds->GetValue();
   return -1;
}

Double_t KVIDTelescope::GetIDGridYCoord(KVIDGraph* g) const
{
   // Return value of Y coordinate to be used with the given ID grid
   // This corresponds to whatever was given as parameter "VARY" for the grid

   KVDetectorSignal* ds = fGraphCoords[g].fVarY;
   if (ds) return ds->GetValue();
   return -1;
}

//____________________________________________________________________________________

Double_t KVIDTelescope::GetIDMapY(Option_t*)
{
   AbstractMethod("GetIDMapY");
   return -1.;
}

//____________________________________________________________________________________

void KVIDTelescope::RemoveGrids()
{
   //Remove all identification grids for this ID telescope
   //Grids are not deleted as this is handled by gIDGridManager
   fIDGrids.Clear();
   fGraphCoords.clear();
}

//____________________________________________________________________________________

KVIDTelescope* KVIDTelescope::MakeIDTelescope(const Char_t* uri)
{
   //Static function which will create an instance of the KVIDTelescope-derived class
   //corresponding to 'name'
   //These are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   // # The KVMultiDetArray::GetIDTelescopes(KVDetector*de, KVDetector*e) method uses these plugins to
   // # create KVIDTelescope instances adapted to the specific array geometry and detector types.
   // # For each pair of detectors we look for a plugin with one of the following names:
   // #    [name_of_dataset].de_detector_type[de detector thickness]-e_detector_type[de detector thickness]
   // # Each characteristic in [] brackets may or may not be present in the name; first we test for names
   // # with these characteristics, then all combinations where one or other of the characteristics is not present.
   // # In addition, we first test all combinations which begin with [name_of_dataset].
   // # The first plugin found in this order will be used.
   // # In addition, if for one of the two detectors there is a plugin called
   // #    [name_of_dataset].de_detector_type[de detector thickness]
   // #    [name_of_dataset].e_detector_type[e detector thickness]
   // # then we add also an instance of this 1-detector identification telescope.

   //check and load plugin library
   TPluginHandler* ph;
   if (!(ph = LoadPlugin("KVIDTelescope", uri)))
      return 0;

   //execute constructor/macro for identification telescope
   KVIDTelescope* mda = (KVIDTelescope*) ph->ExecPlugin(0);
   if (mda) {
      //set label of telescope with URI used to find plugin (minus dataset name)
      mda->SetLabelFromURI(uri);
   }

   return mda;
}

//____________________________________________________________________________________

void KVIDTelescope::SetLabelFromURI(const Char_t* uri)
{
   //PRIVATE METHOD
   //Sets label of telescope based on URI of plugin describing child class for this telescope

   TString _uri(uri);
   if (gDataSet && _uri.BeginsWith(gDataSet->GetName())) _uri.Remove(0, strlen(gDataSet->GetName()) + 1);
   SetLabel(_uri.Data());
}

//____________________________________________________________________________________

Bool_t KVIDTelescope::SetIdentificationParameters(const KVMultiDetArray* multidet)
{
   // Initialise the identification parameters (grids, etc.) of ALL identification telescopes of this
   // kind (label) in the multidetector array. Therefore this method need only be called once, and not
   // called for each telescope. The kind/label (returned by GetLabel) of the telescope corresponds
   // to the URI used to find the plugin class in $KVROOT/KVFiles/.kvrootrc.
   // By default, this method looks for the file with name given by the environment variable
   //
   // [dataset name].IdentificationParameterList.[telescope label]:       [filename]
   //
   // which is assumed to contain the list of files containing the identification grids.
   //
   // If not such envionment variable is found, the method looks for another one:
   //
   // [dataset name].IdentificationParameterFile.[telescope label]:       [filename]
   //
   // which is assumed to contain identification grids.

   TString filename = gDataSet->GetDataSetEnv(Form("IdentificationParameterList.%s", GetLabel()));

   if (filename != "") {
      ReadIdentificationParameterFiles(filename.Data(), multidet);
      return kTRUE;
   }
   else {
      filename = gDataSet->GetDataSetEnv(Form("IdentificationParameterFile.%s", GetLabel()));

      if (filename == "") {
         Warning("SetIdentificationParameters",
                 "No filename defined. Should be given by %s.IdentificationParameterFile.%s or %s.IdentificationParameterFile.%s",
                 gDataSet->GetName(), GetLabel(), gDataSet->GetName(), GetLabel());
         return kFALSE;
      }

      LoadIdentificationParameters(filename, multidet);
   }
}

//____________________________________________________________________________________

void KVIDTelescope::ReadIdentificationParameterFiles(const Char_t* filename, const KVMultiDetArray* multidet)
{
   // In the case where the identification grids are stored in several files, this method parse
   // the file found with the following environment variable:
   //
   // [dataset name].IdentificationParameterList.[telescope label]:       [filename]
   //
   // which contains the list of files containing the identification grids.

   KVFileReader fr;
   fr.OpenFileToRead(gDataSet->GetFullPathToDataSetFile(filename));

   while (fr.IsOK()) {
      fr.ReadLine(0);

      if (fr.GetCurrentLine() != "") LoadIdentificationParameters(fr.GetCurrentLine().Data(), multidet);
   }

   fr.CloseFile();
}

//____________________________________________________________________________________

void KVIDTelescope::LoadIdentificationParameters(const Char_t* filename, const KVMultiDetArray* multidet)
{
   // This method add to the gIDGridManager list the identification grids.

   TString path;

   if ((path = gDataSet->GetFullPathToDataSetFile(filename)) == "") {
      Error("LoadIdentificationParameters",
            "File %s not found. Should be in %s",
            filename, gDataSet->GetDataSetDir());
      return;
   }
   //
   //Read grids from file
   Info("LoadIdentificationParameters", "Using file %s", path.Data());
   multidet->ReadGridsFromAsciiFile(path);
}

//____________________________________________________________________________________

void KVIDTelescope::RemoveIdentificationParameters()
{
   //Remove identification parameters from telescope in such a way that they
   //can subsequently be reset e.g. with a new version.
   //This is used by KVMultiDetArray::UpdateIdentifications.
   //Child classes with specific SetIdentificationParameters methods should
   //also redefine this method in order to remove (destroy) cleanly the objects
   //created in SetIdentificationParameters.
   //
   //This default method takes the list of grids associated to the telescope,
   //and for each one: 1) checks if it is still in the gIDGridManager's list
   //2) if yes, delete the grid and remove it from gIDGridManager

   TIter next_grid(GetListOfIDGrids());
   KVIDGrid* grid;
   while ((grid = (KVIDGrid*)next_grid())) {

      if (gIDGridManager->GetGrids()->FindObject(grid)) {   //this only works if KVIDTelescope uses TObject:IsEqual method (i.e. compares pointers)

         gIDGridManager->DeleteGrid(grid);

      }
   }
   //clear list of grids
   fIDGrids.Clear();
   SetHasMassID(kFALSE);
}

//____________________________________________________________________________________

void KVIDTelescope::CalculateParticleEnergy(KVReconstructedNucleus* nuc)
{
   // The energy of each particle is calculated as follows:
   //
   //      E = dE_1 + dE_2 + ... + dE_N
   //
   // dE_1, dE_2, ... = energy losses measured in each detector through which
   //                          the particle has passed (or stopped, in the case of dE_N).
   //                         These energy losses are corrected for (Z,A)-dependent effects
   //                          such as pulse-heigth defect in silicon detectors, losses in
   //                          windows of gas detectors, etc.
   //
   // Whenever possible, the energy loss for fired detectors which are uncalibrated
   // or not functioning is calculated. In this case the status returned by GetCalibStatus()
   // will be KVIDTelescope::kCalibStatus_Calculated.
   // If none of the detectors is calibrated, the particle's energy cannot be calculated &
   // the status will be KVIDTelescope::kCalibStatus_NoCalibrations.
   // Otherwise, the status code will be KVIDTelescope::kCalibStatus_OK.

   //status code
   fCalibStatus = kCalibStatus_NoCalibrations;

   UInt_t z = nuc->GetZ();
   //uncharged particles
   if (z == 0) return;

   KVDetector* d1 = GetDetector(1);
   KVDetector* d2 = (GetSize() > 1 ? GetDetector(2) : 0);
   Bool_t d1_cal = d1->IsCalibrated();
   Bool_t d2_cal = (d2 ? d2->IsCalibrated() : kFALSE);

   //no calibrations
   if (!d1_cal && !d2)
      return;
   if ((d1 && d2) && !d1_cal && !d2_cal)
      return;

   //status code
   fCalibStatus = kCalibStatus_OK;

   UInt_t a = nuc->GetA();

   // particles stopped in first member of telescope
   if (nuc->GetStatus() == 3) {
      if (d1_cal) {
         nuc->SetEnergy(d1->GetCorrectedEnergy(nuc, -1, kFALSE));  //N.B.: transmission=kFALSE because particle stop in d1
      }
      return;
   }

   Double_t e1, e2, einc;
   e1 = e2 = einc = 0.0;

   if (!d1_cal) {//1st detector not calibrated - calculate from residual energy in 2nd detector

      //second detector must exist and have all acquisition parameters fired with above-pedestal value
      if (d2 && d2->Fired("Pall")) e2 = d2->GetCorrectedEnergy(nuc, -1, kFALSE); //N.B.: transmission=kFALSE because particle stop in d2
      if (e2 <= 0.0) {
         // zero energy loss in 2nd detector ? can't do anything...
         fCalibStatus = kCalibStatus_NoCalibrations;
         return;
      }
      //calculate & set energy loss in dE detector
      //N.B. using e2 for the residual energy after detector 1 means
      //that we are assuming the particle stops in detector 2.
      //if this is not true, we will underestimate the energy of the particle.
      e1 = d1->GetDeltaEFromERes(z, a, e2);
      if (e1 < 0.0) e1 = 0.0;
      else {
         d1->SetEnergyLoss(e1);
         d1->SetEResAfterDetector(e2);
         e1 = d1->GetCorrectedEnergy(nuc);
         //status code
         fCalibStatus = kCalibStatus_Calculated;
      }
   }
   else {  //1st detector is calibrated too: get corrected energy loss

      e1 = d1->GetCorrectedEnergy(nuc);

   }

   if (d2 && !d2_cal) {//2nd detector not calibrated - calculate from energy loss in 1st detector

      e1 = d1->GetCorrectedEnergy(nuc);
      if (e1 <= 0.0) {
         // zero energy loss in 1st detector ? can't do anything...
         fCalibStatus = kCalibStatus_NoCalibrations;
         return;
      }
      //calculate & set energy loss in 2nd detector
      e2 = d1->GetEResFromDeltaE(z, a);
      if (e2 < 0.0) e2 = 0.0;
      else {
         e2 = d2->GetDeltaE(z, a, e2);
         d2->SetEnergyLoss(e2);
         e2 = d2->GetCorrectedEnergy(nuc);
         //status code
         fCalibStatus = kCalibStatus_Calculated;
      }
   }
   else if (d2) {   //2nd detector is calibrated too: get corrected energy loss

      e2 = d2->GetCorrectedEnergy(nuc, -1, kFALSE);//N.B.: transmission=kFALSE because particle assumed to stop in d2
      // recalculate corrected energy in first stage using info on Eres
      d1->SetEResAfterDetector(e2);
      e1 = d1->GetCorrectedEnergy(nuc);
   }

   //incident energy of particle (before 1st member of telescope)
   einc = e1 + e2;

   Double_t coherence_tolerance = gEnv->GetValue("KVIDTelescope.CoherencyTolerance", 1.05);
   if (coherence_tolerance < 1) coherence_tolerance += 1.00;

   //Now we have to work our way up the list of detectors from which the particle was
   //reconstructed. For each fired & calibrated detector which is only associated with
   //one particle in the events, we add the corrected measured energy loss
   //to the particle. For uncalibrated, unfired detectors and detectors through which
   //more than one particle has passed, we calculate the corrected energy loss and add it
   //to the particle.
   int ndets = nuc->GetNumDet();
   if (ndets > (int)GetSize()) { //particle passed through other detectors before this idtelesocpe
      //look at detectors not in this id telescope
      int idet = GetSize();//next detector after delta-e member of IDTelescope (stopping detector = 0)
      while (idet < ndets) {

         KVDetector* det = nuc->GetDetector(idet);
         if (det->Fired() && det->IsCalibrated() && det->GetNHits() == 1) {
            Double_t dE = det->GetEnergy();
            //in order to check if particle was really the only one to
            //hit each detector, we calculate the particle's energy loss
            //from its residual energy. if the measured energy loss is
            //significantly larger, there may be a second particle.
            e1 = det->GetDeltaEFromERes(z, a, einc);
            if (e1 < 0.0) e1 = 0.0;
            det->SetEResAfterDetector(einc);
            dE = det->GetCorrectedEnergy(nuc);
            einc += dE;
         }
         else {
            // Uncalibrated/unfired/multihit detector. Calculate energy loss.
            //calculate energy of particle before detector from energy after detector
            e1 = det->GetDeltaEFromERes(z, a, einc);
            if (e1 < 0.0) e1 = 0.0;
            if (det->GetNHits() > 1) {
               //Info("CalculateParticleEnergy",
               //    "Detector %s was hit by %d particles. Calculated energy loss for particle %f MeV",
               //    det->GetName(), det->GetNHits(), e1);
               if (!(det->Fired() && det->IsCalibrated())) {
                  det->SetEnergyLoss(e1 + det->GetEnergy());// sum up calculated energy losses in uncalibrated detector
               }
               //status code
               fCalibStatus = kCalibStatus_Multihit;
            }
            else if (!det->Fired() || !det->IsCalibrated()) {
               //Info("CalculateParticleEnergy",
               //    "Detector %s uncalibrated/not fired. Calculated energy loss for particle %f MeV",
               //    det->GetName(), e1);
               det->SetEnergyLoss(e1);
               //status code
               fCalibStatus = kCalibStatus_Calculated;
            }
            det->SetEResAfterDetector(einc);
            e1 = det->GetCorrectedEnergy(nuc, e1);
            einc += e1;
         }
         idet++;
      }
   }
   //einc is now the energy of the particle before crossing the first detector
   nuc->SetEnergy(einc);
}

//_____________________________________________________________________________________________________//

const Char_t* KVIDTelescope::GetDefaultIDGridClass()
{
   // Returns name of default ID grid class for this ID telescope.
   // This is defined in a .kvrootrc configuration file by one of the following:
   // KVIDTelescope.DefaultGrid:
   // KVIDTelescope.DefaultGrid.[type]:
   // where [type] is the type of this identification telescope (which is given
   // by the character string returned by method GetLabel()... sorry :( )
   // If no default grid is defined for the specific type of this telescope,
   // the default defined by KVIDTelescope.DefaultGrid is used.

   TString specific;
   specific.Form("KVIDTelescope.DefaultGrid.%s", GetLabel());
   return gEnv->GetValue(specific.Data(), gEnv->GetValue("KVIDTelescope.DefaultGrid", "KVIDGraph"));
}

//_____________________________________________________________________________________________________//
KVIDGrid* KVIDTelescope::CalculateDeltaE_EGrid(const KVNumberList& Zrange, Int_t deltaA, Int_t npoints, Double_t lifetime, UChar_t massformula, Double_t xfactor)
{
   //Genere une grille dE-E (perte d'energie - energie residuelle) pour une gamme en Z donnee
   // - Zrange definit l'ensemble des charges pour lequel les lignes vont etre calculees
   // - deltaA permet de definir si a chaque Z n'est associee qu'un seul A (deltaA=0) ou plusieurs
   //Exemple :
   //      deltaA=1 -> Aref-1, Aref et Aref+1 seront les masses associees a chaque Z et
   //      donc trois lignes de A par Z. le Aref pour chaque Z est determine par
   //      la formule de masse par defaut (Aref = KVNucleus::GetA() voir le .kvrootrc)
   //      deltaA=0 -> pour chaque ligne de Z le A associe sera celui de KVNucleus::GetA()
   // - est le nombre de points par ligne
   //
   //un noyau de A et Z donne n'est considere que s'il retourne KVNucleus::IsKnown() = kTRUE
   //
   if (GetSize() <= 1) return 0;

   TClass* cl = TClass::GetClass(GetDefaultIDGridClass());
   if (!cl || !cl->InheritsFrom("KVIDZAGrid")) cl = TClass::GetClass("KVIDZAGrid");
   KVIDGrid* idgrid = (KVIDGrid*)cl->New();

   idgrid->AddIDTelescope(this);
   idgrid->SetOnlyZId((deltaA == 0));

   KVDetector* det_de = GetDetector(1);
   if (!det_de)      return 0;
   KVDetector* det_eres = GetDetector(2);
   if (!det_eres)    return 0;

   KVNucleus part;
   Info("CalculateDeltaE_EGrid",
        "Calculating dE-E grid: dE detector = %s, E detector = %s",
        det_de->GetName(), det_eres->GetName());

   KVIDCutLine* B_line = (KVIDCutLine*)idgrid->Add("OK", "KVIDCutLine");
   Int_t npoi_bragg = 0;
   B_line->SetName("Bragg_line");
   B_line->SetAcceptedDirection("right");

   Double_t SeuilE = 0.1;

   Zrange.Begin();
   while (!Zrange.End()) {
      Int_t zz = Zrange.Next();
      part.SetZ(zz, massformula);
      Int_t aref = part.GetA();
      //        printf("%d\n",zz);
      for (Int_t aa = aref - deltaA; aa <= aref + deltaA; aa += 1) {
         part.SetA(aa);
         //            printf("+ %d %d %d\n",aa,aref,part.IsKnown());
         if (part.IsKnown() && (part.GetLifeTime() > lifetime)) {

            //loop over energy
            //first find :
            //  ****E1 = energy at which particle passes 1st detector and starts to enter in the 2nd one****
            //      E2 = energy at which particle passes the 2nd detector
            //then perform npoints calculations between these two energies and use these
            //to construct a KVIDZALine

            Double_t E1, E2;
            //find E1
            //go from SeuilE MeV to det_de->GetEIncOfMaxDeltaE(part.GetZ(),part.GetA()))
            Double_t E1min = SeuilE, E1max = det_de->GetEIncOfMaxDeltaE(zz, aa);
            E1 = (E1min + E1max) / 2.;

            while ((E1max - E1min) > SeuilE) {

               part.SetEnergy(E1);
               det_de->Clear();
               det_eres->Clear();

               det_de->DetectParticle(&part);
               det_eres->DetectParticle(&part);
               if (det_eres->GetEnergy() > SeuilE) {
                  //particle got through - decrease energy
                  E1max = E1;
                  E1 = (E1max + E1min) / 2.;
               }
               else {
                  //particle stopped - increase energy
                  E1min = E1;
                  E1 = (E1max + E1min) / 2.;
               }
            }

            //add point to Bragg line
            Double_t dE_B = det_de->GetMaxDeltaE(zz, aa);
            Double_t E_B = det_de->GetEIncOfMaxDeltaE(zz, aa);
            Double_t Eres_B = det_de->GetERes(zz, aa, E_B);
            B_line->SetPoint(npoi_bragg++, Eres_B, dE_B);

            //find E2
            //go from E1 MeV to maximum value where the energy loss formula is valid
            Double_t E2min = E1, E2max = det_eres->GetEmaxValid(part.GetZ(), part.GetA());
            E2 = (E2min + E2max) / 2.;

            while ((E2max - E2min > SeuilE)) {

               part.SetEnergy(E2);
               det_de->Clear();
               det_eres->Clear();

               det_de->DetectParticle(&part);
               det_eres->DetectParticle(&part);
               if (part.GetEnergy() > SeuilE) {
                  //particle got through - decrease energy
                  E2max = E2;
                  E2 = (E2max + E2min) / 2.;
               }
               else {
                  //particle stopped - increase energy
                  E2min = E2;
                  E2 = (E2max + E2min) / 2.;
               }
            }
            E2 *= xfactor;
            if ((!strcmp(det_eres->GetType(), "CSI")) && (E2 > 5000)) E2 = 5000;
            //                printf("z=%d a=%d E1=%lf E2=%lf\n",zz,aa,E1,E2);
            KVIDZALine* line = (KVIDZALine*)idgrid->Add("ID", "KVIDZALine");
            if (TMath::Even(zz)) line->SetLineColor(4);
            line->SetZ(zz);
            line->SetA(aa);

            Double_t logE1 = TMath::Log(E1);
            Double_t logE2 = TMath::Log(E2);
            Double_t dLog = (logE2 - logE1) / (npoints - 1.);

            for (Int_t i = 0; i < npoints; i++) {
               //              Double_t E = E1 + i*(E2-E1)/(npoints-1.);
               Double_t E = TMath::Exp(logE1 + i * dLog);

               Double_t Eres = 0.;
               Int_t niter = 0;
               while (Eres < SeuilE && niter <= 20) {
                  det_de->Clear();
                  det_eres->Clear();

                  part.SetEnergy(E);

                  det_de->DetectParticle(&part);
                  det_eres->DetectParticle(&part);

                  Eres = det_eres->GetEnergy();
                  E += SeuilE;
                  niter += 1;
               }
               if (!(niter > 20)) {
                  Double_t dE = det_de->GetEnergy();
                  Double_t gEres, gdE;
                  line->GetPoint(i - 1, gEres, gdE);
                  line->SetPoint(i, Eres, dE);

               }
            }
            //printf("sort de boucle points");
         }
      }
   }

   idgrid->SetRunList("1-10000");

   return idgrid;

}

//_____________________________________________________________________________________________________//
KVIDGrid* KVIDTelescope::CalculateDeltaE_EGrid(TH2* haa_zz, Bool_t Zonly, Int_t npoints)
{
   //Genere une grille dE-E (perte d'energie - energie residuelle)
   //Le calcul est fait pour chaque couple comptant de charge (Z) et masse (A)
   //au moins un coup dans l'histogramme haa_zz definit :
   // Axe X -> Z
   // Axe Y -> A
   //
   //- Si Zonly=kTRUE (kFALSE par defaut), pour un Z donne, le A choisi est la valeur entiere la
   //plus proche de la valeur moyenne <A>
   //- Si Zonly=kFALSE et que pour un Z donne il n'y a qu'un seul A associe, les lignes correspondants
   //a A-1 et A+1 sont ajoutes
   //- Si a un Z donne, il n'y a aucun A, pas de ligne tracee
   //un noyau de A et Z donne n'est considere que s'il retourne KVNucleus::IsKnown() = kTRUE
   //
   // Warning : the grid is not added to the list of the telescope and MUST BE DELETED by the user !

   if (GetSize() <= 1) return 0;

   TClass* cl = new TClass(GetDefaultIDGridClass());
   KVIDGrid* idgrid = (KVIDGrid*)cl->New();
   delete cl;

   idgrid->AddIDTelescope(this);
   idgrid->SetOnlyZId(Zonly);

   KVDetector* det_de = GetDetector(1);
   if (!det_de)      return 0;
   KVDetector* det_eres = GetDetector(2);
   if (!det_eres)    return 0;

   KVNucleus part;
   Info("CalculateDeltaE_EGrid",
        "Calculating dE-E grid: dE detector = %s, E detector = %s",
        det_de->GetName(), det_eres->GetName());

   KVIDCutLine* B_line = (KVIDCutLine*)idgrid->Add("OK", "KVIDCutLine");
   Int_t npoi_bragg = 0;
   B_line->SetName("Bragg_line");
   B_line->SetAcceptedDirection("right");

   Double_t SeuilE = 0.1;

   for (Int_t nx = 1; nx <= haa_zz->GetNbinsX(); nx += 1) {

      Int_t zz = TMath::Nint(haa_zz->GetXaxis()->GetBinCenter(nx));
      KVNumberList nlA;
      Double_t sumA = 0, sum = 0;
      for (Int_t ny = 1; ny <= haa_zz->GetNbinsY(); ny += 1) {
         Double_t stat = haa_zz->GetBinContent(nx, ny);
         if (stat > 0) {
            Double_t val = haa_zz->GetYaxis()->GetBinCenter(ny);
            nlA.Add(TMath::Nint(val));
            sumA += val * stat;
            sum  += stat;
         }
      }
      sumA /= sum;
      Int_t nA = nlA.GetNValues();
      if (nA == 0) {
         Warning("CalculateDeltaE_EGrid", "no count for Z=%d", zz);
      }
      else {
         if (Zonly) {
            nlA.Clear();
            nlA.Add(TMath::Nint(sumA));
         }
         else {
            if (nA == 1) {
               Int_t aref = nlA.Last();
               nlA.Add(aref - 1);
               nlA.Add(aref + 1);
            }
         }
         part.SetZ(zz);
         //            printf("zz=%d\n",zz);
         nlA.Begin();
         while (!nlA.End()) {
            Int_t aa = nlA.Next();
            part.SetA(aa);
            //                printf("+ aa=%d known=%d\n",aa,part.IsKnown());
            if (part.IsKnown()) {

               //loop over energy
               //first find :
               //  ****E1 = energy at which particle passes 1st detector and starts to enter in the 2nd one****
               //      E2 = energy at which particle passes the 2nd detector
               //then perform npoints calculations between these two energies and use these
               //to construct a KVIDZALine

               Double_t E1, E2;
               //find E1
               //go from SeuilE MeV to det_de->GetEIncOfMaxDeltaE(part.GetZ(),part.GetA()))
               Double_t E1min = SeuilE, E1max = det_de->GetEIncOfMaxDeltaE(zz, aa);
               E1 = (E1min + E1max) / 2.;

               while ((E1max - E1min) > SeuilE) {

                  part.SetEnergy(E1);
                  det_de->Clear();
                  det_eres->Clear();

                  det_de->DetectParticle(&part);
                  det_eres->DetectParticle(&part);
                  if (det_eres->GetEnergy() > SeuilE) {
                     //particle got through - decrease energy
                     E1max = E1;
                     E1 = (E1max + E1min) / 2.;
                  }
                  else {
                     //particle stopped - increase energy
                     E1min = E1;
                     E1 = (E1max + E1min) / 2.;
                  }
               }

               //add point to Bragg line
               Double_t dE_B = det_de->GetMaxDeltaE(zz, aa);
               Double_t E_B = det_de->GetEIncOfMaxDeltaE(zz, aa);
               Double_t Eres_B = det_de->GetERes(zz, aa, E_B);
               B_line->SetPoint(npoi_bragg++, Eres_B, dE_B);

               //find E2
               //go from E1 MeV to maximum value where the energy loss formula is valid
               Double_t E2min = E1, E2max = det_eres->GetEmaxValid(part.GetZ(), part.GetA());
               E2 = (E2min + E2max) / 2.;

               while ((E2max - E2min > SeuilE)) {

                  part.SetEnergy(E2);
                  det_de->Clear();
                  det_eres->Clear();

                  det_de->DetectParticle(&part);
                  det_eres->DetectParticle(&part);
                  if (part.GetEnergy() > SeuilE) {
                     //particle got through - decrease energy
                     E2max = E2;
                     E2 = (E2max + E2min) / 2.;
                  }
                  else {
                     //particle stopped - increase energy
                     E2min = E2;
                     E2 = (E2max + E2min) / 2.;
                  }
               }

               //                    printf("z=%d a=%d E1=%lf E2=%lf\n",zz,aa,E1,E2);
               KVIDZALine* line = (KVIDZALine*)idgrid->Add("ID", "KVIDZALine");
               if (TMath::Even(zz)) line->SetLineColor(4);
               line->SetAandZ(aa, zz);

               Double_t logE1 = TMath::Log(E1);
               Double_t logE2 = TMath::Log(E2);
               Double_t dLog = (logE2 - logE1) / (npoints - 1.);

               for (Int_t i = 0; i < npoints; i++) {
                  Double_t E = TMath::Exp(logE1 + i * dLog);
                  Double_t Eres = 0.;
                  Int_t niter = 0;
                  while (Eres < SeuilE && niter <= 20) {
                     det_de->Clear();
                     det_eres->Clear();

                     part.SetEnergy(E);

                     det_de->DetectParticle(&part);
                     det_eres->DetectParticle(&part);

                     Eres = det_eres->GetEnergy();
                     E += SeuilE;

                     niter += 1;
                  }
                  if (!(niter > 20)) {
                     Double_t dE = det_de->GetEnergy();
                     Double_t gEres, gdE;
                     line->GetPoint(i - 1, gEres, gdE);
                     line->SetPoint(i, Eres, dE);
                  }
               }

            }
         }

      }

   }

   return idgrid;

}
//_________________________________________________________________________________________

Double_t KVIDTelescope::GetMeanDEFromID(Int_t& status, Int_t Z, Int_t A, Double_t Eres)
{
   // Returns the Y-axis value in the 2D identification map containing isotope (Z,A)
   // corresponding to either the given X-axis/Eres value or the current X-axis value given by GetIDGridXCoord()
   // If no mass information is available, just give Z.
   //
   // In this (default) implementation this means scanning the ID grids associated with
   // this telescope until we find an identification line Z or (Z,A), and then interpolating
   // the Y-coordinate for the current X-coordinate value.
   //
   // Status variable can take one of following values:
   //
   //  KVIDTelescope::kMeanDE_OK                    all OK
   //   KVIDTelescope::kMeanDE_XtooSmall      X-coordinate is smaller than smallest X-coordinate of ID line
   //   KVIDTelescope::kMeanDE_XtooLarge     X-coordinate is larger than largest X-coordinate of ID line
   //   KVIDTelescope::kMeanDE_NoIdentifie    No identifier found for Z or (Z,A)

   status = kMeanDE_OK;
   // loop over grids
   TIter next(GetListOfIDGrids());
   KVIDGrid* grid;
   KVIDLine* idline = 0;
   while ((grid = (KVIDGrid*)next())) {
      idline = (KVIDLine*)grid->GetIdentifier(Z, A);
      if (idline) break;
   }
   if (!idline) {
      status = kMeanDE_NoIdentifier;
      return -1.;
   }
   Double_t x, x1, y1, x2, y2;
   x = (Eres < 0 ? GetIDGridXCoord(grid) : Eres);
   idline->GetEndPoint(x2, y2);
   if (x > x2) {
      status = kMeanDE_XtooLarge;
      return -1;
   }
   idline->GetStartPoint(x1, y1);
   if (x < x1) {
      status = kMeanDE_XtooSmall;
      return -1.;
   }
   return idline->Eval(x);
}

void KVIDTelescope::SetIDCode(KVReconstructedNucleus* n, UShort_t c)
{
   n->SetIDCode(c);
}

//_________________________________________________________________________________________

Bool_t KVIDTelescope::CheckTheoreticalIdentificationThreshold(KVNucleus* ION, Double_t EINC)
{
   // Return kTRUE if energy of ION is > minimum incident energy required for identification
   // This theoretical limit is defined here to be the incident energy for which the
   // dE in the first detector of a dE-E telescope is maximum.
   // If EINC>0 it is assumed to be the energy of the ion just before the first detector
   // (case where ion would have to pass other detectors before reaching this telescope).
   //
   // If this is not a dE-E telescope, we return kTRUE by default.

   if (GetSize() < 2) return kTRUE;

   KVDetector* dEdet = GetDetector(1);
   Double_t emin = dEdet->GetEIncOfMaxDeltaE(ION->GetZ(), ION->GetA());
   if (EINC > 0.0) return (EINC > emin);
   return (ION->GetEnergy() > emin);
}

void KVIDTelescope::SetIdentificationStatus(KVReconstructedNucleus* n)
{
   // For filtering simulations
   // Set the n->IsZMeasured() and n->IsAMeasured() status of the particle
   // In principle this depends on whether this telescope provides mass
   // identification or not, but this may depend on the particle's energy.
   // If A was not measured, it will be replaced with a value calculated
   // from whatever mass formula is used for the particle.
   //
   // In order to enable mass identification for certain telescopes without a dedicated
   // implementation (e.g. for simulating array response), put the following lines
   // in your .kvrootrc:
   //
   //   [dataset].[telescope label].MassID:   yes
   //
   // If you want to limit mass identification to certain values of Z and/or A,
   // add the following line:
   //
   //   [dataset].[telescope label].MassID.Validity:    [expression]
   //
   // where [expression] is some valid C++ boolean expression involving Z and/or A,
   // for example
   //
   //   [dataset].[telescope label].MassID.Validity:  (Z>3)&&(A<20)
   //
   // Then this expression will be tested here in order to determine particle
   // identification status

   n->SetZMeasured();
   if (!HasMassID()) {
      n->SetAMeasured(kFALSE);
      // beware - changing particle's mass changes its KE (momentum is conserved)
      double e = n->GetE();
      n->SetZ(n->GetZ());// use mass formula for A
      n->SetE(e);
   }
   else {
      if (fMassIDValidity) n->SetAMeasured(fMassIDValidity->Test(n)); // test expression for mass ID validity
      else n->SetAMeasured();   // no expression set; all nuclei are identified in mass
      if (!n->IsAMeasured()) {
         // beware - changing particle's mass changes its KE (momentum is conserved)
         double e = n->GetE();
         n->SetZ(n->GetZ()); // use mass formula for A
         n->SetE(e);
      }
   }
}

void KVIDTelescope::OpenIdentificationBilan(const TString& path)
{
   // Open IdentificationBilan.dat file with given path

   if (fgIdentificationBilan) delete fgIdentificationBilan;
   fgIdentificationBilan = new TEnv(path);
}

void KVIDTelescope::CheckIdentificationBilan(const TString& system)
{
   // Set status of ID Telescope for given system
   if (!(fgIdentificationBilan->GetValue(Form("%s.%s", system.Data(), GetName()), kTRUE))) ResetBit(kReadyForID);
}
