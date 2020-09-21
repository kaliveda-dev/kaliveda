//Created by KVClassFactory on Fri Nov 30 09:01:11 2012
//Author: dgruyer

#include "KVSpiderLineSiCsI.h"
#include "TMath.h"

ClassImp(KVSpiderLineSiCsI)


KVSpiderLineSiCsI::KVSpiderLineSiCsI(): KVSpiderLine()
{
   // Default constructor
}

KVSpiderLineSiCsI::KVSpiderLineSiCsI(int z_, Double_t pdy_): KVSpiderLine(z_, pdy_)
{
   Info("KVSpiderLineSiCsI", "called...");
}

KVSpiderLineSiCsI::KVSpiderLineSiCsI(int z_, int a_): KVSpiderLine(z_, a_)
{
   Info("KVSpiderLineSiCsI", "called...");
}

//________________________________________________________________

//KVSpiderLineSiCsI::KVSpiderLineSiCsI(const KVSpiderLineSiCsI& obj)  : KVSpiderLine()
//{
//   // Copy constructor
//   // This ctor is used to make a copy of an existing object (for example
//   // when a method returns an object), and it is always a good idea to
//   // implement it.
//   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

//   obj.Copy(*this);
//}

KVSpiderLineSiCsI::~KVSpiderLineSiCsI()
{
   // Destructor
}

//________________________________________________________________

//void KVSpiderLineSiCsI::Copy(TObject& obj) const
//{
//   // This method copies the current state of 'this' object into 'obj'
//   // You should add here any member variables, for example:
//   //    (supposing a member variable KVSpiderLineSiCsI::fToto)
//   //    CastedObj.fToto = fToto;
//   // or
//   //    CastedObj.SetToto( GetToto() );

//   KVSpiderLine::Copy(obj);
//   //KVSpiderLineSiCsI& CastedObj = (KVSpiderLineSiCsI&)obj;
//}

TF1* KVSpiderLineSiCsI::GetFunction(double min_, double max_)
{
   if (!CheckStatus()) return 0;

   double min;
   double max;

//   double xtest = GetX();
//   if (GetX(0) > GetX()) xtest = GetX(0);
//   double ytest = GetY();
//   if (GetY(0) > GetY()) ytest = GetY(0);

   //double p0 = TMath::Power(xtest, 0.4) * ytest;

   if (min_ == -1.) {
      if (GetX(0) < GetX()) min = GetX(0) - 10;
      else min = GetX() - 10;
      if (min <= 0.) min += 10.;
   }
   else min = min_;
   if (max_ == -1.) {
      if (GetX(0) < GetX()) max = GetX() + 10.;
      else max = GetX(0) + 10.;
   }
   else max = max_;

   if (!_ff) {
//       _ff = new TF1(GetName(), Form("[0]*TMath::Power(x,%lf)/(TMath::Power((x+[1]),[2]))", _pow), min, max);
//       _ff->SetParameters(p0, 100., 0.4);

      _ff = new TF1(GetName(), "[0]*(-1+[1]/(TMath::Power(x,[2])+[3]))", min, max);
      _ff->SetParameters(0.01, 3e5, 1.0, 200);
      _ff->FixParameter(2, 1.0);

      _ff->SetParLimits(0, 0.0001, 0.1);
      _ff->SetParLimits(1, 1e4, 1e11);
      _ff->SetParLimits(3, 10, 500);

   }
   else {
      double fmin, fmax;
      _ff->GetRange(fmin, fmax);
      if ((min <= fmin) || (max >= fmax)) {
         _ff->SetRange(min, max);
      }
   }
   _ff->FixParameter(2, 1.0);

   _fitStatus = _line->Fit(_ff, "WQN");
   _fitStatus = _line->Fit(_ff, "WQN");

   _ff->ReleaseParameter(2);
   _ff->SetParLimits(2, 0.5, 1.0);

   _fitStatus = _line->Fit(_ff, "WQN");
   _fitStatus = _line->Fit(_ff, "WQN");


//       printf("\tfit status = %d\n",_fitStatus);
//       for (Int_t ii=0;ii<_ff->GetNpar();ii+=1)
//       {
//          printf("\t%d %lf\n",ii,_ff->GetParameter(ii));
//       }
//       if (_fitStatus==0){
//          for (Int_t ii=0;ii<_ff->GetNpar();ii+=1)
//          {
//             printf("%lf ",_ff->GetParameter(ii));
//          }
//          printf("\n");
//       }
   return _ff;
}
