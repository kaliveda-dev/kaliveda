class KVVarGlob1Template: public KVVarGlob1 {

public:
   KVVarGlob1Template()
      : KVVarGlob1()
   {
      init();
   }
   KVVarGlob1Template(const Char_t* nom)
      : KVVarGlob1(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlob1Template, KVVarGlob1)
   ROOT_COPY_ASSIGN_OP(KVVarGlob1Template)

   virtual ~KVVarGlob1Template(void) {}

   virtual void Copy(TObject& obj) const;

   void Calculate();

   ClassDef(KVVarGlob1Template, 1)
};
