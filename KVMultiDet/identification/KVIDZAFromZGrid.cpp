//Created by KVClassFactory on Tue Mar  8 10:00:16 2016
//Author: Diego Gruyer

#include "KVIDZAFromZGrid.h"
#include "TMultiGraph.h"
#include "KVIDZALine.h"
#include "TCanvas.h"

ClassImp(KVIDZAFromZGrid)
ClassImp(interval)
ClassImp(interval_set)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDZAFromZGrid</h2>
<h4>Hybrid identification grid</h4>
Such a grid can identify simultaneously both the mass and charge of detected
particles (if contains a convertion table for the given element), or solely the charge
(if no convertion table).

The convertion table has to be included in the parameter list of the grid as follow:

<PARAMETER> PIDRANGE=[zmax]
<PARAMETER> PIDRANGE[z]=[a1]:[PID min],[PID mean],[PID max]|[a2]:[PID min],[PID mean],[PID max]|[a3]:[PID min],[PID mean],[PID max]
(...)

with:
<ul>
     <li> [zmax]            highest Z for witch A will be extracted</li>
     <li> [z]               charge corresponding to the following line</li>
     <li> [ai]              mass corresponding to the following interval</li>
     <li> [PID mean]        PID value of the peak of corresponding to mass [ai]</li>
     <li> [PID min/max]     PID interval corresponding to mass [ai] used to attribute the ICODE (optional)</li>
</ul>

<h3>Identification quality codes</h3>
After each identification attempt, the value returned by GetQualityCode() indicates whether the
identification was successful or not. The meaning of the different codes depends on the type
of identification.

<h4>Z & A (mass & charge) isotopic identification grid</h4>
<ul>
     <li> KVIDZAGrid::kICODE0,                   ok</li>
  <li>     KVIDZAGrid::kICODE1,                   Z ok, slight ambiguity of A, which could be larger</li>
  <li>     KVIDZAGrid::kICODE2,                   Z ok, slight ambiguity of A, which could be smaller</li>
    <li>   KVIDZAGrid::kICODE3,                   Z ok, slight ambiguity of A, which could be larger or smaller</li>
   <li>    KVIDZAGrid::kICODE4,                   point out of mass identification intervals, strong ambiguity of A</li>
  <li>     KVIDZAGrid::kICODE5,                   point is in between two isotopes of different Z, too far from either to be considered well-identified</li>
   <li>    KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid</li>
   <li>    KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid</li>
  <li>     KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid</li>
</ul>

<h4>Z-only charge identification grid</h4>
<ul>
     <li> KVIDZAGrid::kICODE0,                   ok</li>
  <li>     KVIDZAGrid::kICODE1,                   slight ambiguity of Z, which could be larger</li>
  <li>     KVIDZAGrid::kICODE2,                   slight ambiguity of Z, which could be smaller</li>
    <li>   KVIDZAGrid::kICODE3,                   slight ambiguity of Z, which could be larger or smaller</li>
   <li>    KVIDZAGrid::kICODE4,                   point is in between two lines of different Z, too far from either to be considered well-identified</li>
  <li>     KVIDZAGrid::kICODE5,                   point is in between two lines of different Z, too far from either to be considered well-identified</li>
   <li>    KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid</li>
   <li>    KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid</li>
  <li>     KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid</li>
</ul>

In both cases, an acceptable identification is achieved if the quality code is kICODE0, kICODE1, kICODE2, or kICODE3.<br>
Points with codes kICODE4 or kICODE5 are normally considered as "noise" and should be rejected.<br>
Points which are (vertically) out of range for this grid have code kICODE6 (point too far below) or kICODE7 (point too far above).<br>
Points with code kICODE8 are totally out of range.
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDZAFromZGrid::KVIDZAFromZGrid()
{
   // Default constructor
   // Grid is declared as a 'ZOnlyGrid' by default (this is internal mechanics)

   init();
   fTables.SetOwner(kTRUE);
   SetOnlyZId();
   fMassCut = nullptr;
   fIgnoreMassID = false;
}

KVIDZAFromZGrid::~KVIDZAFromZGrid()
{
   // Destructor
}

