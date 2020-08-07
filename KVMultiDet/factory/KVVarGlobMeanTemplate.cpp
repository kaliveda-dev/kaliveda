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

