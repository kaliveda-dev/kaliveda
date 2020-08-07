////////////////////////////////////////////////////////////////////////////////////
/// Copy state of 'this' object into the KVVarGlob1Template object referenced by 'a'.
/// This is needed for the automatically generated copy constructor and copy
/// assignment operator to work correctly.

void KVVarGlob1Template::Copy(TObject& a) const
{

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

//////////////////////////////////////////////////////////////////////
/// Calculation of global variable value(s) after filling is finished

void KVVarGlob1Template::Calculate(void)
{
}

