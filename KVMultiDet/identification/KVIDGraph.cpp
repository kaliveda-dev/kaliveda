/*
$Id: KVIDGraph.cpp,v 1.9 2009/04/28 09:07:47 franklan Exp $
$Revision: 1.9 $
$Date: 2009/04/28 09:07:47 $
*/

//Created by KVClassFactory on Mon Apr 14 13:42:47 2008
//Author: franklan

#include "KVIDGraph.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TRandom.h"
#include "TClass.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "KVIDGridManager.h"
#include "KVBase.h"
#include "TEnv.h"
#include "TPluginManager.h"
#include "TTree.h"
#include "TROOT.h"
#include "KVIdentificationResult.h"

using namespace std;

ClassImp(KVIDGraph)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDGraph</h2>
<h4>Base class for particle identification in a 2D map</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVIDGraph::ResetPad()
{
   cout << "The pad showing graph " << GetName() << " has been closed !" << endl;

   if (fPad) {
      fPad->Disconnect("Closed()", this, "ResetPad()");
      if (fPad->GetCanvas()) fPad->GetCanvas()->Disconnect("Cleared(TVirtualPad*)", this, "ClearPad(TVirtualPad*)");
   }
   fPad = 0;

}

void KVIDGraph::ClearPad(TVirtualPad* pad)
{
   if (fPad == pad) {
      cout << "The pad showing graph " << GetName() << " has been cleared" << endl;
      fPad->Disconnect("Closed()", this, "ResetPad()");
      if (fPad->GetCanvas()) fPad->GetCanvas()->Disconnect("Cleared(TVirtualPad*)", this, "ClearPad(TVirtualPad*)");
      fPad = 0;
   }
}

Bool_t KVIDGraph::fAutoAddGridManager = kTRUE;

void KVIDGraph::init()
{
   // Initialisations, used by constructors
   // All graphs are added to gIDGridManager (if it exists).

   fIdentifiers = new KVList;
   fCuts = new KVList;
   fIdentifiers->SetCleanup();
   fCuts->SetCleanup();
   fXmin = fYmin = fXmax = fYmax = 0;
   fPar = new KVNameValueList;
   fLastScaleX = 1.0;
   fLastScaleY = 1.0;
   fOnlyZId = kFALSE;
   fPad = 0;
   SetName("");
   SetEditable(kFALSE);
   if (fAutoAddGridManager && gIDGridManager) gIDGridManager->AddGrid(this);
   fMassFormula = 0;
   fLastSavedVersion = NULL;
}

//________________________________________________________________________________

// void KVIDGraph::Browse(TBrowser* b)
// {
//    // This method is executed when an object is double-clicked in a browser
//    // such as the list of grids in KVIDGridManagerGUI
//    //
//    // Overrides default TGraph::Browse (which calls Draw()) in order to open
//    // the grid in the KVIDGridEditor canvas (if one exists).
//    // If no KVIDGridEditor exists, we perform the default action (Draw()).
//    //
//
//    if( gIDGridEditor ){
//       // avant d'editer la grille, on en fait une copie pour
//       // pouvoir revenir en arriere
//       UpdateLastSavedVersion();
//       if(gIDGridEditor->IsClosed()) gIDGridEditor->StartViewer();
//       gIDGridEditor->SetGrid(this);
//    }
//    else
//       TGraph::Browse(b);
// }

//________________________________________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVIDGraph::Copy(TObject& obj) const
#else
void KVIDGraph::Copy(TObject& obj)
#endif
{
   //Copy this to 'obj'
   TNamed::Copy(obj);
   TAttLine::Copy((TAttLine&)obj);
   TAttFill::Copy((TAttFill&)obj);
   TAttMarker::Copy((TAttMarker&)obj);
   ((TCutG&)obj).SetVarX(GetVarX());
   ((TCutG&)obj).SetVarY(GetVarY());
   KVIDGraph& grid = (KVIDGraph&) obj;
   grid.fOnlyZId = const_cast <KVIDGraph*>(this)->fOnlyZId;
   grid.SetPattern(fPattern);
   //as in WriteAsciiFile, we need to remove any scaling factors from the coordinates
   //before copying the coordinates, then we set the scaling factors at the end

   //remove scaling if there is one
   if (fLastScaleX != 1 || fLastScaleY != 1)
      const_cast < KVIDGraph* >(this)->Scale(1. / fLastScaleX,
                                             1. / fLastScaleY);

   fIdentifiers->Copy((TObject&)(*grid.GetIdentifiers()));
   fCuts->Copy((TObject&)(*grid.GetCuts()));
   // set mass formula of grid (and identifiers)
   grid.SetMassFormula(GetMassFormula());
   //copy all parameters EXCEPT scaling parameters
   KVNamedParameter* par = 0;
   for (int i = 0; i < fPar->GetNpar(); i++) { //loop over all parameters
      par = fPar->GetParameter(i);
      TString parname(par->GetName());
      if (!parname.Contains("ScaleFactor"))
         grid.fPar->SetValue(*par);
   }
   //restore scaling to this grid if there was one, and apply it to the copied grid
   if (fLastScaleX != 1 || fLastScaleY != 1) {
      const_cast < KVIDGraph* >(this)->Scale(fLastScaleX, fLastScaleY);
      grid.SetXScaleFactor(fLastScaleX);
      grid.SetYScaleFactor(fLastScaleY);
   }
}

//________________________________________________________________________________

KVIDGraph::KVIDGraph() : fRunList(""), fDyName(""), fPattern("")

{
   // Default constructor
   init();
}

//________________________________________________________________________________

KVIDGraph::KVIDGraph(const KVIDGraph& grid) : TCutG(), fRunList(""), fDyName(""), fPattern("")
{
   //Copy constructor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   grid.Copy(*this);
#else
   ((KVIDGraph&) grid).Copy(*this);
#endif
}

KVIDGraph::~KVIDGraph()
{
   // Destructor

   fIdentifiers->Delete();
   delete fIdentifiers;
   fCuts->Delete();
   delete fCuts;
   delete fPar;
}

//________________________________________________________________________________

