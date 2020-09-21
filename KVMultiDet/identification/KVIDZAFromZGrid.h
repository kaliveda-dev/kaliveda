//Created by KVClassFactory on Tue Mar  8 10:00:16 2016
//Author: Diego Gruyer

#ifndef __KVIDZAFROMZGRID_H
#define __KVIDZAFROMZGRID_H

#include "KVIDZAGrid.h"
#include "KVList.h"
#include "KVIdentificationResult.h"


class interval;
class interval_set;

/**
\class KVIDZAFromZGrid
\brief Hybrid identification grid
\ingroup Identification

Such a grid can identify simultaneously both the mass and charge of detected
particles (if contains a conversion table for the given element), or solely the charge
(if no conversion table).

The conversion table has to be included in the parameter list of the grid as follow:
~~~~
<PARAMETER> PIDRANGE=[zmax]
<PARAMETER> PIDRANGE[z]=[a1]:[PID min],[PID mean],[PID max]|[a2]:[PID min],[PID mean],[PID max]|[a3]:[PID min],[PID mean],[PID max]
(...)
~~~~
with:
<ul>
     <li> [zmax]            highest Z for which A will be extracted</li>
     <li> [z]               charge corresponding to the following line</li>
     <li> [ai]              mass corresponding to the following interval</li>
     <li> [PID mean]        PID value of the peak corresponding to mass [ai]</li>
     <li> [PID min/max]     PID interval corresponding to mass [ai] used to attribute the ICODE (optional)</li>
</ul>

<h3>Identification quality codes</h3>
After each identification attempt, the value returned by GetQualityCode() indicates whether the
identification was successful or not. The meaning of the different codes depends on the type
of identification.

<h4>Z & A (mass & charge) isotopic identification grid</h4>
<ul>
     <li> KVIDZAGrid::kICODE0,                   ok</li>
  <li>     KVIDZAGrid::kICODE1,                   Z ok, slight ambiguity of A, which could be larger</li>
  <li>     KVIDZAGrid::kICODE2,                   Z ok, slight ambiguity of A, which could be smaller</li>
    <li>   KVIDZAGrid::kICODE3,                   Z ok, slight ambiguity of A, which could be larger or smaller</li>
   <li>    KVIDZAGrid::kICODE4,                   point out of mass identification intervals, strong ambiguity of A</li>
  <li>     KVIDZAGrid::kICODE5,                   point is in between two isotopes of different Z, too far from either to be considered well-identified</li>
   <li>    KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid</li>
   <li>    KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid</li>
  <li>     KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid</li>
</ul>

<h4>Z-only charge identification grid</h4>
<ul>
     <li> KVIDZAGrid::kICODE0,                   ok</li>
  <li>     KVIDZAGrid::kICODE1,                   slight ambiguity of Z, which could be larger</li>
  <li>     KVIDZAGrid::kICODE2,                   slight ambiguity of Z, which could be smaller</li>
    <li>   KVIDZAGrid::kICODE3,                   slight ambiguity of Z, which could be larger or smaller</li>
   <li>    KVIDZAGrid::kICODE4,                   point is in between two lines of different Z, too far from either to be considered well-identified</li>
  <li>     KVIDZAGrid::kICODE5,                   point is in between two lines of different Z, too far from either to be considered well-identified</li>
   <li>    KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid</li>
   <li>    KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid</li>
  <li>     KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid</li>
</ul>

In both cases, an acceptable identification is achieved if the quality code is kICODE0, kICODE1, kICODE2, or kICODE3.<br>
Points with codes kICODE4 or kICODE5 are normally considered as "noise" and should be rejected.<br>
Points which are (vertically) out of range for this grid have code kICODE6 (point too far below) or kICODE7 (point too far above).<br>
Points with code kICODE8 are totally out of range.
*/

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
   void ResetPIDRanges();
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
