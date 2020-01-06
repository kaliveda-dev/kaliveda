/*
$Id: KVIDGraph.h,v 1.7 2009/04/28 09:07:47 franklan Exp $
$Revision: 1.7 $
$Date: 2009/04/28 09:07:47 $
*/

//Created by KVClassFactory on Mon Apr 14 13:42:47 2008
//Author: franklan

#ifndef __KVIDGRAPH_H
#define __KVIDGRAPH_H

#include "TCutG.h"
#include "KVNameValueList.h"
#include "KVIDentifier.h"
#include <RQ_OBJECT.h>
#include <TH2.h>
#include "KVIdentificationResult.h"
#include "KVNumberList.h"
#include "RVersion.h"

class TVirtualPad;
class KVIdentificationResult;

class KVIDGraph : public TCutG {
   friend class KVIDGridManager;

   RQ_OBJECT("KVIDGraph")

protected:

   Bool_t         fOnlyZId;         //set to kTRUE when only to be used to give Z identification of nuclei, no mass info
   KVList*         fIdentifiers;    //-> list of identification objects
   KVList*         fCuts;           //-> cuts used to define area in which identification is possible
   Axis_t         fXmin, fXmax;     //!min/max X coordinates of graph
   Axis_t         fYmin, fYmax;     //!min/max Y coordinates of graph
   KVNameValueList*   fPar;            //-> parameters associated to grid
   Double_t       fLastScaleX;      //last applied scaling factor on X
   Double_t       fLastScaleY;      //last applied scaling factor on Y
   TVirtualPad*    fPad;            //!pad in which graph is drawn
   KVNumberList   fRunList;         //runs for which grid is valid
   TList          fTelescopes;      //!ID telescopes for which grid is valid
   mutable TString fDyName;          //!dynamically generated name
   TString        fPattern;         //pattern of filenames used to write or read grid
   Int_t          fMassFormula;     // *OPTION={GetMethod="GetMassFormula";SetMethod="SetMassFormula";Items=(0="Beta-stability", 1="VEDA mass", 2="EAL mass", 3="EAL residues", 99="2Z+1")}*
   KVIDGraph*     fLastSavedVersion;//!copy of last save version of grid, used for 'undo'
   static Bool_t  fAutoAddGridManager;//if =kTRUE, grids are automatically added to ID grid manager on creation (default)

   Bool_t IsOnlyZId() const
   {
      // Returns kTRUE if the graph is only to be used for Z identification
      // (no isotopic information). Default is to identify both Z & A
      // (fOnlyZid = kFALSE)
      return fOnlyZId;
   }

   void Scale(Double_t sx = -1, Double_t sy = -1);
   virtual void ReadFromAsciiFile(std::ifstream& gridfile);
   virtual void WriteToAsciiFile(std::ofstream& gridfile);
   void init();
   virtual KVIDentifier* New(const Char_t*);
   virtual void ReadIdentifierFromAsciiFile(TString& name, TString& type, TString& cl, std::ifstream& gridfile);
   void WriteParameterListOfIDTelescopes();
   virtual void BackwardsCompatibilityFix();

public:
   virtual Bool_t AcceptIDForTest(const KVIdentificationResult& idr)
   {
      // Used by TestIdentification method
      //
      // The result of the identification may be excluded from the histograms of PID
      // and PID vs. Eres, depending on e.g. some status code of the identification algorithm.
      // By default, this returns the value of idr.IDOK, but may be overridden in child classes.

      return idr.IDOK;
   }

   KVIDGraph();
   KVIDGraph(const KVIDGraph&);
   virtual ~KVIDGraph();

   //virtual void Browse(TBrowser* b);
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   virtual void Identify(Double_t /*x*/, Double_t /*y*/, KVIdentificationResult*) const = 0;
   virtual void Initialize() = 0;
   virtual Bool_t IsIdentifiable(Double_t /*x*/, Double_t /*y*/) const;

   static void SetAutoAdd(Bool_t yes = kTRUE)
   {
      // after calling KVIDGraph::SetAutoAdd(), all grids are automatically added to ID Grid Manager
      // after calling KVIDGraph::SetAutoAdd(kFALSE), this mechanism is disabled
      fAutoAddGridManager = yes;
   }
   void ClearListOfTelescopes()
   {
      fTelescopes.Clear();
   }
   KVIDGraph* GetLastSavedVersion() const
   {
      return fLastSavedVersion;
   }
   void RevertToLastSavedVersion();
   void UpdateLastSavedVersion();

