//Created by KVClassFactory on Fri Feb 21 10:48:23 2014
//Author: Rivet Marie-France

#include "KVSi75_e494s.h"

ClassImp(KVSi75_e494s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSi75_e494s</h2>
<h4>Calib Etalons</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSi75_e494s::KVSi75_e494s()
{
   // Default constructor
}

//________________________________________________________________

KVSi75_e494s::KVSi75_e494s(Float_t thick) : KVSi75(thick)
{
   // Write your code here
}

KVSi75_e494s::~KVSi75_e494s()
{
   // Destructor
}

//________________________________________________________________

Double_t KVSi75_e494s::GetVoltsFromCanalPG(Double_t chan)
{
   //Return calibrated detector signal in Volts calculated from PG channel number.
   //If "chan" is not given, the value of the "PG" acquisition parameter read from
   //data for this detector is used to calculate the signal.
   //If the PG parameter is not present (=-1) or no calib we return 0.
   //Change in the coder pedestal between the current run and the reference pedestal
   //is taken into account, as well as the shift between the pedestal of the
   //calibration run (533) and the reference run (293).
   // To get correct value for Cf alphas, we must add 7 channels, not 3-4

   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return 0;

   if (!chan) {
      chan = GetPG();
   }
   if (chan < -0.5)
      return 0.;          //PG parameter absent
   //correct for pedestal drift
   chan = chan - (Float_t) GetACQParam("PG")->GetDeltaPedestal() + 7.0;
   return (fChVoltPG->Compute(chan));
}
//____________________________________________________________________________________________
Int_t KVSi75_e494s::GetCanalPGFromVolts(Float_t volts)
{
   //Return raw PG channel number corresponding to a given detector signal in volts
   //
   //Any change in the coder pedestal between the current run and the effective pedestal
   //of the channel-volt calibration (GetCanal(V=0)) is automatically corrected for.
   //
   //Returns -1 if PG <-> Volts calibration is not available


   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return -1;
   Int_t chan = TMath::Nint(fChVoltPG->Invert(volts) +
                            (Float_t) GetACQParam("PG")->GetDeltaPedestal() - 3.5);
   return chan;

}

Double_t KVSi75_e494s::GetCanalPGFromVoltsDouble(Float_t volts)
{
   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return -1;
   return fChVoltPG->InvertDouble(volts) +
          (Float_t) GetACQParam("PG")->GetDeltaPedestal() - 3.5;

}

//________________________________________________________________
/*
void KVSi75_e494s::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSi75_e494s::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSi75::Copy(obj);
   //KVSi75_e494s& CastedObj = (KVSi75_e494s&)obj;
}
*/
////////////////////////////////////////////////////////////////////////////////

ClassImp(KVSiLi_e494s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSiLi_e494s</h2>
<h4>Calib Etalons</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSiLi_e494s::KVSiLi_e494s()
{
   // Default constructor
}

//________________________________________________________________

KVSiLi_e494s::KVSiLi_e494s(Float_t thick) : KVSiLi(thick)
{
   // Write your code here
}

KVSiLi_e494s::~KVSiLi_e494s()
{
   // Destructor
}

//________________________________________________________________

Double_t KVSiLi_e494s::GetVoltsFromCanalPG(Double_t chan)
{
   //Return calibrated detector signal in Volts calculated from PG channel number.
   //If "chan" is not given, the value of the "PG" acquisition parameter read from
   //data for this detector is used to calculate the signal.
   //If the PG parameter is not present (=-1) or no calib we return 0.
   //Change in the coder pedestal between the current run and the reference pedestal
   //is taken into account, as well as the shift between the pedestal of the
   //calibration run (546) and the reference run (293).
   // To get correct value for Cf alphas, we must add 7 channels, not 3-4

   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return 0;

   if (!chan) {
      chan = GetPG();
   }
   if (chan < -0.5)
      return 0.;          //PG parameter absent
   //correct for pedestal drift
   chan = chan - (Float_t) GetACQParam("PG")->GetDeltaPedestal() + 7.0;
   return (fChVoltPG->Compute(chan));
}
//____________________________________________________________________________________________
Int_t KVSiLi_e494s::GetCanalPGFromVolts(Float_t volts)
{
   //Return raw PG channel number corresponding to a given detector signal in volts
   //
   //Any change in the coder pedestal between the current run and the effective pedestal
   //of the channel-volt calibration (GetCanal(V=0)) is automatically corrected for.
   //
   //Returns -1 if PG <-> Volts calibration is not available


   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return -1;
   Int_t chan = TMath::Nint(fChVoltPG->Invert(volts) +
                            (Float_t) GetACQParam("PG")->GetDeltaPedestal() - 3.5);
   return chan;

}

Double_t KVSiLi_e494s::GetCanalPGFromVoltsDouble(Float_t volts)
{
   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return -1;
   return fChVoltPG->InvertDouble(volts) +
          (Float_t) GetACQParam("PG")->GetDeltaPedestal() - 3.5;

}

//________________________________________________________________

/*void KVSiLi_e494s::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSiLi_e494s::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSiLi::Copy(obj);
   //KVSiLi_e494s& CastedObj = (KVSiLi_e494s&)obj;
}
*/
