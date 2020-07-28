//Created by KVClassFactory on Thu Oct 11 11:51:14 2012
//Author: Dijon Aurore

#include "KVRTGIDManager.h"
#include "KVTGIDZ.h"
#include "KVTGIDGrid.h"
#include "KVIDGridManager.h"
#include "KVMultiDetArray.h"
#include "KVIDTelescope.h"
using namespace std;

ClassImp(KVRTGIDManager)




KVList* KVRTGIDManager::fIDGlobalList = NULL;

KVRTGIDManager::KVRTGIDManager()
{
   // Default constructor
   // The list fIDList is not owner anymore.
   // Now the identification function belong to fIDGlobalList,
   // the list return by GetListOfIDFunctions() is not the owner
   // of identification functions anymore.

   fIDList.SetOwner(kFALSE);

}
//________________________________________________________________

KVRTGIDManager::~KVRTGIDManager()
{
   // Destructor
   // Remove the global list of identification functions if
   // it is empty.

   if (fIDGlobalList && !fIDGlobalList->GetEntries()) SafeDelete(fIDGlobalList);
}
//________________________________________________________________

void KVRTGIDManager::AddTGID(KVTGID* _tgid)
{
   // Add an identification object to the global list (fIDGlobalList).
   // The identification object is not added to the local list (fIDList) in this method anymore. To do that, see SetIDFunctionInTelescopes)
   // This method just calls AddTGIDToGlobalList(KVTGID *);

   AddTGIDToGlobalList(_tgid);
}
//________________________________________________________________

KVTGID* KVRTGIDManager::GetTGID(const Char_t*,
                                const Char_t* id_type,
                                const Char_t* grid_type)
{
   //Overrides the same method of KVTGIDManager. This method
   //Retrieve the identification object using:
   //      id_type   = type of identification ("Z", "A", "A_for_Z=3", etc.)
   //      grid_type = type of identification grid ("GG", "PG1", etc.)
   // but the name of ID telescope is not used anymore because identification
   // object can be associated to several ID telescopes.
   // If any object is found then it returns the first:
   //      - KVTGIDZA object if id_type contains "A"
   //      - KVTGIDZ  object else.

   Char_t  cname[9] = "KVTGIDZ";
   TString buff     = id_type;

   if (buff.Contains("A")) strcpy(cname, "KVTGIDZA");

   buff.Form("_%s_%s", id_type, grid_type);

   TIter next(&fIDList);
   KVTGID* tgid  = NULL;
   KVTGID* tgid2 = NULL;
   TString name;

   while ((tgid = (KVTGID*)next())) {
      name = tgid->GetName();
      if (name.Contains(buff)) return tgid;
      if (!tgid2 && !strcmp(tgid->ClassName(), cname)) tgid2 = tgid;
   }

   return tgid2;
}
//________________________________________________________________

void KVRTGIDManager::SetTGID(KVTGID* _tgid)

{
   // Add an identification object to the local list (fIDList).
   KVTGIDManager::AddTGID(_tgid);
}
//________________________________________________________________

void KVRTGIDManager::RemoveTGID(const Char_t*)
{
   //Remove the identification object with name from the local list.

   KVTGID* tgid = 0;
   // Remove the function from the current list
   if (!(tgid = (KVTGID*)fIDList.FindObject(tgid))) return;
   fIDList.Remove(tgid);
   //recalculate fID_max if needed
   if (tgid->GetIDmax() == fID_max) {
      fID_max = 0.;
      TIter n(&fIDList);
      KVTGID* t;
      while ((t = (KVTGID*)n())) fID_max = TMath::Max(fID_max, t->GetIDmax());
   }
}
//________________________________________________________________

