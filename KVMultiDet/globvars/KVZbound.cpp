//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des energies cinetiques
//

#include "KVZbound.h"

ClassImp(KVZbound)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the sum of the charges of particles and fragments.
//
// By default this sum includes all nuclei having a charge greater than or equal to 'zmin'.
//The value of zmin can be set calling the method SetZmin(z).
//The value of zmin can be obtained by calling the method GetZmin().
//By default, zmin is set to 3.
//N.B. in the "standard" definition of the Zbound variable used by the ALADIN
//collaboration, zmin=2 (i.e. Zbound=sum of all charged products except hydrogens)
//
//KVZbound can also be used to calculate the sum of Z for
//
// i) all nuclei with Z>=zmin
// ii) all nuclei with Z<=zmax
// iii) all nuclei with Z>=zmin and Z<=zmax
// iv) any of the above conditions + selection based on projection of particle's
//velocity along the 'z'-direction of whatever reference frame you choose (using SetFrame())
//
//EXAMPLE OF USE
//
//In order to add a KVZbound global variable to your analysis, place the following
//line in the InitAnalysis() method of your analysis class:
//
//    KVVarGlob* vg = AddGV("KVZbound", "zbound");
//
//This gives the default behaviour (sum of Z for Z>=3).
//To change the selection conditions:
//
// i) sum of all nuclei with Z>=5:
//    vg->SetParameter("Zmin", 5);
//
// ii) sum of all nuclei with Z<=12:
//    vg->SetParameter("Zmax", 12);
//
// iii) sum of all nuclei with Z>=3 and Z<=20:
//    vg->SetParameter("Zmin", 3);
//    vg->SetParameter("Zmax", 20);
//
// iv) all nuclei with Z<=2 and V>=0 in the centre of mass frame:
//    vg->SetParameter("Zmax", 2);
//    vg->SetFrame("CM");
//    vg->SetParameter("Vmin", 0);
//
// v) all nuclei with 3<=Z<=10 and -1.6<=V<=3.4 (cm/ns) in the "ELLIPSOID"
//     frame (if you define a frame with such a name...)
//    vg->SetParameter("Zmin", 3);
//    vg->SetParameter("Zmax", 10);
//    vg->SetFrame("ELLIPSOID");
//    vg->SetParameter("Vmin", -1.6);
//    vg->SetParameter("Vmax", 3.4);
//

Int_t KVZbound::nb = 0;
Int_t KVZbound::nb_crea = 0;
Int_t KVZbound::nb_dest = 0;

//_________________________________________________________________
void KVZbound::init_KVZbound(void)
{
//
// Initialisation des champs de KVZbound
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;

   zmin = 3;
   zmax = 0;

   fValueType = 'I'; //integer values
}

//_________________________________________________________________
KVZbound::KVZbound(void): KVVarGlob1()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVZbound();
   sprintf(nom, "KVZbound_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVZbound
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVZbound::KVZbound(Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVZbound();
#ifdef DEBUG_KVZbound
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVZbound::KVZbound(const KVZbound& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVZbound();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZbound&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZbound
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVZbound::~KVZbound(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZbound
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVZbound::Copy(TObject& a) const
#else
void KVZbound::Copy(TObject& a)
#endif
{
// Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.

#ifdef DEBUG_KVZbound
   cout << "Copy de " << GetName() << "..." << endl;
#endif
   KVVarGlob1::Copy(a);
   ((KVZbound&) a).SetZmin(GetZmin());
#ifdef DEBUG_KVZbound
   cout << "Nom de la Copy (resultat) : " << a.GetName() << endl;
#endif
}

//_________________________________________________________________
KVZbound& KVZbound::operator =(const KVZbound& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVZbound
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZbound&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZbound
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVZbound::Fill(KVNucleus* c)
{
   //Add the atomic number, Z, of the nucleus to the Zbound variable
   //if it satisfies the conditions imposed by the user

   Int_t z = c->GetZ();
   Bool_t ok = (TestBit(kZMin) ? z >= zmin : (TestBit(kZMax) ? z <= zmax : (z >= zmin && z <= zmax)));
   if (ok && TestBit(kVTest)) {
      Double_t v = c->GetFrame(fFrame.Data())->GetVpar();
      ok = (TestBit(kVMin) ? v >= vmin : (TestBit(kVMax) ? v <= vmax : (v >= vmin && v <= vmax)));
   }
   if (ok) FillVar(z);
}

//_________________________________________________________________
Int_t KVZbound::GetZmin(void) const
{
   //Value of minimum Z included in sum of charges
   if (const_cast<KVZbound*>(this)->IsParameterSet("Zmin")) return (Int_t)const_cast<KVZbound*>(this)->GetParameter("Zmin");
   Info("GetZmin", "No value set for Zmin. Returns 0.");
   return 0;
}

//_________________________________________________________________
void KVZbound::SetZmin(Int_t zm)
{
   //Sets minimum Z to include in sum of charges
   SetParameter("Zmin", zm);
}

//_________________________________________________________________

void KVZbound::Init()
{
   //Initialisation of variable based on current values of options and parameters

   ResetBit(kZMin);
   ResetBit(kZMax);
   ResetBit(kVMin);
   ResetBit(kVMax);
   ResetBit(kZBoth);
   ResetBit(kVBoth);
   //get type of Z limits - min, max, or both
   if (IsParameterSet("Zmin") && IsParameterSet("Zmax")) {
      SetBit(kZBoth); // Z>= Zmin && Z<=Zmax
      zmin = (Int_t)GetParameter("Zmin");
      zmax = (Int_t)GetParameter("Zmax");
   } else if (IsParameterSet("Zmin")) {
      SetBit(kZMin); // Z>= Zmin
      zmin = (Int_t)GetParameter("Zmin");
   } else if (IsParameterSet("Zmax")) {
      SetBit(kZMax); // Z>= Zmax
      zmax = (Int_t)GetParameter("Zmax");
   } else {
      //no Z parameter set
      //default is to calculate Zbound for Z>= 3
      SetBit(kZMin);
      zmin = 3;
   }
   //get type of V limits - min, max, or both
   if (IsParameterSet("Vmin") && IsParameterSet("Vmax")) {
      SetBit(kVBoth); // V>= Vmin && V<=Vmax
      vmin = GetParameter("Vmin");
      vmax = GetParameter("Vmax");
   } else if (IsParameterSet("Vmin")) {
      SetBit(kVMin); // V>= Vmin
      vmin = GetParameter("Vmin");
   } else if (IsParameterSet("Vmax")) {
      SetBit(kVMax); // V>= Vmax
      vmax = GetParameter("Vmax");
   }
   //set kVTest flag if any velocity condition is imposed
   SetBit(kVTest, (TestBit(kVBoth) || TestBit(kVMin) || TestBit(kVMax)));
}
