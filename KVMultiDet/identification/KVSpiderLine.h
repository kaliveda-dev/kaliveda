
#ifndef __KVSPIDERLINE_H
#define __KVSPIDERLINE_H

#include <TGraph.h>
#include <TF1.h>
#include <TLatex.h>
#include <TNamed.h>

#include <Riostream.h>

/**
  \class KVSpiderLine
  \brief Part of Spider Identification
  \ingroup Identification
 */

class KVSpiderLine : public TNamed {
protected:

   TGraph* _line;
   TGraph* _iline;
   TF1*    _ff;
   int     _z;
   int     _a;
   bool    _filled;
   double  _pow;
   Int_t   _fitStatus;
   Int_t   _nAcceptedPoints;
   Int_t   _pointsCounter;

   Double_t _pdy;

public:

   KVSpiderLine();
   KVSpiderLine(int z_, Double_t pdy_ = 0.);
   KVSpiderLine(int z_, int a_);
   virtual ~KVSpiderLine() {}

   bool AddPoint(double x_, double y_, bool test_ = false, int n_ = -1);
   bool AddInterpolatePoint(double x_, double y_, bool test_ = false, int n_ = -1);
   void Apply(TF1* f);

   bool ReplaceLastPoint(double x_, double y_);
   void Sort(bool ascending_ = true);
   void SetZ(int z_);
   void SetA(int a_);
   void SetPower(double pow_)
   {
      _pow = pow_;
   }

   void SetAcceptedPoints(Int_t n)
   {
      _nAcceptedPoints = n;
   }
   void ResetCounter()
   {
      _pointsCounter = 0;
   }

   double GetX(int n_)const;
   double GetX()const;
   double GetInterpolateX(int n_)const;
   double GetInterpolateX()const;

   double GetY(int n_)const;
   double GetY()const;
   double GetInterpolateY(int n_)const;
   double GetInterpolateY()const;

   int GetN()const;
   int GetInterpolateN()const;

   TGraph* GetLine()
   {
      return _line;
   }
   TGraph* GetInterpolateLine()
   {
      return _iline;
   }
   virtual TF1* GetFunction(double min_ = -1., double max_ = -1.);
   int GetZ()
   {
      return _z;
   }
   int GetA()
   {
      return _a;
   }

   bool GetStatus();
   void SetStatus(bool filled_ = true);

   virtual bool TestPoint(double x_, double y_, double dy_ = -1., bool fit = true);
   double GetDistance(double x_, double y_);
   bool CheckStatus()const;
   void Draw(Option_t* opt_ = "");

   ClassDef(KVSpiderLine, 1)
};


#endif

