/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVDBSystem.h"
#include "KVDBRun.h"
#include "KV2Body.h"
#include "KVNumberList.h"
#include "KVUnits.h"
#include "TROOT.h"
#include "KVDBKey.h"
#include "KVDBTable.h"

using namespace std;

ClassImp(KVDBSystem);
//___________________________________________________________________________
KVDBSystem::KVDBSystem()
{
   fZtarget = fAtarget = fZbeam = fAbeam = 0;
   fEbeam = 0.;
   fCinema = 0;
   fTarget = 0;
   fRunlist = 0;
   fRuns = 0;
}

//___________________________________________________________________________
KVDBSystem::KVDBSystem(const Char_t* name): KVDBRecord(name,
         "Physical System")
{
   fZtarget = fAtarget = fZbeam = fAbeam = 0;
   fEbeam = 0.;
   fCinema = 0;
   fTarget = 0;
   KVDBKey* dbk = AddKey("Runs", "List of Runs");
   dbk->SetUniqueStatus(kTRUE);
   fRunlist = 0;
   fRuns = 0;
}

//____________________________________________________________________________
KVDBSystem::~KVDBSystem()
{
   //Delete kinematics, target and associated runlist if they exist
   if (fCinema) {
      delete fCinema;
      fCinema = 0;
   }
   delete fTarget;
   if (fRunlist)
      delete fRunlist;
}

//___________________________________________________________________________

KV2Body* KVDBSystem::GetKinematics()
{
   // Create (if it doesn't already exist) and return pointer to a KV2Body object initialised
   // with the entrance channel corresponding to this system. Use this to obtain information
   // such as the recoil velocity of the CM, available energy, etc. (see KV2Body).
   //
   // If no projectile and/or target are defined for the system, we return 0x0.

   if (GetZbeam()*GetZtarget() == 0) return nullptr;

   if (!fCinema) {
      fCinema = new KV2Body();
      fCinema->SetProjectile(GetZbeam(), GetAbeam());
      fCinema->SetTarget(GetZtarget(), GetAtarget());
      fCinema->GetNucleus(1)->SetEnergy(GetEbeam() * GetAbeam());
      fCinema->CalculateKinematics();
   }
   return fCinema;
}
//___________________________________________________________________________

Bool_t KVDBSystem::IsCollision() const
{
   //retourne kTRUE, si le systeme est une collision ie projectile+cible
   return (GetZbeam() * GetZtarget() != 0);

}

//_____________________________________________________________________________

Double_t KVDBSystem::GetZVtot() const
{
   //Returns product of atomic number and velocity component parallel to beam axis of projectile nucleus in laboratory frame
   //Units are cm/ns (velocity units)
   KV2Body* kin = const_cast < KVDBSystem* >(this)->GetKinematics();
   if (!kin) return 0.;
   return (fZbeam * kin->GetNucleus(1)->GetVpar());
}

//_____________________________________________________________________________

Double_t KVDBSystem::GetPtot() const
{
   //Returns momentum component parallel to beam axis of projectile nucleus in laboratory frame
   //Units are MeV/c
   KV2Body* kin = const_cast < KVDBSystem* >(this)->GetKinematics();
   if (!kin) return 0.;
   return (kin->GetNucleus(1)->GetMomentum().Z());
}

//_____________________________________________________________________________

Double_t KVDBSystem::GetEtot() const
{
   //Returns total (mass + kinetic) energy of entrance channel corresponding to system
   //Units are MeV
   KV2Body* kin = const_cast < KVDBSystem* >(this)->GetKinematics();
   if (!kin) return 0.;
   return (kin->GetNucleus(1)->E() + kin->GetNucleus(2)->E());
}

//_____________________________________________________________________________

Double_t KVDBSystem::GetECM() const
{
   //Returns total available (CM) kinetic energy of entrance channel corresponding to system
   //Units are MeV
   KV2Body* kin = const_cast < KVDBSystem* >(this)->GetKinematics();
   if (!kin) return 0.;
   return (kin->GetCMEnergy());
}

//_____________________________________________________________________________

