class KVDataSelectorTemplate: public KVDataSelector {
public:

   KVDataSelectorTemplate();
   virtual ~KVDataSelectorTemplate();

   virtual void Init(void);
   virtual Bool_t SelectCurrentEntry(void);

   ClassDef(KVDataSelectorTemplate, 1) //KVDataSelector
};