//________________________________________________________________

void KVIDZAFromZGrid::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVIDZAFromZGrid::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVIDZAGrid::Copy(obj);
   //KVIDZAFromZGrid& CastedObj = (KVIDZAFromZGrid&)obj;
}


void KVIDZAFromZGrid::ReadFromAsciiFile(std::ifstream& gridfile)
{
//   fPIDRange = kFALSE;
//   KVIDGraph::ReadFromAsciiFile(gridfile);
//   if (GetParameters()->HasParameter("PIDRANGE")) {
//      fPIDRange = kTRUE;
//      fZmaxInt = GetParameters()->GetIntValue("PIDRANGE");
//      LoadPIDRanges();
//   }

   fPIDRange = kFALSE;
//   fHasMassCut = kFALSE;
   KVIDGraph::ReadFromAsciiFile(gridfile);

//   if (GetIdentifier("MassID")) fHasMassCut = kTRUE;

   if (GetParameters()->HasParameter("PIDRANGE")) {
      fPIDRange = kTRUE;
      TString pidrange = GetParameters()->GetStringValue("PIDRANGE");
      if (pidrange.Contains("-")) {
         TString min = (pidrange(0, pidrange.Index("-")));
         fZminInt = min.Atoi();
         min = (pidrange(pidrange.Index("-") + 1, pidrange.Length()));
         fZmaxInt = min.Atoi();
      }
      else {
         fZminInt = 1;
         fZmaxInt = pidrange.Atoi();
      }
      LoadPIDRanges();
   }
   // if <PARAMETER> IgnoreMassID=1 appears in file, we are only using the PID intervals to clean
   // up a messy de-e plot, not to give mass identification. particles will only be identified in Z.
   if (GetParameters()->HasParameter("IgnoreMassID") && GetParameters()->GetIntValue("IgnoreMassID") == 1)
      fIgnoreMassID = true;
   else
      fIgnoreMassID = false;
}

void KVIDZAFromZGrid::WriteToAsciiFile(std::ofstream& gridfile)
{
   ExportToGrid();
   KVIDGraph::WriteToAsciiFile(gridfile);
}

void KVIDZAFromZGrid::LoadPIDRanges()
{
   fZminInt = 100000;
   fZmaxInt = 0;

   KVIDentifier* id = 0;
   TIter it(GetIdentifiers());
   while ((id = (KVIDentifier*)it())) {
      int zz = id->GetZ();
      if (!GetParameters()->HasParameter(Form("PIDRANGE%d", zz))) continue;
      KVString mes = GetParameters()->GetStringValue(Form("PIDRANGE%d", zz));
      if (mes.IsWhitespace()) continue;
      int type = (mes.Contains(",") ? 2 : 1);
      interval_set* itv = new interval_set(zz, type);
      itv->SetName(GetName());
      mes.Begin("|");
      while (!mes.End()) {
         KVString tmp = mes.Next();
         tmp.Begin(":");
         int aa = tmp.Next().Atoi();
         KVString val = tmp.Next();
         double pidmin, pidmax, pid;
         if (type == 1) itv->add(aa, val.Atof());
         else if (type == 2) {
            val.Begin(",");
            pidmin = val.Next().Atof();
            pid = val.Next().Atof();
            pidmax = val.Next().Atof();
            itv->add(aa, pid, pidmin, pidmax);
//            itv->add(aa, pid, pid-0.02, pid+0.02);
         }
      }
      if (zz < fZminInt) fZminInt = zz;
      if (zz > fZmaxInt) fZmaxInt = zz;
      fTables.Add(itv);
   }
   fPIDRange = kTRUE;
   //    PrintPIDLimits();
}

void KVIDZAFromZGrid::ReloadPIDRanges()
{
   fTables.Clear("all");
   LoadPIDRanges();
}

interval_set* KVIDZAFromZGrid::GetIntervalSet(int zint) const
{
   interval_set* itv = 0;
   TIter it(&fTables);
   while ((itv = (interval_set*)it())) if (itv->GetZ() == zint) return itv;
   return 0;
}