   static KVIDGraph* MakeIDGraph(const Char_t*);

   void FindAxisLimits();
   void Scale(TF1* sx, TF1* sy);

   KVIDentifier* GetIdentifier(Int_t Z, Int_t A) const;
   void RemoveIdentifier(KVIDentifier*);
   void RemoveCut(KVIDentifier*);

   TVirtualPad* GetPad() const
   {
      return fPad;
   }

   void ReadAsciiFile(const Char_t* filename);
   void AddIDTelescopes(const TList*);
   virtual void     SetName(const char* name)
   {
      TNamed::SetName(name); /*Modified()*/;
   } // *MENU*
   virtual void Clear(Option_t* opt = ""); //  *MENU*
   //void SetIDTelescopes();// *MENU={Hierarchy="Set.../ID Telescopes"}*
   void SetXVariable(const char* v)
   {
      SetVarX(v);
      Modified();
   } //  *MENU={Hierarchy="Set.../X Variable"}* *ARGS={v=>fVarX}
   void SetYVariable(const char* v)
   {
      SetVarY(v);
      Modified();
   } //  *MENU={Hierarchy="Set.../Y Variable"}* *ARGS={v=>fVarY}
   void SetRunList(const char* runlist)
   {
      SetRuns(KVNumberList(runlist));
   }  // *MENU={Hierarchy="Set.../List of Runs"}*
   virtual void SetOnlyZId(Bool_t yes = kTRUE);
   void SetMassFormula(Int_t);// *SUBMENU={Hierarchy="Set.../Mass Formula"}*
   void WriteAsciiFile(const Char_t* filename); // *MENU*
   void SetXScaleFactor(Double_t = 0); //  *MENU={Hierarchy="Scale.../X Scale Factor"}*
   void SetYScaleFactor(Double_t = 0);//  *MENU={Hierarchy="Scale.../Y Scale Factor"}*
   //virtual void NewCut();// *MENU={Hierarchy="AddLine.../NewCut"}*
   //virtual void NewIdentifier();// *MENU={Hierarchy="AddLine.../NewIdentifier"}*
   virtual void          SetEditable(Bool_t editable = kTRUE); // *TOGGLE* *GETTER=GetEditable

   virtual Bool_t HasMassIDCapability() const
   {
      // Returns kTRUE if grid can identify masses as well as Z for at least
      // some of the nuclei for which it is defined.
      //
      // Returns kFALSE if grid only provides Z identification.
      return !IsOnlyZId();
   }

   virtual void SetMassIDCapability(Bool_t yes = kTRUE)
   {
      // Used in context menus to define whether grid identifies A & Z or just Z
      SetOnlyZId(!yes);
   } //  *TOGGLE={Hierarchy="A and Z Identification"}* *GETTER=HasMassIDCapability

   void Draw(Option_t* opt = "");  //  *MENU={Hierarchy="View.../Draw"}*
   void UnDraw(); //  *MENU={Hierarchy="View.../UnDraw"}*
   void ResetDraw();// *MENU={Hierarchy="View.../ResetDraw"}*
   void SetLineColor(Color_t lcolor)
   {
      //Set line colour of all objects in grid
      fIdentifiers->Execute("SetLineColor", Form("%d", (Int_t) lcolor));
      fCuts->Execute("SetLineColor", Form("%d", (Int_t) lcolor));
      Modified();
   } // *MENU={Hierarchy="View.../SetLinecolor"}*
   void SetLineStyle(Style_t lstyle)
   {
      //Set line style of all objects in grid
      fIdentifiers->Execute("SetLineStyle", Form("%d", (Int_t) lstyle));
      fCuts->Execute("SetLineStyle", Form("%d", (Int_t) lstyle));
      Modified();
   } // *MENU={Hierarchy="View.../SetLineStyle"}*
   void SetLineWidth(Width_t lwidth)
   {
      //Set line width of all objects in grid
      fIdentifiers->Execute("SetLineWidth", Form("%d", (Int_t) lwidth));
      fCuts->Execute("SetLineWidth", Form("%d", (Int_t) lwidth));
      Modified();
   } // *MENU={Hierarchy="View.../SetLineWidth"}*


