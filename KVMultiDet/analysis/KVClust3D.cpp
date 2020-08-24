//Created by KVClassFactory on Tue Sep 27 10:46:46 2016
//Author: Eric BONNET

#include "KVClust3D.h"
#include "KVNumberList.h"
#include "TMath.h"

ClassImp(KVClust3D)

//____________________________________________________________________________//

void KVClust3D::init()
{
   //initialization method
   fNclusters = 0;
   fVoisins = new TH2D("hvoisins", "FromKVClust3D", 1000, 0.5, 1000.5, 1000, 0.5, 1000.5);

   fNcells = new TArrayI(10);
   fPop = new TArrayD(10);

   fThreshold = 0.0;
   fNvoisins = 1;
   fKeepLonelyCells = kTRUE;

   fCluster = ProduceTH3D("cluster_index");
}

//____________________________________________________________________________//

KVClust3D::KVClust3D()
   : TH3D()
{
   // Default constructor
}

//____________________________________________________________________________//

KVClust3D::KVClust3D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup, Int_t nbinsz, Double_t zlow, Double_t zup)
   : TH3D(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup, nbinsz, zlow, zup)
{
   // Constructor inherited from TH3D
   init();
}

//____________________________________________________________________________//

KVClust3D::KVClust3D(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins, Int_t nbinsy, const Float_t* ybins, Int_t nbinsz, const Float_t* zbins)
   : TH3D(name, title, nbinsx, xbins, nbinsy, ybins, nbinsz, zbins)
{
   // Constructor inherited from TH3D
   init();
}

//____________________________________________________________________________//

KVClust3D::KVClust3D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, const Double_t* ybins, Int_t nbinsz, const Double_t* zbins)
   : TH3D(name, title, nbinsx, xbins, nbinsy, ybins, nbinsz, zbins)
{
   // Constructor inherited from TH3D
   init();
}

//____________________________________________________________________________//

KVClust3D::~KVClust3D()
{
   // Destructor
   /*
   delete fCluster;
   delete fVoisins;
   delete fNcells;
   delete fPop;
   */
}

//____________________________________________________________________________//

void KVClust3D::SetLonelyCells(Bool_t val)
{
   fKeepLonelyCells = val;
}

//____________________________________________________________________________//

Bool_t KVClust3D::IsLonelyCellsAreKept() const
{
   return fKeepLonelyCells;
}

//____________________________________________________________________________//

void KVClust3D::SetNvoisins(Int_t val)
{
   fNvoisins = val;
}

//____________________________________________________________________________//

Int_t KVClust3D::GetNvoisins() const
{
   return fNvoisins;
}

//____________________________________________________________________________//

Double_t KVClust3D::GetVolumeCell() const
{
   return GetXaxis()->GetBinWidth(1) * GetYaxis()->GetBinWidth(1) * GetZaxis()->GetBinWidth(1);
}

//____________________________________________________________________________//

Double_t KVClust3D::GetVolumeVoisin() const
{
   Double_t vol =  GetVolumeCell();
   Int_t ncell = TMath::Power(2 * fNvoisins + 1, 3.);
   return ncell * vol;
}

//____________________________________________________________________________//

void KVClust3D::ResetObjects()
{
   //Reset object associated to the KVClust3D object
   //Don't reset the object it self
   fNcells->Reset();
   fPop->Reset();

   fCluster->Reset();
   fVoisins->Reset();
   fNclusters = 0;
}

//____________________________________________________________________________//

void KVClust3D::SetThreshold(Double_t value)
{
   //Set threshold on the content of a cell to be considered has filled
   fThreshold = value;

}

//____________________________________________________________________________//

void KVClust3D::SetDensityThreshold(Double_t value)
{
   //Set density threshold on the content of a cell to be considered has filled
   fThreshold = value * GetVolumeCell();

}

//____________________________________________________________________________//

Double_t KVClust3D::GetThreshold() const
{
   //return the value of the threshold for the content of a cell to be considered has filled
   return fThreshold;

}

//____________________________________________________________________________//

void KVClust3D::PrintInputs() const
{

   printf("-----------------------------------------\n");
   printf("volume cellule : %lf\n", GetVolumeCell());
   printf("nombre de cellules voisines : %lf\n", TMath::Power(2.*fNvoisins + 1, 3.));
   printf("volume voisin: %lf\n", GetVolumeVoisin());
   printf("-----\nseuil :\n\t- contenu : %lf\n", GetThreshold());
   printf("\t- densite : %lf\n", GetThreshold() / GetVolumeCell());



}