void KVIDZAFromZGrid::PrintPIDLimits()
{
//    ((interval_set*)fTables.At(12))->fIntervals.ls();

//   for (int zz = fZminInt; zz <= fZmaxInt; zz++) {
//      Info("PrintPIDLimits", "Z=%2d    [%.4lf  %.4lf]", zz, ((interval_set*)fTables.At(zz - fZminInt))->fPIDmins.at(0),
//           ((interval_set*)fTables.At(zz - fZminInt))->fPIDmaxs.at(((interval_set*)fTables.At(zz - fZminInt))->fNPIDs - 1));
//      }
}

void KVIDZAFromZGrid::ClearPIDIntervals()
{
   if (fPar->HasParameter("PIDRANGE")) fPar->RemoveParameter("PIDRANGE");
   for (int ii = 1; ii < 30; ii++) {
      if (fPar->HasParameter(Form("PIDRANGE%d", ii))) fPar->RemoveParameter(Form("PIDRANGE%d", ii));
   }
}

int KVIDZAFromZGrid::is_inside(double pid) const
{
   // Look for a set of mass-interval definitions in which the given PID
   // falls (PID from linearisation of Z identification).
   // In principle this should be the set corresponding to Z=nint(PID),
   // but if not Z+/-1 are also tried.
   // Returns the value of Z for the set found (or 0 if no set found)

   int zint = TMath::Nint(pid);
   interval_set* it = GetIntervalSet(zint);
   if (it) {
      if (it->is_inside(pid)) return zint;
      else if (it->is_above(pid)) {

         it = GetIntervalSet(zint + 1);
         if (it && it->is_inside(pid)) return zint + 1;
         else return 0;
      }
      else {
         it = GetIntervalSet(zint - 1);
         if (it && it->is_inside(pid)) return zint - 1;
         else return 0;
      }
   }
   else return 0;
}

void KVIDZAFromZGrid::Initialize()
{
   // General initialisation method for identification grid.
   // This method MUST be called once before using the grid for identifications.
   // The ID lines are sorted.
   // The natural line widths of all ID lines are calculated.
   // The line with the largest Z (Zmax line) is found.

   SetOnlyZId();
   KVIDGrid::Initialize();
   // Zmax should be Z of last line in sorted list

   TIter it(GetIdentifiers());
   KVIDentifier* id = 0;

   fZMax = 0;
   while ((id = (KVIDentifier*)it())) {
      if (!id->InheritsFrom("KVIDZALine")) continue;
      int zz = ((KVIDZALine*)id)->GetZ();
      if (zz > fZMax) fZMax = zz;
   }

   fMassCut = GetIdentifier("MassID");
}

