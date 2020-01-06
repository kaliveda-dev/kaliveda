//Created by KVClassFactory on Tue Mar  8 10:00:16 2016
//Author: Diego Gruyer

#ifndef __KVIDZAFROMZGRID_H
#define __KVIDZAFROMZGRID_H

#include "KVIDZAGrid.h"
#include "KVList.h"
#include "KVIdentificationResult.h"


class interval;
class interval_set;

class KVIDZAFromZGrid : public KVIDZAGrid {

public:
   enum PIDType {
      kNone,
      kPeackType,
      kIntType
   };

protected:
   Int_t  fZmaxInt;
   Bool_t fPIDRange;
   Int_t  fZminInt;
   KVIDentifier* fMassCut;//!
   KVList fTables;

   Bool_t fIgnoreMassID;

   int is_inside(double pid) const;

public:
   KVIDZAFromZGrid();
   virtual ~KVIDZAFromZGrid();

   void Copy(TObject& obj) const;

   virtual void ReadFromAsciiFile(std::ifstream& gridfile);
   virtual void WriteToAsciiFile(std::ofstream& gridfile);

   virtual void Identify(Double_t x, Double_t y, KVIdentificationResult*) const;
   virtual double DeduceAfromPID(KVIdentificationResult* idr) const;
   void LoadPIDRanges();
   void ReloadPIDRanges();
   interval_set* GetIntervalSet(int zint) const;
   KVList* GetIntervalSets()
   {
      return &fTables;
   }

   void PrintPIDLimits();
   void ClearPIDIntervals();
   void ExportToGrid();

   void Initialize();

   Bool_t HasMassIDCapability() const
   {
      // Overrides KVIDGraph::HasMassIDCapability
      //
      // Returns true if at least one mass interval (PID range) is defined,
      // UNLESS fIgnoreMassID=true (set by putting <PARAMETER> IgnoreMassID=1 in grid file)
      //
      // Note that IsOnlyZId() returns kTRUE for KVIDZAFromZGrid
      // and thus should not be used to know if a grid can identify masses

      return (fPIDRange || fTables.GetEntries()) && !fIgnoreMassID;
   }
   void SetOnlyZId(Bool_t /*yes*/ = kTRUE)
   {
      // Overrides KVIDGraph::SetOnlyZId
      //
      // fOnlyZId is really an internal variable, and must always be kTRUE for a KVIDZAFromZGrid.
      //
      // Therefore we disable the possibility to change it.

      KVIDZAGrid::SetOnlyZId(true);
   }

   ClassDef(KVIDZAFromZGrid, 1) //Compute Z and A only from Z lines...
};



class interval: public TNamed {
public:
   int fType;
   int fZ;
   int fA;

   double fPID;
   double fPIDMin;
   double fPIDmax;

   interval(int zz, int aa, double pid, double pidmin = -1., double pidmax = -1.)
   {
      fZ = zz;
      fA = aa;
      fPID = pid;
      fPIDMin = pidmin;
      fPIDmax = pidmax;
      SetName(Form("%d_%d", zz, aa));
   }
   bool is_inside(double pid)
   {
      // return true if pid lies inside this interval
      if (pid > fPIDMin && pid < fPIDmax) return kTRUE;
      return kFALSE;
   }
   bool is_left_of(double pid)
   {
      // return true if interval lies to the left of pid
      return (fPIDmax < pid);
   }
   bool is_right_of(double pid)
   {
      // return true if interval lies to the right of pid
      return (fPIDMin > pid);
   }

   int    GetA()
   {
      return fA;
   }
   int    GetZ()
   {
      return fZ;
   }
   double GetPID()
   {
      return fPID;
   }
   double GetPIDmin()
   {
      return fPIDMin;
   }
   double GetPIDmax()
   {
      return fPIDmax;
   }

   void SetA(int aa)
   {
      fA = aa;
   }
   void SetZ(int zz)
   {
      fZ = zz;
   }
   void SetPID(double pid)
   {
      fPID = pid;
   }
   void SetPIDmin(double pidmin)
   {
      fPIDMin = pidmin;
   }
   void SetPIDmax(double pidmax)
   {
      fPIDmax = pidmax;
   }


   ClassDef(interval, 1) //
};

class interval_set: public TNamed {
public:
   int fType;
   int fZ;
   KVList fIntervals;
   Int_t  fNPIDs;
   TGraph fPIDs;

   int GetZ()
   {
      return fZ;
   }
   int GetNPID()
   {
      return fIntervals.GetSize();
   }
   KVList* GetIntervals()
   {
      return &fIntervals;
   }
   TString GetListOfMasses();

   interval_set(int zz, int type);
   void   add(int aa, double pid, double pidmin = -1., double pidmax = -1.);
   double eval(KVIdentificationResult* idr);
   bool is_inside(double pid);
   bool is_above(double pid);

   ClassDef(interval_set, 1) //
};





#endif