//____________________________________________________________________________//

TH3D* KVClust3D::ProduceTH3D(const Char_t* name) const
{
   //Create TH3D object, clone of the KVClust3D object
   //user has to delete it after its use
   //
   TH3D* histo = new TH3D(name, "FromKVClust3D",
                          GetNbinsX(),
                          GetXaxis()->GetBinLowEdge(1),
                          GetXaxis()->GetBinLowEdge(GetNbinsX() + 1),
                          GetNbinsY(),
                          GetYaxis()->GetBinLowEdge(1),
                          GetYaxis()->GetBinLowEdge(GetNbinsY() + 1),
                          GetNbinsZ(),
                          GetZaxis()->GetBinLowEdge(1),
                          GetZaxis()->GetBinLowEdge(GetNbinsZ() + 1)
                         );

   return histo;

}

//____________________________________________________________________________//

void KVClust3D::Clusterize()
{
   //Apply clusterization of the 3D histogram
   //For each cell, with a content greater than the threshold defined before
   // using the SetThreshold() method, test the content of the closest neighbors (8)
   // to see if they are connected
   // At the end, a clone histogram is available, using this method :  GetClusterIndex().
   // The content of its cells corresponds
   // to the index of each found cluster
   //
   Int_t Ntemp = 1;
   Int_t NCellsTot = 0;
   Double_t PopTot = 0;

   ResetObjects();

//    if (!fCluster)
//       fCluster = ProduceTH3D("cluster_index");

   //Remplissage de l histo 3D fCluster :
   // Contenu de l objet KVClust3D au dessus du seuil -> contenu 1
   //
   for (Int_t nx = 1; nx <= GetNbinsX(); nx += 1) {
      for (Int_t ny = 1; ny <= GetNbinsY(); ny += 1) {
         for (Int_t nz = 1; nz <= GetNbinsZ(); nz += 1) {
            if (GetBinContent(nx, ny, nz) >= fThreshold) {
               fCluster->SetBinContent(nx, ny, nz, 1);
               NCellsTot += 1;
               PopTot += GetBinContent(nx, ny, nz);
            }
            else {
               fCluster->SetBinContent(nx, ny, nz, 0);
            }
         }
      }
   }

   for (Int_t nx = 2; nx <= GetNbinsX() - 1; nx += 1) {
      for (Int_t ny = 2; ny <= GetNbinsY() - 1; ny += 1) {
         for (Int_t nz = 2; nz <= GetNbinsZ() - 1; nz += 1) {
            Int_t idx = fCluster->GetBinContent(nx, ny, nz);
            if (idx >= 1) { //Cellule avec la bonne densite
               if (idx > 1) { //cellule deja affectee
                  Int_t ncv = -1;
                  for (Int_t nxv = nx - fNvoisins; nxv <= nx + fNvoisins; nxv += 1) { //on boucle sur les cellules voisines
                     for (Int_t nyv = ny - fNvoisins; nyv <= ny + fNvoisins; nyv += 1) {
                        for (Int_t nzv = nz - fNvoisins; nzv <= nz + fNvoisins; nzv += 1) {
                           Int_t idxv = fCluster->GetBinContent(nxv, nyv, nzv);
                           //cellule a affilier
                           if (idxv >= 1) {
                              ncv += 1;
                              if (idxv == 1) { //cellule mise dans le cluster idx
                                 fNcells->AddAt(1 + fNcells->At(idx), idx);
                                 fPop->AddAt(GetBinContent(nxv, nyv, nzv) + fPop->At(idx), idx);
                                 fCluster->SetBinContent(nxv, nyv, nzv, idx);
                              }
                              else {
                                 // clusters idx et idxv sont voisins
                                 fVoisins->Fill(TMath::Min(idx, idxv), TMath::Max(idx, idxv));
                              }
                           }
                           else {
                              //cellule vide
                              //? SetBinContent(nxv,nyv,nzv,0);
                              //fCluster->SetBinContent(nxv,nyv,nzv,0);
                           }
                        }
                     }
                  }
                  if (ncv == 0)
                     printf("gros pb de coherence ... cluster %1.0lf declare avec une cellule isolee\n", fCluster->GetBinContent(nx, ny, nz));
               }
               else if (idx == 1) {   //cellule pas encore affectee
                  Int_t ncv = -1;
                  KVNumberList nl;
                  for (Int_t nxv = nx - fNvoisins; nxv <= nx + fNvoisins; nxv += 1) { //on boucle sur les cellules voisines
                     for (Int_t nyv = ny - fNvoisins; nyv <= ny + fNvoisins; nyv += 1) {
                        for (Int_t nzv = nz - fNvoisins; nzv <= nz + fNvoisins; nzv += 1) {
                           Int_t idxv = fCluster->GetBinContent(nxv, nyv, nzv);
                           if (idxv >= 1) { //cellule voisine a affilier
                              if (idxv > 1) { // la cellule appartient a un cluster
                                 //cellule voisine appartenant a un cluster deja identifie
                                 nl.Add(idxv);
                              }
                              else {

                              }
                              ncv += 1;
                           }
                        }
                     }
                  }
                  //au moins une cellule voisine non nulle
                  //
                  if (ncv >= 1) {
                     //les cellules voisines sont associees a un ou plusieur cluster
                     if (nl.GetNValues() >= 1) {
                        //on prend le premier cluster qui vient
                        Int_t newid = nl.First();
                        for (Int_t nxv = nx - fNvoisins; nxv <= nx + fNvoisins; nxv += 1) { //on boucle sur les cellules voisines
                           for (Int_t nyv = ny - fNvoisins; nyv <= ny + fNvoisins; nyv += 1) {
                              for (Int_t nzv = nz - fNvoisins; nzv <= nz + fNvoisins; nzv += 1) {
                                 Int_t idxv = fCluster->GetBinContent(nxv, nyv, nzv);
                                 //on rajoute les cellules au cluster choisi
                                 if (idxv == 1) {
                                    fNcells->AddAt(1 + fNcells->At(newid), newid);
                                    fPop->AddAt(GetBinContent(nxv, nyv, nzv) + fPop->At(newid), newid);
                                    fCluster->SetBinContent(nxv, nyv, nzv, newid);
                                 }
                              }
                           }
                        }
                        if (nl.GetNValues() > 1) {
                           //on met les implementes les clusters contigus
                           nl.Begin();
                           while (!nl.End()) {
                              Int_t compt = nl.Next();
                              if (compt != newid) {
                                 fVoisins->Fill(TMath::Min(compt, newid), TMath::Max(compt, newid));
                              }
                           }
                        }
                     }
                     //creation d un nouveau cluster
                     else {

                        Ntemp += 1;
                        if (Ntemp > 999)
                           Warning("Clusterize", "Ntemp>999 ... should be some problems in the treatment of neigbouring clusters...");
                        //Info("Clusterize","New cluster %d",Ntemp);

                        if (Ntemp >= fNcells->GetSize()) {
                           fNcells->Set(Ntemp + 1);
                           fPop->Set(Ntemp + 1);
                        }
                        for (Int_t nxv = nx - fNvoisins; nxv <= nx + fNvoisins; nxv += 1) { //on boucle sur les cellules voisines
                           for (Int_t nyv = ny - fNvoisins; nyv <= ny + fNvoisins; nyv += 1) {
                              for (Int_t nzv = nz - fNvoisins; nzv <= nz + fNvoisins; nzv += 1) {
                                 Int_t idxv = fCluster->GetBinContent(nxv, nyv, nzv);
                                 if (idxv == 1) {
                                    fNcells->AddAt(1 + fNcells->At(Ntemp), Ntemp);
                                    fPop->AddAt(GetBinContent(nxv, nyv, nzv) + fPop->At(Ntemp), Ntemp);
                                    fCluster->SetBinContent(nxv, nyv, nzv, Ntemp);
                                 }
                              }
                           }
                        }
                     }
                  }
                  else {
                     if (IsLonelyCellsAreKept()) {
                        Ntemp += 1;
                        if (Ntemp > 999)
                           Warning("Clusterize", "Ntemp>999 ... should be some problems in the treatment of neigbouring clusters...");
                        //Info("Clusterize","Lonely cell %d",Ntemp);
                        if (Ntemp >= fNcells->GetSize()) {
                           fNcells->Set(Ntemp + 1);
                           fPop->Set(Ntemp + 1);
                        }
                        fNcells->AddAt(1 + fNcells->At(Ntemp), Ntemp);
                        fPop->AddAt(GetBinContent(nx, ny, nz) + fPop->At(Ntemp), Ntemp);
                        fCluster->SetBinContent(nx, ny, nz, Ntemp);
                     }
                     else {
                        fCluster->SetBinContent(nx, ny, nz, 0);
                     }
                  }
               }
            }
            else {

            }
         }
      }
   }

   //nclus commence a 1
   //le nombre actuel de cluster est nclus-1
   //
   //boucle sur les clusters voisins
   //on rassemble les clusters avec des cellules contigues
   //et on decrement en consequence le nombre de cluster
   //
   fNclusters = Ntemp - 1;

   for (Int_t ii = Ntemp; ii >= 2; ii -= 1) {
      for (Int_t jj = ii + 1; jj <= Ntemp; jj += 1) {
         Int_t contenu =  fVoisins->GetBinContent(ii, jj);
         if (contenu > 0) {
            if (fNcells->At(jj) > 0) {

               fNcells->AddAt(fNcells->At(jj) + fNcells->At(ii), ii);
               fPop->AddAt(fPop->At(jj) + fPop->At(ii), ii);
               for (Int_t nx = 1; nx <= GetNbinsX(); nx += 1) {
                  for (Int_t ny = 1; ny <= GetNbinsY(); ny += 1) {
                     for (Int_t nz = 1; nz <= GetNbinsZ(); nz += 1) {
                        if (fCluster->GetBinContent(nx, ny, nz) == jj)
                           fCluster->SetBinContent(nx, ny, nz, ii);
                     }
                  }
               }
               fNcells->SetAt(0, jj);
               fPop->SetAt(0, jj);

               fNclusters -= 1;
            }
         }
      }
   }

   //on passe les contenus en négatifs si ils appartiennent a aucun cluster
   //
   for (Int_t nx = 1; nx <= GetNbinsX(); nx += 1) {
      for (Int_t ny = 1; ny <= GetNbinsY(); ny += 1) {
         for (Int_t nz = 1; nz <= GetNbinsZ(); nz += 1) {
            if (fCluster->GetBinContent(nx, ny, nz) < 2) {
               fCluster->SetBinContent(nx, ny, nz, -1);
            }
         }
      }
   }

   //on reordonne les clusters, liste possible avec des clusters vides
   //on reprend l indexage en partant de 1 jusqu a nclust
   // pour avoir en sortie les contenus suivants dans hclust :
   // <=0 la cellule appartient a aucun cluster
   // contenu ii>0, la cellule appartient a iieme cluster
   //
   Int_t nreel = 1;
   for (Int_t ii = 0; ii <= Ntemp; ii += 1) {
      if (fNcells->At(ii) > 0) {
         for (Int_t nx = 1; nx <= GetNbinsX(); nx += 1) {
            for (Int_t ny = 1; ny <= GetNbinsY(); ny += 1) {
               for (Int_t nz = 1; nz <= GetNbinsZ(); nz += 1) {
                  if (fCluster->GetBinContent(nx, ny, nz) == ii) {
                     fCluster->SetBinContent(nx, ny, nz, nreel);
                  }
               }
            }
         }
         fNcells->SetAt(fNcells->At(ii), nreel);
         fPop->SetAt(fPop->At(ii), nreel);
         nreel += 1;
      }
   }

   if (nreel != fNclusters + 1)
      printf("pb de coherence %d %d\n", nreel, fNclusters + 1);

}

//____________________________________________________________________________//

Int_t KVClust3D::GetNclusters() const
{
   //return the number of clusters found by the Clusterize() method
   //
   return fNclusters;
}

Int_t* KVClust3D::GetSize() const
{
   //return the array of size (or number of cells) associated to each cluster
   // found by the Clusterize() method
   //
   return fNcells->GetArray();
}

Double_t* KVClust3D::GetPop() const
{
   //return the array of population (or total number of contents) associated to each cluster
   // found by the Clusterize() method
   //
   return fPop->GetArray();
}

//____________________________________________________________________________//

void KVClust3D::PrintResults() const
{
   //print results of the Clusterize() method
   //
   Info("PrintResults", "%d clusters found", fNclusters);
   for (Int_t ii = 0; ii < fNclusters; ii += 1) {
      printf("%d %d %lf\n", ii, fNcells->At(ii), fPop->At(ii));
   }

}

//____________________________________________________________________________//

TH3D* KVClust3D::GetClusterIndex() const
{
   //return the clone 3D histogram where each cell has for content
   //the index of the cluster it is associated to
   //It's allow, afterward, for the user to calculate other observable associated to each
   //clusters
   //
   return fCluster;
}
