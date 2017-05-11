//
//Author: Daniel Cussol
//
// Developpement d'un classe KVTenseur3 permettant de calculer les axes
// propres d'un evenement
//

#include "Riostream.h"
#include <iomanip>
#include <math.h>
#include "TMath.h"
#include "KVTenseur3.h"

#include "TVector.h"
#include "TMatrix.h"

using namespace std;

ClassImp(KVTenseur3)
//////////////////////////////////////////////////////////////////////////////////
//   This class fills, diagonalizes the velocity tensor and returns several
//   useful values. It is used by the global variable classes KVTensP and
//      KVTensPCM.
//
Int_t KVTenseur3::nb_KVTenseur3 = 0;

//_________________________________________________________________
void KVTenseur3::init_KVTenseur3(void)
{
//
// Routine d'Initialisation
//
   Reset();
   nb_KVTenseur3++;
}

//_________________________________________________________________
void KVTenseur3::Diago(void)
{
//
// Routine de diagonalisation en utilisant TMatrix
//
   TMatrix matrice(3, 3);
   TVector vapro(3);
   TMatrix vepro(3, 3);
   Int_t i, j;

#if ROOT_VERSION_CODE >= ROOT_VERSION(4,0,0)

   Double_t check_sum = 0.0;
   //check_sum used to make sure matrix has non-zero elements
   //otherwise call to matrice.Eigenvectors never returns !!
   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++) {
         matrice(i, j) = tenseur[3 * i + j];
         check_sum += tenseur[3 * i + j];
      }

   if (check_sum > 0.0)
      vepro = matrice.EigenVectors(vapro);

   //On va trier les valeurs propres par odre decroissant
   int itri[3] = { 0, 1, 2 };
   Bool_t ok = kFALSE;
   while (!ok) {
      ok = kTRUE;
      for (Int_t ii = 0; ii < 2; ii++)
         if (vapro(itri[ii]) > vapro(itri[ii + 1])) {
            int tmp = itri[ii];
            itri[ii] = itri[ii + 1];
            itri[ii + 1] = tmp;
            ok = kFALSE;
         }
   }
   //

   for (Int_t ii = 0; ii < 3; ii++) {
      i = itri[ii];
      for (j = 0; j < 3; j++)
         vep[3 * ii + j] = vepro(j, i);
      vap[ii] = vapro(i);
   }

   is_diago = 1;
#else

   Double_t check_sum = 0.0;
   //check_sum used to make sure matrix has non-zero elements
   //otherwise call to matrice.Eigenvectors never returns !!
   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++) {
         matrice(i, j) = tenseur[3 * i + j];
         check_sum += tenseur[3 * i + j];
      }

   if (check_sum > 0.0)
      vepro = matrice.EigenVectors(vapro);

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         vep[3 * j + i] = vepro(i, 2 - j);
   for (i = 0; i < 3; i++)
      vap[i] = vapro(2 - i);

   is_diago = 1;
#endif
}

//_________________________________________________________________
KVTenseur3::KVTenseur3(void): KVBase("KVTenseur3", "KVTenseur3")
{
//
// Createur par defaut
//
   init_KVTenseur3();
}

//_________________________________________________________________
KVTenseur3::KVTenseur3(const Char_t* nom): KVBase(nom)
{
//
// Createur avec nom
//
   init_KVTenseur3();
}

//_________________________________________________________________
KVTenseur3::KVTenseur3(const KVTenseur3& t) : KVBase()
{
//
// Createur par Copy
//
   init_KVTenseur3();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   t.Copy(*this);
#else
   ((KVTenseur3&) t).Copy(*this);
#endif
}