   Double_t GetXScaleFactor();
   Double_t GetYScaleFactor();
   void Add(TString, KVIDentifier*);
   KVIDentifier* Add(TString, TString);
   virtual void DrawAndAdd(const Char_t* type = "ID", const Char_t* classname = "KVIDentifier");
   void Print(Option_t* opt = "") const;

   static KVIDGraph* AddGraphs(KVIDGraph* g1, Int_t id1_min, Int_t id1_max,
                               KVIDGraph* g2, Int_t id2_min, Int_t id2_max);
   void SetRuns(const KVNumberList& nl);

   const Char_t* GetName() const;
   virtual void WriteAsciiFile_WP(Int_t version = -1); //WP means with pattern
   virtual void ReadAsciiFile_WP(Int_t version = -1); //WP means with pattern
   virtual Int_t CheckVersion(Int_t version);
   virtual Bool_t ExistVersion(Int_t version)
   {
      if (version != -1) return (version == CheckVersion(version));
      else return (version != -1);
   }
   void SetPattern(TString pattern)
   {
      pattern.ReplaceAll(".", "_");
      fPattern = pattern;
   }
   TString GetPattern(void)
   {
      return fPattern;
   }

   const TList* GetIDTelescopes() const
   {
      return &fTelescopes;
   };
   void Increment(Float_t x)  // *SIGNAL*
   {
      // Used by TestIdentification and KVTestIDGridDialog to send
      // signals to TGHProgressBar about the progress of the identification test
      Emit("Increment(Float_t)", x);
   }
   void Modified() // *SIGNAL*
   {
      // Signal sent out when properties of graph change
      Emit("Modified()");
   }

   const KVNumberList& GetRuns() const
   {
      // Get list of runs for which grid is valid
      return fRunList;
   }
   const Char_t* GetRunList() const
   {
      // Get list of runs for which grid is valid
      return (const char*)fRunList;
   }
   KVIDentifier* GetIdentifierAt(Int_t index) const
   {
      // Return identifier at position 'index' (=0,1,...) in list of identifiers
      return (KVIDentifier*)fIdentifiers->At(index);
   }
   KVIDentifier* GetIdentifier(const Char_t* name) const
   {
      return (KVIDentifier*)fIdentifiers->FindObject(name);
   }
   KVIDentifier* GetCut(const Char_t* name) const
   {
      return (KVIDentifier*)fCuts->FindObject(name);
   }
   KVNameValueList* GetParameters() const
   {
      // Return pointer to list of parameters associated to grid
      return fPar;
   }
   KVList* GetIdentifiers() const
   {
      // Returns list of identifier objects (derived from KVIDentifier)
      return fIdentifiers;
   }
   KVList* GetCuts() const
   {
      // Returns list of cuts (derived from KVIDentifier)
      return fCuts;
   }
   Int_t GetNumberOfIdentifiers() const
   {
      return fIdentifiers->GetSize();
   }
   Int_t GetNumberOfCuts() const
   {
      return fCuts->GetSize();
   }
   virtual void AddIdentifier(KVIDentifier* id)
   {
      // Add identifier to the graph. It will be deleted by the graph.
      fIdentifiers->Add(id);
      id->SetParent(this);
      id->SetVarX(GetVarX());
      id->SetVarY(GetVarY());
      id->SetOnlyZId(IsOnlyZId());
      //if grid is Z-identification only, set mass formula for line
      //according to mass formula of grid
      if (IsOnlyZId()) id->SetMassFormula(GetMassFormula());
      //Modified();
   }
   virtual void AddCut(KVIDentifier* cut)
   {
      // Add cut to the graph. It will be deleted by the graph.
      cut->SetLineColor(kRed);
      cut->SetParent(this);
      fCuts->Add(cut);
      cut->SetVarX(GetVarX());
      cut->SetVarY(GetVarY());
      cut->SetBit(kMustCleanup);
      //Modified();
   }
   void SortIdentifiers()
   {
      fIdentifiers->Sort();
      //Modified();
   }
   Bool_t IsSorted() const
   {
      // Return kTRUE if list of identifiers has been sorted
      return fIdentifiers->IsSorted();
   }
   Axis_t GetXmin() const
   {
      return fXmin;
   }
   Axis_t GetYmin() const
   {
      return fYmin;
   }
   Axis_t GetXmax() const
   {
      return fXmax;
   }
   Axis_t GetYmax() const
   {
      return fYmax;
   }
   virtual Int_t GetQualityCode() const
   {
      // Return quality code related to previously-attempted identification using grid.
      // Redefine in child classes.
      return 0;
   }
   void AddIDTelescope(KVBase* t)
   {
      // Associate this graph with an identification telescope, i.e. add the
      // telescope to the list of telescopes which can use this
      // graph for identification purposes.

      fTelescopes.Add(t);
      Modified();
   }
   void RemoveIDTelescope(KVBase* t)
   {
      // Remove telescope from the list of telescopes
      // which can use this graph for identification purposes.

      fTelescopes.Remove(t);
      Modified();
   }
   Bool_t HandlesIDTelescope(KVBase* t) const
   {
      // Returns kTRUE if telescope can use this graph for identification purposes.

      return fTelescopes.Contains(t);
   }
   const Char_t* GetIDTelescopeLabel() const
   {
      // Returns type of ID telescope to which this grid is associated
      // (actually returns value of KVBase::GetLabel())

      KVBase* id = (KVBase*)fTelescopes.First();
      return (id ? id->GetLabel() : "");
   }
   Int_t GetMassFormula() const
   {
      return fMassFormula;
   }
   void ResetPad();
   void ClearPad(TVirtualPad*);
   TVirtualPad* IsDrawn() const
   {
      // Returns pointer to pad currently displaying this graph.
      // Returns 0x0 if graph is not drawn.
      return fPad;
   }
   virtual void TestIdentification(TH2F* data, KVHashList& histos, KVNameValueList& histo_names);

