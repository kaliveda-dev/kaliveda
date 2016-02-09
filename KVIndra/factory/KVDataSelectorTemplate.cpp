//_____________________________________
KVDataSelectorTemplate::KVDataSelectorTemplate()
{
   //Default constructor
}

//_____________________________________
KVDataSelectorTemplate::~KVDataSelectorTemplate()
{
   //Destructor
}

//_____________________________________
void KVDataSelectorTemplate::Init(void)
{
//
// Initialisations of the KVDataSelector
// This method is called when a TEventList is build
// The pointer to the current KVSelector which is using this KVDataSelector
// can be obtained with the GetKVSelectorMethod(). This may be useful when
// a KVVarGlob is used for the dataselection. It can be added to the list
// of global variables of the KVSelector by a call like:
//
//    GetKVSelector()->AddGV("KVZtot","ztot");
//
//

// Enter your code here

}


//_____________________________________
Bool_t KVDataSelectorTemplate::SelectCurrentEntry(void)
{
//
// returns Ok is the current entry has to be added to the TEventList
//
   Bool_t ok = kFALSE;

// enter you code here
// The pointer to the current event can be obtained by:
//
//  KVINDRAReconEvent *evt=GetKVSelector()->GetEvent();
//
// When a global variable is used, its value can be obtained by:
//
//  Double_t val=GetKVSelector()->GetGV("ztot")->GetValue();
//

   return ok;
}

