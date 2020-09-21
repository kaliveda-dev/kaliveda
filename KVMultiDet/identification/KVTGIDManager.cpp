/***************************************************************************
                          KVTGIDManager.cpp  -  description
                             -------------------
    begin                : 21 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGIDManager.cpp,v 1.12 2008/04/04 09:06:25 franklan Exp $
***************************************************************************/

#include "KVTGIDManager.h"
#include "KVTGIDGrid.h"

ClassImp(KVTGIDManager)

KVTGIDManager::KVTGIDManager()
{
   //Default constructor.
   fID_max = 0.;
   fStatus = -1;
}

//______________________________________________________________________________//

KVTGIDManager::~KVTGIDManager()
{
}

//______________________________________________________________________________//

void KVTGIDManager::AddTGID(KVTGID* _tgid)
{
   //Add an identification object to the list.
   //It will be deleted when the KVTGIDManager is deleted.
   fIDList.Add(_tgid);
   fID_max = TMath::Max(fID_max, _tgid->GetIDmax());
}

//______________________________________________________________________________//

KVTGID* KVTGIDManager::GetTGID(const Char_t* name)
{
   //Retrieve the identification object with name
   return (KVTGID*) fIDList.FindObject(name);
}

//______________________________________________________________________________//

void KVTGIDManager::RemoveTGID(const Char_t* name)
{
   //Delete the identification object with name
   KVTGID* tgid = 0;
   if ((tgid = (KVTGID*)fIDList.FindObject(name))) {
      fIDList.Remove(tgid);
      //recalculate fID_max if needed
      if (tgid->GetIDmax() == fID_max) {
         fID_max = 0.;
         TIter n(&fIDList);
         KVTGID* t;
         while ((t = (KVTGID*)n())) fID_max = TMath::Max(fID_max, t->GetIDmax());
      }
      delete tgid;
   }
}

//______________________________________________________________________________//

void KVTGIDManager::RemoveAllTGID()
{
   //Delete all identification objects
   fIDList.Delete();
   fID_max = 0.;
}

//______________________________________________________________________________//

KVTGID* KVTGIDManager::GetTGID(const Char_t* idt_name,
                               const Char_t* id_type,
                               const Char_t* grid_type)
{
   //Retrieve the identification object using:
   //      idt_name = name of ID telescope
   //      id_type   = type of identification ("Z", "A", "A_for_Z=3", etc.)
   //      grid_type = type of identification grid ("GG", "PG1", etc.)
   return (GetTGID(GetTGIDName(idt_name, id_type, grid_type)));
}

//______________________________________________________________________________//

const Char_t* KVTGIDManager::GetTGIDName(const Char_t* idt_name,
      const Char_t* id_type,
      const Char_t* grid_type)
{
   //Return unique name for KVTGID object using:
   //      idt_name = name of ID telescope
   //      id_type   = type of identification ("Z", "A", "A_for_Z=3", etc.)
   //      grid_type = type of identification grid ("GG", "PG1", etc.)
   return (Form("%s_%s_%s", idt_name, id_type, grid_type));
}

//______________________________________________________________________________//