void KVIDZAFromZGrid::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // Fill the KVIdentificationResult object with the results of identification for point (x,y)
   // corresponding to some physically measured quantities related to a reconstructed nucleus.
   //
   // If identification is successful, idr->IDOK = true.
   // In this case, idr->Aident and idr->Zident indicate whether isotopic or only Z identification
   // was acheived.
   //
   // In case of unsuccessful identification, idr->IDOK = false,
   // BUT idr->Zident and/or idr->Aident may be true: this is to indicate which kind of
   // identification was attempted but failed (this changes the meaning of the quality code)

   idr->Aident = idr->Zident = kFALSE;

   KVIDZAGrid::Identify(x, y, idr);
   idr->Zident = kTRUE; // meaning Z identification was attempted, even if it failed
   if (!idr->IDOK) return;

   bool have_pid_range_for_Z = fPIDRange && (idr->Z <= fZmaxInt) && (idr->Z > fZminInt - 1);
   bool mass_id_success = false;

   if (have_pid_range_for_Z && (!fMassCut || fMassCut->IsInside(x, y))) {
      // try mass identification
      mass_id_success = (DeduceAfromPID(idr) > 0);
      if (mass_id_success) {
         // mass identification was at least attempted
         // make sure grid's quality code is consistent with KVIdentificationResult
         const_cast<KVIDZAFromZGrid*>(this)->fICode = idr->IDquality;
         idr->Aident = kTRUE; // meaning A identification was attempted, even if it failed
      }
      else {
         // the pid falls outside of any mass ranges for a Z which has assigned isotopes
         // therefore although the Z identification was good, we cannot consider this
         // particle to be identified
         const_cast<KVIDZAFromZGrid*>(this)->fICode = kICODE4;
      }
      idr->IDOK = (fICode < kICODE4);
   }

   // ignore isotopic successful isotopic identification if fIgnoreMassID=true
   if (fIgnoreMassID && idr->IDOK && idr->Aident) idr->Aident = false;

   // set comments in identification result
   switch (fICode) {
      case kICODE0:
         idr->SetComment("ok");
         break;
      case kICODE1:
         if (mass_id_success) idr->SetComment("slight ambiguity of A, which could be larger");
         else idr->SetComment("slight ambiguity of Z, which could be larger");
         break;
      case kICODE2:
         if (mass_id_success) idr->SetComment("slight ambiguity of A, which could be smaller");
         else idr->SetComment("slight ambiguity of Z, which could be smaller");
         break;
      case kICODE3:
         if (mass_id_success) idr->SetComment("slight ambiguity of A, which could be larger or smaller");
         else idr->SetComment("slight ambiguity of Z, which could be larger or smaller");
         break;
      case kICODE4:
         if (mass_id_success) idr->SetComment("point is outside of mass identification range");
         else idr->SetComment("point is in between two lines of different Z, too far from either to be considered well-identified");
         break;
      case kICODE5:
         if (mass_id_success) idr->SetComment("point is in between two isotopes A & A+2 (e.g. 5He, 8Be, 9B)");
         else idr->SetComment("point is in between two lines of different Z, too far from either to be considered well-identified");
         break;
      case kICODE6:
         idr->SetComment("(x,y) is below first line in grid");
         break;
      case kICODE7:
         idr->SetComment("(x,y) is above last line in grid");
         break;
      default:
         idr->SetComment("no identification: (x,y) out of range covered by grid");
   }
}


double KVIDZAFromZGrid::DeduceAfromPID(KVIdentificationResult* idr) const
{
   // First look for a set of mass intervals in which the PID of the identification result falls,
   // if there is one (see KVIDZAFromZGrid::is_inside).
   // If an interval set is found for a Z different to the original identification, idr->Z is changed.
   // Then call interval_set::eval for the mass interval for this Z.

   int zint = is_inside(idr->PID);
   if (!zint) return -1;
   if (zint != idr->Z) idr->Z = zint;

   double res = 0.;
   interval_set* it = GetIntervalSet(zint);
   if (it) res = it->eval(idr);
   return res;
}


void KVIDZAFromZGrid::ExportToGrid()
{
   ClearPIDIntervals();
   KVNumberList pids;
   interval_set* itvs = 0;
   TIter npid(GetIntervalSets());
   while ((itvs = (interval_set*)npid())) {
      if (!itvs->GetNPID()) continue;
      pids.Add(itvs->GetZ());
   }
   GetParameters()->SetValue("PIDRANGE", pids.AsString());

   itvs = 0;
   TIter next(GetIntervalSets());
   while ((itvs = (interval_set*)next())) {
      if (!itvs->GetNPID()) continue;
      KVString par = Form("PIDRANGE%d", itvs->GetZ());
      KVString val = "";
      interval* itv = 0;
      TIter ni(itvs->GetIntervals());
      while ((itv = (interval*)ni())) {
         val += Form("%d:%lf,%lf,%lf|", itv->GetA(), itv->GetPIDmin(), itv->GetPID(), itv->GetPIDmax());
      }
      val.Remove(val.Length() - 1);
      GetParameters()->SetValue(par.Data(), val.Data());
   }
}