void KVIDGraph::Clear(Option_t*)
{
   // reset the grid (destroying any previously defined identifiers) ready to start anew.
   // resets axis limits
   // scaling factors (if any) are removed

   fIdentifiers->Delete();
   fCuts->Delete();
   fXmin = fYmin = fXmax = fYmax = 0;
   SetXScaleFactor();
   SetYScaleFactor();
   Modified();
}

////////////////////////////////////////////////////////////////////////////////

void KVIDGraph::SetXScaleFactor(Double_t s)
{
   //Set scaling factor for X-axis - rescales all objects with this factor
   //SetXScaleFactor() or SetXScaleFactor(0) removes scale factor
   if (s > 0) {
      fPar->SetValue("XScaleFactor", s);
      Scale(s / fLastScaleX);
      fLastScaleX = s;
   }
   else {
      fPar->RemoveParameter("XScaleFactor");
      Scale(1.0 / fLastScaleX);
      fLastScaleX = 1.0;
   }
}

////////////////////////////////////////////////////////////////////////////////

void KVIDGraph::SetYScaleFactor(Double_t s)
{
   //Set scaling factor for Y-axis - rescales all objects with this factor
   //SetYScaleFactor() or SetYScaleFactor(0) removes scale factor
   if (s > 0) {
      fPar->SetValue("YScaleFactor", s);
      Scale(-1.0, s / fLastScaleY);
      fLastScaleY = s;
   }
   else {
      fPar->RemoveParameter("YScaleFactor");
      Scale(-1.0, 1.0 / fLastScaleY);
      fLastScaleY = 1.0;
   }
}

////////////////////////////////////////////////////////////////////////////////

Double_t KVIDGraph::GetXScaleFactor()
{
   //Return scaling factor for X-axis
   //If factor not defined, returns 1
   Double_t s = fPar->GetDoubleValue("XScaleFactor");
   if (s > 0)
      return s;
   return 1.0;
}

////////////////////////////////////////////////////////////////////////////////

Double_t KVIDGraph::GetYScaleFactor()
{
   //Return scaling factor for Y-axis
   //If factor not defined, returns 1
   Double_t s = fPar->GetDoubleValue("YScaleFactor");
   if (s > 0)
      return s;
   return 1.0;
}

////////////////////////////////////////////////////////////////////////////////

KVIDentifier* KVIDGraph::GetIdentifier(Int_t Z, Int_t A) const
{
   // Return pointer to identifier with atomic number Z and mass number A.
   // If this is a 'OnlyZId()' graph we ignore A.

   KVIDentifier* id = 0;
   if (!IsOnlyZId()) {
      unique_ptr<KVSeqCollection> isotopes(fIdentifiers->GetSubListWithMethod(Form("%d", Z), "GetZ"));
      TIter next(isotopes.get());
      while ((id = (KVIDentifier*)next())) if (id->GetA() == A) break;
   }
   else {
      TIter next(fIdentifiers);
      while ((id = (KVIDentifier*)next())) if (id->GetZ() == Z) break;
   }
   return id;
}

////////////////////////////////////////////////////////////////////////////////

void KVIDGraph::RemoveIdentifier(KVIDentifier* id)
{
   // Remove and destroy identifier
   fIdentifiers->Remove(id);
   delete id;
   Modified();
}

////////////////////////////////////////////////////////////////////////////////

void KVIDGraph::RemoveCut(KVIDentifier* cut)
{
   // Remove and destroy cut
   fCuts->Remove(cut);
   delete cut;
   Modified();
}

////////////////////////////////////////////////////////////////////////////////

void KVIDGraph::WriteParameterListOfIDTelescopes()
{
   // Fill parameter "IDTelescopes" with list of names of telescopes associated
   // with this grid, ready to write in ascii file

   // if list of telescope pointers is empty, do nothing
   // this is in case there are telescope names already in the IDTelescopes parameter
   // but they are not telescopes in the current multi det array
   if (!fTelescopes.GetEntries()) return;
   fPar->SetValue("IDTelescopes", "");
   KVString tel_list = GetNamesOfIDTelescopes();
   fPar->SetValue("IDTelescopes", tel_list);
}

////////////////////////////////////////////////////////////////////////////////

//_______________________________________________________________________________________________//

void KVIDGraph::WriteAsciiFile(const Char_t* filename)
{
   //Open, write and close ascii file containing this grid.

   ofstream gridfile(filename);
   WriteToAsciiFile(gridfile);
   gridfile.close();
}

//_______________________________________________________________________________________________//

void KVIDGraph::WriteToAsciiFile(ofstream& gridfile)
{
   //Write info on this grid in already-open ascii file stream 'gridfile'.
   //This begins with the line
   //++ClassName
   //where '"ClassName'" is the name of the class of the actual instance of this object
   //followed by informations on the grid and its objects.
   //
   //The names associated to the X & Y axis of the identification map are written
   //on lines beginning with
   //<VARX>
   //<VARY>
   //Any associated parameters are written before the objects, starting with
   //<PARAMETER>
   //then the name of the parameter and its value
   //e.e.
   //<PARAMETER> ChIo pressure=45.5
   //A single '!' on a line by itself signals the end of the informations for this grid.
   //
   //Scaling factors
   //Any scaling factors are written in the file as
   //<PARAMETER> XScaleFactor=...
   //<PARAMETER> YScaleFactor=...
   //However the coordinates written in the file are the unscaled factors
   //When the file is read back again, the unscaled coordinates are read in and
   //then the scaling is applied.

   gridfile << "# ASCII file generated by " << ClassName() <<
            "::WriteToAsciiFile" << endl;
   gridfile << "# ID Graph Name : " << GetName() << endl;
   gridfile << "# This file can be read using " << ClassName() <<
            "::ReadFromAsciiFile" << endl;
   gridfile << "# " << endl;
   gridfile << "++" << ClassName() << endl;

   // write name if given
   if (fName != "") gridfile << "<NAME> " << fName.Data() << endl;
   //write X & Y axis names
   gridfile << "<VARX> " << GetVarX() << endl;
   gridfile << "<VARY> " << GetVarY() << endl;
   //write parameters
   WriteParameterListOfIDTelescopes();
   KVNamedParameter* par = 0;
   for (int i = 0; i < fPar->GetNpar(); i++) { //loop over all parameters
      par = fPar->GetParameter(i);
      gridfile << "<PARAMETER> " << par->GetName() << "=" << par->GetString() << endl;
   }

   //write fOnlyZId & mass formula
   if (IsOnlyZId()) gridfile << "OnlyZId " << GetMassFormula() << endl;

   //remove scaling if there is one
   if (fLastScaleX != 1 || fLastScaleY != 1)
      Scale(1. / fLastScaleX, 1. / fLastScaleY);

   //Write identifiers
   TIter next_IDline(fIdentifiers);
   KVIDentifier* line;
   while ((line = (KVIDentifier*) next_IDline())) {
      line->WriteAsciiFile(gridfile, "ID");
   }
   //Write cuts
   TIter next_OKline(fCuts);
   while ((line = (KVIDentifier*) next_OKline())) {
      line->WriteAsciiFile(gridfile, "OK");
   }
   gridfile << "!" << endl;

   //restore scaling if there is one
   if (fLastScaleX != 1 || fLastScaleY != 1)
      Scale(fLastScaleX, fLastScaleY);

   // if a back-up copy had previously been created (by starting the editor)
   // we replace it by the version read from file
   if (fLastSavedVersion) UpdateLastSavedVersion();
}

