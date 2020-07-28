//Created by KVClassFactory on Wed May 19 17:23:29 2010
//Author: bonnet

#include "KVValues.h"
#include "KVNumberList.h"
#include "KVList.h"

ClassImp(KVValues)


//___________________________________________________________________________________________
void KVValues::init_val_base()
{
//protected method
//Mise en correspondance du noms des differentes valeurs a calculees
//et de leur position dans le tableau values
// Ex:
// nom_valeurs -> id_valeurs -> values[id_valeurs]

   KVString lname = "MIN MAX";
   lname.Begin(" ");
   kval_base = 0;    //variables de base disponibles

   kval_tot = 0; //nombre de variables totales disponibles
   while (!lname.End()) {
      KVString tamp = lname.Next();
      SetValue(tamp.Data(), kval_tot++);
   }
   kdeb = kval_tot;  //ici pos_deb=2 (par defaut)

   KVString smoment;
   for (Int_t mm = 0; mm <= kordre_mom_max; mm += 1) { //Ex: moment_max = 3
      smoment.Form("SUM%d", mm);
      SetValue(smoment.Data(), kval_tot++);
   }
   kval_base = kval_tot;   //ici nbase=6 (par defaut)

   values = new Double_t[knbre_val_max];
   init_val();


}

//___________________________________________________________________________________________
void KVValues::init_val_add()
{
//protected method

   kform_add = new KVList();
   kpar_add = new KVList();

   kval_add = 0;

   kToBeRecalculated = kTRUE;

}

//___________________________________________________________________________________________
void KVValues::init_val()
{
//protected method
   values[GetIntValue("MIN")] = 1e6;
   values[GetIntValue("MAX")] = -1e6;

   for (Int_t nn = kdeb; nn < kval_tot; nn += 1)
      values[nn] = 0;

   kTimesFillVarIsCalled = 0;
}

//___________________________________________________________________________________________
KVValues::KVValues()
{
//default constructor
}

//___________________________________________________________________________________________
KVValues::KVValues(const Char_t* name, Int_t ordre_max, Int_t nbre_max)
{
//constructor
   knbre_val_max = nbre_max;     //nbre max de valeurs stockables
   kordre_mom_max = ordre_max;   //ordre max des moments calcules a chaque iteration
   SetName(name);                //nom correspondant aux valeurs calculees

   init_val_base();
   init_val_add();
}

//___________________________________________________________________________________________
void KVValues::Reset()
{

   init_val();
   kToBeRecalculated = kTRUE;
}

//___________________________________________________________________________________________
KVValues::~KVValues()
{
//destructeur
   Clear();
   delete [] values;
   values = 0;
   kval_tot = kval_base = kdeb = 0;
}


//___________________________________________________________________________________________
void  KVValues::Clear(Option_t*)
{

   Reset();

   delete kform_add;
   kform_add = 0;
   delete kpar_add;
   kpar_add = 0;

   kval_add = 0;
   kval_tot = kval_base;

}

//___________________________________________________________________________________________
void  KVValues::Print(Option_t*) const
{
   //Info("Print","%s : %d values computed",GetName(),kval_tot);
   printf("KVValues::Print_NVL\n%s : %d values computed\n", GetName(), kval_tot);
   for (Int_t nn = 0; nn < kval_tot; nn += 1) {
      printf("- %d %s %lf\n", nn, GetNameAt(nn), GetValue(nn));
   }
}

//___________________________________________________________________________________________
void KVValues::LinkParameters(KVNumberList* nl)
{
   kpar_add->Add(nl);
}

//___________________________________________________________________________________________
void KVValues::ComputeAdditionalValues(Int_t min, Int_t max)
{

   if (min < 0)   min = 0;
   if (max == -1) max = kval_add;

   KVNumberList* nl = 0;
   TFormula* fm = 0;

   for (Int_t ff = min; ff < max; ff += 1) {
      nl = (KVNumberList*)kpar_add->At(ff);
      fm = (TFormula*)kform_add->At(ff);
      nl->Begin();
      while (!nl->End()) {
         Int_t nn = nl->Next();
         fm->SetParameter(nn, values[nn]);
      }
      values[kval_tot - kval_add + ff] = fm->Eval(0);
   }
}


//___________________________________________________________________________________________
void KVValues::DefineAdditionalValue(KVString name, KVString expr)
{

   if (HasParameter(name.Data())) {
      Warning("DefineAdditionalValue", "le nom de variable %s est deja utilise", name.Data());
      return;
   }

   KVString expr2 = expr;
   KVNumberList* nl = TransformExpression(expr2);

   if (nl) {
      if (nl->IsEmpty()) {
         Warning("DefineAdditionalValue", "la KVNumberList est vide #%s#", nl->GetList());
      }
      else {
         if (AddFormula(name, expr2)) {
            LinkParameters(nl);
            ComputeAdditionalValues(kval_add - 1, kval_add);
         }
      }
   }
   else {
      Error("DefineAdditionalValue", "La traduction de l'expression %s a echouee", expr.Data());
   }
}

