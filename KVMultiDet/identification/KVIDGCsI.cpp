#include "KVIDGCsI.h"
#include "KVIDCutLine.h"
#include "KVIDCsIRLLine.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDGCsI)

KVIDGCsI::KVIDGCsI()
{
   //Default constructor
   IMFLine = 0;
   GammaLine = 0;
}

KVIDGCsI::KVIDGCsI(const KVIDGCsI& grid) : KVIDZAGrid()
{
   //Copy constructor
   IMFLine = 0;
   GammaLine = 0;
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   grid.Copy(*this);
#else
   ((KVIDGCsI&) grid).Copy(*this);
#endif
}


KVIDGCsI::~KVIDGCsI()
{
   //Dtor.
}

//______________________________________________________________________________________________________//

Bool_t KVIDGCsI::IsIdentifiable(Double_t x, Double_t y, TString* reject) const
{
   //Performs rejection of gammas - returns kTRUE if point above gamma line
   //(note - position w.r.t. IMF line is not considered)
   //Returns kFALSE for points below gamma line i.e. for gammas ;-)
   //If no gamma line defined, returns kTRUE for all points

   if (!GammaLine) return kTRUE;
   else if ((x < GammaLine->GetX()[0]) && (x < GammaLine->GetX()[GammaLine->GetN() - 1])) return kTRUE;
   else if (GammaLine->WhereAmI(x, y, "above")) return kTRUE;

   if (reject) *reject = GammaLine->GetName();
   return kFALSE;
}

//_______________________________________________________________________________________________//

// small utility class which adds the IMF line to the identifier list when constructed,
// and removes it when it goes out of scope i.e. whenever the program leaves the Identify method
struct add_remove_imf_line {
   KVList* idlines;
   KVIDLine* imfline;
   add_remove_imf_line(KVList* l, KVIDLine* i) : idlines(l), imfline(i)
   {
      if (imfline) idlines->AddLast(imfline);
   }
   ~add_remove_imf_line()
   {
      if (imfline) idlines->Remove(imfline);
   }
};

void KVIDGCsI::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // Set Z and A of nucleus based on position in R-L grid
   // The identification of gammas (kICODE10) and charged particles is performed
   // Note:
   //  for isotopically identified particles, the integer A (KVNucleus::GetA) is the mass assigned to the closest line
   //  [unless the closest line is the IMF line, in which case we use the closest identifier line],
   //  whereas the floating-point A (KVReconstructedNucleus::GetRealA) is calculated by interpolation.
   //  the integer A is not necessarily = nint(floating-point A): for example, if no 5He line is drawn in the grid
   //  (which is usually the case), there will be no isotopically-identified particle with GetA()=5, although
   //  there may be particles with GetRealA() between 4.5 and 5.5

   add_remove_imf_line IMF(fIdentifiers, IMFLine);

//   if (!IsIdentifiable(x, y)) {
//      //point below gamma line
//      const_cast < KVIDGCsI* >(this)->fICode = kICODE10;
//      idr->IDquality = fICode;
//      idr->Z = 0;
//      idr->A = 0;
//      idr->IDOK = kTRUE;
//      idr->SetComment("gamma");
//      return;
//   }
   if (!const_cast<KVIDGCsI*>(this)->FindFourEmbracingLines(x, y, "above")) {
      //no lines corresponding to point were found
      const_cast < KVIDGCsI* >(this)->fICode = kICODE8;         // Z indetermine ou (x,y) hors limites
      idr->IDquality = fICode;
      idr->SetComment("no identification: (x,y) out of range covered by grid");
      return;
   }
   Int_t Z;
   Double_t A;
   const_cast < KVIDGCsI* >(this)->IdentZA(x, y, Z, A);
   idr->Z = Z;
   idr->A = Aint;
   idr->PID = A;
   idr->IDquality = fICode;
   switch (fICode) {

      case kICODE0:
         idr->SetComment("ok");
         break;
      case kICODE1:
         idr->SetComment("Z ok, mass may be greater than A");
         break;
      case kICODE2:
         idr->SetComment("Z ok, mass may be smaller than A");
         break;
      case kICODE3:
         idr->SetComment("Z ok, mass may be greater or smaller than A");
         break;
      case kICODE4:
         idr->SetComment("Z ok, mass out of range, >=A");
         break;
      case kICODE5:
         idr->SetComment("Z ok, mass out of range, <=A");
         break;
      case kICODE6:
         idr->SetComment("point above IMF line, Z is a minimum value");
         break;
      case kICODE7:
         idr->SetComment("point is left of IMF line, Z is the most probable lower limit");
         break;
      case kICODE8:
         idr->SetComment("no identification: (x,y) out of range covered by grid");
         break;
      case kICODE9:
         idr->SetComment("no identification for this module");
         break;
      default:
         idr->SetComment("no identification: (x,y) out of range covered by grid");
   }

   if (fICode < kICODE4) {
      idr->IDOK = kTRUE;
      idr->Zident = kTRUE;
      idr->Aident = kTRUE;
   }
}

