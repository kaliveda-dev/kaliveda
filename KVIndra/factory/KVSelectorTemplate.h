class KVSelectorTemplate : public KVSelector {

public:
   KVSelectorTemplate() {};
   virtual ~KVSelectorTemplate() {};

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   ClassDef(KVSelectorTemplate, 0);
};
