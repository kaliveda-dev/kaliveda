class KVFAZIAReaderTemplate : public KVFAZIAReader {

public:
   KVFAZIAReaderTemplate() {};
   virtual ~KVFAZIAReaderTemplate() {};

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   ClassDef(KVFAZIAReaderTemplate, 0)
};
