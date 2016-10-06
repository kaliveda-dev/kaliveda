//Created by KVClassFactory on Mon Jun 28 15:02:00 2010
//Author: bonnet

#include "KVSimNucleus.h"
#include "TVector3.h"

ClassImp(KVSimNucleus)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimNucleus</h2>
<h4>Classe dérivée de KVNucleus pour gérer des simulations dans KaliVeda</h4>
<!-- */
// --> END_HTML
//Deux TVector3 sont ajoutés en champs de la classe :
// - le vecteur position
// - le vecteur moment angulaire
//
//Cette classe est couplée à KVSimEvent
////////////////////////////////////////////////////////////////////////////////


//___________________________
KVSimNucleus::KVSimNucleus(const KVSimNucleus& o) : KVNucleus()
{
   o.Copy(*this);
}

void KVSimNucleus::Copy(TObject& obj) const
{

   //Copy l'object "this" vers obj

   KVNucleus::Copy(obj);
   ((KVSimNucleus&)obj).position = position;
   ((KVSimNucleus&)obj).angmom = angmom;
}

//___________________________
void KVSimNucleus::SetPosition(Double_t rx, Double_t ry, Double_t rz)
{
   //set the position of the nucleus in position space
   position.SetXYZ(rx, ry, rz);
}

void KVSimNucleus::SetPosition(const TVector3& r)
{
   //set the position of the nucleus in position space
   position = r;
}


//___________________________
void KVSimNucleus::SetAngMom(Double_t lx, Double_t ly, Double_t lz)
{
   //set the angular momentum of the nucleus
   angmom.SetXYZ(lx, ly, lz);
}

//________________________________________________________________________________________

KVSimNucleus KVSimNucleus::operator+(const KVSimNucleus& rhs)
{
   // KVNucleus addition operator.
   // Add two nuclei together to form a compound nucleus whose Z, A, momentum
   // and excitation energy are calculated from energy and momentum conservation.

   KVSimNucleus& lhs = *this;
   Int_t ztot = lhs.GetZ() + rhs.GetZ();
   Int_t atot = lhs.GetA() + ((KVNucleus&) rhs).GetA();
   KVSimNucleus CN(ztot, atot);

   Double_t etot = lhs.E() + rhs.E();
   TVector3 ptot = lhs.GetMomentum() + rhs.GetMomentum();
   CN.SetVect(ptot);
   CN.SetT(etot);


   TVector3 pos;
   const TVector3* prhs = rhs.GetPosition();
   pos.SetX(prhs->X()*rhs.GetA() + position.X()*lhs.GetA());
   pos.SetY(prhs->Y()*rhs.GetA() + position.Y()*lhs.GetA());
   pos.SetZ(prhs->Z()*rhs.GetA() + position.Z()*lhs.GetA());

   pos *= 1. / (atot);
   CN.SetPosition(pos);

   return CN;

}
//________________________________________________________________________________________

KVSimNucleus& KVSimNucleus::operator+=(const KVSimNucleus& rhs)
{
   //KVNucleus addition and assignment operator.

   KVSimNucleus temp = (*this) + rhs;
   (*this) = temp;
   return *this;
}

Double_t KVSimNucleus::GetEnergyLoss(const TString& detname) const
{
   // For particles whose detection has been simulated in a KVMultiDetArray:
   // return the energy loss in given detector
   // returns -1.0 if detector not hit

   TString parname;
   parname.Form("DE:%s", detname.Data());
   return GetParameters()->GetDoubleValue(parname);
}

TVector3 KVSimNucleus::GetEntrancePosition(const TString& detname) const
{
   // For particles whose detection has been simulated in a KVMultiDetArray:
   // returns coordinates of point of entry in detector
   // returns (0,0,0) (coordinate origin) if detector not hit
   TString parname;
   parname.Form("Xin:%s", detname.Data());
   if (GetParameters()->HasParameter(parname)) {
      Double_t x = GetParameters()->GetDoubleValue(parname);
      Double_t y = GetParameters()->GetDoubleValue(Form("Yin:%s", detname.Data()));
      Double_t z = GetParameters()->GetDoubleValue(Form("Zin:%s", detname.Data()));
      return TVector3(x, y, z);
   }
   return TVector3(0, 0, 0);
}

TVector3 KVSimNucleus::GetExitPosition(const TString& detname) const
{
   // For particles whose detection has been simulated in a KVMultiDetArray:
   // returns coordinates of point of exit from detector
   // returns (0,0,0) (coordinate origin) if detector not hit
   TString parname;
   parname.Form("Xout:%s", detname.Data());
   if (GetParameters()->HasParameter(parname)) {
      Double_t x = GetParameters()->GetDoubleValue(parname);
      Double_t y = GetParameters()->GetDoubleValue(Form("Yout:%s", detname.Data()));
      Double_t z = GetParameters()->GetDoubleValue(Form("Zout:%s", detname.Data()));
      return TVector3(x, y, z);
   }
   return TVector3(0, 0, 0);
}

void KVSimNucleus::Print(Option_t* t) const
{
   KVNucleus::Print(t);
   std::cout << "KVSimNucleus: Position : ";
   position.Print();
   std::cout << "KVSimNucleus: Ang. Mom. : ";
   angmom.Print();
}