Double_t KVTGIDManager::IdentZ(const Char_t* idtelescope_name, Double_t id_map_X, Double_t id_map_Y, Double_t& funLTG,
                               const Char_t* grid_type)
{
   //Identification in Z of particle for ID telescope idt using Tassan-Got functional corresponding
   //to grid of type "grid_type". The "signal_type" argument is passed to GetIDMapX/Y(signal_type),
   //in case "grid_type" and "signal_type" are not exactly the same.
   //
   //funLTG gives distance to identification line (absolute value of LTG function for the Z we find)
   // funLTG = 0 -> identification parfaite
   // funLTG = -1 -> no identification
   //If the point to identify is outside of the range of the identification
   //for this 'grid_type' (not between Zmin and Zmax) then we return -1 and funLTG=-1.
   //if no identification of the required type exists, we return -1. and funLTG=-1.

   funLTG = -1.;
   fStatus = kStatus_OK;

   //get Z identification object from KVTGIDManager
   KVTGID* _tgid = GetTGID(idtelescope_name, "Z", grid_type);

   //if no identification object exists, return -1
   if (!_tgid) {
      fStatus = kStatus_noTGID;
      return (-1.0);
   }
   //Enter current point coordinates into Z identification function
   _tgid->SetParameter("X", id_map_X);
   _tgid->SetParameter("Y", id_map_Y);

   Double_t Zfound = -1.;
   Double_t Zmin = _tgid->GetIDmin();
   Double_t Zmax = _tgid->GetIDmax();
   if ((Int_t) Zmax == (Int_t) GetIDmax()) {
      //If this functional corresponds to the grid with the highest ID lines,
      //(i.e. for higher ID there are no more grids) then we need to be able to identify all
      //particles with this grid, even above the last ID line drawn by hand.

      Zmax = 100.;
   }
   //we need to lower and raise the limits in Z in order to pass the Eval(Zmin)*Eval(Zmax)<0
   //condition. E.G. for GG, Zmin=1, which means that any points below the Z=1 line are not
   //identified, whereas we should allow Zreal between 0.5 and 1.5
   //also when several fits are used for different ranges of Z, often the ranges of validity
   //of two successive fits do not overlap, i.e. Z=1-5 in GG, then Z=6-20 in PG etc.
   //in this case somewhere in between Z=5 & Z=6 particles will be left unidentified
   //as they will neither work in GG nor in PG.
   Zmin -= 1.;
   Zmax += 1.;

   Zfound = _tgid->GetIdentification(Zmin, Zmax, funLTG);

   if (_tgid->GetStatus() == KVTGID::kStatus_NotBetween_IDMin_IDMax)
      fStatus = kStatus_OutOfIDRange;

   return Zfound;
}

//______________________________________________________________________________

Double_t KVTGIDManager::IdentA(const Char_t* idtelescope_name, Double_t id_map_X, Double_t id_map_Y, Double_t& funLTG,
                               const Char_t* grid_type, Int_t Z)
{
   //Get A identification (we already know Z) for ID telescope idt using grid "grid_type".
   //The "signal_type" argument is passed to GetIDMapX/Y(signal_type),
   //in case "grid_type" and "signal_type" are not exactly the same.
   //
   //funLTG is the ABSOLUTE value of the Tassan-Got function for the Z and the calculated
   //A i.e. in an ideal world it should equal zero. This gives an idea of the
   //quality of the identification.
   // funLTG = 0 -> identification parfaite
   // funLTG = -1 -> no identification

   funLTG = -1.;
   fStatus = kStatus_OK;

   if (Z < 1) {
      fStatus = kStatus_ZtooSmall;
      return -1.;
   }
   //get A identification object from KVTGIDManager
   KVTGID* _tgid = GetTGID(idtelescope_name, "A", grid_type);
   //if no identification object exists, return -1
   if (!_tgid) {
      fStatus = kStatus_noTGID;
      return (-1.0);
   }
   //check Z is not too big - no isotopic identification for Z > Zmax of grid
   if (Z > _tgid->GetIDmax()) {
      fStatus = kStatus_ZtooLarge;
      return -1.;
   }
   //Initialise parameters of A identification function
   _tgid->SetParameter("X", id_map_X);
   _tgid->SetParameter("Y", id_map_Y);
   _tgid->SetParameter("Z", Z);

   Double_t Afound = -1., Amin, Amax;
   // reasonable limits for nuclear masses:
   if (Z == 1) {
      Amin = 1.;
      Amax = 3.;
   }
   else if (Z == 2) {
      Amin = 3;
      Amax = 8;
   }
   else if (Z == 3) {
      Amin = 6;
      Amax = 11;
   }
   else if (Z == 4) {
      Amin = 7;
      Amax = 14;
   }
   else if (Z == 5) {
      Amin = 8;
      Amax = 17;
   }
   else {
      //formula for Amin reasonable for 6<Z<20
      Amin = TMath::Max((Z + 1.), (1.8 * (Z - 2.) + 1.));
      Amax = 2.*Z + 8.;
   }

   // same limit trick as in IdentZ
   Amin -= 0.5;
   Amax += 0.5;

   Afound = _tgid->GetIdentification(Amin, Amax, funLTG);

   if (_tgid->GetStatus() == KVTGID::kStatus_NotBetween_IDMin_IDMax)
      fStatus = kStatus_OutOfIDRange;

   return Afound;
}