Int_t KVDBSystem::Compare(const TObject* obj) const
{
   //Function used to sort lists of systems.
   //Systems are sorted according to the number of the first run in the
   //(sorted) list of runs associated to the system.
   //Systems with lower first run numbers appear earlier in the list.

   if (!GetRuns())
      return 0;
   KVDBSystem* other_sys =
      dynamic_cast < KVDBSystem* >(const_cast < TObject* >(obj));
   if (!other_sys)
      return 0;
   KVList* other_runs;
   if (!(other_runs = other_sys->GetRuns()))
      return 0;
   Int_t first = ((KVDBRecord*) fRunlist->At(0))->GetNumber();
   Int_t other_first = ((KVDBRecord*) other_runs->At(0))->GetNumber();
   return (first == other_first ? 0 : (other_first > first ? -1 : 1));
}

//_____________________________________________________________________________

KVList* KVDBSystem::_GetRuns()
{
   //"translate" the KVRList returned by GetLinks into a standard
   //TList which can then be sorted (Sort() is not implemented for TRefArray).
   KVRList* _rlist = GetLinks("Runs");
   TIter nxt(_rlist);
   KVDBRun* db;
   SafeDelete(fRunlist);
   fRunlist = new KVList(kFALSE);        //will be deleted with object
   while ((db = (KVDBRun*) nxt()))
      fRunlist->Add(db);
   fRunlist->Sort();
   return fRunlist;
}

//___________________________________________________________________________

void KVDBSystem::GetRunList(KVNumberList& list) const
{
   //Fills the KVNumberList object with the list of all run numbers associated with this system
   list.Clear();
   TIter next(GetRuns());
   KVDBRun* run;
   while ((run = (KVDBRun*)next())) {
      list.Add(run->GetNumber());
   }
}

//___________________________________________________________________________

void KVDBSystem::Save(ostream& f) const
{
   //Write informations on system in the format used in Systems.dat files:
   //
   //+155Gd + 238U 36 MeV/A   '+'  followed by name of system
   //155 64 238 92 36.0    Aproj Zproj Atarg Ztarg Ebeam
   //Target: 3 0.0                    target with 3 layers, angle 0 degrees
   //C 0.02                            1st layer : carbon, 20 g/cm2
   //238U 0.1                          2nd layer : uranium-238, 100 g/cm2
   //C 0.023                           3rd layer : carbon, 23 g/cm2
   //Runs: 770-804             list of runs in KVNumberList format

   f << "+" << GetName() << endl;
   if (fZbeam) f << fAbeam << " " << fZbeam << " " << fAtarget << " " << fZtarget << " " << fEbeam << endl;
   if (fTarget) {
      f << "Target: " << fTarget->NumberOfLayers() << " " <<  fTarget->GetAngleToBeam() << endl;
      TIter next(fTarget->GetLayers());
      KVMaterial* lay;
      while ((lay = (KVMaterial*)next())) {
         if (lay->IsIsotopic()) f << Form("%d%s", (Int_t)lay->GetMass(), lay->GetType());
         else f << lay->GetType();
         f << " " << lay->GetAreaDensity() / KVUnits::mg << endl;
      }
   }
   KVNumberList runlist;
   GetRunList(runlist);
   f << "Runs: " << runlist.AsString() << endl;
}

//___________________________________________________________________________