   const Char_t* GetNamesOfIDTelescopes() const;

   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TGraph.h, to stop these methods appearing in the ID line context menus
   virtual void      SetMaximum(Double_t maximum = -1111)
   {
      TGraph::SetMaximum(maximum);
   }
   virtual void      SetMinimum(Double_t minimum = -1111)
   {
      TGraph::SetMinimum(minimum);
   }
   //virtual void      SetTitle(const char *title="") {TGraph::SetTitle(title);};
   virtual void      FitPanel()
   {
      TGraph::FitPanel();
   }
   virtual void      DrawPanel()
   {
      TGraph::DrawPanel();
   }
#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
   virtual TFitResultPtr Fit(const char* formula, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(formula, option, goption, xmin, xmax);
   }
   virtual TFitResultPtr Fit(TF1* f1, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(f1, option, goption, xmin, xmax);
   }
#else
   virtual Int_t     Fit(const char* formula, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(formula, option, goption, xmin, xmax);
   }
   virtual Int_t     Fit(TF1* f1, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(f1, option, goption, xmin, xmax);
   }
#endif
   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TObject.h, to stop these methods appearing in the ID line context menus
   virtual void        Delete(Option_t* option = "")
   {
      TGraph::Delete(option);
   }
   virtual void        DrawClass() const
   {
      TGraph::DrawClass();
   }
   virtual TObject*    DrawClone(Option_t* option = "") const
   {
      return TGraph::DrawClone(option);
   }
   virtual void        Dump() const
   {
      TGraph::Dump();
   }
   virtual void        Inspect() const
   {
      TGraph::Inspect();
   }
   virtual void        SaveAs(const char* filename = "", Option_t* option = "") const
   {
      TGraph::SaveAs(filename, option);
   }
   virtual void        SetDrawOption(Option_t* option = "")
   {
      TGraph::SetDrawOption(option);
   }
   virtual void        SetLineAttributes()
   {
      TGraph::SetLineAttributes();
   }
   virtual void        SetFillAttributes()
   {
      TGraph::SetFillAttributes();
   }
   virtual void        SetMarkerAttributes()
   {
      TGraph::SetMarkerAttributes();
   }
   virtual void         SetVarX(const char* v)
   {
      TCutG::SetVarX(v);
   }
   virtual void         SetVarY(const char* v)
   {
      TCutG::SetVarY(v);
   }
   virtual Int_t         InsertPoint()
   {
      return TCutG::InsertPoint();
   }
   virtual Int_t         RemovePoint()
   {
      return TCutG::RemovePoint();
   }
   virtual Int_t         RemovePoint(Int_t i)
   {
      return TCutG::RemovePoint(i);
   }

   ClassDef(KVIDGraph, 2) //Base class for particle identification in a 2D map
};

#endif
