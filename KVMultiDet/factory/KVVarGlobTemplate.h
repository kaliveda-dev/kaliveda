class KVVarGlobTemplate: public KVVarGlob {

protected:
   Double_t getvalue_int(Int_t) const;

public:
   KVVarGlobTemplate()
      : KVVarGlob()
   {
      init();
   }
   KVVarGlobTemplate(const Char_t* nom)
      : KVVarGlob(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlobTemplate, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVVarGlobTemplate)

   virtual ~KVVarGlobTemplate() {}

   void Copy(TObject& obj) const;

   void Init();
   void Reset();
   void Calculate();

   ClassDef(KVVarGlobTemplate, 1)
};