void KVRTGIDManager::DeleteTGID(const Char_t* name)
{
   if (!fIDGlobalList) return;

   //Delete the identification object with name from the global list.
   KVTGID* tgid = 0;

   // Delete the function from the global list
   if (!(tgid = (KVTGID*)fIDGlobalList->FindObject(name))) return;
   fIDGlobalList->Remove(tgid);
   delete tgid;
   if (!fIDGlobalList->GetEntries()) SafeDelete(fIDGlobalList);
}
//________________________________________________________________

Bool_t KVRTGIDManager::GetTGIDfromIDGridManager(TList* tgid_list)
{
   // Fill a sublist of TGID found in object inheriting
   // from KVTGIDGrid in gIDGridManager.
   // Return kFALSE if gIDGridManager is not defined.

   if (!gIDGridManager) return kFALSE;

   tgid_list->SetOwner(kFALSE);
   KVList* grid_list = NULL;
   KVIDGridManager* gm = gIDGridManager;
   grid_list = gm->GetGrids();
   TIter  next_g(grid_list);
   TObject* obj  = NULL;
   KVTGID*  tgid = NULL;
   while ((obj = next_g())) {
      if (obj->InheritsFrom("KVTGIDGrid")) {
         tgid = const_cast <KVTGID*>(((KVTGIDGrid*)obj)->GetTGID());
         if (tgid)
            tgid_list->Add(tgid);
      }
   }
   return kTRUE;
}
//________________________________________________________________

Bool_t KVRTGIDManager::ReadAsciiFile(const Char_t* filename)
{

   //Read file, create KVTGID fits corresponding to information in file.
   //Comment line beginning by '#' is skipped.

   ifstream fitfile(filename);
   if (!fitfile.good()) {
      Error("KVRTGIDManager::ReadAsciiFile", "File %s cannot be opened", filename);
      return kFALSE;
   }
   KVString s, cname;
   while (fitfile.good()) {
      //read a line
      s.ReadLine(fitfile);

      if (s.BeginsWith("#")) continue;
      if (s.BeginsWith("++")) {
         //New fit
         //Get name of class by stripping off the '+' at the start of the line
         s.Remove(0, 2);
         cname = s;
         cname.Remove(s.Index("::"));
         TClass* cl = TClass::GetClass(cname.Data());
         if (cl && cl->InheritsFrom("KVTGID")) {

            s.Remove(0, s.Index("::") + 2);

            // Make new identification function
            KVTGID* fit = NULL;
            fit = KVTGID::ReadFromAsciiFile(s.Data(), fitfile);
            AddTGIDToGlobalList(fit);

            // when mass identification is possible, make a copy
            // for Z identification
            if (!fit->GetZorA()) {
               KVTGIDZ* fitz = new KVTGIDZ(*fit);
               fitz->SetTitle(Form("COPY from %p", fit));
               AddTGIDToGlobalList(fitz);
            }
         }
      }
   }
   fitfile.close();
   return kTRUE;
}
//________________________________________________________________

void KVRTGIDManager::RemoveAllTGID()
{
   //Remove all identification objects of the local list.
   fIDList.Clear();
   fID_max = 0.;
}
//________________________________________________________________

void KVRTGIDManager::AddTGIDToGlobalList(KVTGID* tgid)
{
// Add an identification object to the global list (fIDGlobalList).

   if (!tgid) return;
   if (!fIDGlobalList) {
      fIDGlobalList = new KVList;
      fIDGlobalList->SetCleanup();
   }
   fIDGlobalList->Add(tgid);
}
//________________________________________________________________