void KVDBSystem::Load(istream& f)
{
   //Read and set informations on system in the format used in Systems.dat files:
   //
   //+155Gd + 238U 36 MeV/A   '+'  followed by name of system
   //155 64 238 92 36.0    Aproj Zproj Atarg Ztarg Ebeam
   //Target: 3 0.0                    target with 3 layers, angle 0 degrees
   //C 0.02                            1st layer : carbon, 20 ug/cm2
   //238U 0.1                          2nd layer : uranium-238, 100 ug/cm2
   //C 0.023                           3rd layer : carbon, 23 ug/cm2
   //Runs: 770-804             list of runs in KVNumberList format

   TString line;
   Float_t target_thickness;
   fAbeam = fZbeam = fAtarget = fZtarget = 0;
   fEbeam = target_thickness = 0;
   KVNumberList runlist;
   //'peek' at first character of next line
   char next_char = f.peek();
   if (next_char == '+') {
      line.ReadLine(f, kFALSE);
      line.Remove(0, 1);
      SetName(line.Data());
      cout << "New System : " << line.Data() << endl;
   }
   else {
      Error("Load", "Should read system name : %s\n", line.Data());
      return;
   }
   next_char = f.peek();
   while (next_char != '+' && f.good() && !f.eof()) {
      if ((next_char >= '0') && (next_char <= '9')) {
         line.ReadLine(f, kFALSE);
         if (sscanf(line.Data(), "%u %u %u %u %f %f", &fAbeam, &fZbeam, &fAtarget, &fZtarget, &target_thickness, &fEbeam) == 6) {
            cout << "Zproj = " << fZbeam << " Ztarg = " << fZtarget << " targ_thick = " << target_thickness << " Ebeam = " << fEbeam << endl;
         }
         else if (sscanf(line.Data(), "%u %u %u %u %f", &fAbeam, &fZbeam, &fAtarget, &fZtarget, &fEbeam) == 5) {
            cout << "Zproj = " << fZbeam << " Ztarg = " << fZtarget << " Ebeam = " << fEbeam << endl;
         }
      }
      else {
         line.ReadLine(f, kFALSE);
         if (line.BeginsWith("Target")) {
            fTarget = new KVTarget;
            line.Remove(0, line.Index(":") + 1);
            Int_t nlay;
            Float_t angle;
            sscanf(line.Data(), "%d %f", &nlay, &angle);
            Char_t mat[10];
            Float_t thick;
            for (int i = 0; i < nlay; i++) {
               line.ReadLine(f);
               sscanf(line.Data(), "%s %f", mat, &thick);
               fTarget->AddLayer(mat, thick);
            }
            fTarget->SetAngleToBeam(angle);
            fTarget->Print();
         }
         else if (line.BeginsWith("Runs")) {
            line.Remove(0, line.Index(":") + 1);
            runlist.SetList(line.Data());
            cout << "Runs : " << line.Data() << endl;
         }
         else if (line.BeginsWith("Run Range")) {
            line.Remove(0, line.Index(":") + 1);
            Int_t frun, lrun;
            sscanf(line.Data(), "%i %i", &frun, &lrun);
            runlist.Add(Form("%i-%i", frun, lrun));
            cout << "Run range : " << line.Data() << endl;
         }
      }
      next_char = f.peek();
   }
   if (runlist.GetNValues()) {
      SetRuns(runlist);
   }
   //set target if not already done (old versions)
   if (!fTarget && target_thickness > 0 && fZtarget > 0) {
      KVNucleus n(fZtarget, fAtarget);
      fTarget = new KVTarget(n.GetSymbol(), target_thickness);
      fTarget->Print();
   }
}

//___________________________________________________________________________

void KVDBSystem::SetRuns(KVNumberList& rl)
{
   //Associate this system with the runs in the list
   //Any previously associated runs are first removed (links in the runs will be removed too)
   Info("SetRuns", "Setting runs for system %s : %s", GetName(), rl.AsString());
   RemoveAllRuns();
   rl.Begin();
   KVDBTable* runtable = GetRunsTable();
   Int_t run_number;
   while (!rl.End()) {
      run_number = rl.Next();
      KVDBRun* run = (KVDBRun*)runtable->GetRecord(run_number);
      if (run) {
         if (run->GetSystem()) {
            Error("SetRuns", "Associating run %d with system \"%s\" : run already associated with system \"%s\"",
                  run_number, GetName(), run->GetSystem()->GetName());
         }
         if (AddLink("Runs", run)) {
            //use name of system as title of run
            run->SetTitle(GetName());
         }
         else {
            Info("SetRuns", "Could not add link for run %d", run_number);
         }
      }
      else {
         //Info("SetRuns", "Run %d not found in database", run_number);
      }
   }
}

//___________________________________________________________________________

