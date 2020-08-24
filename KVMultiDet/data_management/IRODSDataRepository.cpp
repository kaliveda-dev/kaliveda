//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#include "IRODSDataRepository.h"
#include "IRODS.h"
#include "TSystem.h"
#include "Riostream.h"
using namespace std;

ClassImp(IRODSDataRepository)

IRODSDataRepository::IRODSDataRepository()
{
   // Default constructor
   fDMS = new IRODS();// set up interface to IRODS
}

IRODSDataRepository::~IRODSDataRepository()
{
   // Destructor
}

void IRODSDataRepository::PrintAvailableDatasetsUpdateWarning() const
{
   cout << "**********************************************************************************" << endl << endl;
   cout << "  The list of datasets which are available (i.e. for which data files are" << endl;
   cout << "  present on HPSS) will now be updated by consulting the IRODS catalogue." << endl;
   cout << "  This normally takes about 30 seconds to 1 minute to perform." << endl << endl;
   cout << "  However, if there is a problem with IRODS, this will block KaliVeda!" << endl;
   cout << "  Two kinds of problem exist:" << endl;
   cout << "    1). IRODS is not functioning (check http://cc.in2p3.fr and" << endl;
   cout << "        http://webmail.in2p3.fr/wnr/thread.php?group=in2p3.cc.unix)" << endl;
   cout << "       SOLUTION : wait for Centre de Calcul to fix IRODS ;)" << endl;
   cout << "    2). You have not authenticated your account for use with IRODS" << endl;
   cout << "       SOLUTION : see https://bugs.launchpad.net/kaliveda/+bug/1131725" << endl << endl;
   cout << "  We will now test if IRODS is functioning (for you)." << endl;
   cout << "  If everything is OK, at the end of this message you should see a listing like this :" << endl << endl;
   cout << "    /indra:" << endl;
   cout << "      C- /indra/campagne1" << endl;
   cout << "      C- /indra/campagne2" << endl;
   cout << "      etc. etc." << endl << endl;
   cout << "  If you DON'T see the listing then this is probably the last" << endl;
   cout << "  bit of activity you'll be seeing from KaliVeda for a while... :D" << endl << endl;
   gSystem->Exec("echo '\n' | ils");
   cout << endl;
}
