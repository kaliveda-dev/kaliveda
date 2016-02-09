KVVarGlobMeanTemplate::KVVarGlobMeanTemplate(void): KVVarGlobMean()
{
//
// Default constructor (may not be modified)
//
   init_KVVarGlobMeanTemplate();
   SetName("KVVarGlobMeanTemplate");
   SetTitle("A KVVarGlobMeanTemplate");
}

//_________________________________________________________________
KVVarGlobMeanTemplate::KVVarGlobMeanTemplate(Char_t* nom): KVVarGlobMean(nom)
{
//
// Constructeur avec un nom (may not be modified)
//
   init_KVVarGlobMeanTemplate();
}

//_________________________________________________________________
KVVarGlobMeanTemplate::KVVarGlobMeanTemplate(const KVVarGlobMeanTemplate& a): KVVarGlobMean()
{
//
// Copy constructor (may not be modified)
//
   init_KVVarGlobMeanTemplate();
   a.Copy(*this);
}

//_________________________________________________________________
KVVarGlobMeanTemplate::~KVVarGlobMeanTemplate(void)
{
//
// Destructeur
//
}

//_________________________________________________________________
void KVVarGlobMeanTemplate::Copy(TObject& a) const
{
   // Copy properties of 'this' object into the KVVarGlob object referenced by 'a'

   KVVarGlobMean::Copy(a);// copy attributes of KVVarGlobMean base object

   KVVarGlobMeanTemplate& aglob = (KVVarGlobMeanTemplate&)a;
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
KVVarGlobMeanTemplate& KVVarGlobMeanTemplate::operator = (const KVVarGlobMeanTemplate& a)
{
//
// Operator = (may not be modified)
//
   a.Copy(*this);
   return *this;
}

