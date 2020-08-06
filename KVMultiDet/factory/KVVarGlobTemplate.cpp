////////////////////////////////////////////////////////////////////////////////////
/// Copy state of 'this' object into the KVVarGlobTemplate object referenced by 'a'.
/// This is needed for the automatically generated copy constructor and copy
/// assignment operator to work correctly.

void KVVarGlobTemplate::Copy(TObject& a) const
{
   KVVarGlob::Copy(a);// copy attributes of KVVarGlob base object

   KVVarGlobTemplate& aglob = (KVVarGlobTemplate&)a;
   // Now copy any additional attributes specific to this class:
   // To copy a specific field, do as follows:
   //
   //     aglob.field=field;
   //
   // If setters and getters are available, you can also proceed as follows
   //
   //    aglob.SetField(GetField());
   //
}

/////////////////////////////////////////////////////////////////////////////////
/// Initialisation of internal variables, called once before beginning treatment

void KVVarGlobTemplate::Init(void)
{
}

///////////////////////////////////////////////////////////////////
/// Reset internal variables, called before treatment of each event

void KVVarGlobTemplate::Reset(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Protected method, called by GetValue(Int_t) and GetValue(const Char_t*)
/// If your variable calculates several different values, this method allows to access each value
/// based on a unique index number.
///
/// You should implement something like the following:
///
///   switch(index){
///         case 0:
///            return val0;
///            break;
///         case 1:
///            return val1;
///            break;
///   }
///
/// where 'val0' and 'val1' are the internal variables of your class corresponding to the
/// required values.
///
/// In order for GetValue(const Char_t*) to work, you need to associate each named value
/// with an index corresponding to the above 'switch' statement, e.g.
///
///     SetNameIndex("val0", 0);
///     SetNameIndex("val1", 1);
///
/// This should be done in the init() method.

Double_t KVVarGlobTemplate::getvalue_int(Int_t index) const
{

   return 0;
}

