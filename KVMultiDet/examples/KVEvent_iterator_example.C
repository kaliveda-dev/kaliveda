#include <iostream>
#include <algorithm>
#include "TRandom.h"
#include "KVReconstructedEvent.h"
using std::cout;
using std::endl;

bool compareZ(KVNucleus& a, KVNucleus& b)
{
   return a.GetZ() < b.GetZ();
}

void nucleus_event_iterator(KVEvent* e_ptr)
{
#ifdef WITH_CPP11
   for (auto& n : OKEventIterator(e_ptr)) {
      n.Print();
   }
#else
   for (KVNucleusEvent::Iterator it = OKEventIterator(e_ptr).begin(); it != KVNucleusEvent::Iterator::End(); ++it) {
      (*it).Print();
   }
#endif
}

void recon_event_iterator(KVEvent* e_ptr)
{
#ifdef WITH_CPP11
   for (auto& n : KVReconstructedEvent::OKEventIterator(e_ptr)) {
      n.Print();
   }
#else
   for (KVReconstructedEvent::Iterator it = KVReconstructedEvent::OKEventIterator(e_ptr).begin(); it != KVReconstructedEvent::Iterator::End(); ++it) {
      (*it).Print();
   }
#endif
}

void iterator_examples()
{
   KVNucleusEvent Event;
   for (int i = 0; i < 10; ++i) {
      KVNucleus* n = Event.AddParticle();
      n->SetZ(i);
      n->SetIsOK(i % 2);
      if (i > 4) n->AddGroup("GROUP");
      if (gRandom->Uniform() > .5) n->AddGroup("RANDOM");
   }

   cout << "Loop over all particles (0-9):" << endl;
   for (KVNucleusEvent::Iterator it = Event.begin(); it != Event.end(); ++it) {
      (*it).Print();
   }

   cout << "\nNested loops over N*(N-1)/2 pairs of particles:" << endl;
   for (KVNucleusEvent::Iterator it = Event.begin(); it != Event.end(); ++it) {
      KVNucleusEvent::Iterator it2(it);
      for (++it2; it2 != Event.end(); ++it2) {
         cout << (*it).GetZ() << "-" << (*it2).GetZ() << " ";
      }
      cout << endl;
   }

#ifdef WITH_CPP11
   cout << "\nLoop over all particles (0-9) [range-based for loop]:" << endl;
   for (auto& nuc : Event) {
      nuc.Print();
   }
#endif

   cout << "\nLoop over OK particles (1,3,5,7,9):" << endl;
   for (KVNucleusEvent::Iterator it = OKEventIterator(Event).begin(); it != Event.end(); ++it) {
      (*it).Print();
   }

   cout << "\nLoop over GROUP particles (5,6,7,8,9):" << endl;
   for (KVNucleusEvent::Iterator it = GroupEventIterator(Event, "GROUP").begin(); it != Event.end(); ++it) {
      (*it).Print();
   }

   cout << "\nPerform two different iterations with the same iterator" << endl;
   cout << "\n1.) Loop over OK particles (1,3,5,7,9):" << endl;
#ifdef WITH_CPP11
   KVNucleusEvent::Iterator iter(Event, KVNucleusEvent::Iterator::Type::OK);
#else
   KVNucleusEvent::Iterator iter(Event, KVNucleusEvent::Iterator::OK);
#endif
   for (; iter != KVNucleusEvent::Iterator::End(); ++iter) {
      (*iter).Print();
   }

   cout << "\n2.) Loop over GROUP particles (5,6,7,8,9):" << endl;
#ifdef WITH_CPP11
   iter.Reset(KVNucleusEvent::Iterator::Type::Group, "GROUP");
#else
   iter.Reset(KVNucleusEvent::Iterator::Group, "GROUP");
#endif
   for (; iter != KVNucleusEvent::Iterator::End(); ++iter) {
      (*iter).Print();
   }

#ifdef WITH_CPP11
   cout << "\nLoop over RANDOM particles [range-based for loop]:" << endl;
   for (auto& nuc : GroupEventIterator(Event, "RANDOM")) {
      nuc.Print();
   }
   cout << "\nLoop over OK particles [range-based for loop]:" << endl;
   for (auto& nuc : OKEventIterator(Event)) {
      nuc.Print();
   }
#endif

   cout << "\nSearch using algorithm std::find:" << endl;
   KVNucleus boron;
   boron.SetZ(5);
#if !defined(__ROOTCINT__) && !defined(__ROOTCLING__)
   KVNucleusEvent::Iterator found = std::find(Event.begin(), Event.end(), boron);
   (*found).Print();
#endif

#if !defined(__APPLE__)
   cout << "\nFind largest Z in RANDOM group using std::max_element:" << endl;
   KVNucleusEvent::GroupEventIterator it(Event, "RANDOM");
   KVNucleusEvent::Iterator maxZ = std::max_element(it.begin(), it.end(), compareZ);
   (*maxZ).Print();
#endif

   cout << "\nLoop over OK particles using base pointer (KVEvent*):" << endl;
   KVEvent* _eptr = &Event;
   cout << "\n1.) KVNucleusEventIterator with KVNucleusEvent" << endl;
   nucleus_event_iterator(_eptr);
   cout << "\n2.) KVReconstructedEventIterator with KVNucleusEvent [Warning]" << endl;
   recon_event_iterator(_eptr);
   KVReconstructedEvent recev;
   recev.AddParticle()->SetZ(10);
   _eptr = &recev;
   cout << "\n3.) KVNucleusEventIterator with KVReconstructedEvent" << endl;
   nucleus_event_iterator(_eptr);
   cout << "\n4.) KVReconstructedEventIterator with KVReconstructedEvent" << endl;
   recon_event_iterator(_eptr);

   cout << "\nLoop over all particles (0-9) [GetNextParticle]:" << endl;
   KVNucleus* n;
   while ((n = Event.GetNextParticle())) {
      n->Print();
   }

   cout << "\nInterrupted iteration restarted with different criteria [GetNextParticle]:" << endl;
   cout << "\n1.) Loop over OK particles with Z<=5 (1,3,5):" << endl;
   while ((n = Event.GetNextParticle("ok"))) {
      if (n->GetZ() > 5) break;
      n->Print();
   }
   Event.ResetGetNextParticle();
   cout << "\n2.) Loop over GROUP particles (5,6,7,8,9):" << endl;
   while ((n = Event.GetNextParticle("GROUP"))) {
      n->Print();
   }

   cout << "\nKVEvent::Print():" << endl;
   Event.Print();
   cout << "\nKVEvent::Print(\"ok\"): (1,3,5,7,9)" << endl;
   Event.Print("ok");
   cout << "\nKVEvent::Print(\"group\"): (5,6,7,8,9)" << endl;
   Event.Print("group");

   cout << "\nKVEvent::GetParticle(\"group\"): (5)" << endl;
   Event.GetParticle("group")->Print();

   cout << "\nKVEvent::GetParticle(\"unknown_group\"): (error)" << endl;
   KVNucleus* nuc = Event.GetParticle("unknown_group");
   if (nuc) nuc->Print();

   cout << "\nKVEvent::GetMult(\"ok\"): (5)" << endl;
   cout << Event.GetMult("ok") << endl;
   cout << "\nKVEvent::GetMult(\"group\"): (5)" << endl;
   cout << Event.GetMult("group") << endl;

   cout << "\nKVEvent::GetSum(\"GetZ\"): (45)" << endl;
   cout << Event.GetSum("GetZ") << endl;
   cout << "\nKVEvent::GetSum(\"GetZ\",\"group\"): (35)" << endl;
   cout << Event.GetSum("GetZ", "group") << endl;
   cout << "\nKVEvent::GetSum(\"GetZ\",\"ok\"): (25)" << endl;
   cout << Event.GetSum("GetZ", "ok") << endl;

   cout << "\nKVEvent::FillHisto(h,\"GetZ\",\"ok\"):" << endl;
   TH1F* h = new TH1F("h", "KVEvent::GetZ() for \"ok\" particles", 10, -.5, 9.5);
   Event.FillHisto(h, "GetZ", "ok");
   h->Draw();
}