//_________________________________________________________________________//

KVIDZALine* KVIDGCsI::GetZLine(Int_t z, Int_t& index) const
{
   //Returns first ID line in sorted list for which GetZ() returns 'z'.
   //index=index of line found in fIDLines list (-1 if not found).
   //This is done by looping over all the lines in the list, not by dichotomy as in base class KVIDZAGrid,
   //because of the 8Be line being in between 6He and 6Li.

   index = 0;
   Int_t nlines = GetNumberOfIdentifiers();
   while ((dynamic_cast < KVIDZALine* >(GetIdentifiers()->At(index))->GetZ() != z)
          && (index < (nlines - 1)))
      index++;
   KVIDZALine* line = dynamic_cast < KVIDZALine* >(GetIdentifiers()->At(index));
   if (line->GetZ() != z) {
      index = -1;
      return 0;
   }
   return line;
}

//_________________________________________________________________________//

KVIDZALine* KVIDGCsI::GetZALine(Int_t z, Int_t a, Int_t& index) const
{
   //Returns ID line corresponding to nucleus (Z,A) and its index in fIDLines.
   //This is done by looping over all the lines in the list, not as in base class KVIDZAGrid,
   //because of the 8Be line being in between 6He and 6Li.

   index = 0;
   Int_t nlines = GetNumberOfIdentifiers();
   KVIDZALine* line = dynamic_cast < KVIDZALine* >(GetIdentifiers()->At(index));
   while ((line->GetZ() != z || line->GetA() != a)
          && (index < (nlines - 1))) {
      line = dynamic_cast < KVIDZALine* >(GetIdentifiers()->At(++index));
   }
   if (line->GetZ() != z || line->GetA() != a) {
      index = -1;
      return 0;
   }
   return line;
}

//_________________________________________________________________________//

