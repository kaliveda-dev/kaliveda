//Created by KVClassFactory on Tue Mar  8 10:00:16 2016
//Author: Diego Gruyer

#ifndef __KVIDZAFROMZGRID_H
#define __KVIDZAFROMZGRID_H

#include "KVIDZAGrid.h"
#include "KVList.h"
#include "KVIdentificationResult.h"

class KVIDZAFromZGrid : public KVIDZAGrid {
public:
   enum PIDType {
      kNone,
      kPeackType,
      kIntType
   };

public:
   class interval: public TNamed {
   public:
      int fType;
      int fZ;
      int fNPIDs;

      std::vector<int>    fAs;
      std::vector<double> fPIDmins;
      std::vector<double> fPIDmaxs;
      TGraph fPIDs;

      interval(int zz, int type);
      void   add(int aa, double pid, double pidmin = -1., double pidmax = -1.);
      double eval(KVIdentificationResult* idr);
      bool is_inside(double pid);

      ClassDef(interval, 1) //
   };
//  std::vector<interval> toto;

public:
   Int_t  fZmaxInt;
   Bool_t fPIDRange;
   Int_t  fZminInt;
   Bool_t fHasMassCut;
   KVList fTables;

public:
   KVIDZAFromZGrid();
   virtual ~KVIDZAFromZGrid();
public:
   void Copy(TObject& obj) const;

//   virtual void ReadAsciiFile(const Char_t* filename);
   virtual void ReadFromAsciiFile(std::ifstream& gridfile);
   virtual void Identify(Double_t x, Double_t y, KVIdentificationResult*) const;
   virtual double DeduceAfromPID(KVIdentificationResult* idr);
   void LoadPIDRanges();

   void PrintPIDLimits();

protected:
   bool is_inside(double pid);


   ClassDef(KVIDZAFromZGrid, 1) //Compute Z and A only from Z lines...
};

#endif
