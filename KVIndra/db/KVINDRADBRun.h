/***************************************************************************
$Id: KVINDRADBRun.h,v 1.10 2009/01/22 13:58:33 franklan Exp $
$Revision: 1.10 $
$Date: 2009/01/22 13:58:33 $
$Author: franklan $
 ***************************************************************************/
#ifndef KV_DB_INDRARUN_H
#define KV_DB_INDRARUN_H

#include "KVDBTape.h"
#include "KVString.h"
#include "KVDBRun.h"
#include "KVDatime.h"

class KVINDRADBRun: public KVDBRun {

private:
   KVDatime fStartEndDate;     //! used by GetStartDatime and GetEndDatime methods
public:

   KVINDRADBRun();
   KVINDRADBRun(Int_t number, const Char_t* title = "INDRA Run");
   virtual ~ KVINDRADBRun();

   KVDBTape* GetTape() const;
   Int_t GetTapeNumber() const;
   Double_t GetTMpercent() const
   {
      return Get("Dead time (%)");
   };
   Int_t GetTrigger() const
   {
      return GetScaler("Trigger multiplicity");
   };
   const Char_t* GetTriggerString() const
   {
      if (GetTrigger() > 0) return Form("M>=%d", GetTrigger());
      else return Form("xxx");
   };
   const Char_t* GetCahier() const
   {
      return GetString("Logbook");
   };
   const Char_t* GetLogbook() const
   {
      return GetCahier();
   };

   void SetTMpercent(Float_t tmp)
   {
      Set("Dead time (%)", tmp);
   };
   void SetTrigger(Int_t trig)
   {
      SetScaler("Trigger multiplicity", trig);
   };
   void SetCahier(const Char_t* log)
   {
      SetLogbook(log);
   };
   void SetLogbook(const Char_t* log)
   {
      Set("Logbook", log);
   };

   Double_t GetEventCrossSection(Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetTotalCrossSection(Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;

   Double_t GetTempsMort() const;
   Double_t GetDeadTime() const
   {
      return GetTempsMort();
   };
   Double_t GetNIncidentIons(Double_t Q_apres_cible,
                             Double_t Coul_par_top = 1.e-10) const;

   //The GENE DIRECT of INDRA Selector module is generated from an internal clock with a nominal
   //frequency of 160Hz, but which in reality (as measured by scaler) is approximately 185Hz (it actually
   //appears to fluctuate around this value). Therefore the length of a run can be obtained
   //from the GD simply by dividing by 185 (or another value for the average frequency of the pulser,
   //which may be given as argument). This method returns the result of this
   //division, corresponding to the length of the run (in seconds).
   virtual Double_t GetGeneDirectTime(Double_t average_gene_dir_frequency = 185.0) const
   {
      return GetScaler("Gene DIRECT") / average_gene_dir_frequency;
   }
   virtual void UnsetSystem();

   KVDatime& GetStartDatime()
   {
      // Returns KVDatime with start date/time of run, i.e. string returned by GetStartDate().
      // For INDRA runs, this string is assumed to be in GANIL acquisition format.
      // If date is not set, we return current time & date.
      if (strcmp(GetStartDate(), "")) fStartEndDate.SetGanacqDate(GetStartDate());
      else fStartEndDate.Set();
      return fStartEndDate;
   };
   KVDatime& GetEndDatime()
   {
      // Returns KVDatime with end date/time of run, i.e. string returned by GetEndDate().
      // For INDRA runs, this string is assumed to be in GANIL acquisition format.
      if (strcmp(GetEndDate(), "")) fStartEndDate.SetGanacqDate(GetEndDate());
      else fStartEndDate.Set();
      return fStartEndDate;
   };
   Float_t GetMeanPulser(const Char_t*);

   ClassDef(KVINDRADBRun, 4)    //Class for storing information on INDRA runs
};
#endif
