/***************************************************************************
$Id: KVINDRADBRun.cpp,v 1.11 2009/01/22 13:58:33 franklan Exp $
$Revision: 1.11 $
$Date: 2009/01/22 13:58:33 $
$Author: franklan $
 ***************************************************************************/
#include "KVINDRADBRun.h"
#include "Riostream.h"
#include "KVINDRADB.h"

ClassImp(KVINDRADBRun);

//____________________________________________________________________________

KVINDRADBRun::KVINDRADBRun()
{
   //default ctor
}

//____________________________________________________________________________
KVINDRADBRun::KVINDRADBRun(Int_t number, const Char_t* title): KVDBRun(number,
         title)
{
   //ctor for a given run number
}

//____________________________________________________________________________
KVINDRADBRun::~KVINDRADBRun()
{
   //dtor
}

//___________________________________________________________________________

Double_t KVINDRADBRun::GetEventCrossSection(Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Calculation of cross-section (in millibarn) per event of this run from the measured integrated beam current and correcting for dead time.
   //Based on sect_effic.f by M.F. Rivet
   //
   //For meaning of arguments, see GetNIncidentIons()

   KVTarget* target = GetTarget();
   if (!target)
      return 0;                 //can't calculate X-section without knowing target characteristics

   //calculate number of atoms/cm**2 in target
   Double_t atom_cib = target->GetAtomsPerCM2();

   //number of projectile nuclei during run
   Double_t n_proj = GetNIncidentIons(Q_apres_cible, Coul_par_top);

   Double_t temps_mort = GetTempsMort();
   if (temps_mort >= 1)
      return 0.;

   //X-section [mb] per event = 1e27 / (no. atoms in target * no. of projectile nuclei * (1 - temps mort))
   Double_t sect_effic = 1.e27 / (atom_cib * n_proj * (1 - temps_mort));
   return sect_effic;
}

Double_t KVINDRADBRun::GetTotalCrossSection(Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Calculate total cross-section (in millibarn) measured for this run from the calculated cross-section per event
   //(see GetEventCrossSection()) and the total number of measured events
   return GetEventCrossSection(Q_apres_cible, Coul_par_top) * GetEvents();
}

Double_t KVINDRADBRun::GetTempsMort() const
{
   //Calculate temps mort for run from values of Gene dir, Gene MRQ and Gene TM, according to:
   //      T.M. = Gene TM / (Gene dir - Gene MRQ)
   //If Gene dir = Gene MRQ, temps mort = 0.
   //This can be compared to the value of GetTMpercent() (if present) which is the percentage value filled in the runsheet
   //during the experimental run.
   if (GetScaler("Gene DIRECT") == GetScaler("Gene MARQ")) {
      //if Gene MRQ == Gene Dir, temps mort 0
      return 0;
   }
   return (GetScaler("Gene TM") * 1.0 / (GetScaler("Gene DIRECT") - GetScaler("Gene MARQ")));
}

Double_t KVINDRADBRun::GetNIncidentIons(Double_t Q_apres_cible,
                                        Double_t Coul_par_top) const
{
   //Calculate total number of incident beam particles during run, based on measured integrated beam current
   //Arguments:
   //      Q_apres_cible = (average) charge state of projectile ions AFTER passage through target
   //      Coul_par_top = calibration of Faraday current integrator (default: 1.e-10 Coul/top)
   //      Note (from sect_effic.f):
   //**** Etalonnage de l'integrateur en 1993: voir cahier 4 page 87.
   //***  Si l'oscillateur interne du trigger est a 169 Hz, on a 10**(-10) Cb/top.
   //***  si c'est 160 Hz, on a 1.0610*(10**(-10)).

   return GetScaler("Faraday 1") * Coul_par_top / (TMath::Qe() * Q_apres_cible);
}

//___________________________________________________________________________

void KVINDRADBRun::UnsetSystem()
{
   //If this run has previously been associated with a system in the database,
   //this will remove the association. The run will also be removed from the system's
   //list of associated runs.

   KVDBRun::UnsetSystem();
   SetTitle("INDRA run");
}

KVDBTape* KVINDRADBRun::GetTape() const
{
   if (GetKey("Tapes")) {
      return (KVDBTape*) GetKey("Tapes")->GetLinks()->First();
   }
   return 0;
}

Int_t KVINDRADBRun::GetTapeNumber() const
{
   if (GetTape())
      return GetTape()->GetNumber();
   else
      return 0;
}

//___________________________________________________________________________

Float_t KVINDRADBRun::GetMeanPulser(const Char_t* param)
{
   // Return mean value of pulser/laser for this run.
   //
   // For detectors, param should be name of an acquisition parameter
   // e.g. CI_0201_PG, CSI_1301_L, etc.
   // For pin laser diodes, param should be name of associated acquisition parameter
   // with either '_laser' or '_gene' appended
   // e.g. PILA_05_PG_laser, SI_PIN1_PG_gene
   //
   // Returns -1.0 if no data available for this parameter/run.

   if (gIndraDB && gIndraDB->GetPulserData())
      return gIndraDB->GetPulserData()->GetMean(param, GetNumber());
   return -1.0;
}
