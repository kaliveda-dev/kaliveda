#ifndef _LOGFILE_CLASS
#define _LOGFILE_CLASS

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

#include "Defines.h"
#include "Deprecation.h"
#include "Riostream.h"
#include "Rtypes.h"
#include <cstdlib>
#include <sstream>
#include <string>

using namespace std;

class LogFile {

public:

   LogFile();
   virtual ~LogFile(void);


   Char_t LogFileName;
   std::ofstream Log;
   void Open(const char* LogFileName);
   void Close();

   void Message(const char* location, const char* msg);
   string GetTime();

   ClassDef(LogFile, 0)
};

#endif // _LOGFILE_CLASS not defined

#ifdef _LOGFILE_CLASS
DEPRECATED_CLASS(LogFile);
#endif
