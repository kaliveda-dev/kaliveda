class KVINDRAEventSelectorTemplate : public KVINDRAEventSelector {

public:
   KVINDRAEventSelectorTemplate() {};
   virtual ~KVINDRAEventSelectorTemplate() {};

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(KVINDRAEventSelectorTemplate, 0);
};
