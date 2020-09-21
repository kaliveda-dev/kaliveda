/***************************************************************************
                          KVTGIDZ.cpp  -  description
                             -------------------
    begin                : 5 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGIDZ.cpp,v 1.8 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/
#include "KVTGIDZ.h"
#include "KVIDZAGrid.h"
#include "KVIDZALine.h"

using namespace std;

ClassImp(KVTGIDZ)

KVTGIDZ::KVTGIDZ(const Char_t* name,
                 const Char_t* function,
                 Double_t xmin, Double_t xmax, Int_t npar, Int_t x_par,
                 Int_t y_par)
   : KVTGID(name, function, xmin, xmax, npar, x_par, y_par)
{
   //Create Z identification with given name,
   //using a KVTGIDFunctions namespace function (e.g. "tassangot_Z"),
   //for 'x' values from xmin to xmax, npar parameters, and defining
   //the parameter indices corresponding to 'x' and 'y' coordinates.

   fZorA = 1; //only Z identification
}

//___________________________________________________________________________//

KVTGIDZ::KVTGIDZ(const Char_t* name, Int_t npar, Int_t type, Int_t light, Int_t mass)
   : KVTGID(name, "fede", 0.1, 150., npar, 4, 5)
{
   // Create Z identification with given "name", using the generalised
   // Tassan-Got functional KVTGIDFunctions::fede.
   // npar = total number of parameters
   // type = functional type (0: standard, 1:extended)
   // light = with (1) or without (0) CsI light-energy dependence
   // mass = mass formula used to calculate A for each Z (see KVNucleus::GetAFromZ)

   fType = type;
   fLight = light;
   fZorA = 1; //only Z identification
   fMassFormula = mass;
   SetParameter(0, type);
   SetParameter(1, light);
   SetParameter(2, fZorA);
   SetParameter(3, mass);
   Int_t np = 6;
   fLambda = np++;
   if (type) {
      fAlpha = np++;
      fBeta = np++;
      fMu = np++;
      fNu = np++;
      fXi = np++;
   }
   else {
      fMu = np++;
   }
   fG = np++;
   fPdx = np++;
   fPdy = np++;
   if (light) fEta = np;
   SetLTGParameterNames();
}
//___________________________________________________________________________//

KVTGIDZ::KVTGIDZ(const KVTGID& obj)
{
   // Copy constructor. Can be used with any daughter class of KVTGID.
   // The parameter ZorA is set to 1 (i.e. only Z idenfification) and
   // the Parameter 3 is the mass formula.
   init();
   ((KVTGID&)obj).Copy(*this);

   // Case where obj is a KVTGIDZA
   if (!fZorA) {
      fZorA = 1; //only Z identification
      SetParameter(2, fZorA);
      SetParameter(3, fMassFormula);
      SetParName(3, "MassFormula");
   }
}
//___________________________________________________________________________//

void KVTGIDZ::SetIdent(KVIDLine* line, Double_t Z)
{
   //Set atomic number Z of identification line
   dynamic_cast < KVIDZALine* >(line)->SetZ((Int_t) Z);
}

//___________________________________________________________________________//

KVIDLine* KVTGIDZ::AddLine(KVIDGrid* g)
{
   //Add a new KVIDZALine to the grid
   return (KVIDLine*)g->Add("ID", "KVIDZALine");
}
