/*
$Id: KVIDentifier.cpp,v 1.8 2009/04/20 12:16:50 ebonnet Exp $
$Revision: 1.8 $
$Date: 2009/04/20 12:16:50 $
*/

//Created by KVClassFactory on Mon Apr 14 14:25:38 2008
//Author: franklan

#include "KVIDentifier.h"
#include "KVIDGraph.h"
#include "TF1.h"
#include "TPad.h"
#include "TClass.h"

using namespace std;

ClassImp(KVIDentifier)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDentifier</h2>
<h4>Base class for graphical cuts used in particle identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void  KVIDentifier::SetName(const char* name)
{
   TGraph::SetName(name);
   if (GetParent()) GetParent()->Modified();
}

void KVIDentifier::init()
{
   // Default initialisations
   // We set marker size and style for the underlying TGraph,
   // this way a small circle is drawn at each point.
   // Also SetEditable(kFALSE) is used to stop it being modified using the mouse.

   SetMarkerStyle(kCircle);
   SetMarkerSize(0.8);
   SetLineWidth(2);
   SetEditable(kFALSE);
   fParent = 0;
   fGridOnlyZId = kFALSE;
}

KVIDentifier::KVIDentifier()
   : TCutG(), fIon(1, 1)
{
   // Default constructor
   init();
}

KVIDentifier::~KVIDentifier()
{
   // Destructor
}

KVIDentifier::KVIDentifier(const TCutG& gr)
   : TCutG(gr), fIon(1, 1)
{
   //copy a TCutG
   init();
}

KVIDentifier::KVIDentifier(const TGraph& gr)
   : TCutG(), fIon(1, 1)
{
   //copy a TGraph
   init();
   CopyGraph(gr);
}

KVIDentifier::KVIDentifier(const KVIDentifier& gr)
   : TCutG(), fIon(1, 1)
{
   // copy constructor
   // we do not copy the fParent pointer to parent grid,
   // it is left NULL.
   init();
   gr.Copy(*this);
}

void KVIDentifier::Copy(TObject& obj) const
{
   // Copy attributes of this identifier into 'obj'
   TNamed::Copy(obj);
   TAttLine::Copy((TAttLine&)obj);
   TAttFill::Copy((TAttFill&)obj);
   TAttMarker::Copy((TAttMarker&)obj);
   ((KVIDentifier&)obj).CopyGraph((TGraph*)this);
   ((TCutG&)obj).SetVarX(GetVarX());
   ((TCutG&)obj).SetVarY(GetVarY());
   ((KVIDentifier&)obj).SetZ(GetZ());
   ((KVIDentifier&)obj).SetA(GetA());
   ((KVIDentifier&)obj).SetOnlyZId(OnlyZId());
   ((KVIDentifier&)obj).SetMassFormula(GetMassFormula());
}

KVIDGraph* KVIDentifier::GetParent() const
{
   // Get pointer to parent ID graph
   return fParent;
}

void KVIDentifier::SetParent(KVIDGraph* p)
{
   // Set pointer to parent ID graph
   fParent = p;
}

//_____________________________________________________________________________________________

void KVIDentifier::CopyGraph(TGraph* graph)
{
   // Copy coordinates of points from the TGraph

   Double_t x, y;
   //change number of points
   Set(graph->GetN());
   for (int i = 0; i < GetN(); i++) {
      graph->GetPoint(i, x, y);
      SetPoint(i, x, y);
   }
}

//_____________________________________________________________________________________________

void KVIDentifier::CopyGraph(const TGraph& graph)
{
   // Copy coordinates of points from the TGraph

   Double_t x, y;
   //change number of points
   Set(graph.GetN());
   for (int i = 0; i < GetN(); i++) {
      graph.GetPoint(i, x, y);
      SetPoint(i, x, y);
   }
}

//_____________________________________________________________________________________________

void KVIDentifier::WriteAsciiFile(ofstream& file, const Char_t* name_prefix)
{
   //Write name and coordinates of identifier in file buffer stream
   //Optional string name_prefix will be written just in front of the name of the object.
   //Format is :
   //
   //+classname
   //name_prefix:name_of_object
   //number_of_points
   //x1    y1
   //x2    y2
   //...
   //etc. etc.

   file << '+' << ClassName() << endl;
   if (name_prefix)
      file << name_prefix << ":";
   file << GetName() << endl;
   //any extra stuff defined in child class ?
   WriteAsciiFile_extras(file, name_prefix);
   file << GetN() << endl;
   for (Int_t i = 0; i < GetN(); i++) {
      Double_t x, y;
      GetPoint(i, x, y);
      file << x << "   " << y << endl;
   }
}

