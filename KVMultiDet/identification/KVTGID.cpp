/***************************************************************************
                          KVTGID.cpp  -  description
                             -------------------
    begin                : 5 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGID.cpp,v 1.17 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/
#include "KVTGID.h"
#include "KVTGIDGrid.h"
#include "KVTGIDZA.h"
#include "TMath.h"
#include "Riostream.h"
#include "TClass.h"
#include "TString.h"
#include "KVTGIDFunctions.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVTGID)
void KVTGID::init()
{
   //Default intitialisations
   // We remove this object from gROOT->GetListOfFunctions() to stop the TF1
   // constructors from deleting previous KVTGID's with the same name
   fID_min = 1.;
   fID_max = 100.;
   fStatus = -1;
   fLambda = -1;
   fMu = -1;
   fG = -1;
   fPdx = -1;
   fPdy = -1;
   fAlpha = -1;
   fBeta = -1;
   fNu = -1;
   fXi = -1;
   fEta = -1;
   fType = 0;
   fLight = 0;
   fZorA = 0;
   fMassFormula = KVNucleus::kEALMass;
   fTelescopes = "/";
   gROOT->GetListOfFunctions()->Remove(this);
}

//___________________________________________________________________________//

KVTGID::KVTGID(): TF1()
{
   // Default ctor
   init();
}

//___________________________________________________________________________//

KVTGID::KVTGID(const Char_t* name,
               const Char_t* function, Double_t xmin,
               Double_t xmax, Int_t npar, Int_t par_x, Int_t par_y)
   : TF1(name, KVTGIDFunctions::tassangot_Z, xmin, xmax, npar)
{
   //Create TF1 named "name" using "function", range xmin->xmax and number of
   //parameters npar.
   //
   //The given function name must correspond to one of the functions
   //defined in the KVTGIDFunctions namespace (without the "KVTGIDFunctions::"
   //scope, i.e. function="tassangot_Z" is a valid argument for this constructor).
   //The resulting object can be written to and read back from a ROOT file and the
   //function pointer will be reinitialised automatically when the object is read back
   //from the file.
   //
   //par_x and par_y are the indices of the parameters of the functional which
   //correspond to the 'X' and 'Y' coordinates of the identification map, respectively.
   //
   //NOTE: the function pointer passed to the TF1 constructor is just a dummy to
   //make sure that the right constructor is called, and that the TF1 is set up correctly.
   //The actual function used is set using the name given as argument 'function'.

   init();

   //set names of 'X' and 'Y' parameters. these are used by MakeIDGrid.
   SetParName(par_x, "X");
   SetParName(par_y, "Y");

   fTGIDFunctionName.Form("KVTGIDFunctions::%s", function);
   SetFunction((Double_t(*)(Double_t*, Double_t*)) gROOT->
               ProcessLineFast(fTGIDFunctionName.Data()));
}

//___________________________________________________________________________//

KVTGID::KVTGID(const KVTGID& obj) : TF1()
{
   // copy constructor
   init();
   ((KVTGID&)obj).Copy(*this);
}

//___________________________________________________________________________//
void KVTGID::Copy(TObject& tgid) const
{
   // Copy this KVTGID function into the KVTGID object referenced by tgid
   TF1::Copy(tgid);
   ((KVTGID&) tgid).fID_min           = fID_min;
   ((KVTGID&) tgid).fID_max           = fID_max;
   ((KVTGID&) tgid).fTGIDFunctionName = fTGIDFunctionName;
   ((KVTGID&) tgid).fLambda           = fLambda;
   ((KVTGID&) tgid).fMu               = fMu;
   ((KVTGID&) tgid).fG                = fG;
   ((KVTGID&) tgid).fPdx              = fPdx;
   ((KVTGID&) tgid).fPdy              = fPdy;
   ((KVTGID&) tgid).fAlpha            = fAlpha;
   ((KVTGID&) tgid).fBeta             = fBeta;
   ((KVTGID&) tgid).fNu               = fNu;
   ((KVTGID&) tgid).fXi               = fXi;
   ((KVTGID&) tgid).fEta              = fEta;
   ((KVTGID&) tgid).fType             = fType;
   ((KVTGID&) tgid).fLight            = fLight;
   ((KVTGID&) tgid).fZorA             = fZorA;
   ((KVTGID&) tgid).fMassFormula      = fMassFormula;
   ((KVTGID&) tgid).fRuns             = fRuns;
   ((KVTGID&) tgid).fVarX             = fVarX;
   ((KVTGID&) tgid).fVarY             = fVarY;
   ((KVTGID&) tgid).fTelescopes       = fTelescopes;
}

//___________________________________________________________________________//

Double_t KVTGID::GetIdentification(Double_t ID_min, Double_t ID_max,
                                   Double_t& ID_quality, Double_t* par)
{
   //Use the functional with the current parameter values in order to perform
   //an identification.
   //The value returned is the estimated identification value.
   //If the identification is not possible, -1 is returned
   //(ID_quality=-1 also).
   //
   //For status code use GetStatus().
   //
   //ID_min and ID_max are the lower & upper limits for the identification.
   //The functional must change sign between these two limits in order for
   //the identification to be possible.
   //ID_quality is the (absolute) value of the functional corresponding to
   //the estimated identification: it is the distance from the identified
   //point to the nearest identification line. Ideally it should be zero or
   //as small as possible.
   //
   //Optional argument "par" allows to replace the current parameters.

   if (par)
      SetParameters(par);

   Double_t ID;
   ID = ID_quality = -1.;
   fStatus = kStatus_NotBetween_IDMin_IDMax;

   if (Eval(ID_min) * Eval(ID_max) < 0.) {
      //if the sign of the identification function changes between ID_min
      //and ID_max i.e. if the ID_min line is below the point and ID_max above
      //it this means that the point to identify is in between these two lines
      ID = GetX(0., ID_min, ID_max);
      //what is actual value of Tassan-Got formula for this ID ?
      ID_quality = TMath::Abs(Eval(ID));
      fStatus = kStatus_OK;
   }

   return ID;
}

//_______________________________________________________________________________________//

void KVTGID::AddLineToGrid(KVIDGrid* g, Int_t ID, Int_t npoints,
                           Double_t xmin, Double_t xmax, Bool_t log_scale)
{
   // Add a line to the grid 'g' for identification label 'ID' with 'npoints' points calculated between
   // X-coordinates xmin and xmax. Points are omitted if the resulting value of the functional is
   // not a number (TMath::IsNan = kTRUE).
   //
   // If log_scale=kTRUE (default is kFALSE), more points are used at the beginning
   // of the line than at the end, with a logarithmic dependence.

   //add new line to grid
   KVIDLine* new_line = AddLine(g);

   //set identification label for line
   SetIdent(new_line, ID);
   //set mass formula for line
   new_line->SetMassFormula(fMassFormula);

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

      //set x coordinate of this point
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

      Y += Eval((Double_t) ID);

      if (!TMath::IsNaN(Y))
         new_line->SetPoint(p_index++, X, Y);
      else
         Y = 0.;                //reset Y to 0 if it ever becomes NaN
   }
}

//_______________________________________________________________________________________//

void KVTGID::Print(Option_t* option) const
{
   //Print info on functional and grid
   cout << Class()->
        GetName() <<
        " object for identification using Tassan-Got functional" << endl;
   cout << "Limits for fit : fID_min = " << GetIDmin() << "  fID_max = " <<
        GetIDmax() << endl;
   TF1::Print(option);
}

//_______________________________________________________________________________________//

Int_t KVTGID::Compare(const TObject* obj) const
{
   //Used to sort list of KVTGID in KVTGIDManager
   //ID obj with smallest IDmax will be first in sorted list
   Int_t id1 = (Int_t) GetIDmax();
   if (id1 < 0)
      return 0;
   Int_t id2 = (Int_t) static_cast < const KVTGID* >(obj)->GetIDmax();
   if (id2 < 0)
      return 0;
   if (id1 > id2)
      return 1;
   if (id1 < id2)
      return -1;
   return 0;
}

//_______________________________________________________________________________________//

const Char_t* KVTGID::GetStatusString() const
{
   //Returns explanatory message for value of GetStatus()

   static TString messages[] = {
      "ok",
      "point to identify outside of identification range of function"
   };
   Int_t status = GetStatus() + 1;
   if (status--)
      return messages[status];
   return Form("no call to GetIdentification() performed yet");
}

//_______________________________________________________________________________________//

Double_t KVTGID::GetDistanceToLine(Double_t x, Double_t y, Int_t id,
                                   Double_t* params)
{
   //Given a point (x,y) (which could, for example, be a point in a KVIDGrid line which we want to fit)
   //we give the (vertical) distance to the functional identification line 'id'. If the point is below the line
   //the distance is positive, if the point is above the line it is negative.
   //If the parameter array 'params' is not given, we use the current values of the parameters.

   if (params)
      SetParameters(params);
   SetParameter("X", x);
   SetParameter("Y", y);
   return Eval((Double_t) id);
}

//______________________________________________________________________________

void KVTGID::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVTGID.
   // If the name of the KVTGIDFunction has been set (version > 1) then
   // we use it to reset the function pointer

   if (R__b.IsReading()) {
      KVTGID::Class()->ReadBuffer(R__b, this);
      if (fTGIDFunctionName != "") {
         SetFunction((Double_t(*)(Double_t*, Double_t*)) gROOT->
                     ProcessLineFast(fTGIDFunctionName.Data()));
      }
   }
   else {
      KVTGID::Class()->WriteBuffer(R__b, this);
   }
}

//______________________________________________________________________________

KVTGID* KVTGID::MakeTGID(const Char_t* name, Int_t type, Int_t light, Int_t ZorA, Int_t mass)
{
   // Static function used to create TGID objects
   //
   // type   ----->   type of functional
   //  *  type : =0->basic functional       <>0->extended functional
   //  *      * For the basic formula :
   //  *        yy = ((g*E)**(mu+1)+lambda**(mu+1)*Z**2*A**mu)**(1/(mu+1))-g*E + pdy
   //  *      * For the extended formula :
   //  *        yy = ((g*E)**(mu+nu+1)+(lambda*Z**alpha*A**beta)**(mu+nu+1)+
   //  *                 xi*A**mu*(g*E)**nu)**(1/(mu+mu+1))-g*E + pdy
   //
   // light  ----->   treatment of CsI total light output
   //  * light  :  =0->no non-linear light response    <>0->non-linear light response included
   //  *      *  If ih=0  no non-linear light response : E=xx-pdx
   //  *      *  If ih<>0 non-linear light response included :
   //  *          E = sqrt(h**2+2*rho*h*(1+log(1+h/rho)))
   //  *         rho=eta*Z**2*A    and   h=xx-pdx
   //
   // ZorA   ----->   functional used to find Z or A
   //  *    ZorA :  =0->A    <>0->Z

   Int_t npar = 6 + GetNumberOfLTGParameters(type, light);
   KVTGID* _tgid_ = 0;
   if (ZorA) {
      _tgid_ = new KVTGIDZ(name, npar, type, light, mass);
   }
   else {
      _tgid_ = new KVTGIDZA(name, npar, type, light);
   }
   npar = 6;
   _tgid_->fLambda = npar++;
   if (type) {
      _tgid_->fAlpha = npar++;
      _tgid_->fBeta = npar++;
      _tgid_->fMu = npar++;
      _tgid_->fNu = npar++;
      _tgid_->fXi = npar++;
   }
   else {
      _tgid_->fMu = npar++;
   }
   _tgid_->fG = npar++;
   _tgid_->fPdx = npar++;
   _tgid_->fPdy = npar++;
   if (light) _tgid_->fEta = npar++;
   return _tgid_;
}

//___________________________________________________________________________________________

void KVTGID::SetLTGParameters(Double_t* par)
{
   // 'par' is an array containing the LTG functional parameters,
   // whose number and order depends on fType & fLight:
   //
   // fType=0, fLight=0:            Double_t par[5];
   // ==================
   //  par[0] = lambda
   //  par[1] = mu
   //  par[2] = g
   //  par[3] = pdx
   //  par[4] = pdy
   //
   // fType=0, fLight<>0:            Double_t par[6];
   // ==================
   //  par[0] = lambda
   //  par[1] = mu
   //  par[2] = g
   //  par[3] = pdx
   //  par[4] = pdy
   //  par[5] = eta
   //
   // fType<>0, fLight=0:            Double_t par[9];
   // ==================
   //  par[0] = lambda
   //  par[1] = alpha
   //  par[2] = beta
   //  par[3] = mu
   //  par[4] = nu
   //  par[5] = xi
   //  par[6] = g
   //  par[7] = pdx
   //  par[8] = pdy
   //
   // fType<>0, fLight<>0:            Double_t par[10];
   // ==================
   //  par[0] = lambda
   //  par[1] = alpha
   //  par[2] = beta
   //  par[3] = mu
   //  par[4] = nu
   //  par[5] = xi
   //  par[6] = g
   //  par[7] = pdx
   //  par[8] = pdy
   //  par[9] = eta

   Int_t npar = GetNumberOfLTGParameters(fType, fLight);
   for (Int_t ipar = 0; ipar < npar; ipar++) {
      SetParameter(6 + ipar, par[ipar]);
   }
}


//___________________________________________________________________________________________

void KVTGID::SetLTGParameters(Float_t* par)
{
   // 'par' is an array containing the LTG functional parameters,
   // whose number and order depends on fType & fLight:
   //
   // fType=0, fLight=0:            Double_t par[5];
   // ==================
   //  par[0] = lambda
   //  par[1] = mu
   //  par[2] = g
   //  par[3] = pdx
   //  par[4] = pdy
   //
   // fType=0, fLight<>0:            Double_t par[6];
   // ==================
   //  par[0] = lambda
   //  par[1] = mu
   //  par[2] = g
   //  par[3] = pdx
   //  par[4] = pdy
   //  par[5] = eta
   //
   // fType<>0, fLight=0:            Double_t par[9];
   // ==================
   //  par[0] = lambda
   //  par[1] = alpha
   //  par[2] = beta
   //  par[3] = mu
   //  par[4] = nu
   //  par[5] = xi
   //  par[6] = g
   //  par[7] = pdx
   //  par[8] = pdy
   //
   // fType<>0, fLight<>0:            Double_t par[10];
   // ==================
   //  par[0] = lambda
   //  par[1] = alpha
   //  par[2] = beta
   //  par[3] = mu
   //  par[4] = nu
   //  par[5] = xi
   //  par[6] = g
   //  par[7] = pdx
   //  par[8] = pdy
   //  par[9] = eta

   Int_t npar = GetNumberOfLTGParameters(fType, fLight);
   for (Int_t ipar = 0; ipar < npar; ipar++) {
      SetParameter(6 + ipar, (Double_t)par[ipar]);
   }
}

////////////////////////////////////////////////////////////////////////////////

Int_t KVTGID::GetNumberOfLTGParameters(Int_t type, Int_t light)
{
   // Static function returning number of parameters used by Tassan-Got functional.
   // Depends on type of functional (type) and whether non-linear light response
   // is calculated or not (light).
   // According to the (type, light) combination the numbers and orders
   // of parameters are :
   //      type=0  light=0   5 parameters: lambda, mu, g, pdx, pdy
   //      type=0  light<>0  6 parameters: lambda, mu, g, pdx, pdy, eta
   //      type<>0 light=0   9 parameters: lambda, alpha, beta, mu, nu,
   //                                 xi, g, pdx, pdy
   //      type<>0 light<>0 10 parameters: lambda, alpha, beta, mu, nu,
   //                                 xi, g, pdx, pdy, eta}

   return type ? (light ? 10 : 9) : (light ?  6 : 5);
}

////////////////////////////////////////////////////////////////////////////////

void KVTGID::SetLTGParameterNames()
{
   if (fLambda > -1) SetParName(fLambda, "#lambda");
   if (fAlpha > -1) SetParName(fAlpha, "#alpha");
   if (fBeta > -1) SetParName(fBeta, "#beta");
   if (fMu > -1) SetParName(fMu, "#mu");
   if (fNu > -1) SetParName(fNu, "#nu");
   if (fXi > -1) SetParName(fXi, "#xi");
   if (fG > -1) SetParName(fG, "g");
   if (fPdx > -1) SetParName(fPdx, "pdx");
   if (fPdy > -1) SetParName(fPdy, "pdy");
   if (fEta > -1) SetParName(fEta, "#eta");
}

void KVTGID::SetIDTelescopes(const TCollection* list)
{
   // set list of ID telescopes for which fit is valid.
   // any previous list is deleted.
   ClearIDTelescopes();
   TIter next(list);
   KVBase* t;
   while ((t = (KVBase*)next())) AddIDTelescope(t);
};

void KVTGID::WriteToAsciiFile(ofstream& gridfile) const
{
   // Write parameters of LTG fit in file.

   gridfile << "++KVTGID::" << GetName() << endl;
   gridfile << "Type=" << GetFunctionalType() << endl;
   gridfile << "LightEnergyDependence=" << GetLightEnergyDependence() << endl;
   gridfile << "ZorA=" << GetZorA() << endl;
   if (GetZorA() == 1) {
      gridfile << "MassFormula=" << GetMassFormula() << endl;
   }
   gridfile << "Functional=" << GetFunctionName() << endl;
   gridfile << "Runs=" << ((KVNumberList&)GetValidRuns()).AsString() << endl;
   gridfile << "IDTelescopes=" << fTelescopes.Data() << endl;
   gridfile << "<VARX> " << GetVarX() << endl;
   gridfile << "<VARY> " << GetVarY() << endl;
   gridfile << "ZMIN=" << (Int_t)GetIDmin();
   gridfile << "  ZMAX=" << (Int_t)GetIDmax() << endl;
   Bool_t type1 = (GetFunctionalType() == 1);
   gridfile << Form("Lambda=%20.13e", GetLambda()) << endl;
   if (type1) {
      gridfile << Form("Alpha=%20.13e", GetAlpha()) << endl;
      gridfile << Form("Beta=%20.13e", GetBeta()) << endl;
   }
   gridfile << Form("Mu=%20.13e", GetMu()) << endl;
   if (type1) {
      gridfile << Form("Nu=%20.13e", GetNu()) << endl;
      gridfile << Form("Xi=%20.13e", GetXi()) << endl;
   }
   gridfile << Form("G=%20.13e", GetG()) << endl;
   gridfile << Form("Pdx=%20.13e", GetPdx()) << endl;
   gridfile << Form("Pdy=%20.13e", GetPdy()) << endl;
   if (GetLightEnergyDependence() == 1) {
      gridfile << Form("Eta=%20.13e", GetEta()) << endl;
   }
   gridfile << "!" << endl << endl;
}

KVString  GetValue(KVString& l, char c)
{
   l.Begin(c);
   l.Next();
   return l.Next(kTRUE);
}

KVTGID* KVTGID::ReadFromAsciiFile(const Char_t* name, ifstream& gridfile)
{
   // Read parameters of LTG fit in file (which must have been written with
   // current version of KVTGID::WriteToAsciiFile)

   KVTGID* LTGfit = 0;
   Int_t Ftyp, Flite, FZorA, Fmass;
   FZorA = Fmass = Ftyp = Flite = 0;

   KVString line;
   line.ReadLine(gridfile);
   TString buff = GetValue(line, '=');
   Ftyp = buff.Atoi();
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   Flite = buff.Atoi();
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   FZorA = buff.Atoi();
   if (FZorA) {
      line.ReadLine(gridfile);
      buff = GetValue(line, '=');
      Fmass = buff.Atoi();
   }

   LTGfit = MakeTGID(name, Ftyp, Flite, FZorA, Fmass);

   line.ReadLine(gridfile); // skip "Functional=" line : name of functional not used
   line.ReadLine(gridfile);
   LTGfit->SetValidRuns(KVNumberList(GetValue(line, '=').Data()));
   line.ReadLine(gridfile);
   LTGfit->SetStringTelescopes(GetValue(line, '='));
   line.ReadLine(gridfile);
   line.Remove(0, 6);
   line.Remove(KVString::kBoth, ' ');
   LTGfit->SetVarX(line.Data());
   line.ReadLine(gridfile);
   line.Remove(0, 6);
   line.Remove(KVString::kBoth, ' ');
   LTGfit->SetVarY(line.Data());
   line.ReadLine(gridfile);
   Int_t zmin, zmax;
   sscanf(line.Data(), "ZMIN=%d ZMAX=%d", &zmin, &zmax);
   LTGfit->SetIDmin(zmin);
   LTGfit->SetIDmax(zmax);
   Bool_t type1 = (Ftyp == 1);
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   LTGfit->SetLambda(buff.Atof());
   if (type1) {
      line.ReadLine(gridfile);
      buff = GetValue(line, '=');
      LTGfit->SetAlpha(buff.Atof());
      line.ReadLine(gridfile);
      buff = GetValue(line, '=');
      LTGfit->SetBeta(buff.Atof());
   }
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   LTGfit->SetMu(buff.Atof());
   if (type1) {
      line.ReadLine(gridfile);
      buff = GetValue(line, '=');
      LTGfit->SetNu(buff.Atof());
      line.ReadLine(gridfile);
      buff = GetValue(line, '=');
      LTGfit->SetXi(buff.Atof());
   }
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   LTGfit->SetG(buff.Atof());
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   LTGfit->SetPdx(buff.Atof());
   line.ReadLine(gridfile);
   buff = GetValue(line, '=');
   LTGfit->SetPdy(buff.Atof());
   if (Flite == 1) {
      line.ReadLine(gridfile);
      buff = GetValue(line, '=');
      LTGfit->SetEta(buff.Atof());
   }
   line.ReadLine(gridfile);
   return LTGfit;
}