void KVIDGCsI::IdentZA(Double_t x, Double_t y, Int_t& Z, Double_t& A)
{
   //Finds Z, A and 'real A' for point (x,y) once closest lines to point have been found.
   // Double_t A = mass calculated by interpolation
   //This is a line-for-line copy of the latter part of IdnCsOr, even the same
   //variable names and comments have been used (as much as possible).

   fICode = kICODE0;
   A = -1.;
   Aint = 0;

//   if(fIdxClosest==ksups) cout << "*** ";
//   cout << "ksups = " << ksups << " Zsups = " << Zsups << "  Asups = " << Asups << "  wsups = " << wsups << "  dsups = " << dsups << endl;
//   if(fIdxClosest==ksup) cout << "*** ";
//   cout << "ksup = " << ksup << " Zsup = " << Zsup << "  Asup = " << Asup << "  wsup = " << wsup << "  dsup = " << dsup << endl;
//   if(fIdxClosest==kinf) cout << "*** ";
//   cout << "kinf = " << kinf << " Zinf = " << Zinf << "  Ainf = " << Ainf << "  winf = " << winf << "  dinf = " << dinf << endl;
//   if(fIdxClosest==kinfi) cout << "*** ";
//   cout << "kinfi = " << kinfi << " Zinfi = " << Zinfi << "  Ainfi = " << Ainfi << "  winfi = " << winfi << "  dinfi = " << dinfi << endl;
   Int_t ibif = 0;
   Int_t k = -1;
   Double_t yy, y1, y2;
   Int_t ix1, ix2;
   yy = y1 = y2 = 0;
   ix1 = ix2 = 0;
   if (ksup > -1) {
      if (kinf > -1) {
         //cout << " /******************* 2 lignes encadrant le point ont ete trouvees ************************/" << endl;
         Double_t dt = dinf + dsup;     //distance between the 2 lines
         if (Zinf == Zsup) {
            //   cout << "      /****************meme Z**************/" << endl;
            Z = Zinf;
            Int_t dA = Asup - Ainf;
            Double_t dist = dt / dA;    //distance between the 2 lines normalised to difference in A of lines
            /*** A = Asup ***/
            if (dinf > dsup) {  //point is closest to upper line, 'sup' line
               ibif = 1;
               k = ksup;
               yy = -dsup;
               A = Asup;
               Aint = Asup;
               if (ksups > -1) {        // there is a 'sups' line above the 2 which encadrent le point
                  y2 = dsups - dsup;
                  if (Zsups == Zsup) {
                     ibif = 0;
                     y2 /= 2.;
                     ix2 = Asups - Asup;
                  }
                  else {
                     if (Zsups > 0)
                        y2 /= 2.;       // 'sups' line is not IMF line
                     Double_t x2 = wsup;
                     x2 = 0.5 * TMath::Max(x2, dist);
                     y2 = TMath::Min(y2, x2);
                     ix2 = 1;
                  }
               }
               else {         // ksups == -1 i.e. no 'sups' line
                  y2 = wsup;
                  y2 = 0.5 * TMath::Max(y2, dist);
                  ix2 = 1;
               }
               y1 = -dt / 2.;
               ix1 = -dA;
            }
            /*** A = Ainf ***/
            else {              //point is closest to lower line, 'inf' line
               ibif = 2;
               k = kinf;
               yy = dinf;
               A = Ainf;
               Aint = Ainf;
               if (kinfi > -1) {        // there is a 'infi' line below the 2 which encadrent le point
                  y1 = 0.5 * (dinfi - dinf);
                  if (Zinfi == Zinf) {
                     ibif = 0;
                     ix1 = Ainfi - Ainf;
                     y1 = -y1;
                  }
                  else {
                     Double_t x1 = winf;
                     x1 = 0.5 * TMath::Max(x1, dist);
                     y1 = -TMath::Min(y1, x1);
                     ix1 = -1;
                  }
               }
               else {         // kinfi = -1 i.e. no 'infi' line
                  y1 = winf;
                  y1 = -0.5 * TMath::Max(y1, dist);
                  ix1 = -1;
               }
               y2 = dt / 2.;
               ix2 = dA;
            }
         }
         else {
            //cout << "         /****************Z differents**************/ " << endl;
            if (Zsup == -1) {   //'sup' is the IMF line
               dt *= 2.;
               dsup = dt - dinf;
            }
            /*** Z = Zsup ***/
            ibif = 3;
            if (dinf > dsup) {  // closest to upper 'sup' line
               k = ksup;
               yy = -dsup;
               Z = Zsup;
               A = Asup;
               Aint = Asup;
               y1 = 0.5 * wsup;
               if (ksups > -1) {        // there is a 'sups' line above the 2 which encadrent the point
                  y2 = dsups - dsup;
                  if (Zsups == Zsup) {
                     ibif = 2;
                     ix2 = Asups - Asup;
                     Double_t x1 = y2 / ix2 / 2.;
                     y1 = TMath::Max(y1, x1);
                     y1 = -TMath::Min(y1, dt / 2.);
                     ix1 = -1;
                     y2 /= 2.;
                  }
                  else {
                     if (Zsups > 0)
                        y2 /= 2.;       // 'sups" is not IMF line
                     y2 = TMath::Min(y1, y2);
                     ix2 = 1;
                     y1 = -TMath::Min(y1, dt / 2.);
                     ix1 = -1;
                  }
               }
               else {         // ksups == -1, i.e. no 'sups' line
                  fICode = kICODE7;     //a gauche de la ligne fragment, Z est alors un Zmin et le plus probable
                  y2 = y1;
                  ix2 = 1;
                  y1 = -TMath::Min(y1, dt / 2.);
                  ix1 = -1;
               }
            }
            /*** Z = Zinf ***/
            else {              // closest to lower 'inf' line
               k = kinf;
               yy = dinf;
               Z = Zinf;
               A = Ainf;
               Aint = Ainf;
               y2 = 0.5 * winf;
               if (kinfi > -1) {        // there is a 'infi' line below the 2 which encadrent the point
                  y1 = dinfi - dinf;
                  if (Zinfi == Zinf) {
                     ibif = 1;
                     ix1 = Ainfi - Ainf;
                     Double_t x2 = -y1 / ix1 / 2.;
                     y2 = TMath::Max(y2, x2);
                     y2 = TMath::Min(y2, dt / 2.);
                     ix2 = 1;
                     y1 /= -2.;
                  }
                  else {
                     y1 = -TMath::Min(y2, y1 / 2.);
                     ix1 = -1;
                     y2 = TMath::Min(y2, dt / 2.);
                     ix2 = 1;
                  }
               }
               else {         // no kinfi line, kinfi==-1
                  y1 = -y2;
                  ix1 = -1;
                  y2 = TMath::Min(y2, dt / 2.);
                  ix1 = 1;
               }
            }
         }
      }//if(kinf>-1)...
      else if (Zsup > 0) {      // 'sup' is not IMF line
         //cout<<" /****************** Seule une ligne superieure a ete trouvee *********************/" << endl;
         ibif = 3;
         k = ksup;
         yy = -dsup;
         Z = Zsup;
         A = Asup;
         Aint = Asup;
         y1 = 0.5 * wsup;
         if (ksups > -1) {      // there is a 'sups' line above the closest line to the point
            y2 = dsups - dsup;
            if (Zsups == Zsup) {
               ibif = 2;
               ix2 = Asups - Asup;
               Double_t x1 = y2 / ix2 / 2.;
               y1 = -TMath::Max(y1, x1);
               ix1 = -1;
               y2 /= 2.;
            }
            else {
               if (Zsups > 0)
                  y2 /= 2.;     // 'sups' is not IMF line
               y2 = TMath::Min(y1, y2);
               ix2 = 1;
               y1 = -y1;
               ix1 = -1;
            }
         }
         else {               // no 'sups' line above closest line
            fICode = kICODE7;   //a gauche de la ligne fragment, Z est alors un Zmin et le plus probable
            y2 = y1;
            ix2 = 1;
            y1 = -y1;
            ix1 = -1;
         }
      }
      else {
         fICode = kICODE8;      //  Z indetermine ou (x,y) hors limites
      }
   }
   else if (kinf > -1) {
      //cout <<"/****************** Seule une ligne inferieure a ete trouvee ***********************/" << endl;
      /*** Sep. fragment ***/
      if (Zinf == -1) {         // 'inf' is IMF line
         //point is above IMF line. Z = Z of last line in grid, A = -1
         k = -1;
         Z = GetZmax();
         A = -1;
         Aint = 0;
         fICode = kICODE6;      // au-dessus de la ligne fragment, Z est alors un Zmin
      }
      /*** Ligne de crete (Z,A line)***/
      else {
         ibif = 3;
         k = kinf;
         Z = Zinf;
         A = Ainf;
         Aint = Ainf;
         yy = dinf;
         y2 = 0.5 * winf;
         if (kinfi > -1) {
            y1 = dinfi - dinf;
            if (Zinfi == Zinf) {
               ibif = 1;
               ix1 = Ainfi - Ainf;
               Double_t x2 = -y1 / ix1 / 2.;
               y2 = TMath::Max(y2, x2);
               ix2 = 1;
               y1 /= -2.;
            }
            else {
               y1 = -TMath::Min(y2, y1 / 2.);
               ix1 = -1;
               ix2 = 1;
            }
         }
         else {
            y1 = -y2;
            ix1 = -1;
            ix2 = 1;
         }
         fICode = kICODE7;      // a gauche de la ligne fragment, Z est alors un Zmin et le plus probable
      }
   }
   /*****************Aucune ligne n'a ete trouvee*********************************/
   else {
      fICode = kICODE8;         // Z indetermine ou (x,y) hors limites
   }
   /****************Test des bornes********************************************/
   if (k > -1 && fICode == kICODE0) {
      if (yy > y2)
         fICode = kICODE4;      // Z ok, masse hors limite superieure ou egale a A
   }
   if (k > -1 && (fICode == kICODE0 || fICode == kICODE7)) {
      if (yy < y1)
         fICode = kICODE5;      // Z ok, masse hors limite inferieure ou egale a A
   }
   if (fICode == kICODE4 || fICode == kICODE5) {
      A = -1;
      Aint = 0;
   }

   /****************Interpolation de la masse: da = f*log(1+b*dy)********************/
   if (fICode == kICODE0 || (fICode == kICODE7 && yy <= y2)) {
      Double_t deltaA = 0.;
      Bool_t i = kFALSE;
      Double_t dt, dist = y1 * y2;
      dt = 0.;
      if (ix2 == -ix1) {        //dA1 = dA2
         if (dist != 0) {
            dt = -(y1 + y2) / dist;
            i = kTRUE;
         }
         /*else
            Warning("IdentZA","%s : cannot calculate interpolated mass (dist=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
               GetName(), dist, Z, Aint, fICode);*/
      }
      else if (ix2 == -ix1 * 2) {     // dA2 = 2*dA1
         Double_t tmp = y1 * y1 - 4. * dist;
         if (tmp > 0. && dist != 0) {
            dt = -(y1 + 2. * y2 -
                   TMath::Sqrt(tmp)) / dist / 2.;
            i = kTRUE;
         }
         /*else
            Warning("IdentZA","%s : cannot calculate interpolated mass (y1*y1-4*dist=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
               GetName(), tmp, Z, Aint, fICode);*/
      }
      else if (ix1 == -ix2 * 2) {     // dA1 = 2*dA2
         Double_t tmp = y2 * y2 - 4. * dist;
         if (tmp > 0. && dist != 0) {
            dt = -(y2 + 2. * y1 +
                   TMath::Sqrt(tmp)) / dist / 2.;
            i = kTRUE;
         }
         /*else
            Warning("IdentZA","%s : cannot calculate interpolated mass (y2*y2-4*dist=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
               GetName(), tmp, Z, Aint, fICode);*/
      }
      if (i) {
         dist = dt * y2;
         if (TMath::Abs(dist) < 0.001) {
            if (y2 != 0)
               deltaA = yy * ix2 / y2 / 2.;
            /*else
               Warning("IdentZA","%s : cannot calculate interpolated mass (y2=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
                  GetName(), y2, Z, Aint, fICode);*/
         }
         else {
            if (dist > -1. && dt * yy > -1.)
               deltaA = ix2 / 2. / TMath::Log(1. + dist) * TMath::Log(1. + dt * yy);
            /*else
               Warning("IdentZA","%s : cannot calculate interpolated mass (dist=%f dt*yy=%f), Areal will equal Aint (Z=%d Aint=%d fICode=%d)",
                  GetName(), dist, dt*yy, Z, Aint, fICode);*/
         }
         A += deltaA;
      }
   }
   /***************D'autres masses sont-elles possibles ?*************************/
   if (fICode == kICODE0) {
      //cout << "icode = 0, ibif = " << ibif << endl;
      /***Masse superieure***/
      if (ibif == 1 || ibif == 3) {
         //We look at next line in the complete list of lines, after the closest line.
         //If it has the same Z as the closest line, but was excluded from research for closest line
         //because the point lies outside the endpoints, there remains a doubt about the mass:
         //on rajoute 1 a fICode, effectivement on le met = kICODE1
         Int_t idx = fIdxClosest;
         if (idx > -1 && ++idx < GetNumberOfIdentifiers()) {
            KVIDCsIRLLine* nextline =
               (KVIDCsIRLLine*) GetIdentifierAt(idx);
            if (nextline->GetZ() == Z
                  && !nextline->IsBetweenEndPoints(x, y, "x")) {
               fICode++;        // Z ok, mais les masses superieures a A sont possibles
               //cout <<"//on rajoute 1 a fICode, effectivement on le met = kICODE1" << endl;
            }
         }
      }
      /***Masse inferieure***/
      if (ibif == 2 || ibif == 3) {
         //We look at line below the closest line in the complete list of lines.
         //If it has the same Z as the closest line, but was excluded from research for closest line
         //because the point lies outside the endpoints, there remains a doubt about the mass:
         //on rajoute 2 a fICode, so it can be = kICODE2 or kICODE3
         Int_t idx = fIdxClosest;
         if (idx > -1 && --idx >= 0) {
            KVIDCsIRLLine* nextline =
               (KVIDCsIRLLine*) GetIdentifierAt(idx);
            if (nextline->GetZ() == Z
                  && !nextline->IsBetweenEndPoints(x, y, "x")) {
               fICode += 2;
               //cout << "//on rajoute 2 a fICode, so it can be = kICODE2 or kICODE3" << endl;
            }
         }
      }
   }

   // cout << "Z = " << Z << " A = " << A << " icode = " << fICode << endl;
}

