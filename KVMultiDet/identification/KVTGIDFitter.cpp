/*
$Id: KVTGIDFitter.cpp,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Apr 21 09:26:24 2008
//Author: franklan

#include "KVTGIDFitter.h"
#include "fit_ede.h"
#include "KVIDZAGrid.h"
#include "KVIDGridManager.h"
#include "KVTGIDGrid.h"

using namespace std;

ClassImp(KVTGIDFitter)


KVTGIDFitter::KVTGIDFitter()
{
   // Default constructor
   fLight = fType = fZorA = 0;
   fPar = zd = ad = xd = yd = 0;
   istate = 0;
   fTGID = 0;
   fGrid = 0;
   fXmin = fXmax = 0.0;
}

////////////////////////////////////////////////////////////////////////////////

KVTGIDFitter::~KVTGIDFitter()
{
   // Destructor
   if (fPar) delete [] fPar;
   if (istate) delete [] istate;
   if (zd) delete [] zd;
   if (ad) delete [] ad;
   if (xd) delete [] xd;
   if (yd) delete [] yd;
}

////////////////////////////////////////////////////////////////////////////////

void KVTGIDFitter::Fit(KVIDGraph* theGrid)
{
   // Fit the grid using the functional chosen with SetType and SetLight.
   // Status of fit after this call can be retrieved with GetFitStatus().

   // must inherit from KVIDZAGrid!
   if (!theGrid->InheritsFrom(KVIDZAGrid::Class())) {
      Error("Fit(KVIDGraph*)",
            "Can only fit graphs inheriting from KVIDZAGrid");
      fGrid = 0;
      return;
   }
   fGrid = theGrid;
   fZorA = (Int_t)(!theGrid->HasMassIDCapability());
   // prepare new parameter array and status array
   if (fPar) delete [] fPar;
   Int_t npar = KVTGID::GetNumberOfLTGParameters(fType, fLight);
   fPar = new Float_t[npar];
   if (istate) delete [] istate;
   istate = new Int_t[npar];

   //initialise grid - this should sort lines in order of Z & A
   theGrid->Initialize();

   //limit points according to abscissa ?
   Bool_t with_xlimits = (fXmax > fXmin);

   // calculate total number of points to fit
   Int_t npts = 0;
   TIter next_id(theGrid->GetIdentifiers());
   KVIDentifier* id;
   while ((id = (KVIDentifier*)next_id())) {
      if (!with_xlimits) {
         npts += id->GetN(); // count all points
      }
      else {
         Double_t x, y;
         for (int i = 0; i < id->GetN(); i++) {
            id->GetPoint(i, x, y);
            if (x >= fXmin && x <= fXmax) ++npts; // only points between [fXmin,fXmax]
         }
      }
   }
   cout << "Points to fit = " << npts << endl;
   if (npts < 2) {
      Error("Fit(KVIDGraph*)",
            "Too few points for fit");
      irc = -2;
      return;
   }

   // prepare and fill arrays with Z, A, X & Y
   if (zd) delete [] zd;
   if (ad) delete [] ad;
   if (xd) delete [] xd;
   if (yd) delete [] yd;
   zd = new Float_t[npts];
   ad = new Float_t[npts];
   xd = new Float_t[npts];
   yd = new Float_t[npts];
   next_id.Reset();
   npts = 0;
   Double_t x, y;
   while ((id = (KVIDentifier*)next_id())) {
      for (int i = 0; i < id->GetN(); i++) {
         id->GetPoint(i, x, y);
         if ((!with_xlimits) || (x >= fXmin && x <= fXmax)) {
            xd[npts] = (Float_t)x;
            yd[npts] = (Float_t)y;
            zd[npts] = id->GetZ();
            ad[npts] = id->GetA();
            ++npts;
         }
      }
   }

   /*** FIT FONCTIONNELLE ***/
   irc = globede_c(npts, zd, ad, xd, yd, fType, fLight, fPar, istate);

   //generate TGID corresponding to fit
   MakeTGID();
}

////////////////////////////////////////////////////////////////////////////////

Int_t KVTGIDFitter::GetFitStatus() const
{
   // Return status code of last fit.
   //  0 -> convergence reached
   //  1 -> convergence reached, but not well marked minimum
   //  2 -> too many iterations, convergence not reached
   // -1 -> no identification line with at least 2 points
   // -2 -> too few data points
   // -3 -> addressing problem between Fortran and C
   return irc;
}