//_____________________________________________________________________________________________

void KVIDentifier::ReadAsciiFile(ifstream& file)
{
   //Read coordinates of identifier in file buffer stream
   //Format is :
   //
   //number_of_points
   //x1    y1
   //x2    y2
   //...
   //etc. etc.

   ReadAsciiFile_extras(file);//any extra stuff defined in child class ?
   Int_t N;
   file >> N;
   for (Int_t i = 0; i < N; i++) {
      Double_t x, y;
      file >> x >> y;
      SetPoint(i, x, y);
   }
}

//_____________________________________________________________________________________________

Int_t KVIDentifier::Compare(const TObject*) const
{
   //Dummy method used for sorting, can be redefined in more specific implementations.
   //A priori we don't know how to "order" the identification cuts.

   return 0;
}

//_____________________________________________________________________________________________

void KVIDentifier::Print(Option_t* opt) const
{
   // Print out for identification cut
   // The optional "opt" string, if given, is printed in parentheses after the line's name
   // This is used by KVIDGraph in order to show which are "ID" cuts and which are
   // "OK" cuts (i.e. used to define an identifiable area in a data map).

   cout << ClassName() << " : " << GetName() << "(" << opt << ")" << endl;
   TCutG::Print();
}

//_____________________________________________________________________________________________

void KVIDentifier::Scale(Double_t sx, Double_t sy)
{
   //Scale the coordinates of every point of the cut according to
   //      X_0 ---> sx*X_0
   //      Y_0 ---> sy*Y_0
   //Default argument value '-1' means leave coordinate unchanged
   if (TMath::Abs(sx) == 1 && TMath::Abs(sy) == 1)
      return;
   for (int i = 0; i < fNpoints; i++) {
      if (sx > 0.)
         fX[i] *= sx;
      if (sy > 0.)
         fY[i] *= sy;
   }
}

//_____________________________________________________________________________________________

void KVIDentifier::Scale(TF1* sx, TF1* sy)
{
   // Apply to the coordinates of every point the transformation
   //      X_0 ---> sx(X_0,Y_0)
   //      Y_0 ---> sy(X_0,Y_0)
   // if sx & sy are two-dimensional functions (inherits from TF2),
   // otherwise we apply the transformation
   //      X_0 ---> sx(X_0)
   //      Y_0 ---> sy(Y_0)
   // for sx & sy one-dimensionsal (TF1) functions.

   if (!sx && !sy) return;   // if the two functions are NULL we leave coordinate unchanged
   Bool_t sx_2d, sy_2d;
   sx_2d = (sx && sx->InheritsFrom("TF2"));
   sy_2d = (sy && sy->InheritsFrom("TF2"));
   if (sx && sy) {
      for (Int_t  ii = 0; ii < fNpoints; ii++) {
         Double_t fXp = 0.;  // valeur intermediaire pour calculer X' et Y'
         Double_t fYp = 0.;  //a partir des coordonnees initiales
         if (sx_2d) fXp = sx->Eval(fX[ii], fY[ii]);
         else fXp = sx->Eval(fX[ii]);
         if (sy_2d) fYp = sy->Eval(fX[ii], fY[ii]);
         else fYp = sy->Eval(fY[ii]);
         fX[ii] = fXp;
         fY[ii] = fYp;
      }
   }
   else if (sx) {
      for (Int_t  ii = 0; ii < fNpoints; ii++) {
         if (sx_2d) fX[ii] = sx->Eval(fX[ii], fY[ii]);
         else fX[ii] = sx->Eval(fX[ii]);
      }
   }
   else {
      for (Int_t  ii = 0; ii < fNpoints; ii++) {
         if (sy_2d) fY[ii] = sy->Eval(fX[ii], fY[ii]);
         else fY[ii] = sy->Eval(fY[ii]);
      }
   }
}

//_____________________________________________________________________________________________

void KVIDentifier::WaitForPrimitive()
{
   // Method used to draw a new identifier in the active pad
   // Override in child classes so that gPad->WaitPrimitive has correct arguments

   if (!gPad) return;
   TGraph* gr = (TGraph*) gPad->WaitPrimitive("CUTG", "CutG");
   //copy coordinates of user's line
   CopyGraph(gr);
   //remove TGraph and draw the KVIDentifier in its place
   gPad->GetListOfPrimitives()->Remove(gr);
   delete gr;
   Draw("PL");
}