void KVIDGraph::UpdateLastSavedVersion()
{
   //update last saved version. mkae copy of current state of graph.
   if (!fLastSavedVersion) {
      SetAutoAdd(kFALSE); // disable auto add to grid manager - or we'll have every grid appearing twice!
      fLastSavedVersion = (KVIDGraph*)IsA()->New();
      SetAutoAdd(); // re-enable auto add
   }
   Copy(*fLastSavedVersion);
}

void KVIDGraph::RevertToLastSavedVersion()
{
   // Revert to last saved version of grid
   // this will destroy any existing lines in the grid and replace them with
   // copies of the lines as they were at the last moment the grid was saved
   // If the grid is (was) visible in a pad, we redraw the new lines.

   if (!fLastSavedVersion) {
      Info("RevertToLastSavedVersion", "No saved version to revert to! (Sorry)");
      return;
   }
   Bool_t wasDrawn = kFALSE;
   if (fPad) {
      // if grid was visible, remove it from pad before modifying (deleting) lines
      wasDrawn = kTRUE;
      fPad->cd();// make sure pad with grid is active (gPad) pad
      UnDraw();
   }
   Clear();
   fLastSavedVersion->Copy(*this);
   if (wasDrawn) {
      // if grid was visible, redraw it with new lines
      Draw();
   }
}

//_______________________________________________________________________________________________//

void KVIDGraph::ReadFromAsciiFile(ifstream& gridfile)
{
   //Read info from already open ascii file stream containing each line of this grid.
   //Any previous lines in the grid are destroyed.
   //Comment lines begin with #
   //Grid info begins with
   //++ClassName
   //where ClassName is actual class name of instance of this object
   //The names associated to the X & Y axis of the identification map are written
   //on lines beginning with
   //<VARX>
   //<VARY>
   //Parameter lines begin with <PARAMETER>
   //New lines begin with +name_of_class
   //'!' signals end of grid info
   //Any scaling factors are applied once the coordinates have been read

   //reset grid - destroy old lines, axis limits...
   Clear();
   //clear parameters
   fPar->Clear();

   KVString s;
   int line_no = 0; // counter for lines read
   SetOnlyZId(kFALSE);
   Int_t mass_formula = -1;

   while (gridfile.good()) {
      //read a line
      s.ReadLine(gridfile);
      if (s.BeginsWith('!')) {
         //end of grid signalled
         break;
      }
      else if (s.BeginsWith("++")) {    //will only happen if we are reading a file using ReadAsciiFile(const char*)
         //check classname
         s.Remove(0, 2);
         s.Remove(TString::kBoth, ' '); //remove whitespace
         if (s != ClassName())
            Warning("ReadFromAsciiFile(ofstream&)",
                    "Class name in file %s does not correspond to this class (%s)",
                    s.Data(), ClassName());
      }
      else if (s.BeginsWith("<NAME>")) {
         //name of grid
         s.Remove(0, 7);
         s.Remove(TString::kBoth, ' '); //remove whitespace
         SetName(s.Data());
      }
      else if (s.BeginsWith("<VARX>")) {
         //x-axis definition
         s.Remove(0, 7);
         s.Remove(TString::kBoth, ' '); //remove whitespace
         SetVarX(s.Data());
      }
      else if (s.BeginsWith("<VARY>")) {
         //y-axis definition
         s.Remove(0, 7);
         s.Remove(TString::kBoth, ' '); //remove whitespace
         SetVarY(s.Data());
      }
      else if (s.BeginsWith("<PARAMETER>")) {
         //parameter definition
         s.Remove(0, 11);
         //split into tokens separated by '='
         TObjArray* toks = s.Tokenize('=');
         if (toks->GetEntries() > 1) {
            TString name =
               ((TObjString*) toks->At(0))->GetString().
               Strip(TString::kBoth);
            KVString value(((TObjString*) toks->At(1))->GetString());
            value.Remove(TString::kBoth, ' '); //remove whitespace
            if (name != "" && value != "") {
               if (value.IsFloat())
                  fPar->SetValue(name.Data(), value.Atof());
               else if (value.IsDigit())
                  fPar->SetValue(name.Data(), value.Atoi());
               else
                  fPar->SetValue(name.Data(), value.Data());
            }
         }
         delete toks;           //must clean up
      }
      else if (s.BeginsWith("<LIST>")) {        // number list definition
         s.Remove(0, 6);        // get rid of "<LIST>"
         //split into tokens separated by '='
         TObjArray* toks = s.Tokenize('=');
         if (toks->GetEntries() > 1) {
            TString name =
               ((TObjString*) toks->At(0))->GetString().
               Strip(TString::kBoth);
            TString list(((TObjString*) toks->At(1))->GetString());
            //set appropriate list
            if (name == "Runs")
               SetRuns(list.Data());
            else
               fPar->SetValue(name.Data(), list.Data());
         }
         delete toks;           //must clean up
      }
      else if (s.BeginsWith("OnlyZId")) {
         SetOnlyZId(kTRUE);
         s.ReplaceAll("OnlyZId", "");
         s.Remove(TString::kBoth, ' ');
         if (s != "") { //older versions did not write mass formula after OnlyZId
            mass_formula = s.Atoi();
            if (mass_formula > -1) SetMassFormula(mass_formula);
         }
      }
      else if (s.BeginsWith('+')) {
         //New line
         line_no++;
         //Get name of class by stripping off the '+' at the start of the line
         s.Remove(0, 1);
         TString lineclass = s;
         //next line is type ("ID" or "OK") followed by ':', followed by name of line
         s.ReadLine(gridfile);
         //split into tokens separated by ':'
         TObjArray* toks = s.Tokenize(':');
         TString type = ((TObjString*) toks->At(0))->GetString();
         TString name = "";
         // make sure the line actually has a name !
         if (toks->GetEntries() > 1) name = ((TObjString*) toks->At(1))->GetString();
         else {
            // print warning allowing to find nameless line in file
            Warning("ReadFromAsciiFile",
                    "In graph : %s\nIdentifier with no name. Identifier type = %s. Line number in graph = %d",
                    GetName(), type.Data(), line_no);
         }
         delete toks;           //must clean up
         ReadIdentifierFromAsciiFile(name, type, lineclass, gridfile);
      }
   }
   fLastScaleX = GetXScaleFactor();
   fLastScaleY = GetYScaleFactor();
   if (fLastScaleX != 1 || fLastScaleY != 1) {
      Scale(fLastScaleX, fLastScaleY);
   }
   BackwardsCompatibilityFix();
   //set runlist
   if (fPar->HasParameter("Runlist")) SetRuns(fPar->GetStringValue("Runlist"));
   else SetRuns("");

   // if a back-up copy had previously been created (by starting the editor)
   // we replace it by the version read from file
   if (fLastSavedVersion) UpdateLastSavedVersion();
}

