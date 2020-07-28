/*******************************************************************************
$Id: KVDatime.cpp,v 1.7 2009/01/15 16:10:13 franklan Exp $
$Revision: 1.7 $
$Date: 2009/01/15 16:10:13 $
$Author: franklan $
*******************************************************************************/

#include "KVDatime.h"
#include "KVList.h"
#include "KVString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVError.h"

ClassImp(KVDatime)

KVList* KVDatime::fmonths = 0;
Int_t KVDatime::ndatime = 0;

void KVDatime::init()
{
   //called by all ctors
   //sets up static list of months' names if not already done
   if (!fmonths) {
      fmonths = new KVList;
      fmonths->Add(new TObjString("JAN"));
      fmonths->Add(new TObjString("FEB"));
      fmonths->Add(new TObjString("MAR"));
      fmonths->Add(new TObjString("APR"));
      fmonths->Add(new TObjString("MAY"));
      fmonths->Add(new TObjString("JUN"));
      fmonths->Add(new TObjString("JUL"));
      fmonths->Add(new TObjString("AUG"));
      fmonths->Add(new TObjString("SEP"));
      fmonths->Add(new TObjString("OCT"));
      fmonths->Add(new TObjString("NOV"));
      fmonths->Add(new TObjString("DEC"));
   }
   ndatime++;
   fStr = "";
}

KVDatime::KVDatime()
   : TDatime()
{
   init();
}

KVDatime::KVDatime(const Char_t* DateString, EKVDateFormat f)
   : TDatime()
{
   //if f = KVDatime::kGANACQ:
   //
   //Decodes GANIL acquisition (INDRA) run-sheet format date into a TDatime
   //
   //Format of date string is: `29-SEP-2005 09:42:17.00`
   //
   //if f = KVDatime::kGANACQ2010:
   //
   //Decodes GANIL acquisition runfile format date into a TDatime
   //
   //Format of date string is: `27Nov10_02h07m40s`
   //
   //if f = KVDatime::kGANACQNarval:
   //
   //Decodes GANIL Narval acquisition runfile format date into a TDatime
   //
   //Format of date string is: `12-04-18_17h09m41s`
   //
   //if f = KVDatime::kSQL:
   //
   //Decodes SQL format date into a TDatime (i.e. same format as returned
   //by TDatime::AsSQLString(): `2007-05-02 14:52:18`)
   //
   //if f = KVDatime::kSRB:
   //
   //Decodes SRB format date into a TDatime
   //
   //Format of date string is: `2008-12-19-15.21`
   //
   //if f = KVDatime::kIRODS:
   //
   //Decodes IRODS format date into a TDatime
   //
   //Format of date string is: `2008-12-19.15:21`
   //
   //if f = KVDatime::kDMY:
   //
   //Decodes DMY format date into a TDatime
   //
   //Format of date string is: `19/12/2008`

   init();
   switch (f) {
      case kGANACQ:
         SetGanacqDate(DateString);
         break;
      case kGANACQ2010:
         SetGanacq2010Date(DateString);
         break;
      case kGANACQNarval:
         SetGanacqNarvalDate(DateString);
         break;
      case kSQL:
         SetSQLDate(DateString);
         break;
      case kSRB:
         SetSRBDate(DateString);
         break;
      case kIRODS:
         SetIRODSDate(DateString);
         break;
      case kDMY:
         SetDMYDate(DateString);
         break;
      default:
         Error(KV__ERROR(KVDatime), "Unknown date format");
   }
}

KVDatime& KVDatime::operator=(const KVDatime& d)
{
   // copy assignment operator
   return dynamic_cast<KVDatime&>(TDatime::operator=(d));
}

KVDatime::~KVDatime()
{
   ndatime--;
   if (!ndatime) {
      delete fmonths;
      fmonths = 0;
   }
}

void KVDatime::SetSQLDate(const Char_t* DateString)
{
   //Decodes SQL format date into a TDatime (i.e. same format as returned
   //by TDatime::AsSQLString(): "2007-05-02 14:52:18")
   Int_t Y, M, D, h, m, s;
   sscanf(DateString, "%4d-%02d-%02d %02d:%02d:%02d",
          &Y, &M, &D, &h, &m, &s);
   Set(Y, M, D, h, m, s);
}

void KVDatime::SetSRBDate(const Char_t* DateString)
{
   //Decodes SRB format date into a TDatime
   //Format of date string is:
   //         2008-12-19-15.21

   Int_t Y, M, D, h, m, s;
   sscanf(DateString, "%4d-%02d-%02d-%02d.%02d",
          &Y, &M, &D, &h, &m);
   s = 0;
   Set(Y, M, D, h, m, s);
}

void KVDatime::SetIRODSDate(const Char_t* DateString)
{
   //Decodes IRODS format date into a TDatime
   //Format of date string is:
   //         2008-12-19.15:21

   Int_t Y, M, D, h, m, s;
   sscanf(DateString, "%4d-%02d-%02d.%02d:%02d",
          &Y, &M, &D, &h, &m);
   s = 0;
   Set(Y, M, D, h, m, s);
}

