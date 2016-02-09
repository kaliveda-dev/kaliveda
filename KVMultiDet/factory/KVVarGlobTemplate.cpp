KVVarGlobTemplate::KVVarGlobTemplate(void): KVVarGlob()
{
   // Default constructor
   init_KVVarGlobTemplate();
   SetName("KVVarGlobTemplate");
   SetTitle("A KVVarGlobTemplate");
}

//_________________________________________________________________
KVVarGlobTemplate::KVVarGlobTemplate(Char_t* nom): KVVarGlob(nom)
{
   // Constructor with a name for the global variable
   init_KVVarGlobTemplate();
}

//_________________________________________________________________
KVVarGlobTemplate::KVVarGlobTemplate(const KVVarGlobTemplate& a): KVVarGlob()
{
   // Copy constructor
   init_KVVarGlobTemplate();
   a.Copy(*this);
}

//_________________________________________________________________
KVVarGlobTemplate::~KVVarGlobTemplate(void)
{
   // Destructor
}

//_________________________________________________________________
void KVVarGlobTemplate::Copy(TObject& a) const
{
   // Copy properties of 'this' object into the KVVarGlob object referenced by 'a'

   KVVarGlob::Copy(a);// copy attributes of KVVarGlob base object

   KVVarGlobTemplate& aglob = (KVVarGlobTemplate&)a;
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
KVVarGlobTemplate& KVVarGlobTemplate::operator = (const KVVarGlobTemplate& a)
{
   // Operateur =

   a.Copy(*this);
   return *this;
}

//_________________________________________________________________
void KVVarGlobTemplate::Init(void)
{
   // Initialisation of internal variables, called once before beginning treatment
}

//_________________________________________________________________
void KVVarGlobTemplate::Reset(void)
{
   // Reset internal variables, called before treatment of each event
}

//_________________________________________________________________
Double_t* KVVarGlobTemplate::GetValuePtr(void)
{
   // You can use this method to return an array of values calculated by your global variable.
   // The order of the array should correspond to the index set for each value
   // using the method SetNameIndex(const Char_t*, Int_t) (see comments in method
   // getvalue_int(Int_t))

   return NULL;
}

//_________________________________________________________________
TObject* KVVarGlobTemplate::GetObject(void)
{
   // You can use this method to return the address of an object associated with your global variable.
   // This may be a list of particles, an intermediate object used to compute values, etc.

   return NULL;
}

Double_t KVVarGlobTemplate::getvalue_void() const
{
   // Protected method, called by GetValue()
   // This method should return the 'principal' or 'default' value associated with this global variable.
   //
   // For historical reasons, this method is 'const', which means that you cannot change the values
   // of any internal variables in this method (trying to do so will lead to your class not compiling).
   // If you need to get around this restriction, then you can use the following workaround:
   //
   //    const_cast<KVVarGlobTemplate*>(this)->MemberVariable = NewValue;
   //
   // The restriction also applies to calling other methods of the class which are not 'const'.
   // If you need to do this, you can use the same workaround:
   //
   //   const_cast<KVVarGlobTemplate*>(this)->NotAConstMethod(someArguments);
}

Double_t KVVarGlobTemplate::getvalue_int(Int_t index)
{
   // Protected method, called by GetValue(Int_t) and GetValue(const Char_t*)
   // If your variable calculates several different values, this method allows to access each value
   // based on a unique index number.
   //
   // You should implement something like the following:
   //
   //   switch(index){
   //         case 0:
   //            return val0;
   //            break;
   //         case 1:
   //            return val1;
   //            break;
   //   }
   //
   // where 'val0' and 'val1' are the internal variables of your class corresponding to the
   // required values.
   //
   // In order for GetValue(const Char_t*) to work, you need to associate each named value
   // with an index corresponding to the above 'switch' statement, e.g.
   //
   //     SetNameIndex("val0", 0);
   //     SetNameIndex("val1", 1);
   //
   // This should be done in the init_KVVarGlobTemplate() method.
}