void KVRTGIDManager::BuildGridForAllTGID(const Char_t* idtype, Double_t xmin, Double_t xmax, Int_t ID_min, Int_t ID_max, Int_t npoints, Bool_t logscale)
{
   // Build a grid (KVTGIDGrid) for all the identification functions
   // of the global list. The new grids are automatically loaded in
   // gIDGridManager and are visible in the Grid Manager GUI.
   // If a function is already associated to a grid then a new grid
   // is not built. No grid is built for copies of KVTGID's made in
   // the method ReadAsciiFile(...).
   //
   // Inputs:  idtype - type of the identification for which the
   //                   grids will be built (CI-SI, SI-CSI, CI-CSI,
   //                   SI75-SILI, ...). By default, all grids are
   //                   built
   //          xmin
   //          xmax
   //          ID_min
   //          ID_max
   //          npoints
   //          logscale - see KVTGIDGrid::Generate(...)

   if (!fIDGlobalList) return;

   // First make a sublist of TGID found in object inheriting
   // from KVTGIDGrid in gIDGridManager
   TList  tgid_list;
   GetTGIDfromIDGridManager(&tgid_list);
   KVIDGridManager* gm = gIDGridManager;
   KVList* grid_list = NULL;
   if (gm) {
      grid_list = gm->GetGrids();
      grid_list->Disconnect("Modified()", gm, "Modified()");
   }
   // If the TGID of the global list is not in the sublist then
   // build grid
   TIter next(fIDGlobalList);
   Bool_t IDtypeOK = strcmp(idtype, "");
   KVTGID* tgid  = NULL;
   while ((tgid = (KVTGID*)next())) {
      if (tgid_list.FindObject(tgid)) continue;

      if (IDtypeOK) {
         KVBase* idt  = NULL;
         TSeqCollection* idt_list = (TSeqCollection*)GetIDTelescopesForTGID(tgid);
         if (!idt_list) continue;
         if (!(idt = (KVBase*)idt_list->First())) continue;
         SafeDelete(idt_list);
         if (strcmp(idtype, idt->GetLabel())) continue;
      }
      // Not built grid for a KVTGID copy
      TString tmp = tgid->GetTitle();
      if (tmp.Contains("COPY")) {
         tmp.Remove(0, tmp.Index("0x"));
         KVTGID* tmp_tgid = reinterpret_cast<KVTGID*>((Int_t)tmp.Atof());
         Warning("KVRTGIDManager::BuildGridForAllTGID", "No grid built for %s (%s, %p) because it is a copy of %s (%s, %p)"
                 , tgid->GetName(), tgid->ClassName(), tgid
                 , tmp_tgid->GetName(), tmp_tgid->ClassName(), tmp_tgid);
         continue;
      }
      KVTGIDGrid* grid = new KVTGIDGrid(tgid);
      grid->SetOnlyZId((Bool_t)tgid->GetZorA());
      if (tgid->GetZorA()) grid->SetMassFormula(tgid->GetMassFormula());
      grid->Generate(xmax, xmin, ID_min, ID_max, npoints, logscale);
      Info("KVRTGIDManager::BuildGridForAllTGID", "grid built from its TGID function %s (%s, %p)"
           , tgid->GetName(), tgid->ClassName(), tgid);
   }
   if (grid_list) grid_list->Connect("Modified()", "KVIDGridManager", gm, "Modified()");
   gm->Modified();
}
//________________________________________________________________

void KVRTGIDManager::Clear()
{
   //Delete the global list of identification objects. Since this
   //list is owner then all listed identification objects are deleted.

   SafeDelete(fIDGlobalList);
}
//________________________________________________________________

void KVRTGIDManager::SetIDFuncInTelescopes(UInt_t run)
{
   // For each identification function of the global list which is valid for this run, we add it in the associated telescope which
   // have to inherits from KVTGIDManager and from a KVIDTelescope.

   TIter next(fIDGlobalList);
   KVTGID* tgid = NULL;

   while ((tgid = (KVTGID*)next())) {

      if (!tgid->IsValidForRun(run)) continue;

      // this list have to be deleted after use
      TCollection* lidtel = GetIDTelescopesForTGID(tgid);
      TIter nextidt(lidtel);
      TObject*        idt   = NULL;
      KVRTGIDManager* tgidm = NULL;

      while ((idt = nextidt())) {
         if (!idt->InheritsFrom("KVRTGIDManager")) {
            Error("KVRTGIDManager::SetIDFuncInTelescopes", "The IDtelescope %s does not inherit from KVRTGIDManager", idt->GetName());
            continue;
         }
         tgidm = (KVRTGIDManager*)idt->IsA()->DynamicCast(KVRTGIDManager::Class(), idt);
         tgidm->SetTGID(tgid);
      }
      delete lidtel;
   }
}
//________________________________________________________________

