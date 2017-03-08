//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#include "KVZGOUBIInversePosition.h"

ClassImp(KVZGOUBIInversePosition)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVZGOUBIInversePosition</h2>
<h4>Class used to access ZGOUBI Trajectories in KVZGOUBIInverseMatrix</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVZGOUBIInversePosition::KVZGOUBIInversePosition()
   : KVBase()
{
   // Default constructor
}

//____________________________________________________________________________//

KVZGOUBIInversePosition::KVZGOUBIInversePosition(const KVZGOUBIInversePosition& obj) : KVBase()
{
   // Copy constructor
   // Use this constructor to copy an existing object
   obj.Copy(*this);
}

//____________________________________________________________________________//

KVZGOUBIInversePosition::KVZGOUBIInversePosition(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Constructor inherited from KVBase
}

//____________________________________________________________________________//

KVZGOUBIInversePosition::~KVZGOUBIInversePosition()
{
   // Destructor
}

//____________________________________________________________________________//

void KVZGOUBIInversePosition::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVZGOUBIInversePosition
   // object into 'obj'.

   KVBase::Copy(obj);
   KVZGOUBIInversePosition& CastedObj = (KVZGOUBIInversePosition&)obj;
   CastedObj.InversePositionVector = InversePositionVector;

}
//____________________________________________________________________________//

void KVZGOUBIInversePosition::AddtoInversePositionVector(Int_t TrajectoryPosition)
{
   InversePositionVector.push_back(TrajectoryPosition);
}

//____________________________________________________________________________//

Int_t KVZGOUBIInversePosition::GetInversePositionVectorValue(Int_t vectorposition)
{
   if (vectorposition < 0 || vectorposition >= (Int_t) InversePositionVector.size()) {
      return -1;
   } else {
      return InversePositionVector[vectorposition];
   }
}

Bool_t KVZGOUBIInversePosition::GetInversePositionVectorStatus()
{
   if (InversePositionVector.size() == 0) {
      return false;
   } else {
      return true;
   }
}

Int_t KVZGOUBIInversePosition::GetInversePositionVectorSize()
{
   return (Int_t) InversePositionVector.size();
}
