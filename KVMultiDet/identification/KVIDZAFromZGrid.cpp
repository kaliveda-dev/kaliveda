//Created by KVClassFactory on Tue Mar  8 10:00:16 2016
//Author: Diego Gruyer

#include "KVIDZAFromZGrid.h"
#include "TMultiGraph.h"
#include "TCanvas.h"

ClassImp(KVIDZAFromZGrid)

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
   <li>    KVIDZAGrid::kICODE4,                   point is in between two isotopes of different Z, too far from either to be considered well-identified</li>
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
//    Info("KVIDZAFromZGrid","called...");
   init();
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

//void KVIDZAFromZGrid::ReadAsciiFile(const Char_t* filename)
//{
//   fPIDRange = kFALSE;
//   KVIDGraph::ReadAsciiFile(filename);
//}

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
   fHasMassCut = kFALSE;
   KVIDGraph::ReadFromAsciiFile(gridfile);

   if (GetIdentifier("MassID")) fHasMassCut = kTRUE;

   if (GetParameters()->HasParameter("PIDRANGE")) {
      fPIDRange = kTRUE;
      TString pidrange = GetParameters()->GetStringValue("PIDRANGE");
      if (pidrange.Contains("-")) {
         TString min = (pidrange(0, pidrange.Index("-")));
         fZminInt = min.Atoi();
         min = (pidrange(pidrange.Index("-") + 1, pidrange.Length()));
         fZmaxInt = min.Atoi();
      } else {
         fZminInt = 1;
         fZmaxInt = pidrange.Atoi();
      }
      LoadPIDRanges();
   }

}

