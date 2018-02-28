/*
$Id: KVIDentifier.h,v 1.7 2009/04/20 10:01:58 ebonnet Exp $
$Revision: 1.7 $
$Date: 2009/04/20 10:01:58 $
*/

//Created by KVClassFactory on Mon Apr 14 14:25:38 2008
//Author: franklan

#ifndef __KVIDENTIFIER_H
#define __KVIDENTIFIER_H

#include "Riostream.h"
#include "TCutG.h"
#include "KVNucleus.h"
#include "RVersion.h"

class TF1;
class KVIDGraph;

class KVIDentifier : public TCutG {
   friend class KVIDGraph;

protected:
   KVIDGraph*   fParent;   //!parent identification map or grid
   KVNucleus   fIon;       //ion/nucleus corresponding to this identifier
   Bool_t      fGridOnlyZId;//=kTRUE if parent grid has OnlyZId set

   Int_t fZ;//! dummy variables used by context menu dialog boxes
   Int_t fA;//! dummy variables used by context menu dialog boxes
   Int_t fMassFormula; //! *OPTION={GetMethod="GetMassFormula";SetMethod="SetMassFormula";Items=(0="Beta-stability", 1="VEDA mass", 2="EAL mass", 3="EAL residues", 99="2Z+1")}*

   // Does nothing. Can be overridden in child classes in order to write any
   // extra information in between the name of the object and the number of points.
   virtual void WriteAsciiFile_extras(std::ofstream&, const Char_t* /*name_prefix*/ = "") {};

   // Does nothing. Can be overridden in child classes in order to read any
   // extra information in between the name of the object and the number of points.
   virtual void ReadAsciiFile_extras(std::ifstream&) {};
   virtual void SetNameFromNucleus()
   {
      SetName(Form("Z=%d A=%d", GetZ(), GetA()));
   };

private:
   void init();

public:
   KVIDentifier();
   KVIDentifier(const KVIDentifier&);
   KVIDentifier(const TCutG&);
   KVIDentifier(const TGraph&);
   virtual ~KVIDentifier();

   KVIDGraph*  GetParent()     const;
   void        SetParent(KVIDGraph*);

   Int_t Compare(const TObject*) const;
   void Copy(TObject& obj) const;

   virtual void WriteAsciiFile(std::ofstream&, const Char_t* name_prefix = "");
   virtual void ReadAsciiFile(std::ifstream&);

   virtual Int_t GetID() const
   {
      return 0;
   };
   virtual Int_t GetA() const
   {
      return fIon.GetA();
   };
   virtual Int_t GetZ() const
   {
      return fIon.GetZ();
   };
   virtual void SetZ(Int_t ztnum)
   {
      fIon.SetZ(ztnum);
      SetNameFromNucleus();
   }; // *MENU={Hierarchy="SetNucleus.../Z"}*
   virtual void SetA(Int_t atnum)
   {
      fIon.SetA(atnum);
      SetNameFromNucleus();
   };  // *MENU={Hierarchy="SetNucleus.../A"}*
   virtual void SetAandZ(Int_t atnum, Int_t ztnum)
   {
      fIon.SetZ(ztnum);
      fIon.SetA(atnum);
      SetNameFromNucleus();
   };  // *MENU={Hierarchy="SetNucleus.../A and Z"}* *ARGS={atnum=>fA,ztnum=>fZ}
   virtual void SetOnlyZId(Bool_t onlyz = kTRUE)
   {
      fGridOnlyZId = onlyz;
   };
   virtual Bool_t OnlyZId()const
   {
      return fGridOnlyZId;
   }
   virtual void SetMassFormula(Int_t mf)
   {
      if (OnlyZId()) {
         fIon.SetMassFormula(mf);
         fMassFormula = mf;
         SetNameFromNucleus();
      }
   };   // *SUBMENU={Hierarchy="SetNucleus.../Mass Formula"}*
   virtual Int_t GetMassFormula()const
   {
      return fIon.GetMassFormula();
   }

   virtual Bool_t TestPoint(Double_t /*x*/, Double_t /*y*/)
   {
      // Abstract method, should be overridden in child classes.
      // Used to test whether a point (x,y) in the ID map is identifiable.

      AbstractMethod("TestMethod(Double_t x, Double_t y");
      return kFALSE;
   }

