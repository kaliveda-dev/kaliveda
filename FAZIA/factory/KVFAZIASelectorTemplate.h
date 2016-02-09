class KVFAZIASelectorTemplate : public KVFAZIASelector {

public:
   KVFAZIASelectorTemplate() {};
   virtual ~KVFAZIASelectorTemplate() {};

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   ClassDef(KVFAZIASelectorTemplate, 0);
};
