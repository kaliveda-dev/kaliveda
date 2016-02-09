/*******************************************************************************
$Id: KVDatime.h,v 1.4 2008/04/10 15:43:45 franklan Exp $
$Revision: 1.4 $
$Date: 2008/04/10 15:43:45 $
$Author: franklan $
*******************************************************************************/

#ifndef __KVDATIME_H
#define __KVDATIME_H

#include "TDatime.h"
#include "TString.h"

class KVList;

class KVDatime: public TDatime {
   static KVList* fmonths;      //list of months
   static Int_t ndatime;        //counter
   void init();
   TString fStr;//! internal buffer used by String method

public:

   enum EKVDateFormat {
      kCTIME,
      kGANACQ,
      kSQL,
      kSRB,
      kIRODS,
      kDMY
   };

   KVDatime();
   KVDatime(const Char_t* DateString, EKVDateFormat f = kGANACQ);
   KVDatime(const KVDatime& d)
      : TDatime((const TDatime&)d)
   {
      init();
   };
   KVDatime(const TDatime& d)
      : TDatime(d)
   {
      init();
   };
   KVDatime(UInt_t tloc, Bool_t dosDate = kFALSE)
      : TDatime(tloc, dosDate)
   {
      init();
   };
   KVDatime(Int_t date, Int_t time)
      : TDatime(date, time)
   {
      init();
   };
   KVDatime(Int_t year, Int_t month, Int_t day,
            Int_t hour, Int_t min, Int_t sec)
      : TDatime(year, month, day, hour, min, sec)
   {
      init();
   };
   virtual ~ KVDatime();

   void SetGanacqDate(const Char_t* GanacqDateString);
   void SetGanacq2010Date(const Char_t* GanacqDateString);
   const Char_t* AsGanacqDateString() const;
   const Char_t* AsDMYDateString() const;
   void SetSQLDate(const Char_t* SQLDateString);
   void SetSRBDate(const Char_t* SRBDateString);
   void SetIRODSDate(const Char_t* IRODSDateString);
   void SetDMYDate(const Char_t* DMYString);

   const Char_t* String(EKVDateFormat fmt = kCTIME);

   static Bool_t IsSQLFormat(const Char_t* date);
   static Bool_t IsGANACQFormat(const Char_t* date);
   static Bool_t IsGANACQ2010Format(const Char_t* date);
   static Bool_t IsSRBFormat(const Char_t* date);
   static Bool_t IsIRODSFormat(const Char_t* date);

   static const Char_t* Month(Int_t m);
   Double_t GetNumberOfSeconds(Int_t ref_year = 0);
   Double_t GetDureeInSeconds(KVDatime from);

   ClassDef(KVDatime, 0)       //Extension of TDatime class
};

#endif
