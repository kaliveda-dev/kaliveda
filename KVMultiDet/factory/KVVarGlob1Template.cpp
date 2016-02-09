//_________________________________________________________________
KVVarGlob1Template::KVVarGlob1Template(void): KVVarGlob1()
{
//
// Default constructor (may not be modified)
//
   init_KVVarGlob1Template();
   SetName("KVVarGlob1Template");
   SetTitle("A KVVarGlob1Template");
}

//_________________________________________________________________
KVVarGlob1Template::KVVarGlob1Template(Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom (may not be modified)
//
   init_KVVarGlob1Template();
}

//_________________________________________________________________
KVVarGlob1Template::KVVarGlob1Template(const KVVarGlob1Template& a): KVVarGlob1()
{
//
// Copy constructor (may not be modified)
//
   init_KVVarGlob1Template();
   a.Copy(*this);
}

//_________________________________________________________________
KVVarGlob1Template::~KVVarGlob1Template(void)
{
//
// Destructeur
//
}

//_________________________________________________________________
void KVVarGlob1Template::Copy(TObject& a) const
{
   // Copy properties of 'this' object into the KVVarGlob object referenced by 'a'

   KVVarGlob1::Copy(a);// copy attributes of KVVarGlob1 base object

   KVVarGlob1Template& aglob = (KVVarGlob1Template&)a;
   // Now copy any additional attributes specific to this class:
   // To copy a specific field, do as follows:
   //
   //     aglob.field=field;
   //
   // If setters and getters are available, proceed as follows
   //
   //    aglob.SetField(GetField());
   //

}

//_________________________________________________________________
KVVarGlob1Template& KVVarGlob1Template::operator = (const KVVarGlob1Template& a)
{
//
// Operator = (may not be modified)
//
   a.Copy(*this);
   return *this;
}

