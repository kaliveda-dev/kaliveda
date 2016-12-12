class KVSelectorTemplate : public KVSelector {

   TFile* my_file;
   TH1F* mult, *zdist;

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
