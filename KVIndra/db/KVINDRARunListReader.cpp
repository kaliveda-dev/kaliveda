/*
$Id: KVINDRARunListReader.cpp,v 1.8 2007/04/26 16:38:57 franklan Exp $
*/

#include <KVINDRARunListReader.h>
#include <TClass.h>
#include <TError.h>
#include <KVRunListLine.h>


using namespace std;

ClassImp(KVINDRARunListReader)

KVINDRARunListReader::KVINDRARunListReader()
{
   //Default ctor.
   //We set default values for recognising comment lines ('#') and the separator character ('|')

   fLineReader = new KVRunListLine;
   fComment = '#';
   fSeparator = '|';
   fTrigger = -1;
   fNewRunList = kFALSE;
   fVersion = 0;
}

KVINDRARunListReader::~KVINDRARunListReader()
{
   //Dtor.
   //Delete KVRunListLine object if one exists; close runlist file if open
   CloseRLFile();
   if (fLineReader) {
      delete fLineReader;
      fLineReader = 0;
   }
}

Bool_t KVINDRARunListReader::OpenRLFile(const Char_t* filename)
{
   //Open file 'filename' for reading.
   //Returns kTRUE if file can be opened (info message), kFALSE if not (error message)
   //Reset fTrigger to -1 (in case we already read a different file)

   fRunsStream.open(filename);
   if (fRunsStream.is_open()) {
      fRunsFile = filename;
      Info(Form("%s::OpenRLFile", IsA()->GetName()),
           "Opened file %s for reading", filename);
      fTrigger = -1;
      return kTRUE;
   }
   Error(Form("%s::OpenRLFile", IsA()->GetName()),
         "Cannot open file %s for reading", filename);
   //reset ifstream ready to try opening another file
   fRunsStream.close();
   fRunsStream.clear();
   return kFALSE;
}

void KVINDRARunListReader::CloseRLFile()
{
   //Close current runlist file. Another file can be opened after this.
   //If no file is open, do nothing.
   if (fRunsStream.is_open()) {
      fRunsStream.close();
      fRunsStream.clear();
      fRunsFile = "";
   }
}

Bool_t KVINDRARunListReader::ReadRLLine()
{
   //Read a line from current runlist file. If not a comment line (i.e. does not begin with 'fComment' character)
   //then analyse it with fLineReader object.
   //Returns kFALSE if no file is open or if file is not good for i/o
   //Returns kFALSE if fLineReader has not been initialised
   //Returns kFALSE if line cannot be read / end of file is reached
   if (!fRunsStream.is_open()) {
      Error(Form("%s::ReadRLLine", IsA()->GetName()),
            "No file open for reading");
      return kFALSE;
   }
   if (!fRunsStream.good()) {
      Error(Form("%s::ReadRLLine", IsA()->GetName()),
            "File %s is not good for reading", fRunsFile.Data());
      return kFALSE;
   }
   fLine.ReadLine(fRunsStream);

   if (!fRunsStream.good())
      return kFALSE;

   if (fLine.BeginsWith("Version")) {
      //read version number in file
      if (sscanf(fLine.Data(), "Version=%d", &fVersion) == 1) {
         //set flag to force reading runlist with new method
         fNewRunList = kTRUE;
         return kFALSE;
      }
   }
   else if (!fLine.BeginsWith(fComment)) {
      fLineReader->SetLine(fLine);
      fLineReader->BreakLineIntoFields(fSeparator);
   }
   else {
      //comment line - reset line reader
      fLineReader->Clear();
   }
   return kTRUE;
}

void KVINDRARunListReader::ReadRunList(const Char_t* t)
{
   //This method will open the runlist file 't' and read its entire contents, calling user-overridable method "GoodRunLine(KVRunListLine*)"
   //every time a line defining a new run is read from the file.
   //If 't' is not given, we assume that the name of the run list file was passed to the constructor, and so the file should
   //already be open in this case.
   //Before calling this method, you should call SetRunListLineType() to define the class used to read each line, and
   //SetRLCommentChar()/SetRLSeparatorChar() if you want to override the default comment/separator characters.

   if (!strcmp(t, "")) {
      //no filename given - file should have been opened by OpenRLFile - check !
      if (!fRunsStream.is_open()) {
         Error(Form("%s::ReadRunList", IsA()->GetName()),
               "No runlist file open for reading");
         return;
      }
   }
   else {
      //open new file - check OK
      //close any previously opened file
      CloseRLFile();
      if (!OpenRLFile(t))
         return;                //error message will be given by OpenRLFile
   }
   //reset last trigger read
   fTrigger = -1;
   fCurrentLine = 1;
   while (ReadRLLine()) {       // read lines in file while we can

      if (fLineReader->GoodRunLine()) {
         //call user's function
         GoodRunLine();
      }
      //reset line reader
      fLineReader->Clear();
      fCurrentLine++;
   }

   //close run file
   CloseRLFile();
}

void KVINDRARunListReader::GoodRunLine()
{
   //This method should be overridden in derived class by the user.
   //It is called every time that the fLineReader finds a line which is a "good run line" (see KVRunListLine::GoodRunLine()).
   //The default behaviour is just to print the breakdown of the line in question on the screen.

   fLineReader->Print();
}

Int_t KVINDRARunListReader::GetRunListTrigger(const Char_t* field,
      const Char_t* fmt)
{
   //Get trigger multiplicity for current line.
   //Give field name which contains trigger information and format string 'fmt' containing e.g. "M>=%d"
   //if trigger field info is written in form "M>=4" etc. etc.
   //The last value read is kept (fTrigger), so that, if no trigger value is given for the current line,
   //but one was given on a previous line, we assume that the previous value is valid until a new one is given.
   //A value "-1" means no trigger has been set for any line in the file so far.

   Int_t trig = fLineReader->GetTrigger(field, fmt);
   if (trig < 0 && fLineReader->HasFieldValue(field))
      cout << "...apparently on line " << GetRLLineNumber() << endl;
   return (trig < 0 ? fTrigger : fTrigger = trig);
}