//_________________________________________________________________
KVTenseur3::~KVTenseur3(void)
{
//
// Destructeur
//
   nb_KVTenseur3--;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTenseur3::Copy(TObject& a) const
#else
void KVTenseur3::Copy(TObject& a)
#endif
{
//
// Copy
//
   ((KVTenseur3&) a).Reset();
   KVBase::Copy(a);
   for (int i = 1; i <= 3; i++) {
      for (int j = 1; j <= 3; j++) {
         ((KVTenseur3&) a).SetElem(i, j, GetElem(i, j));
         ((KVTenseur3&) a).SetVep(i, j,
                                  ((KVTenseur3*) this)->GetVep(i, j));
      }
      ((KVTenseur3&) a).SetVap(i, ((KVTenseur3*) this)->GetVap(i));
   }
}

//_________________________________________________________________
KVTenseur3& KVTenseur3::operator =(const KVTenseur3& t)
{
//
// Operateur =
//
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   t.Copy(*this);
#else
   ((KVTenseur3&) t).Copy(*this);
#endif
   return *this;
}

//_________________________________________________________________
void KVTenseur3::Print(Option_t*) const
{
//
// Affichage
//
   Int_t n1 = 3;
   Int_t i;

   cout << endl << "Matrice a diagonaliser:" << endl;
   for (i = 0; i < n1; i++) {
      cout << i + 1 << " : ";
      for (Int_t j = 0; j < n1; j++)
         cout << tenseur[n1 * i + j] << " ";
      cout << endl;
   }
   if (is_diago) {
      cout << endl << "Resultat de la diagonalisation:" << endl;
      for (i = 0; i < n1; i++) {
         cout << "Vap " << i + 1 << " : " << vap[i] << endl << "   ";
         for (Int_t j = 0; j < n1; j++)
            cout << vep[n1 * i + j] << " ";
         cout << endl;
      }
      cout << "ThetaFlot = " << const_cast <
           KVTenseur3* >(this)->GetThetaFlot() << endl;
      cout << "PhiPlan   = " << const_cast <
           KVTenseur3* >(this)->GetPhiPlan() << endl << endl;
   }
}

//_________________________________________________________________
void KVTenseur3::Fill(const TVector3& v, Double_t w)
{
//
// Ajout d'un vecteur
//
   Int_t i, j;
   Double_t vec[3];

   for (i = 0; i < 3; i++)
      vec[i] = v[i];
   for (i = 0; i < 3; i++) {
      for (j = i; j < 3; j++) {
         tenseur[3 * i + j] += w * vec[i] * vec[j];
         tenseur[3 * j + i] = tenseur[3 * i + j];
      }
   }
   is_diago = 0;
}

//_________________________________________________________________
void KVTenseur3::Fill(const KVNucleus& c, Double_t w)
{
//
// Ajout d'un fragment
//
   TVector3 vec = c.GetV();

   Fill(vec, w);
}

//_________________________________________________________________
void KVTenseur3::Reset(void)
{
//
// Remise a zero
//
   Int_t i, j;

   for (i = 0; i < 3; i++) {
      vap[i] = 0.;
      for (j = 0; j < 3; j++) {
         tenseur[3 * i + j] = 0.;
         vep[3 * i + j] = 0.;
      }
   }
   is_diago = 0;

}

//_________________________________________________________________
Double_t KVTenseur3::GetThetaFlot(void)
{
//
// Obtention du Theta Flot (en degrès). NB. valeur entre 0 et +90 deg.
//
   Double_t tf;

   if (is_diago == 0)
      Diago();

   TVector3 vp = GetVep(3);

   if (vp.Z() < 0)
      vp = -vp;
   tf = vp.Theta() * 180. / TMath::Pi();
   return tf;
}

//_________________________________________________________________
Double_t KVTenseur3::GetPhiPlan(void)
{
   // Obtention du Phi du plan de reaction (en degrès).
   // Cet angle suit les conventions de KaliVeda i.e. il est entre 0 et 360 degrees
   if (is_diago == 0)
      Diago();

   TVector3 vp = GetVep(3);

   if (vp.Z() < 0)
      vp = -vp;
   Double_t phi = vp.Phi() * TMath::RadToDeg();
   return (phi < 0 ? 360. + phi : phi);
}

//_________________________________________________________________
Double_t KVTenseur3::GetElem(Int_t i, Int_t j) const
{
//
// Obtention de l'element [i][j]
//
   if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
      return tenseur[3 * (i - 1) + (j - 1)];
   } else {
      return 0.;
   }
}

