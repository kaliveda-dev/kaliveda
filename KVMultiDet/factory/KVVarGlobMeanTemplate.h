class KVVarGlobMeanTemplate: public KVVarGlobMean {
public:
   KVVarGlobMeanTemplate(void);
   KVVarGlobMeanTemplate(Char_t* nom);
   KVVarGlobMeanTemplate(const KVVarGlobMeanTemplate& a);

   virtual ~KVVarGlobMeanTemplate(void);

   virtual void Copy(TObject& obj) const;

   KVVarGlobMeanTemplate& operator = (const KVVarGlobMeanTemplate& a);

   ClassDef(KVVarGlobMeanTemplate, 1)
};