//___________________________________________________________________________________

void KVIDGCsI::Initialize()
{
   // General initialisation method for CsI R-L identification grid.
   // This method MUST be called once before using the grid for identifications.
   // The ID lines are sorted.
   // The natural line widths of all ID lines are calculated.
   // The line with the largest Z (Zmax line) is found.
   // IMF & Gamma line pointers are initialised

   // if grid has already been used for identification, IMF_line will be in identifiers list.
   TObject* imfline = fIdentifiers->FindObject("IMF_line");
   if (imfline) fIdentifiers->Remove(imfline); // remove to avoid problems with CalculateLineWidths
   KVIDZAGrid::Initialize();
   GammaLine = (KVIDLine*)GetCut("gamma_line");
   if (!GammaLine) {
      GetName();
      Error("Initialize", "%s: Cut 'gamma_line' not found in grid. Not a valid CsI R-L grid. Listing existing cuts:",
            GetName());
      GetCuts()->ls();
   }
   IMFLine = (KVIDLine*)GetCut("IMF_line");
   if (!IMFLine) {
      Error("Initialize", "%s: Cut 'IMF_line' not found in grid. Not a valid CsI R-L grid. Listing existing cuts:",
            GetName());
      GetCuts()->ls();
   }
}

//___________________________________________________________________________________

void KVIDGCsI::BackwardsCompatibilityFix()
{
   // Called after reading a grid from an ascii file.
   // Tries to convert information written by an old version of the class:
   //
   //<PARAMETER> Ring min=...  ----> <PARAMETER> IDTelescopes=...
   //<PARAMETER> Ring max=...
   //<PARAMETER> Mod min=...
   //<PARAMETER> Mod max=...
   //
   // This will fail. The fix is no longer supported. Such files should
   // no longer exist.

   KVIDZAGrid::BackwardsCompatibilityFix();
   if (fPar->HasParameter("IDTelescopes")) return;

   Fatal("BackwardsCompatibilityFix",
         "This fix no longer works. There will be problems.");
   GammaLine = (KVIDLine*)GetCut("gamma_line");
   IMFLine = (KVIDLine*)GetCut("IMF_line");
   if (GammaLine)((KVIDCutLine*)GammaLine)->SetAcceptedDirection("above");
   if (IMFLine)((KVIDCutLine*)IMFLine)->SetAcceptedDirection("below");
   SetVarY("CSI-R");
   SetVarX("CSI-L");
}