Int_t KVRTGIDManager::UpdateListFromIDGridManager()
{
   // Returns the number of identification functions found
   // in grids of gIDGridManager which were missing in the
   // the global list and updates this list.

   TList tgid_list;
   if (!GetTGIDfromIDGridManager(&tgid_list)) return 0;

   // If the TGID of tgid_list is not in the global list then
   // add it
   TIter next(&tgid_list);
   KVTGID* tgid  = NULL;
   Int_t ntgid   = 0;
   while ((tgid = (KVTGID*)next())) {
      if (!fIDGlobalList) {
         AddTGIDToGlobalList(tgid);
         ntgid++;
         continue;
      }
      if (fIDGlobalList->FindObject(tgid)) continue;
      AddTGIDToGlobalList(tgid);
      ntgid++;
   }
   return ntgid;
}
//________________________________________________________________

Int_t KVRTGIDManager::WriteAsciiFile(const Char_t* filename, const TCollection* selection, Bool_t update)
{
   // Write identification functions  in file 'filename'.
   // If selection=0 (default), write all grids.
   // If update=true, call UpdateListFromIDGridManager() before writing
   // If selection!=0, write only grids in list.
   // Returns number of functions written in file.

   if (update) UpdateListFromIDGridManager();
   if (!fIDGlobalList) {
      Warning("KVRTGIDManager::WriteAsciiFile", "No listed identification functions to write");
      return 0;
   }

   ofstream tgidfile(filename);
   if (!tgidfile.is_open()) {
      Error("KVRTGIDManager::WriteAsciiFile", "No write permission for file %s", filename);
      return 0;
   }

   const TCollection* list_tgid = (selection ? selection : fIDGlobalList);
   TIter next(list_tgid);
   KVTGID* tgid    = NULL;
   Int_t   n_saved = 0;

   while ((tgid = (KVTGID*)next())) {

      // Not write a KVTGID copy
      TString tmp = tgid->GetTitle();
      if (tmp.Contains("COPY")) {
         tmp.Remove(0, tmp.Index("0x"));
         KVTGID* tmp_tgid = reinterpret_cast<KVTGID*>((Int_t)tmp.Atof());
         Warning("KVRTGIDManager::WriteAsciiFile", "The function %s (%s, %p) is not written because it is a copy of %s (%s, %p)"
                 , tgid->GetName(), tgid->ClassName(), tgid
                 , tmp_tgid->GetName(), tmp_tgid->ClassName(), tmp_tgid);
         continue;
      }
      tgid->WriteToAsciiFile(tgidfile);
      Info("KVRTGIDManager::WriteAsciiFile", "%s (%s, %p) saved", tgid->GetName(), tgid->ClassName(), tgid);
      n_saved++;
   }

   tgidfile.close();
   return n_saved;
}

TCollection* KVRTGIDManager::GetIDTelescopesForTGID(KVTGID* tgid)
{
   // Return TList of all ID telescopes for which fit is valid.
   // Returns 0 if no telescopes are defined, or if gMultiDetArray object is not defined
   // (we search for the telescopes using their name and gMultiDetArray->GetIDTelescope()).
   // WARNING: DELETE this list after use !!!
   if (tgid->GetIDTelescopes() == "/" || !gMultiDetArray) return 0;
   TList* list = new TList;
   tgid->GetIDTelescopes().Begin("/");
   while (!tgid->GetIDTelescopes().End()) {
      list->Add(gMultiDetArray->GetIDTelescope(tgid->GetIDTelescopes().Next()));
   }
   return list;
}
