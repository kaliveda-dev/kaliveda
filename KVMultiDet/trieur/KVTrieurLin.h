/**
  \class KVTrieurLin
  \brief Class for a linear sorting
  \ingroup Analysis
~~~~{.cpp}
   This class returns an index determined via a linear interpolation between
 two values Xmin and Xmax. The following methods are provided by this class:
      virtual Int_t GetNumCase(Double_t x)   Gives an index corresponding to
                                             the value of x. nb_cells being
                                             the total number of indexes, the
                                             returned value is

                                                       (x-xmin)*nb_cells
                                             1+(Int_t)-------------------
                                                          xmax-xmin

                                             if x<xmin it returns 1
                                             if x>xmax it returns nb_cells

      virtual Int_t GetNumCase(void *,...)   return -1,not to use.

 BEWARE! : the index value ranges between 1 and nb_cells.

  The Xmin and Xmax values can be adjusted with the methods
      virtual void SetXmin(Double_t x)
      virtual void SetXmax(Double_t x)

  The name of the sorting variable can be set withe the method
      virtual void SetNomVar(Char_t *s)

 The number of indexes can be set with the method
      virtual void SetNbCases(Int_t n)

  Setting the number of indexes, the name of the sorting variable or the Xmin
 or Xmax values automatically generates the names for each index.

  In a treatment program, this can be used to set histogram titles and to
 manage efficiently arrays of histograms. Here is an example where the sorting
 variable is Ekin (calculated using KVEkin class).
== Example ==================================================================================================
 // Declarations and initialisations
 ...
 KVEkin Sekin;
 Sekin.Init();
 KVZmax Zmax;
 Zmax.Init();
 ...
 KVTrieurLin sortEkin;
 sortEkin.SetNbCases(10);       // 10 indexes
 sortEkin.SetNomVar("E_{kin}"); // name of the sorting variable
 sortEkin.SetXmin(0.);          // minimum value for Ekin
 sortEkin.SetXmax(800.);        // maximum value for Ekin
 ...
 // Declaration of histograms
 TList *lekin=new TList();      // list to store histograms
 for(Int_t i=0;i<sortEkin.GetNbCases();i++)
  {
  TString sname("histo"); // TString for the histogram name
  sname+=i;
  TString stitle("Z_{max} for "); // TString for the histogram title
  stitle+=sortEkin.GetNomCase(i+1);
  TH1F *h1=new TH1F(sname.Data(),stitle.Data(),100,0.5,100.5);
  lekin->Add(h1); // Add the histogram to the list
  }
 ...
 // Treatment loop for each event called for each event
 ...
 Sekin.Reset();
 Zmax.Reset();
 KVINDRAReconNuc *part = 0;
 while( (part = GetEvent()->GetNextParticle("ok")) ){//loop over particles with correct codes
  Sekin.Fill(part);
  Zmax.Fill(part);
 }
 // Filling the appropriate histogram
 Int_t index=sortEkin(Sekin());       // Determine the index value according to Sekin()
 TH1F *h1=(TH1F *)lekin->At(index-1); // retrieve the histogram in the list
 h1->Fill(Zmax());                    // fill it!
 ...
~~~~
*/
#include "KVTrieur.h"

class KVTrieurLin: public KVTrieur {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;

   Double_t xmin;
   Double_t xmax;
   Char_t nom_var[80];

// Methodes
protected:
   void initKVTrieurLin(void);  // Initialisations
   void SetNomsCases(void);     // Initialisations des noms de cases

public:
   KVTrieurLin(void);          // constructeur par defaut
   KVTrieurLin(Char_t* nom);
   KVTrieurLin(Int_t nbcases, Char_t* nom);
   KVTrieurLin(const KVTrieurLin& a);  // constructeur par copie

   virtual ~ KVTrieurLin(void);        // destructeur


#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVTrieurLin& operator =(const KVTrieurLin& a);      // operateur =



   virtual Int_t GetNumCase(void* argus ...);   // Pour une situation donnee,
   // retourne le numero de la case
   // correspondante.
   virtual Int_t GetNumCase(Double_t x);        // Pour un x donne,
   // retourne le numero de la case
   // correspondante.

   virtual void SetNbCases(Int_t n);    // ajuste le nombre de cases.
   virtual void SetNomVar(Char_t* x);
   virtual void SetXmin(Double_t x);
   virtual void SetXmax(Double_t x);
   virtual const Char_t* GetNomVar(void);
   virtual Double_t GetXmin(void);
   virtual Double_t GetXmax(void);

   ClassDef(KVTrieurLin, 1)    // Class for linear sorting
};
