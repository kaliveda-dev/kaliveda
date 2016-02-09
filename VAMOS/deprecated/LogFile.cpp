#include "LogFile.h"

//Author: Maurycy Rejmund

/**
   WARNING: This class has been deprecated and will eventually be removed. Do
   not use!

   This class is only compiled if the deprecated code is enabled in the build
   configuration (e.g. cmake -DUSE_DEPRECATED_VAMOS=yes). If you enable the
   deprecated code then a large number of warnings will be printed to the
   terminal. To disable these warnings (not advised) compile VAMOS with
   -Wno-deprecated-declarations. Despite these warnings the code should compile
   just fine. The warnings are there to prevent the unwitting use of the
   deprecated code (which should be strongly discouraged).

   BY DEFAULT THIS CLASS IS NOT COMPILED.

   Deprecated by: Peter Wigg (peter.wigg.314159@gmail.com)
   Date:          Thu 17 Dec 17:24:38 GMT 2015
*/

using namespace std;

ClassImp(LogFile)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Logfile</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

LogFile::LogFile(void)
{
#ifdef DEBUG
   cout << "LogFile::Constructor" << endl;
#endif
}
LogFile::~LogFile(void)
{
#ifdef DEBUG
   cout << "LogFile::Destructor" << endl;
#endif
   Close();
}

void LogFile::Open(const char* LogFileName)
{
#ifdef DEBUG
   cout << "LogFile::Open" << endl;
#endif

   Log.open(LogFileName);
   if (!Log) {
      cout << "Could not open the LogFile: " << LogFileName << " !" << endl;
      exit(EXIT_FAILURE);
   } else {
      cout << "LogFile: " << LogFileName << " opened." << endl;
   }

   Log.setf(ios::showpoint);

}
void LogFile::Close(void)
{
#ifdef DEBUG
   cout << "LogFile::Close" << endl;
#endif

   Log.close();

}

void LogFile::Message(const char* location, const char* msg)
{

   if (Log.good() == 1) {
      Log << "[" << this->GetTime() << "][" << location << "] " << msg << endl;
   } else {
      cerr << "Stream not good for I/O operations" << endl;
   }
}

string LogFile::GetTime()
{

   string dateString;

   struct tm* result = new struct tm;
   time_t* t = new time_t;
   time(t);

   localtime_r((const time_t*)t, result);

   stringstream dateStringStream;
   dateStringStream
         << setfill('0') << setw(2) << result->tm_hour << ":"
         << setfill('0') << setw(2) << result->tm_min << ":"
         << setfill('0') << setw(2) << result->tm_sec;

   dateString = dateStringStream.str();

   delete t;
   delete result;

   return dateString;
}