void KVIDZAFromZGrid::LoadPIDRanges()
{
   KVIDentifier* id = 0;
   TIter it(GetIdentifiers());
   while ((id = (KVIDentifier*)it())) {
      int zz = id->GetZ();
      if (!GetParameters()->HasParameter(Form("PIDRANGE%d", zz))) continue;
      KVString mes = GetParameters()->GetStringValue(Form("PIDRANGE%d", zz));
      if (mes.IsWhitespace()) continue;
      int type = (mes.Contains(",") ? 2 : 1);
      interval* itv = new interval(zz, type);
      mes.Begin("|");
      while (!mes.End())

      {
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
      fTables.Add(itv);
   }
//    PrintPIDLimits();
}

void KVIDZAFromZGrid::PrintPIDLimits()
{
   for (int zz = fZminInt; zz <= fZmaxInt; zz++) {
      Info("PrintPIDLimits", "Z=%2d    [%.4lf  %.4lf]", zz, ((interval*)fTables.At(zz - fZminInt))->fPIDmins.at(0),
           ((interval*)fTables.At(zz - fZminInt))->fPIDmaxs.at(((interval*)fTables.At(zz - fZminInt))->fNPIDs - 1));
   }
}

bool KVIDZAFromZGrid::is_inside(double pid)
{
   int zint = TMath::Nint(pid);
   if (fTables.At(zint - fZminInt)) return ((interval*)fTables.At(zint - fZminInt))->is_inside(pid);
   else return kFALSE;
}


void KVIDZAFromZGrid::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // Fill the KVIdentificationResult object with the results of identification for point (x,y)
   // corresponding to some physically measured quantities related to a reconstructed nucleus.
   //
   // By default (OnlyZId()=kFALSE) this means identifying the Z & A of the nucleus.
   // In this case, we consider that the nucleus' Z & A have been correctly measured
   // if the 'quality code' returned by IdentZA() is < kICODE4:
   //   we set idr->Zident and idr->Aident to kTRUE if fICode<kICODE4
   //
   // If OnlyZId()=kTRUE, only the Z of the nucleus is established.
   // In this case, we consider that the nucleus' Z has been correctly measured
   // if the 'quality code' returned by IdentZ() is < kICODE4, thus:
   //   we set idr->Zident to kTRUE if fICode<kICODE4
   // The mass idr->A is set to the mass of the nearest line.
   //
   // Real & integer masses for isotopically identified particles
   // ===================================================
   // For points lying between two lines of same Z and different A (fICode<kIDCode4)
   // the "real" mass is given by interpolation between the two masses.
   // The integer mass is the A of the line closest to the point.
   // This means that the integer A is not always = nint("real" A),
   // as for example if a grid is drawn with lines for 7Be & 9Be but not 8Be
   // (usual case), then particles between the two lines can have "real" masses
   // between 7.5 and 8.5, but their integer A will be =7 or =9, never 8.
   //
   idr->IDOK = kFALSE;
   if (!const_cast<KVIDZAFromZGrid*>(this)->FindFourEmbracingLines(x, y, "above")) {
      //no lines corresponding to point were found
      const_cast < KVIDZAFromZGrid* >(this)->fICode = kICODE8;         // Z indetermine ou (x,y) hors limites
      idr->IDquality = kICODE8;
      idr->SetComment("no identification: (x,y) out of range covered by grid");
      return;
   }
   if (OnlyZId()) {
      Double_t Z;
      const_cast < KVIDZAFromZGrid* >(this)->IdentZ(x, y, Z);
      idr->IDquality = fICode;
      if (fICode < kICODE4 || fICode == kICODE7) {
         idr->Zident = kTRUE;
      }
      if (fICode < kICODE4) {
         idr->IDOK = kTRUE;
      }
      idr->Z = Zint;
      idr->PID = Z;
      idr->A = Aint;
      idr->Aident = kFALSE;

      if ((fPIDRange && (idr->IDOK) && (idr->Z <= fZmaxInt) && (idr->Z > 0) && !fHasMassCut && (const_cast < KVIDZAFromZGrid* >(this)->is_inside(Z))) ||
            (fPIDRange && (idr->IDOK) && (idr->Z <= fZmaxInt) && (idr->Z >= fZminInt - 1) && fHasMassCut && GetIdentifier("MassID")->IsInside(x, y))) {
         const_cast < KVIDZAFromZGrid* >(this)->DeduceAfromPID(idr);
         if (idr->IDquality < kICODE4) {
            idr->Aident = kTRUE;
            idr->IDOK = kTRUE;
         }


//         pid = 0;
//         idr->PID = std::abs(pid);
//         idr->A = TMath::Nint(pid);
//         idr->Aident = kTRUE;
//         if (pid > 0) {
//            idr->IDquality = kICODE0;
//            idr->SetComment("ok");
//         } else {
//            idr->IDquality = kICODE3;
//            idr->SetComment("slight ambiguity of A, which could be larger or smaller");
//         }
      } else {
         switch (fICode) {

            case kICODE0:
               idr->SetComment("ok");
               break;
            case kICODE1:
               idr->SetComment("slight ambiguity of Z, which could be larger");
               break;
            case kICODE2:
               idr->SetComment("slight ambiguity of Z, which could be smaller");
               break;
            case kICODE3:
               idr->SetComment("slight ambiguity of Z, which could be larger or smaller");
               break;
            case kICODE4:
               idr->SetComment("point is in between two lines of different Z, too far from either to be considered well-identified");
               break;
            case kICODE5:
               idr->SetComment("point is in between two lines of different Z, too far from either to be considered well-identified");
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
   } else {
      Int_t Z;
      Double_t A;
      const_cast < KVIDZAFromZGrid* >(this)->IdentZA(x, y, Z, A);
      idr->IDquality = fICode;
      idr->Z = Z;
      idr->PID = A;
      if (fICode < kICODE4 || fICode == kICODE7) {
         idr->Zident = kTRUE;
      }
      idr->A = Aint;
      if (fICode < kICODE4) {
         idr->Aident = kTRUE;
         idr->IDOK = kTRUE;
      }
      switch (fICode) {
         case kICODE0:
            idr->SetComment("ok");
            break;
         case kICODE1:
            idr->SetComment("slight ambiguity of A, which could be larger");
            break;
         case kICODE2:
            idr->SetComment("slight ambiguity of A, which could be smaller");
            break;
         case kICODE3:
            idr->SetComment("slight ambiguity of A, which could be larger or smaller");
            break;
         case kICODE4:
            idr->SetComment("point is in between two isotopes of different Z, too far from either to be considered well-identified");
            break;
         case kICODE5:
            idr->SetComment("point is in between two isotopes of different Z, too far from either to be considered well-identified");
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
}


double KVIDZAFromZGrid::DeduceAfromPID(KVIdentificationResult* idr)
{
   int zint = idr->Z;
   double res = 0.;
   if (fTables.At(zint - fZminInt)) res = ((interval*)fTables.At(zint - fZminInt))->eval(idr);
   return res;
}

double KVIDZAFromZGrid::interval::eval(KVIdentificationResult* idr)
{
   double pid = idr->PID;
   if (pid < 0.5) return 0.;
   double res = fPIDs.Eval(pid);
   int ares = 0;

   if (fType == KVIDZAFromZGrid::kIntType) {
      for (int ii = 0; ii < fNPIDs; ii++) {
         if (pid > fPIDmins.at(ii) && pid < fPIDmaxs.at(ii)) {
            ares = fAs.at(ii);
            break;
         }
      }
      if (ares != 0) {
         idr->A = ares;
         idr->PID = res;
         idr->IDquality = kICODE0;
         idr->SetComment("ok");
//            Info("eval","A id ok -> code0");
      } else {
//            Info("eval","A id outside interval -> code3");
         ares = TMath::Nint(res);
         idr->A = ares;
         idr->PID = res;
         if (ares > fAs.at(0) && ares < fAs.at(fNPIDs - 1)) {
            idr->IDquality = kICODE3;
            idr->SetComment("slight ambiguity of A, which could be larger or smaller");
         } else {
            idr->IDquality = kICODE3;
            idr->SetComment("slight ambiguity of A, which could be larger or smaller");
         }
      }
   } else {
      ares = TMath::Nint(res);
      idr->A = ares;
      idr->PID = res;
      if (ares > fAs.at(0) && ares < fAs.at(fNPIDs - 1)) {
         idr->IDquality = kICODE0;
         idr->SetComment("ok");
      } else {
         idr->IDquality = kICODE3;
         idr->SetComment("slight ambiguity of A, which could be larger or smaller");
      }
   }
   return res;
}

bool KVIDZAFromZGrid::interval::is_inside(double pid)
{
   if (fType != KVIDZAFromZGrid::kIntType) return kTRUE;
   if (pid > fPIDmins.at(0) && pid < fPIDmaxs.at(fNPIDs - 1)) return kTRUE;
   else return kFALSE;
}


KVIDZAFromZGrid::interval::interval(int zz, int type)//:TNamed(Form("interval%d",zz),Form("interval%d",zz))
{
   fType = type;
   fZ = zz;
   fNPIDs = 0;
}

void KVIDZAFromZGrid::interval::add(int aa, double pid, double pidmin, double pidmax)
{
   if (fNPIDs && pid < fPIDs.GetX()[fNPIDs - 1]) {
      Error("add", "Please give me peaks in the right order for Z=%d and A=%d...", fZ, aa);
      return;
   }
   if (fType == KVIDZAFromZGrid::kIntType && !(pid > pidmin && pid < pidmax)) {
      Error("add", "Wrong interval for Z=%d and A=%d: [%.4lf  %.4lf  %.4lf]", fZ, aa, pidmin, pid, pidmax);
      return;
   }

   fPIDs.SetPoint(fNPIDs, pid, aa);
   if (fType == KVIDZAFromZGrid::kIntType) {
      fAs.push_back(aa);
      fPIDmins.push_back(pidmin);
      fPIDmaxs.push_back(pidmax);
      if (!(pid > pidmin && pid < pidmax)) Error("add", "Wrong interval for Z=%d and A=%d: [%.4lf  %.4lf  %.4lf]", fZ, aa, pidmin, pid, pidmax);
   }
   fNPIDs++;
}
