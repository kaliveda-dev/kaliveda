//Created by KVClassFactory on Thu Jun 19 10:20:29 2014
//Author: Guilain ADEMARD

#include "KVIDQAGrid.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TTree.h"
#include "KVIDGridEditor.h"

ClassImp(KVIDQAGrid)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDQAGrid</h2>
<h4>Base class for 2D Q & A identification grids</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDQAGrid::KVIDQAGrid() : fToF(NULL)
{
   // Default constructor
}

KVIDQAGrid::~KVIDQAGrid()
{
   // Destructor
}
//________________________________________________________________

void KVIDQAGrid::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVIDQAGrid::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVIDGrid::Copy(obj);
   //KVIDQAGrid& CastedObj = (KVIDQAGrid&)obj;
}
//________________________________________________________________

void KVIDQAGrid::init()
{
   //initialisation
   fICode = kICODE8;
   fRealQ = -1.0;
}
//________________________________________________________________

void KVIDQAGrid::RemoveLine(Int_t Q)
{
   // Remove and destroy identifier

   gROOT->ProcessLine("if(gROOT->FindObject(\"gIDGridEditorCanvas\")) gIDGridEditor->Clear()");

   Int_t tmp = -1;
   KVIDQALine* tmpline = NULL;
   if ((tmpline = GetQLine(Q, tmp))) RemoveIdentifier((KVIDentifier*)tmpline);
}
//________________________________________________________________

void KVIDQAGrid::RemoveQLines(const Char_t* QList)
{
   // Remove and destroy identifiers

   KVNumberList l(QList);
   l.Begin();
   while (!l.End()) RemoveLine(l.Next());
}
//________________________________________________________________

KVIDQALine* KVIDQAGrid::GetQLine(Int_t q, Int_t& index) const
{
   //Returns ID line for which GetQ() returns 'q'. index=index of line found in
   //fIDLines list (-1 if not found).
   //To increase speed, this is done by dichotomy, rather than by looping over
   //all the lines in the list.

   index = -1;
   Int_t nlines = GetNumberOfIdentifiers();
   UInt_t idx_min = 0;          //minimum index
   UInt_t idx_max = nlines - 1; // maximum index
   //as a first guess, we suppose that the lines are numbered sequentially from
   //Q=1 (index 0) to Q=qmax=nlines (index nlines-1) with no gaps
   UInt_t idx = ((UInt_t) q - 1 > idx_max ? idx_max : (UInt_t) q - 1);

   while (idx_max > idx_min + 1) {
      KVIDQALine* line = (KVIDQALine*) GetIdentifierAt(idx);
      Int_t qline = line->GetQ();
      //Found it ?
      if (qline == q) {
         index = idx;
         return line;
      }

      if (q > qline) {
         //increase index
         idx_min = idx;
         idx += (Int_t)((idx_max - idx) / 2 + 0.5);
      }
      else {
         //decrease index
         idx_max = idx;
         idx -= (Int_t)((idx - idx_min) / 2 + 0.5);
      }
   }
   //if one of these two lines has the right Q, return its pointer
   KVIDQALine* line = (KVIDQALine*) GetIdentifierAt(idx_min);
   if (line->GetQ() == q) {
      index = idx_min;
      return line;
   }
   line = (KVIDQALine*) GetIdentifierAt(idx_max);
   if (line->GetQ() == q) {
      index = idx_max;
      return line;
   }
   //if not, there is no line in the grid with the right Q
   return 0;
}
//________________________________________________________________

