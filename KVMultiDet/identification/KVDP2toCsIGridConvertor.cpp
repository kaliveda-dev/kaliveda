/*
$Id: KVDP2toCsIGridConvertor.cpp,v 1.2 2009/03/06 08:59:18 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/06 08:59:18 $
*/

//Created by KVClassFactory on Thu Oct  9 17:23:14 2008
//Author: franklan

#include "KVDP2toCsIGridConvertor.h"
#include "TList.h"
#include "KVIDGrid.h"
#include "KVIDGridManager.h"
#include "KVError.h"

using namespace std;

ClassImp(KVDP2toCsIGridConvertor)

KVDP2toCsIGridConvertor::KVDP2toCsIGridConvertor()
{
   // Default constructor
}

KVDP2toCsIGridConvertor::~KVDP2toCsIGridConvertor()
{
   // Destructor
}

////////////////////////////////////////////////////////////////////////////////

void KVDP2toCsIGridConvertor::Convert(const Char_t* id_and_imf_file, const Char_t* gamma_file)
{
   //Read ID and Gamma-line files for CsI R-L identifications.
   //Initialise grids in KVIDCsI telescopes

   if (fGrids) fGrids->Clear();
   else fGrids = new TList;
   ReadFile(id_and_imf_file);
   // set name of all IMF lines
   TIter nextGrid(fGrids);
   KVIDGrid* grid;
   KVIDentifier* line;
   while ((grid = (KVIDGrid*)nextGrid())) {
      TIter nextOK(grid->GetCuts());
      while ((line = (KVIDentifier*) nextOK())) {
         line->SetName("IMF_line");
      }
   }
   ReadGammaFile(gamma_file);
   if (gIDGridManager) gIDGridManager->GetGrids()->AddAll(fGrids);
   else Error(KV__ERROR(Convert), "gIDGridManager=0x0: create an ID grid manager first!");
}

////////////////////////////////////////////////////////////////////////////////

void KVDP2toCsIGridConvertor::ReadGammaFile(const Char_t* gammafile)
{
   ifstream gamfile;
   gamfile.open(gammafile);

   if (!gamfile.good()) {
      Error(KV__ERROR(ReadGammaFile),
            "Problem reading file %s", gammafile);
      return;
   }
   KVString s;

   s.ReadLine(gamfile);

   while (gamfile.good()) {

      if (s == "") {
         s.ReadLine(gamfile);
         continue;
      }

      if (!s.BeginsWith('#')) { //'#' sign signals comments
         int ring, modu, frun, lrun;
         if (sscanf(s.Data(), "%d %d %d %d", &ring, &modu, &frun, &lrun) !=
               4) {
            Error(KV__ERROR(ReadGammaFile), "Problem reading file %s\nLast line read: %s",
                  gammafile, s.Data());
            return;
         };
         //get grid for this ring,mod
         TString name;
         name.
         Form
         ("CsI R-L Grid First run=%d Last run=%d Ring min=%d Ring max=%d Mod min=%d Mod max=%d",
          frun, lrun, ring, ring, modu, modu);
         KVIDGrid* grid = (KVIDGrid*)fGrids->FindObject(name.Data());
         if (!grid) {
            Error(KV__ERROR(ReadGammaFile), "Can't find grid %s", name.Data());
         }
         int npoints;
         gamfile >> npoints;
         //found gamma line
         KVIDLine* line = 0;
         if (grid) {
            line = grid->NewLine("ok");
            line->SetName("gamma_line");
            line->Set(npoints);
            grid->Add("ok", line);
         }
         //read in points
         for (int i = 0; i < npoints; i++) {
            Double_t x, y;
            gamfile >> x >> y;
            if (grid)
               line->SetPoint(i, x, y);
         }
      }
      s.ReadLine(gamfile);
   }
   gamfile.close();
}
