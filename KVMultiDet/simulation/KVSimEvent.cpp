//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#include "KVSimEvent.h"

#include <KVSimNucleus.h>
using namespace std;

ClassImp(KVSimEvent)


void KVSimEvent::Print(Option_t* t) const
{
   //Print a list of all particles in the event with some characteristics.
   //Optional argument t can be used to select particles (="ok", "groupname", ...)

   cout << "\nKVSimEvent with " << GetMult(t) << " particles :" << endl;
   cout << "------------------------------------" << endl;
   fParameters.Print();
   KVSimNucleus* frag = 0;
   ResetGetNextParticle();
   while ((frag = GetNextParticle(t))) {
      frag->Print();
      cout << "   Position: (" << frag->GetPosition().x() << "," << frag->GetPosition().y() << "," << frag->GetPosition().z() << ")" << endl;
   }

}

Double_t KVSimEvent::GetTotalCoulombEnergy() const
{
   // Calculate total Coulomb potential energy of all particles in event
   // Units are MeV.

   Double_t Vtot = 0;
   for (KVSimEvent::Iterator it = begin(); it != end(); ++it) {
      KVSimEvent::Iterator it2(it);
      for (++it2; it2 != end(); ++it2) {
         TVector3 D12 = (*it).GetPosition() - (*it2).GetPosition();
         Vtot += (*it).GetZ() * (*it2).GetZ() * KVNucleus::e2 / D12.Mag();
      }
   }
   return Vtot;

}
void KVSimEvent::Streamer(TBuffer& R__b)
{
   //Stream an object of class KVReconstructedEvent.
   //We set the particles' angles depending on whether mean or random angles
   //are wanted (fMeanAngles = kTRUE or kFALSE)

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      UInt_t R__v  = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 5) {
         // all versions written with automatic streamer
         // (version is not read correctly from TTree branches)
         Clear();
         KVBase::Streamer(R__b);
         fParticles->Streamer(R__b);
         fParameters.Streamer(R__b);
      }
      else
         R__b.ReadClassBuffer(KVSimEvent::Class(), this, R__v, R__s, R__c);
   }
   else {
      R__b.WriteClassBuffer(KVSimEvent::Class(), this);
   }
}