   void CopyGraph(TGraph*);
   void CopyGraph(const TGraph&);

   virtual void Scale(Double_t sx = -1, Double_t sy = -1);
   virtual void Scale(TF1* sx, TF1* sy);

   virtual void Print(Option_t* opt) const;

   virtual void WaitForPrimitive();

   virtual void ExtendLine(Double_t, Option_t* Direction = ""); // *MENU={Hierarchy="Modify Line.../ExtendLine"}*

   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TGraph.h, to stop these methods appearing in the ID line context menus
   virtual void      SetMaximum(Double_t maximum = -1111)
   {
      TGraph::SetMaximum(maximum);
   };
   virtual void      SetMinimum(Double_t minimum = -1111)
   {
      TGraph::SetMinimum(minimum);
   };
   virtual void      SetTitle(const char* title = "")
   {
      TGraph::SetTitle(title);
   };
   virtual void      FitPanel()
   {
      TGraph::FitPanel();
   };
   virtual void      DrawPanel()
   {
      TGraph::DrawPanel();
   };
#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
   virtual TFitResultPtr Fit(const char* formula, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(formula, option, goption, xmin, xmax);
   };
   virtual TFitResultPtr Fit(TF1* f1, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(f1, option, goption, xmin, xmax);
   };
#else
   virtual Int_t     Fit(const char* formula, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(formula, option, goption, xmin, xmax);
   };
   virtual Int_t     Fit(TF1* f1, Option_t* option = "", Option_t* goption = "", Axis_t xmin = 0, Axis_t xmax = 0)
   {
      return TGraph::Fit(f1, option, goption, xmin, xmax);
   };
#endif
   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TNamed.h, to stop these methods appearing in the ID line context menus
   virtual void     SetName(const char* name);
   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TObject.h, to stop these methods appearing in the ID line context menus
   virtual void        Delete(Option_t* option = "")
   {
      TGraph::Delete(option);
   };
   virtual void        DrawClass() const
   {
      TGraph::DrawClass();
   };
   virtual TObject*    DrawClone(Option_t* option = "") const
   {
      return TGraph::DrawClone(option);
   };
   virtual void        CloneScaleStore(Int_t newzt, Int_t newat, Double_t dy = 0., Double_t sx = 1., Double_t sy = 1.); // *MENU* *ARGS={newat=>fA,newzt=>fZ}
   virtual void        Dump() const
   {
      TGraph::Dump();
   };
   virtual void        Inspect() const
   {
      TGraph::Inspect();
   };
   virtual void        SaveAs(const char* filename = "", Option_t* option = "") const
   {
      TGraph::SaveAs(filename, option);
   };
   virtual void        SetDrawOption(Option_t* option = "")
   {
      TGraph::SetDrawOption(option);
   };
   virtual void        SetLineAttributes()
   {
      TGraph::SetLineAttributes();
   };
   virtual void        SetFillAttributes()
   {
      TGraph::SetFillAttributes();
   };
   virtual void        SetMarkerAttributes()
   {
      TGraph::SetMarkerAttributes();
   };

   virtual Int_t         InsertPoint(); // *MENU={Hierarchy="Modify Line.../InsertPoint"}*
   virtual Int_t         AddPointAtTheEnd(); // *MENU={Hierarchy="Modify Line.../AddPointAtTheEnd"}*
   virtual Int_t         RemoveFirstPoint(); // *MENU={Hierarchy="Modify Line.../RemoveFirstPoint"}*
   virtual Int_t         ContinueDrawing(); // *MENU={Hierarchy="Modify Line.../ContinueDrawing"}*
   virtual void         ChechHierarchy(KVIDentifier* gr);
   virtual Int_t         RemovePoint()
   {
      if (GetEditable()) {
         return TCutG::RemovePoint();
      } else {
         return -1;
      }
   }; // *MENU={Hierarchy="Modify Line.../RemovePoint"}*
   virtual Int_t         RemovePoint(Int_t i)
   {
      return TCutG::RemovePoint(i);
   };

   virtual Double_t GetPID() const;

   ClassDef(KVIDentifier, 2) //Base class for graphical cuts used in particle identification
};

#endif