//_______________________________________________________________________________________________//

void KVIDGraph::ReadIdentifierFromAsciiFile(TString& name, TString& type, TString& cl, ifstream& gridfile)
{
   // Read in new identifier object from file

   KVIDentifier* line = New(cl.Data());
   line->SetName(name.Data());
   Add(type, line);
   //now use ReadAscii method of class to read coordinates and other informations
   line->ReadAsciiFile(gridfile);
}

//_______________________________________________________________________________________________//

void KVIDGraph::ReadAsciiFile(const Char_t* filename)
{
   //Open, read and close ascii file containing this grid.
   //Any previous lines in the grid are destroyed.
   //Comment lines begin with #
   //Grid info begins with
   //++ClassName
   //where ClassName is actual class name of instance of this object
   //New lines begin with +name_of_class
   //'!' signals end of grid

   ifstream gridfile(filename);

   if (gridfile.good())
      ReadFromAsciiFile(gridfile);

   gridfile.close();
}

//_______________________________________________________________________________________________//

KVIDentifier* KVIDGraph::New(const Char_t* id_class)
{
   //Create new object of class "id_class" which derives from KVIDentifier

   KVIDentifier* line = 0;
   TClass* clas = TClass::GetClass(id_class);
   if (!clas) {
      Error("New",
            "%s is not a valid classname. No known class.", id_class);
   }
   else {
      if (!clas->InheritsFrom(KVIDentifier::Class())) {
         Error("New",
               "%s is not a valid class deriving from KVIDentifier",
               id_class);
      }
      else {
         line = (KVIDentifier*) clas->New();
      }
   }
   return line;
}

//_______________________________________________________________________________________________//

void KVIDGraph::Add(TString type, KVIDentifier* id)
{
   // Add the object to the list of cuts or identifiers:
   //  type = "ID"            -->  calls AddIdentifier(id)
   //  type = "OK" or "cut"   -->  calls AddCut(id)

   type.ToUpper();
   if (type == "ID") AddIdentifier(id);
   else if (type == "OK" || type == "CUT") AddCut(id);
}

//_______________________________________________________________________________________________//

KVIDentifier* KVIDGraph::Add(TString type, TString classname)
{
   // Create and add the object to the list of cuts or identifiers:
   //  type = "ID"            -->  calls AddIdentifier(id)
   //  type = "OK" or "cut"   -->  calls AddCut(id)

   type.ToUpper();
   KVIDentifier* id = New(classname);
   if (!id) return 0;
   if (type == "ID") AddIdentifier(id);
   else if (type == "OK" || type == "CUT") AddCut(id);
   return id;
}

//_______________________________________________________________________________________________//

void KVIDGraph::Draw(Option_t*)
{
   //Draw all objects in graph on the current display, if one exists: i.e. in order to superimpose the grid
   //on a 2D display of data, use KVIDGraph::Draw() (no option "same" needed).
   //
   //If no TCanvas exists, or if one exists but it is empty,  a frame large enough to display all
   //the objects in the graph will be generated.
   //
   //The title of the canvas is set to the name of the graph
   //
   //If the graph is already displayed (i.e. if fPad!=0), we call UnDraw() in order to remove it from the display.
   //This is so that double-clicking a graph in the IDGridManagerGUI list makes it disappear if it is already drawn.

   if (fPad) {  /* graph already displayed. undraw it! */
      UnDraw();
      return;
   }

   if (!gPad) {
      fPad = new TCanvas("c1", GetName());
   }
   else {
      fPad = (TPad*)gPad;
      gPad->SetTitle(GetName());
   }
   if (!gPad->GetListOfPrimitives()->GetSize()) {
      //calculate size of pad necessary to show graph
      if (GetXmin() == GetXmax())
         const_cast < KVIDGraph* >(this)->FindAxisLimits();
      gPad->DrawFrame(GetXmin(), GetYmin(), GetXmax(), GetYmax());
   }
   {
      fIdentifiers->R__FOR_EACH(KVIDentifier, Draw)("PL");
   }
   {
      fCuts->R__FOR_EACH(KVIDentifier, Draw)("PL");
   }
   gPad->Modified();
   gPad->Update();
   // connect canvas' Closed() signal to KVIDGraph::ResetPad so that if the
   // canvas GUI is closed (thereby deleting the fPad object), we reset fPad
   // and do not try to Undraw in a non-existent canvas
//   fPad->Connect("Closed()",  "KVIDGraph", this, "ResetPad()");
//   if (fPad->GetCanvas()) fPad->GetCanvas()->Connect("Cleared(TVirtualPad*)", "KVIDGraph", this, "ClearPad(TVirtualPad*)");
}

