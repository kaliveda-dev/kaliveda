//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#include "KVZGOUBITrajectory.h"

ClassImp(KVZGOUBITrajectory)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVZGOUBITrajectory</h2>
<h4>Class used to access one ZGOUBI Trajectory</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVZGOUBITrajectory::KVZGOUBITrajectory()
   : KVBase()
{
   // Default constructor
}

//____________________________________________________________________________//

KVZGOUBITrajectory::KVZGOUBITrajectory(Float_t ThetaV_t, Float_t PhiV_t, Float_t Delta_t, Float_t XF_t, Float_t ThetaF_t, Float_t YF_t, Float_t PhiF_t, Float_t Path_t)
   : KVBase()
{
   ThetaV = ThetaV_t;
   PhiV = PhiV_t;
   Delta = Delta_t;
   XF = XF_t;
   YF = YF_t;
   ThetaF = ThetaF_t;
   PhiF = PhiF_t;
   Path = Path_t;
}

//____________________________________________________________________________//

KVZGOUBITrajectory::KVZGOUBITrajectory(const KVZGOUBITrajectory& obj) : KVBase()
{
   // Copy constructor
   // Use this constructor to copy an existing object
   obj.Copy(*this);
}

//____________________________________________________________________________//

KVZGOUBITrajectory::KVZGOUBITrajectory(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Constructor inherited from KVBase
}

//____________________________________________________________________________//

KVZGOUBITrajectory::~KVZGOUBITrajectory()
{
   // Destructor
}

//____________________________________________________________________________//

void KVZGOUBITrajectory::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVZGOUBITrajectory
   // object into 'obj'.

   KVBase::Copy(obj);
   KVZGOUBITrajectory& CastedObj = (KVZGOUBITrajectory&)obj;
   CastedObj.ThetaV = ThetaV;
   CastedObj.PhiV = PhiV;
   CastedObj.Delta = Delta;
   CastedObj.XF = XF;
   CastedObj.YF = YF;
   CastedObj.ThetaF = ThetaF;
   CastedObj.PhiF = PhiF;
   CastedObj.Path = Path;
}
//____________________________________________________________________________//

void KVZGOUBITrajectory::SetTrajectoryParameters(Float_t ThetaV_t, Float_t PhiV_t, Float_t Delta_t, Float_t XF_t, Float_t ThetaF_t, Float_t YF_t, Float_t PhiF_t, Float_t Path_t)
{
   ThetaV = ThetaV_t;
   PhiV = PhiV_t;
   Delta = Delta_t;
   XF = XF_t;
   YF = YF_t;
   ThetaF = ThetaF_t;
   PhiF = PhiF_t;
   Path = Path_t;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetThetaV()
{
   return ThetaV;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetPhiV()
{
   return PhiV;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetDelta()
{
   return Delta;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetXF()
{
   return XF;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetYF()
{
   return YF;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetThetaF()
{
   return ThetaF;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetPhiF()
{
   return PhiF;
}

//____________________________________________________________________________//

Float_t KVZGOUBITrajectory::GetPath()
{
   return Path;
}

//____________________________________________________________________________//


