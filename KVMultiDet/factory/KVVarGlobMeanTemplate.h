class KVVarGlobMeanTemplate: public KVVarGlobMean {

public:
   KVVarGlobMeanTemplate()
      : KVVarGlobMean()
   {
      init();
   }
   KVVarGlobMeanTemplate(const Char_t* nom)
      : KVVarGlobMean(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlobMeanTemplate, KVVarGlobMean)
   ROOT_COPY_ASSIGN_OP(KVVarGlobMeanTemplate)

   virtual ~KVVarGlobMeanTemplate(void) {}

   virtual void Copy(TObject& obj) const;

   ClassDef(KVVarGlobMeanTemplate, 1)
};