void KVDBSystem::AddRun(KVDBRecord* rec)
{
   //Associate the given run with this system.
   //If the run was previously associated with another system, this association
   //will be removed.
   if (!rec) return;
   if (!rec->InheritsFrom("KVDBRun")) {
      Error("AddRun", "Called with pointer to an object of class %s; should inherit from KVDBRun!",
            rec->ClassName());
      return;
   }
   KVDBRun* run = (KVDBRun*)rec;
   if (run->GetSystem()) run->GetSystem()->RemoveRun(run);
   if (AddLink("Runs", run)) {
      //Info("AddRun", "Added link for run %d", run->GetNumber());
      //use name of system as title of run
      run->SetTitle(GetName());
   }
   else {
      Info("AddRun", "Could not add link for run %d", run->GetNumber());
   }
}

//___________________________________________________________________________

void KVDBSystem::AddRun(Int_t run)
{
   //Associate the given run with this system.
   //If the run was previously associated with another system, this association
   //will be removed.
   AddRun(GetRunsTable()->GetRecord(run));
}

//___________________________________________________________________________

void KVDBSystem::RemoveRun(KVDBRecord* run)
{
   //Unassociate the given run from this system. Cross-reference link to this system
   //is removed from the run at the same time.
   RemoveLink("Runs", run);
}

//___________________________________________________________________________

void KVDBSystem::RemoveRun(Int_t run)
{
   //Unassociate the given run from this system. Cross-reference link to this system
   //is removed from the run at the same time.
   RemoveRun(GetRunsTable()->GetRecord(run));
}

//___________________________________________________________________________

void KVDBSystem::RemoveAllRuns()
{
   //Unassociate all runs from this system. Cross-reference links to this system
   //are removed from the runs at the same time.
   RemoveAllLinks("Runs");
}

void KVDBSystem::Print(Option_t*) const
{
   cout << "________________________________________________________" <<
        endl << "System : " << GetName() << endl;
   KVNumberList r;
   GetRunList(r);
   cout << "Runs : " << r.AsString() << endl;
   cout << "  Zbeam : " << fZbeam
        << endl << "  Abeam : " << fAbeam << endl << "  Ebeam : " << fEbeam
        << " A.MeV" << endl << "  Ztarget : " << fZtarget << endl <<
        "  Atarget : " << fAtarget << endl << "  Target Thickness : " <<
        const_cast <
        KVDBSystem*
        >(this)->
        GetTargetThickness() << " mg/cm2" << endl <<
        "________________________________________________________" << endl;

}

void KVDBSystem::ls(Option_t*) const
{
   KVNumberList r;
   GetRunList(r);
   cout << "KVDBSystem : " << GetName() << " Runs : " << r.AsString() << endl;
}

KVDBTable* KVDBSystem::GetRunsTable()
{
   //Deduce path to runs table in database from full path to parent table of this record.
   //The systems are stored in a table called "Systems"
   //The runs are stored in a table called "Runs"
   //Therefore if we take the full path to the Systems table and replace Systems with Runs,
   //we can then use gROOT->FindObject to get the pointer to the Runs table.

   TString path = fFullPathTable.Data();
   path.ReplaceAll("Systems", "Runs");
   return (KVDBTable*)gROOT->FindObject(path.Data());
}

//__________________________________________________________________________________//

const Char_t* KVDBSystem::GetBatchName()
{
   // Gives name of system in compact form with all (unix-)illegal characters
   // replaced by '_'. Can be used for naming batch jobs, files, etc.

   static KVString tmp;
   tmp = "";
   if (GetKinematics()) {
      if (GetKinematics()->GetNucleus(1)) {
         tmp = GetKinematics()->GetNucleus(1)->GetSymbol();
      }
      if (GetKinematics()->GetNucleus(2)) {
         tmp += GetKinematics()->GetNucleus(2)->GetSymbol();
      }
      if (GetEbeam() > 0) {
         tmp += TMath::Nint(GetEbeam());
      }
   }
   if (tmp == "") {
      tmp = GetName();
      tmp.ReplaceAll(" ", "_");
      tmp.ReplaceAll("/", "_");
   }
   return tmp.Data();
}