//_______________________________________________________________________________________________//

void KVIDGraph::UnDraw()
{
   //Make the graph disappear from the current canvas/pad
   //In case any objects have (accidentally) been drawn more than once, we keep calling
   //gPad->GetListOfPrimitives()->Remove() as long as gPad->GetListOfPrimitives()->FindObject()
   //returns kTRUE for each identifier.

//   if (!fPad) {
//      Error("UnDraw", "Cannot undraw, no pad stored ??");
//      return;
//   }
//   TIter next_id(fIdentifiers);
//   KVIDentifier* line;
//   //remove the rest of the lines
//   while ((line = (KVIDentifier*) next_id())) {
//      while (fPad->GetListOfPrimitives()->FindObject(line))
//         fPad->GetListOfPrimitives()->Remove(line);
//   }
//   TIter next_ok(fCuts);
//   while ((line = (KVIDentifier*) next_ok())) {
//      while (fPad->GetListOfPrimitives()->FindObject(line))
//         fPad->GetListOfPrimitives()->Remove(line);
//   }
//   fPad->Modified();
//   fPad->Update();
//   fPad->Disconnect("Closed()", this, "ResetPad()");
//   if (fPad->GetCanvas()) fPad->GetCanvas()->Disconnect("Cleared(TVirtualPad*)", this, "ClearPad(TVirtualPad*)");
   fPad = 0;
}

//_______________________________________________________________________________________________//

void KVIDGraph::ResetDraw()
{
   // In case the graph can no longer be drawn/undrawn (because fPad contains address of a canvas
   // which died unexpectedly), use this method to reset fPad=0 and hopefully start afresh.
   fPad = 0;
}

//_______________________________________________________________________________________________//

void KVIDGraph::Print(Option_t*) const
{
   //Print out all objects in graph

   cout << ClassName() << " : " << GetName() << endl;
   cout << "Title : " << GetTitle() << endl;
   //print list of parameters
   fPar->Print();
   KVIDentifier* line = 0;
   TIter nextOK(fCuts);
   while ((line = (KVIDentifier*) nextOK()))
      line->ls();
   TIter nextID(fIdentifiers);
   while ((line = (KVIDentifier*) nextID()))
      line->ls();
}

//_______________________________________________________________________________________________//

void KVIDGraph::FindAxisLimits()
{
   //Calculate X/Y min/max of all objects in graph

   KVIDentifier* line;
   Double_t x, y, xmin, ymin, xmax, ymax;
   xmax = ymax = -999999;
   xmin = ymin = 999999;
   TIter nextOK(fCuts);
   while ((line = (KVIDentifier*) nextOK())) {
      for (int i = 0; i < line->GetN(); i++) {
         line->GetPoint(i, x, y);
         xmin = (x < xmin ? x : xmin);
         xmax = (x > xmax ? x : xmax);
         ymin = (y < ymin ? y : ymin);
         ymax = (y > ymax ? y : ymax);
      }
   }
   TIter nextID(fIdentifiers);
   while ((line = (KVIDentifier*) nextID())) {
      for (int i = 0; i < line->GetN(); i++) {
         line->GetPoint(i, x, y);
         xmin = (x < xmin ? x : xmin);
         xmax = (x > xmax ? x : xmax);
         ymin = (y < ymin ? y : ymin);
         ymax = (y > ymax ? y : ymax);
      }
   }
   fXmin = xmin;
   fYmin = ymin;
   fXmax = xmax;
   fYmax = ymax;
   //   Info("FindAxisLimits()", "Xmin=%f Ymin=%f Xmax=%f Ymax=%f", GetXmin(), GetYmin(), GetXmax(), GetYmax());
}

//___________________________________________________________________________________

void KVIDGraph::Scale(TF1* sx, TF1* sy)
{
   // Rescale coordinates of every object in graph according to arbitrary functions
   // of the coordinates (sx & sy can be 1-dimensional or 2-dimensional functions).
   //
   // If sy is a TF1 function f(x), then the 'x' in this function refers to the y-coordinate.
   // If sy is a TF2 function f(x,y), 'x' and 'y' refer to the x & y-coordinates.

   if (!sx && !sy)
      return;
   if (GetNumberOfIdentifiers() > 0) {
      fIdentifiers->R__FOR_EACH(KVIDentifier, Scale)(sx, sy);
   }
   if (GetNumberOfCuts() > 0) {
      fCuts->R__FOR_EACH(KVIDentifier, Scale)(sx, sy);
   }
   Modified();
}

//___________________________________________________________________________________

void KVIDGraph::Scale(Double_t sx, Double_t sy)
{
   // Called by SetX/YScaleFactor methods to rescale every point of every object in the grid

   if (TMath::Abs(sx) == 1 && TMath::Abs(sy) == 1)
      return;
   if (GetNumberOfIdentifiers() > 0) {
      fIdentifiers->R__FOR_EACH(KVIDentifier, Scale)(sx, sy);
   }
   if (GetNumberOfCuts() > 0) {
      fCuts->R__FOR_EACH(KVIDentifier, Scale)(sx, sy);
   }
}

//___________________________________________________________________________________


//___________________________________________________________________________________


//___________________________________________________________________________________