//_____________________________________________________________________________________________

void KVIDentifier::ExtendLine(Double_t Limit, Option_t* Direction)
{
   // Right-clicking an identifier and selecting this method from the context
   // menu allows to add a straight-line segment at the end or the beginning
   // of the line (whichever is closest to the mouse).
   //
   //  Direction = "" (default) - continue in the direction of first/last segment
   //  Direction = "H", "h", "hori", "HORI" etc. - add horizontal segment
   //  Direction = "v", "V", "vert", "VERT" etc. - add vertical segment

   if (!GetEditable()) return;

   // find closest point
   Int_t px = gPad->GetEventX(); // mouse position
   Int_t py = gPad->GetEventY(); // mouse position
   Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(fX[0])); // first point
   Int_t dpy = py - gPad->YtoAbsPixel(gPad->XtoPad(fY[0])); // first point
   Int_t dist_1 = dpx * dpx + dpy * dpy;
   dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(fX[fNpoints - 1])); // last point
   dpy = py - gPad->YtoAbsPixel(gPad->XtoPad(fY[fNpoints - 1])); // last point
   Int_t dist_N = dpx * dpx + dpy * dpy;
   Int_t ipoint = (dist_N < dist_1 ? fNpoints - 1 : 0);

   // coordinates of new point
   TString opt(Direction);
   opt.ToUpper();
   Double_t newX = fX[ipoint];
   Double_t newY = fY[ipoint];
   if (opt.BeginsWith("H")) newX = Limit;
   else if (opt.BeginsWith("V")) newY = Limit;
   else {
      if (fNpoints < 2) {
         Error("ExtendLine", "Cannot extend line, need at least one segment!");
         return;
      }
      // trouver equation de la droite du dernier/premier segment
      Double_t u, v, uu, vv;
      Int_t iipoint = (ipoint == 0 ? 1 : fNpoints - 2);
      u = fX[ipoint];
      uu = fX[iipoint];
      v = fY[ipoint];
      vv = fY[iipoint];
      Double_t a = vv - v;
      Double_t b = u - uu;
      Double_t c = -(b * v + a * u);
      //
      // Check that 'Limit' the x-coordinate of the new point is larger than the x-coordinate of the last existing point, in order to avoid issues
      if (Limit < u) {
         Error("ExtendLine", "Cannot extend line, with x-coordinate wanted smaller than the one of the existing last point!");
         return;
      }
      //
      // use 'Limit' as x-coordinate of new point
      newX = Limit;
      newY = -(a * newX + c) / b;
   }

   // add point
   Int_t iend = (ipoint > 0 ? fNpoints : 0);
   Double_t** ps = ExpandAndCopy(fNpoints + 1, iend);
   CopyAndRelease(ps, ipoint, fNpoints++, ipoint + 1);

   // To avoid redefinitions in descendant classes
   FillZero(iend, iend + 1);

   if (ipoint > 0) ipoint = fNpoints - 1;
   fX[ipoint] = newX;
   fY[ipoint] = newY;
   gPad->Modified();
}

void KVIDentifier::CloneScaleStore(Int_t newzt, Int_t newar, Double_t dy, Double_t sx, Double_t sy)
{
   // Create a new line from the selected one
   // with a new Z and A (optional)
   // this new line is scale from the selected one with a vertical sy
   // and horizontal sx (optional) factor
   // you need to undraw and draw the grid to see its implementation

   TClass* cl = new TClass(this->IsA()->GetName());
   KVIDentifier* idbis = (KVIDentifier*)cl->New();
   Double_t xx, yy;
   for (Int_t nn = 0; nn < this->GetN(); nn += 1) {
      this->GetPoint(nn, xx, yy);
      idbis->SetPoint(nn, xx, yy + dy);
   }
   idbis->SetOnlyZId(OnlyZId());
   idbis->SetZ(newzt);
   idbis->SetMassFormula(GetMassFormula());
   idbis->SetEditable(IsEditable());
   if (newar != -1) {
      idbis->SetA(newar);
   }
   if ((sx > 0.) && (sy > 0.)) idbis->Scale(sx, sy);
   this->GetParent()->AddIdentifier(idbis);

   this->GetParent()->UnDraw();
   this->GetParent()->Draw();


   delete cl;
}

