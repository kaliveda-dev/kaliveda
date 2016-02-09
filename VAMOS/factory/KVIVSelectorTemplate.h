class KVIVSelectorTemplate : public KVIVSelector {

public:
   KVIVSelectorTemplate() {};
   virtual ~KVIVSelectorTemplate() {};

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   ClassDef(KVIVSelectorTemplate, 0);
};