void KVIDGraph::DrawAndAdd(const Char_t* Type, const Char_t* Classname)
{
   //Use this method to add objects to a grid which is drawn in the current pad gPad.
   //We wait for the user to draw the line with the graphical editor
   //(we put the pad automatically into this mode,
   //there is no need to use the Editor toolbar).
   //Then we retrieve this TGraph, create a new KVIDentifier-derived object of
   //class "classname", and add it to the "type" list (= "OK"/"cut" or "ID").

   if (!fPad && !gPad) return;
   // if grid already drawn somewhere, make grid's pad active
   if (fPad && fPad != gPad) {
      fPad->cd();
      Warning("DrawAndAdd", "Changed active pad to pad containing this graph");
   }
   if (!fPad) fPad = gPad;

   TString type(Type);
   TString classname(Classname);
   //create new ID line
   KVIDentifier* line = New(classname);
   //wait for user to draw object
   line->WaitForPrimitive();
   //add line to list
   Add(type, line);
   Modified();
}



//___________________________________________________________________________________

KVIDGraph* KVIDGraph::AddGraphs(KVIDGraph* g1, Int_t id1_min, Int_t id1_max,
                                KVIDGraph* g2, Int_t id2_min, Int_t id2_max)
{
   //Static method taking lines id1_min to id2_min from grid g1 and lines id2_min to id2_max from
   //grid g2 and making a new grid containing them. Returns the pointer to the new grid.
   //
   //It is assumed that g1 and g2 are grids of the same class. The new grid will be an object of the
   //same class as g1.
   //
   //Any scaling factors are applied to the line coordinates as they are copied from their parent grids.
   //As scaling may be different in g1 and g2, the resulting grid has no scaling factor, but the coordinates
   //of all its lines result from application of the different scaling factors of the two grids.

   //make new grid
   KVIDGraph* merger = (KVIDGraph*) g1->IsA()->New();

   //loop over lines in first grid
   TIter next_id1(g1->GetIdentifiers());
   KVIDentifier* line;
   while ((line = (KVIDentifier*) next_id1())) {
      if (line->GetID() >= id1_min && line->GetID() <= id1_max)
         merger->Add("ID", (KVIDentifier*) line->Clone());      //add clone of lines with right ID
   }

   //loop over lines in second grid
   TIter next_id2(g2->GetIdentifiers());
   while ((line = (KVIDentifier*) next_id2())) {
      if (line->GetID() >= id2_min && line->GetID() <= id2_max)
         merger->Add("ID", (KVIDentifier*) line->Clone());
   }

   //just in case any of the lines were highlighted...
   merger->SetLineColor(kBlack);

   return merger;
}

//___________________________________________________________________________________

Bool_t KVIDGraph::IsIdentifiable(Double_t x, Double_t y) const
{
   // Default method for deciding if point (x,y) corresponds to an identifiable
   // particle or not: we loop over the list of cuts and test the
   // point with the method TestPoint(x,y). If the point is accepted
   // by all cuts, then (x,y) is identifiable (return kTRUE).

   TIter next(fCuts);
   KVIDentifier* id = 0;
   while ((id = (KVIDentifier*)next())) if (!id->TestPoint(x, y)) return kFALSE;
   return kTRUE;
}

//___________________________________________________________________________________

void KVIDGraph::SetRuns(const KVNumberList& runs)
{
   // Set list of runs for which grid is valid
   fRunList = runs;
   fPar->SetValue("Runlist", fRunList.AsString());
   Modified();
}

//___________________________________________________________________________________

const Char_t* KVIDGraph::GetName() const
{
   // If name of grid is not set explicitly with SetName(const Char_t*),
   // we return the name of the first ID telescope in the list of telescopes
   // with which this grid is associated.

   if (fName != "") return fName;
   fDyName = "";
   if (fTelescopes.At(0)) fDyName = fTelescopes.At(0)->GetName();
   else {
      if (fPar->HasParameter("IDTelescopes")) {
         KVString tel_list = fPar->GetStringValue("IDTelescopes");
         tel_list.Begin(",");
         fDyName = tel_list.Next();
      }
   }
   return fDyName;
}

//___________________________________________________________________________________

void KVIDGraph::BackwardsCompatibilityFix()
{
   // Called after reading a grid from an ascii file.
   // Tries to convert information written by an old version of the class:
   //
   //<PARAMETER> First run=... ----> <PARAMETER> Runlist=...
   //<PARAMETER> Last run=...

   if (fPar->HasParameter("Runlist")) return;
   if (fPar->HasParameter("First run") && fPar->HasParameter("Last run")) {
      fRunList.SetMinMax(fPar->GetIntValue("First run"), fPar->GetIntValue("Last run"));
      fPar->SetValue("Runlist", fRunList.AsString());
      fPar->RemoveParameter("First run");
      fPar->RemoveParameter("Last run");
   }
}
//_______________________________________________________________________________________________//
void KVIDGraph::WriteAsciiFile_WP(Int_t version)
{
   // ecriture des grilles avec version suivant un modele de fichier (fPattern)
   // fPattern_v%d.grid
   // le modele doit etre initialiser par la methode SetPattern(TString)
   // la valeur par default version=-1 permet l ecriture de la derniere version
   // apres un check des versions existantes (methode CheckVersion)
   // la premiere version est indexee v0
   Int_t vv = 0;
   if (version == -1) vv = CheckVersion(version) + 1;
   WriteAsciiFile(fPattern + "_v" + Form("%d", vv) + ".grid");
}
//_______________________________________________________________________________________________//
void KVIDGraph::ReadAsciiFile_WP(Int_t version)
{
   // lecture des grilles avec version suivant un modele de fichier
   Int_t vv = CheckVersion(version);
   if (vv == -1) {
      if (version == -1)  cout << "no file " + fPattern + "_v%d.grid" << endl;
      else              cout << "no file " + fPattern + Form("_v%d.grid", version) << endl;
   }
   else ReadAsciiFile(fPattern + "_v" + Form("%d", vv) + ".grid");
}
//___________________________________________________________________________________//
Int_t KVIDGraph::CheckVersion(Int_t version)
{
   // version>=0 check si la version demandee du fichier grille existe
   // version-1 renvoie la derniere version existante
   // si la version demandee n existe pas return -1
   // creation d un fichier tampon dans le repertoire ou se situe les fichiers grilles
   // efface a la fin de la methode
   Int_t temoin = -1;
   TString pbis = fPattern, chemin = "";
   TObjArray* tok = NULL;
   if (fPattern.Contains("/")) {    // prise en compte de fPattern contenant un chemin pour les fichiers
      tok = fPattern.Tokenize("/");
      Int_t nb = tok->GetEntries();
      pbis = ((TObjString*)(*tok)[nb - 1])->GetString();
      chemin = "";
      for (Int_t ii = 0; ii < nb - 1; ii += 1) chemin += ((TObjString*)(*tok)[ii])->GetString();
      chemin += "/";
   }
   gROOT->ProcessLine(".! ls " + fPattern + "_v*.grid >> " + chemin + "listKVIDGraph_" + pbis);
   ifstream f_in(chemin + "listKVIDGraph_" + pbis);
   TString line = "";
   Int_t num = -2;
   while (f_in.good() && num != version) {
      line.ReadLine(f_in);
      if (line.Contains(fPattern + "_v") && line.Contains(".grid")) {
         tok = line.Tokenize(".");
         line = ((TObjString*)(*tok)[0])->GetString();
         line.ReplaceAll(fPattern + "_v", "template");
         tok = line.Tokenize("template");
         num = ((TObjString*)(*tok)[0])->GetString().Atoi();
         if (num > temoin) temoin = num;
      }
   }
   f_in.close();
   gROOT->ProcessLine(".! rm -f " + chemin + "listKVIDGraph_" + pbis);
   delete tok;
   if (version == -1) return temoin;
   else if (temoin != version) {
      return -1;
   }
   else return temoin;
}