//______________________________________________________________________________
Int_t KVIDentifier::InsertPoint()
{
   // Insert a new point at the mouse position
   if (!GetEditable()) return -2;

   Int_t px = gPad->GetEventX();
   Int_t py = gPad->GetEventY();

   Info("InsertPoint", "x=%d y=%d", px, py);
   Int_t ifound = -1;
   //while ( gPad->XtoAbsPixel(gPad->XtoPad(fX[ii]))>px && ii<fNpoints-1) ii+=1;
   //Recherche en pixel absolu
   //pour trouver le point a partir duquel le TGraph
   //doit bouger
   Print("");
   for (Int_t ii = 0; ii < fNpoints; ii += 1) {
      Info("InsertPoint", "np=%d ii=%d %lf %lf %d\n",
           fNpoints,
           ii,
           fX[ii],
           gPad->XtoPad(fX[ii]),
           gPad->XtoAbsPixel(gPad->XtoPad(fX[ii]))
          );
      if (gPad->XtoAbsPixel(gPad->XtoPad(fX[ii])) > px) {
         ifound = ii;
         break;
      }
   }

   Info("InsertPoint", "point ifound=%d", ifound);

   Double_t deltaX = fX[ifound] - fX[ifound - 1];
   Double_t aa = (fY[ifound] - fY[ifound - 1]) / (deltaX);
   Double_t bb = fY[ifound] - fX[ifound] * aa;
   //Conversion du px pour le TGraph et extrapole le new Y
   Double_t newX = gPad->PadtoX(gPad->AbsPixeltoX(px));
   Double_t newY = aa * newX + bb;
   Info("InsertPoint", "nouveau point %lf %lf", newX, newY);

   //Dernier point du graph
   Double_t lastX = fX[fNpoints - 1];
   Double_t lastY = fY[fNpoints - 1];
   //Le dernier point est ajoute en fin de graph
   //fNpoint -> fNpoint+1
   SetPoint(fNpoints, lastX, lastY);
   //On deplace les autres points
   for (Int_t ii = fNpoints - 2; ii >= ifound; ii -= 1)
      SetPoint(ii, fX[ii - 1], fY[ii - 1]);
   //Ajout du nouveau point
   SetPoint(ifound, newX, newY);

//   Print("");

   if (gPad) gPad->Modified();

   return ifound;
}

//______________________________________________________________________________
Int_t KVIDentifier::ContinueDrawing()
{
   // Continue to draw an existing the line
   if (!GetEditable()) return -2;

   KVIDentifier* gr = (KVIDentifier*)this->IsA()->New();
   gr->WaitForPrimitive();
   ChechHierarchy(gr);

   Int_t np = gr->GetN();

   Double_t last, first, yy;
   gr->GetPoint(np - 1, last, yy);
   gr->GetPoint(0, first, yy);

   Double_t xmax = fX[fNpoints - 1];
   Double_t xmin = fX[0];

   Info("ContinueDrawing", "Existing Line %lf %lf, Added line %lf %lf", xmin, xmax, last, first);

   if (first > xmax) {
      //A rajouter apres (a droite) de la ligne existante
      Double_t xx;
      for (Int_t ii = 0; ii < np; ii += 1) {
         gr->GetPoint(ii, xx, yy);
         this->SetPoint(fNpoints, xx, yy);
      }
   }
   else if (last < xmin) {
      //A rajouter avant (a gauche) la ligne existante
      Double_t xx;
      for (Int_t ii = 0; ii < fNpoints; ii += 1) {
         this->GetPoint(ii, xx, yy);
         gr->SetPoint(gr->GetN(), xx, yy);
      }

      for (Int_t ii = 0; ii < gr->GetN(); ii += 1) {
         gr->GetPoint(ii, xx, yy);
         this->SetPoint(ii, xx, yy);
      }
   }
   else {
      Info("ContinueDrawing", "Faire une extension a droite ou a gauche\nsans recouvrement avec la ligne existante");
   }

   delete gr;
   gPad->Modified();

   return np;
}

//______________________________________________________________________________
void KVIDentifier::ChechHierarchy(KVIDentifier* gr)
{
   //Check if the line has been draw from left to right
   //or right to left
   //In this last case, we invert the filling order

   Int_t np = gr->GetN();
   Double_t* xx = gr->GetX();
   Double_t* yy = gr->GetY();
   Double_t Delta = xx[np - 1] - xx[0];
   if (Delta > 0) return;

   Info("ChechHierarchy", "Invert filling order");
   Double_t* xp = new Double_t[np];
   Double_t* yp = new Double_t[np];
   for (Int_t nn = 0; nn < np; nn += 1) {
      xp[nn] = xx[nn];
      yp[nn] = yy[nn];
   }

   for (Int_t ii = 1; ii <= np; ii += 1) {
      gr->SetPoint(ii - 1, xp[np - ii], yp[np - ii]);
   }
   delete [] xp;
   delete [] yp;

}

