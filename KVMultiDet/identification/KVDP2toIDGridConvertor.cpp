/*
$Id: KVDP2toIDGridConvertor.cpp,v 1.2 2009/03/06 08:59:18 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/06 08:59:18 $
*/

//Created by KVClassFactory on Thu Oct  9 17:21:55 2008
//Author: franklan

#include "KVDP2toIDGridConvertor.h"
#include "TList.h"
#include "KVIDGrid.h"
#include "KVError.h"
#include "KVIDGridManager.h"

using namespace std;

ClassImp(KVDP2toIDGridConvertor)

KVDP2toIDGridConvertor::KVDP2toIDGridConvertor()
{
   // Default constructor
   fGridClass = 0;
   fGrids = 0;
}

////////////////////////////////////////////////////////////////////////////////

KVDP2toIDGridConvertor::~KVDP2toIDGridConvertor()
{
   // Destructor
   if (fGrids) {
      fGrids->Clear();
      delete fGrids;
   }
}

////////////////////////////////////////////////////////////////////////////////

void KVDP2toIDGridConvertor::SetGridClass(const Char_t* cl)
{
   fGridClass = TClass::GetClass(cl);
   if (!fGridClass) Error(KV__ERROR(SetGridClass), "Unknown grid class %s", cl);
}

////////////////////////////////////////////////////////////////////////////////

void KVDP2toIDGridConvertor::Convert(const Char_t* filename)
{
   if (fGrids) fGrids->Clear();
   else fGrids = new TList;
   ReadFile(filename);
   if (gIDGridManager) gIDGridManager->GetGrids()->AddAll(fGrids);
   else Error(KV__ERROR(Convert), "gIDGridManager=0x0: create an ID grid manager first!");
}

////////////////////////////////////////////////////////////////////////////////

void KVDP2toIDGridConvertor::ReadFile(const Char_t* filename)
{
   // Read grids in file and add to fGrids list

   if (!fGridClass) {
      Error(KV__ERROR(ReadFile),
            "Set class of ID grids to generate first!");
      return;
   }

   ifstream datfile;
   datfile.open(filename);

   if (!datfile.good()) {
      Error(KV__ERROR(ReadFile),
            "Problem reading file %s", filename);
      return;
   }
   KVString s;

   s.ReadLine(datfile);

   while (datfile.good()) {

      if (s == "") {
         s.ReadLine(datfile);
         continue;
      }

      if (!s.BeginsWith('#')) { //'#' sign signals comments
         int ring, modu, frun, lrun, totpoints;
         if (sscanf(s.Data(), "%d %d %d %d", &ring, &modu, &frun, &lrun) !=
               4) {
            Error(KV__ERROR(ReadFile), "Problem reading file %s\nLast line read: %s",
                  filename, s.Data());
            return;
         };
         KVIDGrid* grid = (KVIDGrid*) fGridClass->New(); //create new grid

         //add to list of grids
         fGrids->Add(grid);

         grid->GetParameters()->SetValue("First run", frun);
         grid->GetParameters()->SetValue("Last run", lrun);
         grid->GetParameters()->SetValue("Ring min", ring);
         grid->GetParameters()->SetValue("Ring max", ring);
         grid->GetParameters()->SetValue("Mod min", modu);
         grid->GetParameters()->SetValue("Mod max", modu);
         int nlines;
         datfile >> nlines;
         totpoints = 0;
         //read Z,A and number of points in each line
         for (int i = 0; i < nlines; i++) {
            int z, a, npoints;
            datfile >> z >> a >> npoints;
            totpoints += 2 * npoints;
            if (z > 0) {
               //identification line
               KVIDLine* line = (KVIDLine*)grid->NewLine("id");
               line->SetZ(z);
               line->SetA(a);
               line->Set(npoints);
               grid->Add("id", line);
            }
            else {
               //"ok" line
               KVIDLine* line = (KVIDLine*)grid->NewLine("ok");
               line->Set(npoints);
               grid->Add("ok", line);
            }
         }
         ReadLineCoords(grid, datfile);
         //calculate line widths in grid
         grid->CalculateLineWidths();
      }
      s.ReadLine(datfile);
   }
   datfile.close();
}

void KVDP2toIDGridConvertor::ReadLineCoords(KVIDGrid* grid, ifstream& datfile)
{
   // Read coordinates of lines in the order they were created.
   // We assume that the 'ID' lines are first, then the 'OK' lines

   TIter nextID(grid->GetIdentifiers());
   KVIDentifier* line;
   while ((line = (KVIDentifier*) nextID())) {
      //read in points
      for (int i = 0; i < line->GetN(); i++) {
         Double_t x, y;
         datfile >> x >> y;
         line->SetPoint(i, x, y);
      }
   }
   TIter nextOK(grid->GetCuts());
   while ((line = (KVIDentifier*) nextOK())) {
      //read in points
      for (int i = 0; i < line->GetN(); i++) {
         Double_t x, y;
         datfile >> x >> y;
         line->SetPoint(i, x, y);
      }
   }
}