//___________________________________________________________________________________

void KVIDGraph::SetMassFormula(Int_t mass)
{
   // Set mass formula for all identifiers if graph has OnlyZId()=kTRUE.
   // This will change the mass (A) of each identifier.

   fMassFormula = mass;
   if (IsOnlyZId()) {
      if (GetNumberOfIdentifiers() > 0) {
         fIdentifiers->R__FOR_EACH(KVIDentifier, SetMassFormula)(mass);
      }
   }
   Modified();
}

//___________________________________________________________________________________

void KVIDGraph::SetOnlyZId(Bool_t yes)
{
   // Use this method if the graph is only to be used for Z identification
   // (no isotopic information). Default is to identify both Z & A
   // (fOnlyZid = kFALSE). Note that setting fOnlyZid=kTRUE changes the way line
   // widths are calculated (see KVIDGrid::CalculateLineWidths)
   fOnlyZId = yes;
   if (GetNumberOfIdentifiers() > 0) {
      fIdentifiers->R__FOR_EACH(KVIDentifier, SetOnlyZId)(yes);
   }
   Modified();
}


//___________________________________________________________________________________

void KVIDGraph::AddIDTelescopes(const TList* tels)
{
   // Associate this graph with all ID telescopes in list

   TIter next(tels);
   KVBase* tel;
   while ((tel = (KVBase*)next())) AddIDTelescope(tel);
   Modified();
   if (gIDGridManager) gIDGridManager->Modified();
}

//___________________________________________________________________________________

//void KVIDGraph::SetIDTelescopes()
//{
//   // Opens dialog box to view and/or choose ID telescopes associated with this
//   // identification graph

//   Bool_t cancel;
//   TString old_label = GetIDTelescopeLabel();
//   new KVIDGUITelescopeChooserDialog(gMultiDetArray, &fTelescopes, &cancel,
//                                     gClient->GetRoot(), gClient->GetRoot());
//   if(!cancel){
//      // if type of associated id telescope has changed, we need to
//      // update the grid manager and associated gui (grid will appear
//      // on a different tab).
//      if(old_label != GetIDTelescopeLabel()) gIDGridManager->Modified();
//      else // same type; just the grid changes
//         Modified();
//   }
//}

//_________________________________________________________________________________

KVIDGraph* KVIDGraph::MakeIDGraph(const Char_t* class_name)
{
   // Static function which will create and 'Build' the identification graph of
   // class 'class_name', one of the plugins defined in either $KVROOT/KVFiles/.kvrootrc,
   // or in the user's .kvrootrc file.

   //check and load plugin library
   TPluginHandler* ph;
   if (!(ph = KVBase::LoadPlugin("KVIDGraph", class_name)))
      return 0;

   //execute constructor/macro for graph - assumed without arguments
   KVIDGraph* gr = (KVIDGraph*) ph->ExecPlugin(0);
   return gr;
}

//___________________________________________________________________________________

void KVIDGraph::SetEditable(Bool_t editable)
{
   // Toggles 'editable' status of all lines/contours in graph
   // If editable = kTRUE this makes it possible to modify the graph
   //    we then take a snapshot of the graph before editing begins
   //    and store it in fLastSavedVersion. we can always revert to
   //    this version if we want

   if (editable && !GetEditable()) UpdateLastSavedVersion();

   TCutG::SetEditable(editable);
   if (GetNumberOfIdentifiers() > 0) {
      fIdentifiers->R__FOR_EACH(KVIDentifier, SetEditable)(editable);
   }
   if (GetNumberOfCuts() > 0) {
      fCuts->R__FOR_EACH(KVIDentifier, SetEditable)(editable);
   }
}

//___________________________________________________________________________________

const Char_t* KVIDGraph::GetNamesOfIDTelescopes() const
{
   // Returns a comma-separated list of the names of all ID telescopes for which
   // this grid is valid.

   static TString tel_list;
   if (!fTelescopes.GetEntries()) {
      // case of grid with "unknown" ID telescopes
      // we return the value of the IDTelescopes parameter (if defined)
      if (fPar->HasParameter("IDTelescopes")) {
         tel_list = fPar->GetStringValue("IDTelescopes");
         return tel_list.Data();
      }
   }
   TIter next(&fTelescopes);
   TObject* id = next();
   if (id) tel_list = id->GetName();
   while ((id = next())) tel_list += Form(",%s", id->GetName());
   return tel_list;
}

//______________________________________________________________________________