void KVIDQAGrid::CalculateLineWidths()
{
   //Calculate natural "width" of each line in the grid.
   //The lines in the grid are first sorted so that they are in order of ascending 'Y'
   //i.e. first line is Qmin, last line is Qmax (highest line).
   //
   //Then, for a given line we use the separation between each pair of lines.
   //
   //In each case we find D_L (the separation between the two lines at their extreme left)
   //and  D_R (their separation at extreme right). The width of the line is then calculated
   //from these two using the method KVIDQALine::SetAsymWidth (which may be overridden in child
   //classes).


//    Info("CalculateLineWidths",
//          "For grid %s (%s vs. %s, runs %s).", GetName(), GetVarY(), GetVarX(), GetRunList());

   for (Int_t i = 0; i < (Int_t) GetNumberOfIdentifiers(); i++) {

      KVIDQALine* _line = (KVIDQALine*) GetIdentifierAt(i);

      //Q of lines above and below this line - Qxx=-1 if there is no line above or below
      Int_t Qhi =
         ((i < (Int_t) GetNumberOfIdentifiers() - 1) ? ((KVIDQALine*) GetIdentifierAt(i + 1))->GetQ() : -1);
      Int_t Qlo = (i > 0 ? ((KVIDQALine*) GetIdentifierAt(i - 1))->GetQ() : -1);

      // find line for comparison.
      // It should have only one line per Q, so we calculate
      // widths by comparing neighbouring Q.

      Int_t i_other;// index of line used to calculate width
      i_other = (Qhi > -1 ? i + 1 : (Qlo > -1 ? i - 1 : -1));

      //default width of 16000 in case of "orphan" line
      if (i_other < 0) {
         Info("CalculateLineWidths", "No line found for comparison with %s. Width set to 16000", _line->GetName());
         _line->SetWidth(16000.);
         continue;              // skip to next line
      }

      KVIDQALine* _otherline = (KVIDQALine*) GetIdentifierAt(i_other);

      //calculate asymptotic distances between lines at left and right.
      //do this by finding which line's endpoint is between both endpoints of the other line
      Double_t D_L, D_R;

      //***************    LEFT SIDE    ********************************

      //get coordinates of first point of our line
      Double_t x1, y1;
      _line->GetStartPoint(x1, y1);
      KVIDQALine* _aline, *_bline;

      if (_otherline->IsBetweenEndPoints(x1, y1, "x")) {

         //first point of our line is "inside" the X-coords of the other line:
         //asymptotic distance LEFT is distance from our line's starting point (x1,Y1) to the other line
         _aline = _otherline;
         _bline = _line;

      }
      else {

         //first point of other line is "inside" the X-coords of the our line:
         //asymptotic distance LEFT is distance from other line's starting point (x1,Y1) to our line
         _aline = _line;
         _bline = _otherline;
         _otherline->GetStartPoint(x1, y1);

      }

      //calculate D_L
      Int_t dummy = 0;
      D_L = _aline->DistanceToLine(x1, y1, dummy);

      //make sure that D_L is positive : if not, we try to calculate other way (inverse line and starting point)
      if (D_L < 0.) {

         Double_t oldD_L = D_L;

         _aline->GetStartPoint(x1, y1);
         D_L = _bline->DistanceToLine(x1, y1, dummy);

         //still no good ? then we keep the smallest absolute value
         if (D_L < 0.) {
            D_L = TMath::Abs(TMath::Max(D_L, oldD_L));
         }
      }
      //***************   RIGHT SIDE    ********************************

      //get coordinates of last point of our line
      _line->GetEndPoint(x1, y1);

      if (_otherline->IsBetweenEndPoints(x1, y1, "x")) {

         //last point of our line is "inside" the X-coords of the other line:
         //asymptotic distance RIGHT is distance from our line's end point (x1,Y1) to the other line
         _aline = _otherline;
         _bline = _line;

      }
      else {

         //last point of other line is "inside" the X-coords of the our line:
         //asymptotic distance RIGHT is distance from other line's end point (x1,Y1) to our line
         _aline = _line;
         _bline = _otherline;
         _otherline->GetEndPoint(x1, y1);

      }

      //calculate D_R
      D_R = _aline->DistanceToLine(x1, y1, dummy);

      //make sure that D_R is positive : if not, we try to calculate other way (inverse line and end point)
      if (D_R < 0.) {

         Double_t oldD_R = D_R;

         _aline->GetEndPoint(x1, y1);
         D_R = _bline->DistanceToLine(x1, y1, dummy);

         //still no good ? then we keep the smallest absolute value
         if (D_R < 0.) {
            D_R = TMath::Abs(TMath::Max(D_R, oldD_R));
         }
      }
      //***************  SET NATURAL WIDTH OF LINE   ********************************

      _line->SetAsymWidth(D_L, D_R);


      //Info("CalculateLineWidths", "...width for line %s set to : %f (D_L=%f,D_R=%f)", _line->GetName(), _line->GetWidth(), D_L, D_R);
   }
}
//________________________________________________________________

void KVIDQAGrid::DrawLinesWithWidth()
{
   //This method displays the grid as in KVIDGrid::Draw, but
   //the natural line widths are shown as error bars

   if (!gPad) {
      new TCanvas("c1", GetName());
   }
   else {
      gPad->SetTitle(GetName());
   }
   if (!gPad->GetListOfPrimitives()->GetSize()) {
      //calculate size of pad necessary to show grid
      if (GetXmin() == GetXmax())
         const_cast < KVIDQAGrid* >(this)->FindAxisLimits();
      gPad->DrawFrame(GetXmin(), GetYmin(), GetXmax(), GetYmax());
   }
   TIter nextID(GetIdentifiers());
   KVIDQALine* line;
   while ((line = (KVIDQALine*)nextID())) {
      line->GetLineWithWidth()->Draw("3PL");
   }
   {
      GetCuts()->R__FOR_EACH(KVIDLine, Draw)("PL");
   }
   gPad->Modified();
   gPad->Update();
}
//________________________________________________________________

