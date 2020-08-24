//Created by KVClassFactory on Tue Sep 27 10:46:46 2016
//Author: Eric BONNET

#ifndef __KVCLUST3D_H
#define __KVCLUST3D_H

#include "TH3D.h"
#include "TH2D.h"
#include "TArray.h"

/**
\class KVClust3D
\brief TH3 object which allow clusterization in cell density
\ingroup Simulation
*/

class KVClust3D : public TH3D {

protected:

   Int_t fNclusters; //number of clusters found by the Clusterize method
   Double_t fThreshold; //threshold on the content of each cell to be considered has filled
   TH2D* fVoisins; //->   internal histogram to gather possible connected clusters
   TH3D* fCluster; //->   3D histogram where index of each cluster is putted as content in the associated cells
   Bool_t fKeepLonelyCells; // tells if we keep or not lonely cells, par default kTRUE
   Int_t fNvoisins;  // nombre de plus proches voisins a prendre en compte:
   // exemple sur une dimension les cellules de nx-fNvoisins a nx+fNvoisins sont prises en compte, par defaut =1
   TArrayI* fNcells; //->  array where size of the clusters are stored
   TArrayD* fPop; //->  array where total contents of the clusters are stored

   void init();

public:

   KVClust3D();
   KVClust3D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup, Int_t nbinsz, Double_t zlow, Double_t zup);
   KVClust3D(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins, Int_t nbinsy, const Float_t* ybins, Int_t nbinsz, const Float_t* zbins);
   KVClust3D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, const Double_t* ybins, Int_t nbinsz, const Double_t* zbins);

   virtual ~KVClust3D();

   TH3D* ProduceTH3D(const Char_t* name) const;
   void PrintInputs() const;
   void ResetObjects();
   void PrintResults() const;
   void Clusterize();
   void SetThreshold(Double_t);
   void SetDensityThreshold(Double_t);
   Double_t GetThreshold() const;

   void SetLonelyCells(Bool_t);
   Bool_t IsLonelyCellsAreKept() const;

   Double_t GetVolumeCell() const;
   Double_t GetVolumeVoisin() const;

   void SetNvoisins(Int_t);
   Int_t GetNvoisins() const;

   Int_t GetNclusters() const;
   Int_t* GetSize() const;
   Double_t* GetPop() const;
   TH3D* GetClusterIndex() const;

   ClassDef(KVClust3D, 1) //TH3 object which allow clusterization in cell density
};

#endif