void KVIDGraph::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVIDGraph.
   // Resets the 'fParent' pointer of each KVIDentifier after reading them in.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVIDGraph::Class(), this);
      TIter nxt_id(fIdentifiers);
      KVIDentifier* id;
      while ((id = (KVIDentifier*)nxt_id())) id->fParent = this;
      TIter nxt_cut(fCuts);
      while ((id = (KVIDentifier*)nxt_cut())) id->fParent = this;
   }
   else {
      R__b.WriteClassBuffer(KVIDGraph::Class(), this);
   }
}

struct kvidgraph_idresult_filler {
   KVHashList& histos;
   KVNameValueList& histo_names;

   kvidgraph_idresult_filler(KVHashList& h, KVNameValueList& n)
      : histos(h), histo_names(n)
   {}
   void fill(const char* name, double x, double y = 0., double w = 0.)
   {
      TH1* h = (TH1*)histos.get_object<TH1>(histo_names.GetStringValue(name));
      if (h) {
         if (w > 0)((TH2*)h)->Fill(x, y, w);
         else h->Fill(x, y);
      }
   }
};

void KVIDGraph::TestIdentification(TH2F* data, KVHashList& histos, KVNameValueList& histo_names)
{
   //This method allows to test the identification capabilities of the grid using data in a TH2F.
   //We assume that 'data' contains an identification map, whose 'x' and 'y' coordinates correspond
   //to this grid. Then we loop over every bin of the histogram, perform the identification (if
   //IsIdentifiable() returns kTRUE) and fill the histograms with the results of the identification
   //
   //The "identification" or PID we represent is the result of the KVReconstructedNucleus::GetPID()
   //method for the identified nucleus.
   //
   // The KVHashList contains histograms to be filled with results
   // The KVNameValueList histo_names contains the name of each histogram passed in the KVHashList
   //  i.e. it may have any of the following parameters
   //
   //        "ID_REAL"="[name of histo to fill with PID spectrum]"
   //        "ID_REAL_VS_ERES" - PID vs. Eres histo
   //        "Z_A_REAL" - 2D map (nuclear chart)
   //        "ZADIST_AIDENT" - integer A  vs. integer Z distribution for isotopically-identified particles
   //        "ZIDENT_ICODE0" - map of points leading to Z-only identification with quality code 0
   //        "ZIDENT_ICODES_1_2" - map of points leading to Z-only identification with quality codes 1 or 2

   //Initialize the grid: calculate line widths etc.
   Initialize();

   kvidgraph_idresult_filler idresults(histos, histo_names);

   // reset contents of all histos
   histos.Execute("Reset", "");

   Int_t tot_events = (Int_t) data->GetSum();
   Int_t events_read = 0;
   Int_t percent = 0, cumul = 0;

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
         //If bin content ('poids') is <=20, we perform the identification 'poids' times, each time with
         //randomly-drawn x and y coordinates inside this bin
         //If 'poids'>20, we perform the identification 20 times and we fill the histograms with
         //a weight poids/20
         Int_t kmax = (Int_t) TMath::Min(20., poids);
         Double_t weight = (kmax == 20 ? poids / 20. : 1.);
         for (int k = 0; k < kmax; k++) {
            double x = gRandom->Uniform(x0 - .5 * wx, x0 + .5 * wx);
            double y = gRandom->Uniform(y0 - .5 * wy, y0 + .5 * wy);
            if (IsIdentifiable(x, y)) {
               KVIdentificationResult idr;
               Identify(x, y, &idr);
               if (AcceptIDForTest(idr)) {
                  Float_t PID = idr.PID;
                  if (idr.Aident) PID = (idr.Z + 0.1 * (idr.PID - 2. * idr.Z));
                  Float_t RealA, RealZ;
                  RealA = (idr.Aident ? idr.PID : (Float_t)idr.A);
                  RealZ = (idr.Aident ? (Float_t)idr.Z : idr.PID);
                  idresults.fill("ID_REAL", PID, weight);
                  idresults.fill("ID_REAL_VS_ERES", x, PID, weight);
                  if (idr.Aident) {
                     idresults.fill("Z_A_REAL", RealA - RealZ, gRandom->Gaus(RealZ, 0.15), weight);
                     idresults.fill("ZADIST_AIDENT", idr.Z, idr.A, weight);
                     switch (idr.IDquality) {
                        case 0:
                           idresults.fill("AIDENT_ICODE0", x, y, weight);
                           break;
                        case 1:
                        case 2:
                        case 3:
                           idresults.fill("AIDENT_ICODE123", x, y, weight);
                           break;
                        default:
                           break;
                     }
                  }
                  else if (idr.Zident) {
                     switch (idr.IDquality) {
                        case 0:
                           idresults.fill("ZIDENT_ICODE0", x, y, weight);
                           break;
                        case 1:
                        case 2:
                        case 3:
                           idresults.fill("ZIDENT_ICODE123", x, y, weight);
                           break;
                        default:
                           break;
                     }
                  }
               }
               else {
                  if (idr.Aident) {
                     switch (idr.IDquality) {
                        case 4:
                           idresults.fill("AIDENT_ICODE4", x, y, weight);
                           break;
                        case 5:
                           idresults.fill("AIDENT_ICODE5", x, y, weight);
                           break;
                        case 6:
                           idresults.fill("AIDENT_ICODE6", x, y, weight);
                           break;
                        case 7:
                           idresults.fill("AIDENT_ICODE7", x, y, weight);
                           break;
                        default:
                           break;
                     }
                  }
                  else if (idr.Zident) {
                     switch (idr.IDquality) {
                        case 4:
                           idresults.fill("ZIDENT_ICODE4", x, y, weight);
                           break;
                        case 5:
                           idresults.fill("ZIDENT_ICODE5", x, y, weight);
                           break;
                        case 6:
                           idresults.fill("ZIDENT_ICODE6", x, y, weight);
                           break;
                        case 7:
                           idresults.fill("ZIDENT_ICODE7", x, y, weight);
                           break;
                        default:
                           break;
                     }
                  }

               }
            }
         }
         events_read += (Int_t) poids;
         percent = (1. * events_read / tot_events) * 100.;
         if (percent > cumul) {
            Increment((Float_t) events_read);      //sends signal to GUI progress bar
            gSystem->ProcessEvents();
            cumul = percent;
         }
      }
   }
}