//___________________________________________________________________________________________
Bool_t KVValues::AddFormula(KVString name, KVString expr)
{

   TFormula* fm = new TFormula(name.Data(), expr.Data());
   if (fm->GetExpFormula().IsNull()) {
      Warning("AddFormula", "%s considere comme nulle", expr.Data());
      return kFALSE;
      delete fm;
   }
   kform_add->Add(fm);
   SetValue(name.Data(), kval_tot++);
   kval_add += 1;
   return kTRUE;
}

//___________________________________________________________________________________________
KVNumberList* KVValues::TransformExpression(KVString& expr)
{

   KVNumberList* nl = new KVNumberList();

   const char* O = "[";
   const char* F = "]";

   KVString clone;
   clone.Form("%s", expr.Data());
   const char* expression = clone.Data();

   Int_t nouvert = 0, nferme = 0;
   Int_t posouvert = 0, posferme = 0;
   Int_t nvar = 0;

   Int_t nsize = clone.Length();
   for (Int_t ii = 0; ii <= nsize; ii += 1) {
      if (expression[ii] == O[0]) {
         nouvert += 1;
         posouvert = ii;
      }
      else if (expression[ii] == F[0]) {
         nferme += 1;
         posferme = ii;
         KVString st(clone(posouvert + 1, posferme - posouvert - 1));
         if (st.IsDigit()) {
            Int_t idx = st.Atoi();
            if (0 <= idx && idx < kval_tot) {
               nl->Add(idx);
               nvar += 1;
            }
            else {
               delete nl;
               return 0;
            }
         }
         else {

            Int_t idx = GetValuePosition(st.Data());
            if (idx == -1) {
               delete nl;
               return 0;
            }
            nl->Add(idx);
            nvar += 1;
            KVString s1;
            s1.Form("[%s]", st.Data());
            KVString s2;
            s2.Form("[%d]", idx);
            expr.ReplaceAll(s1, s2);
         }
      }
      else {}
   }

   if (nouvert != nferme || nvar != nouvert) {
      Error("TransformExpr", "nombre [ : %d - nombre de ] : %d - nombre de variables %d", nouvert, nferme, nvar);
   }

   return nl;

}

//___________________________________________________________________________________________
void KVValues::FillVar(Double_t val)
{

   FillVar(val, 1.);

}

//___________________________________________________________________________________________
void KVValues::FillVar(Double_t val, Double_t weight)
{

   if (weight >= 0) {
      if (val < values[0]) values[0] = val; //GetIntValue("MIN")=0
      if (val > values[1]) values[1] = val; //GetIntValue("MAX")=1
   }
   for (Int_t nn = 0; nn <= kordre_mom_max; nn += 1) values[nn + kdeb] += weight * TMath::Power(val, Double_t(nn));
   kToBeRecalculated = kTRUE;
   kTimesFillVarIsCalled += 1;
}

//___________________________________________________________________________________________
Int_t KVValues::GetOrdreMax() const
{

   return kordre_mom_max;

}
//___________________________________________________________________________________________
Int_t KVValues::GetNumberOfFilling() const
{

   return kTimesFillVarIsCalled;

}
//___________________________________________________________________________________________
Int_t KVValues::GetShift() const
{

   return kdeb;

}

//___________________________________________________________________________________________
Double_t KVValues::GetValue(Int_t idx) const
{

   const_cast<KVValues*>(this)->ComputeAdditionalValues();
   const_cast<KVValues*>(this)->kToBeRecalculated = kFALSE;
   return values[idx];

}

//___________________________________________________________________________________________
Double_t KVValues::GetValue(const Char_t* name) const
{

   return GetValue(GetValuePosition(name));

}
//___________________________________________________________________________________________
Int_t KVValues::GetValuePosition(const Char_t* name) const
{

   return GetIntValue(name);

}

//___________________________________________________________________________________________
TString KVValues::GetValueExpression(const Char_t* name) const
{

   Int_t idx = GetValuePosition(name);
   return GetValueExpression(idx);
}

//___________________________________________________________________________________________
TString KVValues::GetValueExpression(Int_t idx) const
{
   Int_t new_idx = idx - kval_base;
   if (new_idx < 0) {
      Warning("GetValueExpression", "Cette methode n'est implementee que pour les valeurs additionelles idx > %d", kval_base);
      return "";
   }
   return ((TFormula*)kform_add->At(new_idx))->GetExpFormula();
}

//___________________________________________________________________________________________
Int_t KVValues::GetNValues(KVString opt) const
{
   if (opt == "base")     return kval_base;
   else if (opt == "add") return kval_add;
   else {
      return GetNpar();
   }
}

//___________________________________________________________________________________________
Bool_t KVValues::Add(KVValues* val)
{
   if (this->GetNValues("base") != val->GetNValues("base")) return kFALSE;
   if (this->GetValue("MIN") > val->GetValue("MIN"))
      values[GetValuePosition("MIN")] = val->GetValue("MIN");
   if (this->GetValue("MAX") < val->GetValue("MAX"))
      values[GetValuePosition("MAX")] = val->GetValue("MAX");

   for (Int_t ii = kdeb; ii < this->GetNValues("base"); ii += 1) {
      values[ii] += val->GetValue(ii);
   }
   kTimesFillVarIsCalled += val->GetNumberOfFilling();
   return kTRUE;
}
