//Created by KVClassFactory on Tue Jan 20 16:29:33 2015
//Author: ,,,

#include "KVFAZIADBRun.h"

ClassImp(KVFAZIADBRun)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIADBRun</h2>
<h4>run description for FAZIA experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIADBRun::KVFAZIADBRun()
{
   // Default constructor
}

//________________________________________________________________

KVFAZIADBRun::KVFAZIADBRun(Int_t number, const Char_t* title) : KVDBRun(number, title)
{
   // Write your code here
}

KVFAZIADBRun::~KVFAZIADBRun()
{
   // Destructor
}

void KVFAZIADBRun::SetTrigger(Int_t mt)
{
   SetScaler("Trigger multiplicity", mt);
}

Int_t KVFAZIADBRun::GetTrigger() const
{
   return GetScaler("Trigger multiplicity");
}

const Char_t* KVFAZIADBRun::GetTriggerString() const
{
   if (GetTrigger() > 0)
      return Form("M>=%d", GetTrigger());
   else
      return Form("xxx");
}

void KVFAZIADBRun::SetACQStatus(const KVString& status)
{
   Set("ACQ Status", status);
}

const Char_t* KVFAZIADBRun::GetACQStatus() const
{
   return GetString("ACQ Status");
}

void KVFAZIADBRun::SetGoodEvents(Int_t evt_number)
{
   SetScaler("Good Events", evt_number);
}

Int_t KVFAZIADBRun::GetGoodEvents() const
{
   return GetScaler("Good Events");
}

void KVFAZIADBRun::SetError_WrongNumberOfBlocks(Int_t evt_number)
{
   SetScaler("Bad Events with wrong number of blocks", evt_number);
}

Int_t KVFAZIADBRun::GetError_WrongNumberOfBlocks() const
{
   return GetScaler("Bad Events with wrong number of blocks");
}

void KVFAZIADBRun::SetError_InternalBlockError(Int_t evt_number)
{
   SetScaler("Bad Events internal error in one block", evt_number);
}

Int_t KVFAZIADBRun::GetError_InternalBlockError() const
{
   return GetScaler("Bad Events internal error in one block");
}

void KVFAZIADBRun::SetNumberOfAcqFiles(Int_t number)
{
   SetScaler("Number of ACQ files", number);
}

Int_t KVFAZIADBRun::GetNumberOfAcqFiles() const
{
   return GetScaler("Number of ACQ files");
}

void KVFAZIADBRun::SetDuration(Double_t duration)
{
   Set("Duration (s)", duration);
}

Double_t KVFAZIADBRun::GetDuration() const
{
   return Get("Duration (s)");
}

void KVFAZIADBRun::SetFrequency(Double_t frequency)
{
   Set("ACQ rate (evts/s)", frequency);
}

Double_t KVFAZIADBRun::GetFrequency() const
{
   return Get("ACQ rate (evts/s)");
}

void KVFAZIADBRun::SetTriggerRate(Double_t triggerrate)
{
   Set("trigger rate (evts/s)", triggerrate);
}

Double_t KVFAZIADBRun::GetTriggerRate() const
{
   return Get("trigger rate (evts/s)");
}

void KVFAZIADBRun::SetDeadTime(Double_t deadtime)
{
   Set("deadtime [0;1]", deadtime);
}

Double_t KVFAZIADBRun::GetDeadTime() const
{
   return Get("deadtime [0;1]");
}

void KVFAZIADBRun::SetNumberOfTriggerBlocks(Double_t number)
{
   Set("number of trigger blocks", number);
}

Double_t KVFAZIADBRun::GetNumberOfTriggerBlocks() const
{
   return Get("number of trigger blocks");
}