void KVDatime::SetDMYDate(const Char_t* DMYString)
{
   // Set date from string in format "DD/MM/YYYY"
   Int_t Y, M, D;
   sscanf(DMYString, "%2d/%2d/%4d", &D, &M, &Y);
   Set(Y, M, D, 0, 0, 0);
}

void KVDatime::SetGanacq2010Date(const Char_t* GanacqDateString)
{
   // Decodes dates in new (2010) format of GANIL acquisition
   // run files, e.g. run_0058.dat.27Nov10_02h07m40s
   Int_t Y, D, H, M, S;
   Char_t Month[5];
   TString tmp(GanacqDateString);
   tmp.ToUpper();
   tmp.ReplaceAll("_", " ");
   if (sscanf(tmp.Data(), "%02d%3s%02d %02dH%02dM%02dS", &D, Month, &Y, &H, &M, &S) == 6) {
      TObject* mm = fmonths->FindObject(Month);
      Int_t month = 0;
      if (mm) month = fmonths->IndexOf(mm) + 1;
      Y += 2000;
      Set(Y, month, D, H, M, S);
   }
}

void KVDatime::SetGanacqNarvalDate(const Char_t* GanacqDateString)
{
   // Decodes dates in format of GANIL Narval acquisition
   // run files, e.g. run_0058.dat.27-11-10_02h07m40s
   Int_t Y, D, H, M, S, Month;
   TString tmp(GanacqDateString);
   tmp.ToUpper();
   tmp.ReplaceAll("_", " ");
   if (sscanf(tmp.Data(), "%02d-%02d-%02d %02dH%02dM%02dS", &D, &Month, &Y, &H, &M, &S) == 6) {
      Y += 2000;
      Set(Y, Month, D, H, M, S);
   }
}

void KVDatime::SetGanacqDate(const Char_t* GanacqDateString)
{
   //Decodes GANIL acquisition (INDRA) run-sheet format date into a TDatime
   //Format of date string is:
   //      29-SEP-2005 09:42:17.00
   //  or  29-SEP-2005 09:42:17
   //  or  29-SEP-05 09:42:17.00
   //  or  29-SEP-05 09:42:17
   //
   //If format is not respected, we set to current time & date

   KVString tmp(GanacqDateString);
   TObjArray* toks = tmp.Tokenize("-:. ");
   if (toks->GetEntries() < 6 || toks->GetEntries() > 7) {
      // if format is correct, there should be 6 or 7 elements in toks
      delete toks;
      Error("SetGanacqDate", "Format is incorrect: %s (should be like \"29-SEP-2005 09:42:17.00\" or \"29-SEP-05 09:42:17\")",
            GanacqDateString);
      Set();
      return;
   }
   KV__TOBJSTRING_TO_INT(toks, 0, day)
   TObject* mm =
      fmonths->FindObject(((TObjString*) toks->At(1))->String().Data());
   Int_t month = 0;
   if (mm)
      month = fmonths->IndexOf(mm) + 1;
   KV__TOBJSTRING_TO_INT(toks, 2, year)
   // year may be written in shortened form: 97 instead of 1997
   if (year < 100) {
//      Warning("SetGanacqDate",
//              "Ambiguous value for year: %d. Assuming this means: %d",
//              year, (year < 82 ? year + 2000 : year + 1900));
      (year < 82 ? year += 2000 : year += 1900);
   }
   KV__TOBJSTRING_TO_INT(toks, 3, hour)
   KV__TOBJSTRING_TO_INT(toks, 4, min)
   KV__TOBJSTRING_TO_INT(toks, 5, sec)
   delete toks;
   Set(year, month, day, hour, min, sec);
}

const Char_t* KVDatime::AsGanacqDateString() const
{
   //Return date and time string with format "29-SEP-2005 09:42:17.00"
   //Copy the string immediately if you want to reuse/keep it
   return Form("%d-%s-%4d %02d:%02d:%02d.00",
               GetDay(),
               ((TObjString*) fmonths->At(GetMonth() - 1))->String().
               Data(), GetYear(), GetHour(), GetMinute(), GetSecond());
}

const Char_t* KVDatime::AsDMYDateString() const
{
   return Form("%02d/%02d/%4d",
               GetDay(),
               GetMonth(), GetYear());
}

const Char_t* KVDatime::String(EKVDateFormat fmt)
{
   // Returns date & time as a string in required format:
   //  fmt = kCTIME (default)  :  ctime format e.g. Thu Apr 10 10:48:34 2008
   //  fmt = kSQL              :  SQL format e.g. 1997-01-15 20:16:28
   //  fmt = kGANACQ           :  GANIL acquisition format e.g. 29-SEP-2005 09:42:17.00
   //  fmt = kDMY           :  DD/MM/YYYY
   switch (fmt) {
      case kCTIME:
         fStr = AsString();
         break;
      case kSQL:
         fStr = AsSQLString();
         break;
      case kGANACQ:
         fStr = AsGanacqDateString();
         break;
      case kDMY:
         fStr = AsDMYDateString();
         break;
      default:
         fStr = "";
   }
   return fStr.Data();
}