//______________________________________________________________________________
Int_t KVIDentifier::AddPointAtTheEnd()
{
   // Insert a new point at the end
   // its position is extrapolated from the two last points
   // assuming linear evolution (yy = a*xx + bb)
   // The KVIDentifier has to have at least 2 points
   if (!GetEditable()) return -2;
   if (fNpoints < 2) return -3;

   Int_t np = fNpoints;
   Double_t deltaX = fX[np - 1] - fX[np - 2];
   Double_t aa = (fY[np - 1] - fY[np - 2]) / (deltaX);
   Double_t bb = fY[np - 1] - fX[np - 1] * aa;
   //yy = aa*xx+bb -> bb = yy-aa*xx
   Double_t newX = fX[np - 1] + deltaX;
   Double_t newY = aa * newX + bb;
   SetPoint(np, newX, newY);
   gPad->Modified();
   return np + 1;

}
//______________________________________________________________________________
Int_t KVIDentifier::RemoveFirstPoint()
{
   // Remove the first point
   // The KVIDentifier has to have at least 2 points
   if (!GetEditable()) return -2;
   if (fNpoints < 2) return -3;

   RemovePoint(0);
   gPad->Modified();
   return fNpoints - 1;

}

//______________________________________________________________________________
Int_t KVIDentifier::RemoveLastPoint()
{
   // Remove the first point
   // The KVIDentifier has to have at least 2 points
   if (!GetEditable()) return -2;
   if (fNpoints < 2) return -3;

   RemovePoint(GetN() - 1);
   gPad->Modified();
   return fNpoints - 1;
}

Int_t KVIDentifier::IncreaseNumberOfPoints()
{
   if (!GetEditable()) return -2;
   if (fNpoints < 2) return -1;

   Int_t nNpoints = (fNpoints - 1) * 2 + 1;
   Double_t* nX = new Double_t[nNpoints];
   Double_t* nY = new Double_t[nNpoints];

   for (int ii = 0; ii < nNpoints; ii++) {
      if (!(ii % 2)) {
         nX[ii] = fX[ii / 2];
         nY[ii] = fY[ii / 2];
      }
      else {
         nX[ii] = 0.5 * (fX[(ii - 1) / 2] + fX[(ii + 1) / 2]);
         nY[ii] = Eval(nX[ii], 0, "S");
      }
   }

   Set(0);
   for (int ii = 0; ii < nNpoints; ii++) SetPoint(ii, nX[ii], nY[ii]);

   delete[] nX;
   delete[] nY;

   if (gPad) gPad->Modified();

   return fNpoints;
}

Int_t KVIDentifier::DecreaseNumberOfPoints()
{
   if (!GetEditable()) return -2;
   if (fNpoints < 2) return -1;

   Int_t nNpoints = fNpoints / 2 + 1; //(fNpoints%2)?(fNpoints/2):((fNpoints+1)/2);
   Double_t* nX = new Double_t[nNpoints];
   Double_t* nY = new Double_t[nNpoints];

   nX[0] = fX[0];
   nY[0] = fY[0];

   for (int ii = 1; ii < nNpoints - 1; ii++) {
      nX[ii] = fX[ii * 2];
      nY[ii] = fY[ii * 2];
   }

   nX[nNpoints - 1] = fX[fNpoints - 1];
   nY[nNpoints - 1] = fY[fNpoints - 1];

   Set(0);
   for (int ii = 0; ii < nNpoints; ii++) SetPoint(ii, nX[ii], nY[ii]);

   delete[] nX;
   delete[] nY;

   if (gPad) gPad->Modified();

   return fNpoints;
}

Int_t KVIDentifier::SortPoints(Bool_t ascending)
{
   Sort(&TGraph::CompareX, ascending);
   return 0;
}

//______________________________________________________________________________
Double_t KVIDentifier::GetPID() const
{
   if (OnlyZId()) return (Double_t)GetZ();
   return (GetZ() + 0.1 * (GetA() - 2.*GetZ()));
}