double interval_set::eval(KVIdentificationResult* idr)
{
   double pid = idr->PID;
   if (pid < 0.5) return 0.;
   // calculate interpolated mass from PID
   double res = fPIDs.Eval(pid);
   int ares = 0;

   if (fType == KVIDZAFromZGrid::kIntType) {

      // look for mass interval PID is in
      // in case it falls between two intervals remember also the interval
      // immediately to the left & right of the PID
      interval* left_int(nullptr), *right_int(nullptr);
      interval* inter;
      TIter it(&fIntervals);
      while ((inter = (interval*)it())) {
         if (inter->is_inside(pid)) {
            ares = inter->GetA();
            break;
         }
         else if (inter->is_left_of(pid)) {
            left_int = inter;
         }
         else if (!right_int && inter->is_right_of(pid)) {
            right_int = inter;
         }
      }
      if (ares != 0) {
         // the PID is inside a defined mass interval
         idr->A = ares;
         idr->PID = res;
         idr->IDquality = KVIDZAGrid::kICODE0;
      }
      else {
         // the PID is not inside a defined mass interval
         //
         // * if it is in between two consecutive masses i.e. A and A+1 then it is
         //   Z- and A-identified with a slight ambiguity of A
         // * if it is in between two non-consecutive masses i.e. A and A+2 then it
         //   is not identified (e.g. 5He, 8Be, 9B)
         int dA = right_int->GetA() - left_int->GetA();
         if (dA == 1) {
            // OK, slight ambiguity of A
            ares = TMath::Nint(res);
            idr->A = ares;
            idr->PID = res;
            idr->IDquality = KVIDZAGrid::kICODE3;
         }
         else {
            // in a hole where no isotopes should be (e.g. 5He, 8Be, 9B)
            idr->A = ares;
            idr->PID = res;
            idr->IDquality = KVIDZAGrid::kICODE5;
         }
      }
   }
   else {
      ares = TMath::Nint(res);
      idr->A = ares;
      idr->PID = res;
      if (ares > fPIDs.GetX()[0] && ares < fPIDs.GetX()[fNPIDs - 1]) {
         idr->IDquality = KVIDZAGrid::kICODE0;
      }
      else {
         idr->IDquality = KVIDZAGrid::kICODE4;
      }
   }
   return res;
}

bool interval_set::is_inside(double pid)
{
   if (fType != KVIDZAFromZGrid::kIntType) return kTRUE;

//   Info("is_inside","min: %d max:%d npids:%d", ((interval*)fIntervals.At(0))->GetA(), ((interval*)fIntervals.At(fNPIDs-1))->GetA(), fNPIDs);

   if (pid > ((interval*)fIntervals.At(0))->GetPIDmin() && pid < ((interval*)fIntervals.At(fNPIDs - 1))->GetPIDmax()) return kTRUE;
   else return kFALSE;
}

bool interval_set::is_above(double pid)
{
   if (fType != KVIDZAFromZGrid::kIntType) return kTRUE;

   if (pid > ((interval*)fIntervals.At(fNPIDs - 1))->GetPIDmax()) return kTRUE;
   else return kFALSE;
}


TString interval_set::GetListOfMasses()
{
   if (!GetNPID()) return "-";
   KVNumberList alist;
   for (int ii = 0; ii < GetNPID(); ii++) alist.Add(((interval*)fIntervals.At(ii))->GetA());
   return alist.AsString();
}

interval_set::interval_set(int zz, int type)
{
   fType = type;
   fZ = zz;
   fNPIDs = 0;
   fIntervals.SetOwner(kTRUE);
}

void interval_set::add(int aa, double pid, double pidmin, double pidmax)
{
//   if (fNPIDs && pid < fPIDs.GetX()[fNPIDs - 1]) {
//      Error("add", "Please give me peaks in the right order for Z=%d and A=%d...", fZ, aa);
//      return;
//   }
   if (fType == KVIDZAFromZGrid::kIntType && !(pid > pidmin && pid < pidmax)) {
      Error("add", "Wrong interval for Z=%d and A=%d: [%.4lf  %.4lf  %.4lf] (%s)", fZ, aa, pidmin, pid, pidmax, GetName());
      return;
   }

   fPIDs.SetPoint(fNPIDs, pid, aa);
   if (fType == KVIDZAFromZGrid::kIntType) {
      if (pid) fIntervals.AddLast(new interval(fZ, aa, pid, pidmin, pidmax));
   }
   fNPIDs++;
}



