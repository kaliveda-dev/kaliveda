class KVVarGlob1Template: public KVVarGlob1 {
public:

   KVVarGlob1Template(void);
   KVVarGlob1Template(Char_t* nom);
   KVVarGlob1Template(const KVVarGlob1Template& a);

   virtual ~KVVarGlob1Template(void);

   virtual void Copy(TObject& obj) const;

   KVVarGlob1Template& operator = (const KVVarGlob1Template& a);

   ClassDef(KVVarGlob1Template, 1)
};
