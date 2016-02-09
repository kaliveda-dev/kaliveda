class KVVarGlobTemplate: public KVVarGlob {

protected:
   Double_t getvalue_void() const;
   Double_t getvalue_int(Int_t);

public:
   KVVarGlobTemplate(void);
   KVVarGlobTemplate(Char_t* nom);
   KVVarGlobTemplate(const KVVarGlobTemplate& a);

   virtual ~KVVarGlobTemplate(void);

   virtual void Copy(TObject& obj) const;

   KVVarGlobTemplate& operator = (const KVVarGlobTemplate& a);

   virtual void Init(void);
   virtual void Reset(void);

   virtual Double_t* GetValuePtr(void);
   virtual TObject* GetObject(void) const;
   ClassDef(KVVarGlobTemplate, 1)
};