void KVIDQAGrid::Identify(Double_t x, Double_t y, KVIdentificationResult* idr) const
{
   // Fill the KVIdentificationResult object with the results of identification for point (x,y)
   // corresponding to some physically measured quantities related to a reconstructed nucleus.
   //
   // By default (OnlyQId()=kFALSE) this means identifying the Q & A of the nucleus.
   // In this case, we consider that the nucleus' Q & A have been correctly measured
   //
   //
   // if the returned 'quality code' is < kICODE4:
   //   we set idr->Zident (Z<->Q) and idr->Aident is determined if
   //   OnlyQId()=kFALSE.
   // The mass idr->A is set to the mass of the nearest A marker of the KVIDQALine if the A identification went well .
   //
   // If OnlyQId()=kTRUE, only the Q of the nucleus is established.
   // In this case, we consider that only the nucleus' charge state Q has been correctly measured.
   //
   // Real & integer masses for A-identified nuclei
   // ========================================================
   // For points anddlying between two A markers of a same Q line (fICode<kIDCode4)
   // the "real" mass is given by interpolation between the two masses.
   // The integer mass is the A of the marker closest to the point.


   idr->IDOK = kFALSE;
   const_cast < KVIDQAGrid* >(this)->fRealQ = -1.0;

   Int_t idx, idx_inf, idx_sup;
   Double_t dist, dist_inf, dist_sup;
   KVIDLine* line = const_cast<KVIDQAGrid*>(this)->FindNearestIDLineFast(x, y, "above", idx, idx_inf, idx_sup, dist, dist_inf, dist_sup);
   KVIDQALine* closest_line = NULL;

   if (!line || !line->InheritsFrom(KVIDQALine::Class())) {
      //no line corresponding to point was found
      const_cast < KVIDQAGrid* >(this)->fICode = kICODE8;         // Q indetermine ou (x,y) hors limites
   }
   else {
      //the closest_line is found
      closest_line = (KVIDQALine*)line;

      // line below the point
      line = (KVIDLine*)GetIdentifierAt(idx_inf);
      KVIDQALine* inf_line = (line && line->InheritsFrom(KVIDQALine::Class()) ? (KVIDQALine*)line : NULL);

      // line above the point
      line = (KVIDLine*)GetIdentifierAt(idx_sup);
      KVIDQALine* sup_line = (line && line->InheritsFrom(KVIDQALine::Class()) ? (KVIDQALine*)line : NULL);

      // WARNING: the method KVIDQAGrid::FindNearestIDLineFast always find
      // 2 nearest lines even for point below (above) the first (last) line.
      // we have to remove the second found line for these points

      if ((closest_line == GetIdentifiers()->First()) && closest_line->WhereAmI(x, y, "below"))
         sup_line = NULL;
      else if (closest_line == GetIdentifiers()->Last() && closest_line->WhereAmI(x, y, "above"))
         inf_line = NULL;


      Double_t Q      = 0.;
      Double_t deltaQ = 0.;

      // case where 2 embracing lines are found
      if (inf_line && sup_line && (inf_line != sup_line)) {
         Int_t dQ = sup_line->GetQ() - inf_line->GetQ();
         Double_t tot_dist = (dist_inf + dist_sup) / (1.0 * dQ);
         deltaQ = dist / tot_dist;

         // if the point is above the closest_line (i.e. the
         // closest_line is the inf_line) we must add deltaQ
         // to Q, otherwise, we must take deltaQ away from Q
         // (i.e. closest_line is the sup_line)
         if (closest_line == sup_line) deltaQ *= -1.;

         if (dist > closest_line->GetWidth() / 2.) {
            if (deltaQ > 0)
               const_cast < KVIDQAGrid* >(this)->fICode = kICODE1;  // "slight ambiguity of Q, which could be larger"
            else
               const_cast < KVIDQAGrid* >(this)->fICode = kICODE2;  // "slight ambiguity of Q, which could be smaller"

         }
         else const_cast < KVIDQAGrid* >(this)->fICode = kICODE0;   // ok

         if (deltaQ > 0.5) Info("Identify", "deltaQ= %f, Qclosest= %d, Qinf= %d, Qsup= %d, icode= %d, X= %f, Y= %f", deltaQ, closest_line->GetQ(), inf_line->GetQ(), sup_line->GetQ(), fICode, x, y);
      }
      // case where only 1 embracing line is found.
      // in this case, the distance between the point and the line
      // has to be lower than the line width
      else {
         if (dist > closest_line->GetWidth() / 2.) {
            // the distance from the closest line is to high

            if (closest_line == inf_line)
               const_cast < KVIDQAGrid* >(this)->fICode = kICODE6;  // "(x,y) is below first line in grid"
            else
               const_cast < KVIDQAGrid* >(this)->fICode = kICODE7;  // "(x,y) is above last line in grid"
         }
         else {
            deltaQ = dist / closest_line->GetWidth();
            if (closest_line == sup_line) deltaQ *= -1.;
            const_cast < KVIDQAGrid* >(this)->fICode = kICODE3;  // "slight ambiguity of Q, which could be larger or smaller"
         }
         if (deltaQ > 0.5) Info("Identify", "deltaQ= %f, Qclosest= %d, icode= %d, X= %f, Y= %f", deltaQ,  closest_line->GetQ(), fICode, x, y);
      }

      Q = closest_line->GetQ() + deltaQ;
      const_cast < KVIDQAGrid* >(this)->fRealQ = Q;
      idr->Z   = closest_line->GetQ();
      idr->PID = Q;
   }


   switch (fICode) {
      case kICODE0:
         idr->SetComment("ok");
         break;
      case kICODE1:
         idr->SetComment("slight ambiguity of Q, which could be larger");
         break;
      case kICODE2:
         idr->SetComment("slight ambiguity of Q, which could be smaller");
         break;
      case kICODE3:
         idr->SetComment("slight ambiguity of Q, which could be larger or smaller");
         break;
      case kICODE4:
         idr->SetComment("point is in between two lines of different Q, too far from either to be considered well-identified");
         break;
      case kICODE5:
         idr->SetComment("point is in between two lines of different Q, too far from either to be considered well-identified");
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

   idr->IDquality = fICode;
   if (fICode < kICODE4) {
      idr->Zident = kTRUE;
      idr->IDOK   = kTRUE;

      // if the grid is also used for A identification then start it
      if (!OnlyQId()) {
         //idr->IDOK   = kFALSE;
         Int_t Ai;
         Double_t Ar;
         Int_t code;
         closest_line->IdentA(x, y, Ai, Ar, code);
         idr->A   = Ai;

         switch (code) {
            case kICODE0:
               idr->SetComment("ok");
               break;
            case kICODE1:
               idr->SetComment("slight ambiguity of Q or A, which could be larger");
               break;
            case kICODE2:
               idr->SetComment("slight ambiguity of Q or A, which could be smaller");
               break;
            case kICODE3:
               idr->SetComment("slight ambiguity of Q or A, which could be larger or smaller");
               break;
            case kICODE4:
               idr->SetComment("point is in between two lines (markers) of different Q (A), too far from either to be considered well-identified");
               break;
            case kICODE5:
               idr->SetComment("point is in between two lines (markers) of different Q (A), too far from either to be considered well-identified");
               break;
            case kICODE6:
               idr->SetComment("(x,y) is below (on the left of) first line (marker) in grid (line)");
               break;
            case kICODE7:
               idr->SetComment("(x,y) is above (on the right of) last line (marker) in grid (line)");
               break;
            default:
               idr->SetComment("no identification: (x,y) out of range covered by grid (line)");
         }

         if (code < kICODE4) {
            if (code > kICODE0) {
               const_cast < KVIDQAGrid* >(this)->fICode = code;
               idr->IDquality = fICode;
            }
            idr->PID = Ar;
            idr->Aident = kTRUE;
            idr->IDOK   = kTRUE;
         }
      }
   }
}
//________________________________________________________________

void KVIDQAGrid::Initialize()
{
   // General initialisation method for identification grid.
   // This method MUST be called once before using the grid for identifications.
   // The ID lines are sorted.
   // The natural line widths of all ID lines are calculated.
   // Each line are initialized (see KVIDQALine::Initialize method).
   // It determines if this grid represents A vs A/Q or Q vs A/Q map
   // from the name of the Y variable.

   KVIDGrid::Initialize();

   TString yvar = GetVarY();
   yvar.ToUpper();
   if (yvar.Contains("Q")) yvar = "Q";
   else yvar = "A";
   SetVarY(yvar.Data());
   KVIDQALine* line = NULL;
   TIter next(fIdentifiers);
   while ((line = (KVIDQALine*)next())) {
      line->Initialize();
      line->SetVarY(yvar.Data());
   }
}
//________________________________________________________________

void KVIDQAGrid::MakeQvsAoQGrid(Int_t Qmin, Int_t Qmax, Int_t Amin, Int_t Amax)
{
   // Generate Q-A/Q grid.
   // 1 line per Q (charge state) is generated from Qmin to Qmax.
   // 'Amax' is the maximal mass number expected in the 2D matrix.
   // For each line, one point is set at each mass.

   MakeYvsAoQGrid("Q", Qmin, Qmax, Amin, Amax);
}
//________________________________________________________________

void KVIDQAGrid::MakeAvsAoQGrid(Int_t Qmin, Int_t Qmax, Int_t Amin, Int_t Amax)
{
   // Generate A-A/Q grid.
   // 1 line per Q (charge state) is generated from Qmin to Qmax.
   // 'Amax' is the maximal mass number expected in the 2D matrix.
   // For each line, one point is set at each mass.

   MakeYvsAoQGrid("A", Qmin, Qmax, Amin, Amax);
}
//________________________________________________________________

void KVIDQAGrid::MakeYvsAoQGrid(const Char_t* Y, Int_t Qmin, Int_t Qmax, Int_t Amin, Int_t Amax)
{
   // Generate Y-A/Q grid where Y can be "A" or "Q".
   // 1 line per Q is generated from Qmin to Qmax.
   // 'Amax' is the maximal mass number expected in the 2D matrix.
   // Amin is taken equal to 2*Qmin.
   // For each line, one point is set at each mass.

   Double_t x_scale = GetXScaleFactor();
   Double_t y_scale = GetYScaleFactor();
   //clear old lines from grid (and scaling parameters)
   Clear();

   Int_t q, a;
   Int_t* y;

   if (Y[0] == 'A') y = &a;
   else y = &q;
   //loop over Q
   for (q = Qmin; q <= Qmax; q++) {
      KVIDQALine* line = (KVIDQALine*)NewLine("ID");
      Add("ID", line);
      line->SetQ(q);

      Int_t npoints_added = 0;
      for (a = Amin; a <= Amax; a++) {
         //for (a = Amin; a <= Amax; a = a + Amax - Amin) {
         line->SetPoint(npoints_added, (1.*a) / q, 1.*(*y));
         npoints_added++;
      }
   }
   //if this grid has scaling factors, we need to apply them to the result
   if (x_scale != 1)
      SetXScaleFactor(x_scale);
   if (y_scale != 1)
      SetYScaleFactor(y_scale);

   // Set name of X and Y variables
   SetVarX("A/Q");
   SetVarY(Y);
}
//________________________________________________________________

KVIDGraph* KVIDQAGrid::MakeSubsetGraph(TList* lines, TClass* graph_class)
{
   // Create a new graph/grid using the subset of lines of this grid contained in TList 'lines'.
   // By default the new graph/grid will be of the same class as this one, unless graph_class !=0,
   // in which case it must contain the address of a TClass object representing a class which
   // derives from KVIDGraph.
   // A clone of each line will be made and added to the new graph, which will have the same
   // name and be associated with the same ID telescopes as this one.

   if (!graph_class) graph_class = IsA();
   if (!graph_class->InheritsFrom("KVIDGraph")) {
      Error("MakeSubsetGraph", "Called with graph class %s, does not derive from KVIDGraph",
            graph_class->GetName());
      return 0;
   }
   KVIDGraph* new_graph = (KVIDGraph*)graph_class->New();
   new_graph->AddIDTelescopes((TList*)&fTelescopes);
   new_graph->SetOnlyZId(OnlyZId());
   new_graph->SetRuns(GetRuns());
   new_graph->SetVarX(GetVarX());
   new_graph->SetVarY(GetVarY());
   new_graph->SetMassFormula(GetMassFormula());
   // loop over lines in list, make clones and add to graph
   TIter next(lines);
   KVIDentifier* id;
   while ((id = (KVIDentifier*)next())) {
      KVIDentifier* idd = (KVIDentifier*)id->Clone();
      //id->Copy(*idd);
      //idd->ResetBit(kCanDelete);
      new_graph->AddIdentifier(idd);
   }
   return new_graph;
}
//________________________________________________________________

KVIDGraph* KVIDQAGrid::MakeSubsetGraph(Int_t Qmin, Int_t Qmax, const Char_t* graph_class)
{
   // Create a new graph/grid using the subset of lines of this grid with Qmin <= Q <= Qmax.
   // By default the new graph/grid will be of the same class as this one, unless graph_class !="",
   // in which case it must contain the name of a class which derives from KVIDGraph.
   // A clone of each line will be made and added to the new graph, which will have the same
   // name and be associated with the same ID telescopes as this one.

   TList* lines = new TList; // subset of lines to clone
   TIter next(fIdentifiers);
   KVIDQALine* l;
   while ((l = (KVIDQALine*)next())) {
      if (l->GetQ() >= Qmin && l->GetQ() <= Qmax) lines->Add(l);
   }
   TClass* cl = 0;
   if (strcmp(graph_class, "")) cl = TClass::GetClass(graph_class);
   KVIDGraph* gr = MakeSubsetGraph(lines, cl);
   lines->Clear();
   delete lines;
   return gr;
}
//________________________________________________________________

TFile* KVIDQAGrid::FindAMarkers(const Char_t* name_of_data_histo, const Char_t* q_list)
{
   // This method find automatically from the histogram the KVIDQAMarker's of each Q-line
   // listed in the argument q_list. The histogram must be two dimensional
   // plot reprensenting matrix A vs A/Q or Q vs A/Q.

   //Initialize the grid: calculate line widths etc.
   Initialize();

   TH2F* data = (TH2F*)gROOT->FindObject(name_of_data_histo);
   if (!data && (!gIDGridEditor  || !(data = (TH2F*)gIDGridEditor->GetHisto()))) {
      Error("FindAMarkers", "histogram %s not found", name_of_data_histo);
      return 0;
   }

   // list of Q-line to process
   KVNumberList ql(q_list);

   KVIdentificationResult* idr = new KVIdentificationResult;
   KVReconstructedNucleus nuc;

   // store current memory directory
   TDirectory* CWD = gDirectory;

   // store current status IsOnlyQId
   Bool_t q_id = IsOnlyQId();
   SetOnlyQId(kTRUE);

   // create temporary file for tree
   TString fn("IDQAGrid_FindAMarkers.root");
   KVBase::GetTempFileName(fn);
   TFile* tmpfile = new TFile(fn.Data(), "recreate");

   KVHashList l_histos;
   l_histos.SetOwner(kFALSE);
   TH2F* idmap = (TH2F*)data->Clone("idcode_map");
   idmap->Reset();
   TH2F* hh = (TH2F*)idmap->Clone("all");
   l_histos.Add(hh);
   TIter next(GetIdentifiers());
   KVIDQALine* qline = NULL;
   while ((qline = (KVIDQALine*)next())) {
      if (!ql.GetNValues() || ql.Contains(qline->GetQ()))
         l_histos.Add(hh = (TH2F*)idmap->Clone(Form("Q%.2d", qline->GetQ())));
   }

   Int_t tot_events = (Int_t) data->GetSum();
   Int_t events_read = 0;
   Float_t percent = 0., cumul = 10.;

   //loop over data in histo
   for (int i = 1; i <= data->GetNbinsX(); i++) {
      for (int j = 1; j <= data->GetNbinsY(); j++) {

         Stat_t poids = data->GetBinContent(i, j);
         if (poids == 0)
            continue;

         Axis_t x0 = data->GetXaxis()->GetBinCenter(i);
         Axis_t y0 = data->GetYaxis()->GetBinCenter(j);
         Axis_t wx = data->GetXaxis()->GetBinWidth(i);
         Axis_t wy = data->GetYaxis()->GetBinWidth(j);

         Double_t x, y;
         Int_t kmax = 20;

         for (int k = 0; k < kmax; k++) {

            x = gRandom->Uniform(x0 - .5 * wx, x0 + .5 * wx);
            y = gRandom->Uniform(y0 - .5 * wy, y0 + .5 * wy);
            Int_t idcode;
            if (IsIdentifiable(x, y)) {
               Identify(x, y, idr);
               if (AcceptIDForTest()) {
                  nuc.SetIdentification(idr);
                  idcode = GetQualityCode();
                  idmap->SetBinContent(i, j, idcode);

                  hh = (TH2F*)l_histos.FindObject("all");
                  hh->Fill(x, y, poids / kmax);

                  hh = (TH2F*)l_histos.FindObject(Form("Q%.2d", idr->Z));
                  if (hh) hh->Fill(x, y, poids / kmax);
               }
            }
            else {
               idcode = -1;
               idmap->SetBinContent(i, j, idcode);
            }
         }
         events_read += (Int_t) poids;
         percent = (1. * events_read / tot_events) * 100.;
         Increment((Float_t) events_read);      //sends signal to GUI progress bar
         if (percent >= cumul) {
            std::cout << (Int_t) percent << "% processed" << std::endl;
            cumul += 10;
         }
      }
   }

   delete idr;

   // build projection for each 2D maps and find KVIDQAMarker's
   // from them
   next.Reset();
   while ((qline = (KVIDQALine*)next())) {
      hh = (TH2F*)l_histos.FindObject(Form("Q%.2d", qline->GetQ()));
      if (hh) {
         TH1* proj = hh->ProjectionX();
         qline->FindAMarkers(proj);
      }
   }

   CWD->cd();
   SetOnlyQId(q_id);
   return tmpfile;
}
//________________________________________________________________

void KVIDQAGrid::UnDraw()
{
   //Make the grid, its lines and their markers disappear from the current canvas/pad

   if (fPad) fIdentifiers->R__FOR_EACH(KVIDQALine, UnDraw)();
   KVIDGrid::UnDraw();
}
//________________________________________________________________

Int_t KVIDQAGrid::GetNumberOfMasses() const
{
   // returns number of masses identifiable by this grid
   // i.e. sum over Q-lines of the number of A markers

   TIter next(fIdentifiers);
   KVIDQALine* ql = NULL;
   Int_t N = 0;
   while ((ql = (KVIDQALine*)next())) {
      N += ql->GetNumberOfMasses();
   }
   return N;
}
//________________________________________________________________

//void KVIDQAGrid::TestIdentification(TH2F* data, TH1F* h1_q,
//                                    TH2F* h2_q_qxaoq, TH2F* h2_q_a)
//{
//   //This method allows to test the identification capabilities of the grid using data in a TH2F.
//   //We assume that 'data' contains an identification map, whose 'x' and 'y' coordinates correspond
//   //to this grid. Then we loop over every bin of the histogram, perform the identification (if
//   //IsIdentifiable() returns kTRUE) and fill the two histograms with the resulting identification
//   //and its dependence on the 'x'-coordinate of the 'data' histogram,
//   //each identification weighted by the contents of the original data bin.
//   //
//   // Returned histograms:
//   //   h1_q       -- 1D histogram with distribution of identified charge states (Qreal).
//   //   h2_q_qxaoq -- 2D map showing the distibrution on Qreal vs Qint*X plot.
//   //                 where Qint is the integer value of Qreal and X the
//   //                 X-coordinate of the initial data plot.
//   //   h2_q_a     -- 2D map showing the distribution on Qreal vs Areal plot
//   //                 where Areal is the mass identified with the markers.
//   //                 This map is filled only if the grid has OnlyQId()=false.


//   //Initialize the grid: calculate line widths etc.
//   Initialize();

//   KVIdentificationResult* idr = new KVIdentificationResult;

//   h1_q->Reset();
//   h2_q_qxaoq->Reset();
//   Int_t tot_events = (Int_t) data->GetSum();
//   Int_t events_read = 0;
//   Float_t percent = 0., cumul = 10.;
//   Bool_t qaMap = (!IsOnlyQId()) && (h2_q_a);

//   Int_t Amin, Amax, Qmin, Qmax;
//   Double_t AoQmin, AoQmax;
//   GetLimitsOf_A_Q_AoQ(Amin, Amax, Qmin, Qmax, AoQmin, AoQmax);
//   Amin = AoQmin * Qmin;
//   Amax = AoQmax * Qmax;
//   Info("TestIdentification", "Amin %d, Amax %d, Qmin %d, Qmax %d, AoQmin %f, AoQmax %f", Amin, Amax, Qmin, Qmax, AoQmin, AoQmax);

//   // change ranges and titles of histograms
//   TString title;
//   title = "Q_{real} distribution;Q_{real}";
//   h1_q->SetTitle(title.Data());
//   h1_q->SetBins(data->GetNbinsY(), Float_t(Qmin - 1), Float_t(Qmax + 1));

//   title.Form("A vs. Q_{real};Q_{int}#times(%s);Q_{real}", data->GetXaxis()->GetTitle());
//   h2_q_qxaoq->SetTitle(title.Data());
//   h2_q_qxaoq->SetBins(data->GetNbinsX(), Float_t(Amin - 1), Float_t(Amax + 1), data->GetNbinsY(), Float_t(Qmin - 1), Float_t(Qmax + 1));

//   if (qaMap) {
//      title = "A vs. Q_{real};A_{real};Q_{real}";
//      h2_q_a->SetTitle(title.Data());
//      h2_q_a->SetBins(data->GetNbinsX(), Float_t(Amin - 1), Float_t(Amax + 1), data->GetNbinsY(), Float_t(Qmin - 1), Float_t(Qmax + 1));
//   }

//   //loop over data in histo
//   for (int i = 1; i <= data->GetNbinsX(); i++) {
//      for (int j = 1; j <= data->GetNbinsY(); j++) {

//         Stat_t poids = data->GetBinContent(i, j);
//         if (poids == 0)
//            continue;

//         Axis_t x0 = data->GetXaxis()->GetBinCenter(i);
//         Axis_t y0 = data->GetYaxis()->GetBinCenter(j);
//         Axis_t wx = data->GetXaxis()->GetBinWidth(i);
//         Axis_t wy = data->GetYaxis()->GetBinWidth(j);
//         //If bin content ('poids') is <=20, we perform the identification 'poids' times, each time with
//         //randomly-drawn x and y coordinates inside this bin
//         //If 'poids'>20, we perform the identification 20 times and we fill the histograms with
//         //a weight poids/20
//         Double_t x, y;
//         Int_t kmax = (Int_t) TMath::Min(20., poids);
//         Double_t weight = (kmax == 20 ? poids / 20. : 1.);
//         for (int k = 0; k < kmax; k++) {

//            x = gRandom->Uniform(x0 - .5 * wx, x0 + .5 * wx);
//            y = gRandom->Uniform(y0 - .5 * wy, y0 + .5 * wy);
//            if (IsIdentifiable(x, y)) {
//               Identify(x, y, idr);
//               if (AcceptIDForTest()) {
//                  Double_t realQ = 0.;
//                  Double_t realA = 0.;

//                  if (idr->Aident) {
//                     realQ = fRealQ;
//                     realA = idr->PID;
//                  }
//                  else if (idr->Zident) realQ = idr->PID;
//                  h1_q->Fill(realQ, weight);
//                  h2_q_qxaoq->Fill(x * idr->Z, realQ, weight);
//                  if (qaMap) h2_q_a->Fill(realA, realQ, weight);
//               }
//            }
//         }
//         events_read += (Int_t) poids;
//         percent = (1. * events_read / tot_events) * 100.;
//         if (events_read % (tot_events / 10) == 0) Increment((Float_t) events_read); //sends signal to GUI progress bar
//         if (percent >= cumul) {
//            //std::cout << (Int_t) percent << "\% processed" << std::endl;
//            cumul += 10;
//         }
//         gSystem->ProcessEvents();
//      }
//   }

//   delete idr;
//}
//________________________________________________________________

void KVIDQAGrid::GetLimitsOf_A_Q_AoQ(Int_t& Amin, Int_t& Amax, Int_t& Qmin, Int_t& Qmax, Double_t& AoQmin, Double_t& AoQmax)
{

   Amin = Amax = Qmin = Qmax = -1;
   AoQmin = AoQmax = -1.;
   TIter next_line(fIdentifiers);
   KVIDQALine* line = NULL;
   while ((line = (KVIDQALine*)next_line())) {

      Qmin = (Qmin < 0 ? line->GetQ() : TMath::Min(Qmin, line->GetQ()));
      Qmax = TMath::Max(Qmax, line->GetQ());

      line->SortMarkers();
      KVIDQAMarker* first_mk = (KVIDQAMarker*)line->GetMarkers()->First();
      KVIDQAMarker* last_mk  = (KVIDQAMarker*)line->GetMarkers()->Last();
      if (first_mk && last_mk) {
         Amin = (Amin < 0 ? first_mk->GetA() : TMath::Min(Amin, first_mk->GetA()));
         Amax = TMath::Max(Amax, last_mk->GetA());
      }
      AoQmin = (AoQmin < 0 ? line->GetX()[0] : TMath::Min(AoQmin, line->GetX()[0]));
      AoQmax = TMath::Max(AoQmax, line->GetX()[line->GetN() - 1]);

   }
}
//________________________________________________________________

void KVIDQAGrid::QAConvertYaxis()
{
   // convert Q-vs-A/Q grid into A-vs-A/Q grid, and vice versa

   Bool_t isAvsAoQ = (GetVarY()[0] == 'A');
   if (isAvsAoQ) SetVarY("Q");
   else SetVarY("A");
   // change Y coordinates of ID lines
   TIter next(fIdentifiers);
   KVIDQALine* line = NULL;
   while ((line = (KVIDQALine*)next())) {
      line->SetVarY(GetVarY());
      Int_t N = line->GetN();
      for (Int_t i = 0; i < N; i++) {
         Double_t x = line->GetX()[i];
         Double_t y = line->GetY()[i];
         if (isAvsAoQ) y /= x;
         else y *= x;
         line->SetPoint(i, x, y);
         line->GetMarkers()->R__FOR_EACH(KVIDQAMarker, UpdateXandY)();
      }
   }

   // change Y coordinates of cuts
   TIter nextc(fCuts);
   KVIDentifier* cut = NULL;
   while ((cut = (KVIDentifier*)nextc())) {
      Int_t N = cut->GetN();
      for (Int_t i = 0; i < N; i++) {
         Double_t x = cut->GetX()[i];
         Double_t y = cut->GetY()[i];
         if (isAvsAoQ) y /= x;
         else y *= x;
         cut->SetPoint(i, x, y);
      }
   }

   Modified();
}