//___________________________________________________________________________________________//

KVTGIDGrid* KVTGIDManager::GetTGIDGrid(KVTGID* tgid,
                                       Double_t xmax, Double_t xmin,
                                       Int_t ID_min, Int_t ID_max,
                                       Int_t npoints, Bool_t logscale)
{
   //Generates and returns a pointer to a KVIDGrid object permitting to visualise the effective
   //identification lines represented by the Tassan-Got functional "tgid".
   //The KVIDGrid object must be deleted by the user.
   //See KVTGID::MakeIDGrid for meaning of other arguments.

   if (!tgid)
      return 0;
   KVTGIDGrid* gr = new KVTGIDGrid(tgid);
   gr->Generate(xmax, xmin, ID_min, ID_max, npoints, logscale);
   return gr;
}
//___________________________________________________________________________________________//

KVTGIDGrid* KVTGIDManager::GetTGIDGrid(const Char_t* tgid_name,
                                       Double_t xmax, Double_t xmin,
                                       Int_t ID_min, Int_t ID_max,
                                       Int_t npoints, Bool_t logscale)
{
   //Generates and returns a pointer to a KVIDGrid object permitting to visualise the effective
   //identification lines represented by the Tassan-Got functional "tgid_name".
   //The KVIDGrid object must be deleted by the user.
   //See KVTGID::MakeIDGrid for meaning of other arguments.

   return GetTGIDGrid(GetTGID(tgid_name), xmax, xmin, ID_min, ID_max, npoints, logscale);
}

//___________________________________________________________________________________________//

KVTGIDGrid* KVTGIDManager::GetTGIDGrid(const Char_t* idt_name,
                                       const Char_t* id_type,
                                       const Char_t* grid_type,
                                       Double_t xmax, Double_t xmin,
                                       Int_t ID_min, Int_t ID_max,
                                       Int_t npoints, Bool_t logscale)
{
   //Generates and returns a pointer to a KVIDGrid object permitting to visualise the effective
   //identification lines represented by the Tassan-Got functional of ID telescope "idt_name", of
   //identification type "id_type", and identification grid type "grid_type".
   //The KVIDGrid object must be deleted by the user.
   //See KVTGID::MakeIDGrid for meaning of other arguments.

   return GetTGIDGrid(GetTGID(idt_name, id_type, grid_type), xmax,
                      xmin, ID_min, ID_max, npoints, logscale);
}

//_______________________________________________________________________________________//

const Char_t* KVTGIDManager::GetStatusString(ETGIDMStatus s) const
{
   //Returns explanatory message for status code 's'
   //If no code argument is given, the status string corresponds to the current value
   //of GetStatus(), i.e. only valid if an identification has just been performed

   static TString messages[] = {
      "ok",
      "no KVTGID for requested identification",
      "point to identify outside of identification range of function",
      "IdentA called with Z<1",
      "IdentA called with Z larger than max Z defined for KVTGIDZA isotopic identification object"
   };
   Int_t status = (s != kCurrentStatus ? s + 1 : GetStatus() + 1);
   if (status--)
      return messages[status];
   return Form("no call to IdentZ()/IdentA() performed yet");
}