Bool_t KVDatime::IsGANACQFormat(const Char_t* date)
{
   // Static method, returns kTRUE if 'date' is in format of GANIL acquisition
   // e.g. 29-SEP-2005 09:42:17.00

   KVString tmp(date);
   TObjArray* toks = tmp.Tokenize("-:. ");
   if (toks->GetEntries() < 6 || toks->GetEntries() > 7) {
      // if format is correct, there should be 6 or 7 elements in toks
      delete toks;
      return kFALSE;
   }
   delete toks;
   return kTRUE;
}

Bool_t KVDatime::IsGANACQ2010Format(const Char_t* date)
{
   // Static method, returns kTRUE if 'date' is in new (2010) format of GANIL acquisition
   // run files, e.g. run_0058.dat.27Nov10_02h07m40s

   Int_t Y, D, H, M, S;
   Char_t Month[5];
   TString tmp(date);
   tmp.ToUpper();
   tmp.ReplaceAll("_", " ");
   if (sscanf(tmp.Data(), "%02d%3s%02d %02dH%02dM%02dS", &D, Month, &Y, &H, &M, &S) == 6)
      return kTRUE;
   return kFALSE;
}

Bool_t KVDatime::IsGANACQNarvalFormat(const Char_t* date)
{
   // Static method, returns kTRUE if 'date' is in format of GANIL Narval acquisition
   // run files, e.g. run_0058.dat.27-11-10_02h07m40s

   Int_t Y, D, H, M, S, Month;
   TString tmp(date);
   tmp.ToUpper();
   tmp.ReplaceAll("_", " ");
   if (sscanf(tmp.Data(), "%02d-%02d-%02d %02dH%02dM%02dS", &D, &Month, &Y, &H, &M, &S) == 6)
      return kTRUE;
   return kFALSE;
}

Bool_t KVDatime::IsSQLFormat(const Char_t* date)
{
   // Static method, returns kTRUE if 'date' is in SQL format
   // e.g. 2007-05-02 14:52:18

   Int_t Y, M, D, h, m, s;
   if (sscanf(date, "%4d-%02d-%02d %02d:%02d:%02d",
              &Y, &M, &D, &h, &m, &s) != 6) return kFALSE;
   return kTRUE;
}

Bool_t KVDatime::IsSRBFormat(const Char_t* date)
{
   // Static method, returns kTRUE if 'date' is in SRB format
   // e.g. 2008-12-19-15.21

   Int_t Y, M, D, h, m;
   if (sscanf(date, "%4d-%02d-%02d-%02d.%02d",
              &Y, &M, &D, &h, &m) != 5) return kFALSE;
   return kTRUE;
}

Bool_t KVDatime::IsIRODSFormat(const Char_t* date)
{
   // Static method, returns kTRUE if 'date' is in IRODS format
   // e.g. 2008-12-19.15:21

   Int_t Y, M, D, h, m;
   if (sscanf(date, "%4d-%02d-%02d.%02d:%02d",
              &Y, &M, &D, &h, &m) != 5) return kFALSE;
   return kTRUE;
}

const Char_t* KVDatime::Month(Int_t m)
{
   return ((TObjString*) fmonths->At(m - 1))->String().Data();
}

Double_t KVDatime::GetNumberOfSeconds(Int_t ref_year)
{

   Double_t total_ins = 0;

   Double_t year_ins = GetYear();
   year_ins -= ref_year;

   year_ins *= 365.25; //1y=365.25 d
   year_ins *= 24; //1d=24 h
   year_ins *= 3660; //1h=60x60=3660 s

   total_ins += year_ins;

   //nombre de jours depuis le debut de l annee
   Double_t month_ins = GetMonth() - 1;
   month_ins *= 365.25 / 12; //1month = 30.437d (365.25/12) approximatif
   month_ins += GetDay(); //-> nombre de jours
   month_ins *= 24;  //1d=24 h
   month_ins *= 3660; //1h=60x60=3660 s

   total_ins += month_ins; //nombre de jours totals en second

   total_ins += GetHour() * 3600;
   total_ins += GetMinute() * 60;
   total_ins += GetSecond();

   return total_ins;


}

Double_t KVDatime::GetDureeInSeconds(KVDatime from)
{
   Int_t year_ref = from.GetYear();
   if (year_ref > this->GetYear()) year_ref = this->GetYear();

   Double_t sum = from.GetNumberOfSeconds(year_ref);
   return this->GetNumberOfSeconds(year_ref) - sum;
}