const Char_t* KVTGIDFitter::GetFitStatusString() const
{
   // String with meaning of fit status codes (see GetFitStatus)

   static TString message[] = {
      "addressing problem between Fortran and C",
      "too few data points",
      "no identification line with at least 2 points",
      "convergence reached",
      "convergence reached, but not well marked minimum",
      "too many iterations, convergence not reached"
   };
   return message[irc + 3].Data();
}

////////////////////////////////////////////////////////////////////////////////

Int_t* KVTGIDFitter::GetStatusOfParameters() const
{
   // Returns array containing status of each parameter:
   //  =0 -> free parameter
   //  =1 -> parameter constrained by the lower limit
   //  =2 -> parameter constrained by the upper limit
   //  =3 -> constant parameter (bl(i)=bu(i)=par(i))
   return istate;
}

////////////////////////////////////////////////////////////////////////////////

void KVTGIDFitter::MakeTGID()
{
   // Make a KVTGID out of fit result, if fit converged (irc<2)

   if (irc < 0 || irc > 1) return;
   if (!fGrid) return;

   fTGID = KVTGID::MakeTGID(Form("%s_fit", fGrid->GetName()), fType, fLight, fZorA,
                            fGrid->GetMassFormula());
   fTGID->SetLTGParameters(fPar);
   fTGID->SetIDmin(((KVIDentifier*)fGrid->GetIdentifiers()->First())->GetZ());
   fTGID->SetIDmax(((KVIDentifier*)fGrid->GetIdentifiers()->Last())->GetZ());
   fTGID->SetValidRuns(fGrid->GetRuns());
   fTGID->SetVarX(fGrid->GetVarX());
   fTGID->SetVarY(fGrid->GetVarY());
   fTGID->SetIDTelescopes(fGrid->GetIDTelescopes());
}

////////////////////////////////////////////////////////////////////////////////

void KVTGIDFitter::FitPanel(Int_t functional_type, Bool_t with_csi_light_energy,
                            Int_t first_Z, Int_t last_Z, Double_t xmin, Double_t xmax)
{
   // GUI method used to fit grid previously set with SetGrid(KVIDGraph*).
   //  functional_type = 0 (standard) or 1 (extended functional)
   //  with_csi_light_energy = kTRUE (with) or kFALSE (without CsI light-energy relation for 'X')
   //  first_Z, last_Z: set minimum & maximum Z for which fit is valid in KVTGID object
   //                   resulting from fit. (default: -1, take first and last Z of grid)
   //  xmin, xmax: only fit points with abscissa between limits [xmin,xmax]
   //                     (default: use all points regardless of abscissa)

   if (!fGrid) {
      Error("FitPanel", "Set grid to fit with SetGrid(KVIDGraph*)");
      return;
   }
   SetFunctionalType(functional_type);
   SetLight((Int_t)with_csi_light_energy);
   fXmin = xmin;
   fXmax = xmax;
   Fit(fGrid);
   if (irc > 1 || irc < 0) {
      Warning("FitPanel", "Fit failed. Status of fit = %d", irc);
      return;
   }
   if (!fTGID) {
      Error("FitPanel", "Fit failed : %s", GetFitStatusString());
      return;
   }

   if (first_Z != -1) fTGID->SetIDmin(first_Z);
   if (last_Z != -1) fTGID->SetIDmax(last_Z);

   if (fGrid->GetXmin() == fGrid->GetXmax()) fGrid->FindAxisLimits();
   // generate grid representing fit
   KVTGIDGrid* fitgr = new KVTGIDGrid(fTGID, (KVIDZAGrid*)fGrid);
   // make fitted grid 'onlyzid' if parent grid was
   fitgr->SetOnlyZId(!fGrid->HasMassIDCapability());
   if (!fGrid->HasMassIDCapability()) fitgr->SetMassFormula(fGrid->GetMassFormula());
   fitgr->Generate(fGrid->GetXmax(), fGrid->GetXmin());
   gIDGridManager->Modified();

   if (fPad) {
      // draw fitted grid in same pad as original
      fPad->cd();
      fitgr->Draw();
      fPad->Modified();
      fPad->Update();
   }
}
