/***************************************************************************
                          KVTGIDZA.cpp  -  description
                             -------------------
    begin                : 5 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGIDZA.cpp,v 1.12 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/
#include "KVTGIDZA.h"
#include "KVIDZAGrid.h"
#include "KVIDZALine.h"
#include "KVNucleus.h"

ClassImp(KVTGIDZA)

KVTGIDZA::KVTGIDZA(const Char_t* name,
                   const Char_t* function,
                   Double_t xmin, Double_t xmax, Int_t npar,
                   Int_t x_par, Int_t y_par)
   : KVTGIDZ(name, function, xmin, xmax, npar, x_par, y_par)
{
   //Create A identification with given name,
   //using a KVTGIDFunctions namespace function (e.g. "tassangot_A"),
   //for 'x' values from xmin to xmax, npar parameters, and defining
   //the parameter indices corresponding to 'x' and 'y' coordinates.

   fZorA = 0;
}

//___________________________________________________________________________//

KVTGIDZA::KVTGIDZA(const Char_t* name, Int_t npar, Int_t type, Int_t light)
   : KVTGIDZ(name, npar, type, light, -1)
{
   // Create A identification with given "name", using the generalised
   // Tassan-Got functional KVTGIDFunctions::fede.
   // npar = total number of parameters
   // type = functional type (0: standard, 1:extended)
   // light = with (1) or without (0) CsI light-energy dependence

   fZorA = 0;
   SetParameter(2, fZorA);
   SetParName(3, "Z");
}

//___________________________________________________________________________//

void KVTGIDZA::SetIdent(KVIDLine* line, Double_t Id)
{
   //Set atomic number Z and mass of identification line (KVIDZALine)
   //Id should be given as 100*Z + A
   // i.e. proton Id = 101
   // alpha Id = 204
   // lithium-11 Id = 311

   //decode Z and A
   Int_t za = (Int_t) Id;
   Int_t z = za / 100;
   Int_t a = za % 100;
   line->SetZ(z);
   line->SetA(a);
}

//___________________________________________________________________________//

KVIDLine* KVTGIDZA::AddLine(KVIDGrid* g)
{
   //Add a new KVIDZALine to the grid
   return (KVIDLine*)g->Add("ID", "KVIDZALine");
}

//_______________________________________________________________________________________//

void KVTGIDZA::AddLineToGrid(KVIDGrid* g, Int_t Z, Int_t npoints,
                             Double_t xmin, Double_t xmax, Bool_t log_scale)
{
   //Add a line to the grid 'g' for identification label 'Z' with 'npoints' points calculated between
   //X-coordinates xmin and xmax. Points are omitted if the resulting value of the functional is
   //not a number (TMath::IsNan = kTRUE).
   //
   //For Z<=8 several KVIDZALines are added for common isotopes (p,d,t, 3He, etc.)
   //For Z>8 a line for the current Z is drawn, with the A corresponding to Charity's EAL formula
   //
   //Before using this method the user must have set the name of
   //the parameter in the identification functional which corresponds to the
   //atomic number Z i.e. using
   //      SetParName(i,"Z")
   //with i=the relevant parameter index.
   //
   // If log_scale=kTRUE (default is kFALSE), more points are used at the beginning
   // of the line than at the end, with a logarithmic dependence.

   //A & Z identification
   //on va faire: p,d,t,3He,4He,6He,6Li,7Li,8Li,7Be,9Be, etc.
   static Int_t Ziso[] = {
      1, 1, 1,
      2, 2, 2,
      3, 3, 3,
      4, 4, 4,
      5, 5, 5,
      6, 6, 6,
      7, 7, 7,
      8, 8, 8
   };
   static Int_t Aiso[] = {
      1, 2, 3,
      3, 4, 6,
      6, 7, 8,
      7, 9, 10,
      10, 11, 12,
      12, 13, 14,
      14, 15, 16,
      15, 16, 17
   };
   static Int_t nisotopes = 24; //number of isotopes in arrays
   static Int_t Zindex_start[] = { -1, 0, 3, 6, 9, 12, 15, 18, 21 };    //index of first isotope for each Z
   static Int_t Zindex_stop[] = { -1, 2, 5, 8, 11, 14, 17, 20, 23 };    //index of last isotope for each Z

   if (Z <= Ziso[nisotopes - 1]) {
      //Z is included in table of isotopes
      //we add several lines corresponding to the tabulated isotopes
      for (int iso = Zindex_start[Z]; iso <= Zindex_stop[Z]; iso++) {   //loop over isotopes for this Z
         //add new line to grid
         KVIDLine* new_line = AddLine(g);

         //set identification label for line
         Double_t idlab = 100. * Ziso[iso] + Aiso[iso];
         SetIdent(new_line, idlab);

         //set Z of line in functional
         SetParameter("Z", (Double_t) Ziso[iso]);
         //loop over points of line
         Int_t p_index = 0;
         Double_t X, dX;
         Double_t Y = 0.;
         Double_t logXmin = TMath::Log(TMath::Max(xmin, 1.0));
         if (log_scale)
            dX = (TMath::Log(xmax) - logXmin) / (npoints - 1.);
         else
            dX = (xmax - xmin) / (Double_t)(npoints - 1);

         for (Int_t i = 0; i < npoints; i++) {

            //x coordinate of this point
            if (log_scale)
               X = TMath::Exp(logXmin + i * dX);
            else
               X = xmin + dX * ((Double_t) i);

            //leave value Y as it is. The value of GetIDFunc()->Eval(ID)
            //is the vertical distance delta_Y from point (X,Y) to the line; therefore the
            //Y coordinate of the point on the line is Y + delta_Y, whatever the value Y.

            //set values of parameters which correspond to X and Y coordinates in grid
            SetParameter("X", X);
            SetParameter("Y", Y);

            Y += Eval((Double_t) Aiso[iso]);

            if (!TMath::IsNaN(Y))
               new_line->SetPoint(p_index++, X, Y);
            else
               Y = 0.;          //reset Y to 0 if it ever becomes NaN
         }
      }
   }
   else {
      //Z is bigger than largest Z for which we have isotopes
      //we add one line for this Z using mass formula

      //get mass for nucleus
      Int_t A = KVNucleus::GetAFromZ(Z, fMassFormula);
      //add new line to grid
      KVIDLine* new_line = AddLine(g);
      //set identification label for line
      Double_t idlab = 100. * Z + A;
      SetIdent(new_line, idlab);

      //set Z of line in functional
      SetParameter("Z", (Double_t) Z);
      //loop over points of line
      Int_t p_index = 0;
      Double_t Y = 0., X, dX = (xmax - xmin) / (Double_t)(npoints - 1);

      for (Int_t i = 0; i < npoints; i++) {

         //x coordinate of this point
         X = xmin + dX * ((Double_t) i);

         //leave value Y as it is. The value of GetIDFunc()->Eval(ID)
         //is the vertical distance delta_Y from point (X,Y) to the line; therefore the
         //Y coordinate of the point on the line is Y + delta_Y, whatever the value Y.

         //set values of parameters which correspond to X and Y coordinates in grid
         SetParameter("X", X);
         SetParameter("Y", Y);

         Y += Eval((Double_t) A);

         if (!TMath::IsNaN(Y))
            new_line->SetPoint(p_index++, X, Y);
         else
            Y = 0.;             //reset Y to 0 if it ever becomes NaN
      }
   }
}