//_________________________________________________________________
Double_t KVTenseur3::GetVap(Int_t i)
{
//
// Valeur Propre
//
   if (is_diago == 0)
      Diago();

   return vap[i - 1];
}

//_________________________________________________________________
void KVTenseur3::SetVap(Int_t i, Double_t x)
{
//
//Set  Valeur Propre
//
   vap[i - 1] = x;
}

//_________________________________________________________________
Double_t KVTenseur3::GetVapNorm(Int_t i)
{
//
// Valeur propre normalisee
//
   if (is_diago == 0)
      Diago();

   Double_t vsum = 0.;
   Double_t val = 0;
   for (Int_t j = 0; j < 3; j++) {
      if (TMath::Log(vap[j]) > -100)
         vsum += vap[j];
   }

   if (vsum > 0.) {
      if (TMath::Log(vap[i - 1]) > -100)
         val = vap[i - 1] / vsum;
   } else {
      val = -2.;
   }
   return val;
}

//_________________________________________________________________
TVector3 KVTenseur3::GetVep(Int_t i)
{
//
// Vecteur propre
//
   if (is_diago == 0)
      Diago();

   Int_t dec = 3 * (i - 1);
   TVector3 vec(vep[dec], vep[dec + 1], vep[dec + 2]);

   return vec;
}

//_________________________________________________________________
Double_t KVTenseur3::GetVep(Int_t i, Int_t j)
{
//
// Element (i,j) of "vep" matrix
//
   if (is_diago == 0)
      Diago();

   if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
      return vep[3 * (i - 1) + (j - 1)];
   } else {
      return 0.;
   }
}

//_________________________________________________________________
void KVTenseur3::SetVep(Int_t i, Int_t j, Double_t x)
{
//
// Set element (i,j) of "vep" matrix
//
   if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
      vep[3 * (i - 1) + (j - 1)] = x;
   }
}

//_________________________________________________________________
void KVTenseur3::SetElem(Int_t i, Int_t j, Double_t x)
{
//
// Set element (i,j) of tenseur matrix
//
   if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
      tenseur[3 * (i - 1) + (j - 1)] = x;
   }
}

//_________________________________________________________________
Double_t KVTenseur3::GetSphericite(void)
{
//
// Sphericite
//
   return 3. * (1. - GetVapNorm(3)) / 2.;
}

//_________________________________________________________________
Double_t KVTenseur3::GetCoplanarite(void)
{
//
// Coplanarite
//
   return TMath::Sqrt(3.) * (GetVapNorm(2) - GetVapNorm(1)) / 2.;
}

//_________________________________________________________________
Double_t KVTenseur3::GetDirectivite(void)
{
//
// Directivite
//
   Warning("GetDirectivity", "Not implemented");
   return 1.;
}

//_________________________________________________________________

void KVTenseur3::GetRotation(TRotation& rot)
{
   //Sets rotation matrix corresponding to eigenvectors of diagonalized tensor.
   //According to ROOT physics vectors convention, this is an 'active' rotation
   //(see TRotation class description). Note that KVParticle::SetFrame automatically
   //uses this matrix in such a way as to describe particles in the coordinate frame
   //corresponding to the rotated axes.
   //
   //By convention, the three eigenvectors (V1, V2, V3) are numbered in increasing
   //order of size of the corresponding eigenvalue (V3 <--> largest eignevalue <--> "flow" axis).
   //The new coordinate axes are defined (arbitrarily) as follows :
   //      V1 --->  X'
   //      V2 --->  Y'
   //      V3 --->  Z'

   //make orthonormal basis vectors from 3 eignevectors
   TVector3 V1 = GetVep(1);
   TVector3 V2 = GetVep(2);
   TVector3 V3 = GetVep(3);
   if (V3.Z() < 0)
      V3 = -V3;
   rot.MakeBasis(V1, V2, V3);
   //set rotation matrix
   rot.SetToIdentity();
   rot.RotateAxes(V1, V2, V3);
}
