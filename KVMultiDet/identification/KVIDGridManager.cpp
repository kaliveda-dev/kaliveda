/***************************************************************************
                          KVIDGridManager.cpp  -  description
                             -------------------
    begin                : Jan 27 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDGridManager.cpp,v 1.13 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVIDGridManager.h"
#include "Riostream.h"
#include "KVString.h"
#include "TClass.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVIDGridManager)
KVIDGridManager* gIDGridManager;

KVIDGridManager::KVIDGridManager()
{
   //Default constructor
   //Initialise global pointer gIDGridManager
   //Create list for ID grids
   gIDGridManager = this;
   fGrids = new KVList;
   fGrids->SendModifiedSignals(kTRUE);
   fGrids->Connect("Modified()", "KVIDGridManager", this, "Modified()");
   fGrids->SetCleanup();
}

KVIDGridManager::~KVIDGridManager()
{
   //Destructor
   //Reset global pointer gIDGridManager
   //Delete list of ID grids (this deletes all the grids)
   Info("~KVIDGridManager", "DELETING ID GRID MANAGER");
   if (gIDGridManager == this)
      gIDGridManager = 0;
   fGrids->Disconnect("Modified()", this, "Modified()");
   fLastReadGrids.Clear();
   fGrids->Delete();
   delete fGrids;
}

void KVIDGridManager::AddGrid(KVIDGraph* grid)
{
   // Add a grid to the collection. It will be deleted by the manager.

   fGrids->Add(grid);
}

void KVIDGridManager::DeleteGrid(KVIDGraph* grid, Bool_t update)
{
   //Remove grid from manager's list and delete it
   //update flag allows to disable the emission of the 'Modified' signal in case the GUI
   //is deleting a list of grids - in this case we don't want to update until the end

   if (!update) fGrids->Disconnect("Modified()", this, "Modified()");
   fGrids->Remove(grid);
   delete grid;
   if (!update) fGrids->Connect("Modified()", "KVIDGridManager", this, "Modified()");
}

void KVIDGridManager::Clear(Option_t*)
{
   //Delete all grids and empty list, ready to start anew
   Info("Clear", "DELETING ALL GRIDS IN IDGRIDMANAGER");
   fGrids->Disconnect("Modified()", this, "Modified()");
   fLastReadGrids.Clear();
   fGrids->Delete();
   Modified();                  // emit signal to say something changed
   fGrids->Connect("Modified()", "KVIDGridManager", this, "Modified()");
}

Bool_t KVIDGridManager::ReadAsciiFile(const Char_t* filename)
{
   //read file, create grids corresponding to information in file.
   //note: any existing grids are not destroyed, use Clear() beforehand if you want to
   //start afresh and anew (ais athat aOK?)
   //
   // N.B. the links between each grid and the IDtelescope(s) for which it is to be used
   //      are not set up by this method.
   //
   //the list of grids created by reading the file can be accessed with method
   //GetLastReadGrids() after calling this method

   // clear list of read grids
   fLastReadGrids.Clear();

   Bool_t is_it_ok = kFALSE;
   ifstream gridfile(filename);
   if (!gridfile.good()) {
      Error("ReadAsciiFile", "File %s cannot be opened", filename);
      return is_it_ok;
   }

   KVString s;

   fGrids->Disconnect("Modified()", this, "Modified()");
   while (gridfile.good()) {
      //read a line
      s.ReadLine(gridfile);
      if (s.BeginsWith("++")) {
         //New grid
         //Get name of class by stripping off the '+' at the start of the line
         s.Remove(0, 2);
         /************ BACKWARDS COMPATIBILITY FIX *************
           Old grid files may contain obsolete KVIDZGrid class
           We replace by KVIDZAGrid with SetOnlyZId(kTRUE)
         */
         //Make new grid using this class
         KVIDGraph* grid = 0;
         Bool_t onlyz = kFALSE;
         if (s == "KVIDZGrid") {
            s = "KVIDZAGrid";
            onlyz = kTRUE;
         }
         TClass* clas = TClass::GetClass(s.Data());
         if (!clas) {
            Fatal("ReadAsciiFile",
                  "Cannot load TClass information for %s", s.Data());
         }
         grid = (KVIDGraph*) clas->New();
         fLastReadGrids.Add(grid);
         //read grid
         grid->ReadFromAsciiFile(gridfile);
         if (onlyz) grid->SetOnlyZId(kTRUE);
      }
   }

   gridfile.close();
   is_it_ok = kTRUE;
   Modified();                  // emit signal to say something changed
   fGrids->Connect("Modified()", "KVIDGridManager", this, "Modified()");
   return is_it_ok;
}

Int_t KVIDGridManager::WriteAsciiFile(const Char_t* filename, const TCollection* selection)
{
   // Write grids in file 'filename'.
   // If selection=0 (default), write all grids.
   // If selection!=0, write only grids in list.
   // Returns number of grids written in file.

   ofstream gridfile(filename);

   const TCollection* list_of_grids = (selection ? selection : fGrids);
   TIter next(list_of_grids);
   KVIDGraph* grid = 0;
   Int_t n_saved = 0;
   while ((grid = (KVIDGraph*) next())) {

      grid->WriteToAsciiFile(gridfile);
      Info("WriteAsciiFile", "%s saved", grid->GetName());
      n_saved++;

   }

   gridfile.close();
   return n_saved;
}

KVIDGraph* KVIDGridManager::GetGrid(const Char_t* name)
{
   //Return pointer to grid with name "name"
   return (KVIDGraph*) GetGrids()->FindObjectByName(name);
}

void KVIDGridManager::StartViewer() const
{
   //Opens GUI for managing grids
   if (gROOT->IsBatch()) {
      Warning("StartViewer", "To launch graphical interface, you should not use ROOT in batch mode");
      return;
   }
   TClass* cl = TClass::GetClass("KVIDGridManagerGUI");
   cl->New();
}

void KVIDGridManager::GetListOfIDTelescopeLabels(KVString& list)
{
   // Replace contents of KVString with a comma-separated list of all
   // different labels of ID telescopes associated with current list of ID grids.

   list = "";
   TIter next(fGrids);
   KVIDGraph* grid = 0;
   KVString lab;
   while ((grid = (KVIDGraph*) next())) {
      //cout << "grid=" << grid->GetName() << " label=" << grid->GetIDTelescopeLabel() << endl;
      lab.Form("/%s/", grid->GetIDTelescopeLabel());
      if (!list.Contains(lab)) list.Append(lab);
   }
   //cout << "list=" << list << endl;
   list.ReplaceAll("//", ",");
   list.ReplaceAll("/", "");
   if (list.EqualTo(",")) list = "";
}

void KVIDGridManager::Initialize(Option_t*)
{
   // Initialize all grids in ID grid manager's list, i.e. we call the Initialize() method
   // of every grid/graph.
   TIter next(fGrids);
   KVIDGraph* gr = 0;
   while ((gr = (KVIDGraph*) next())) gr->Initialize();
}

