//Created by KVClassFactory on Wed Apr  3 14:15:55 2013
//Author: John Frankland,,,

#include "KVGEBatchJob.h"
#include "TSystem.h"

ClassImp(KVGEBatchJob)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGEBatchJob</h2>
<h4>Job handled by Grid Engine batch system</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVGEBatchJob::KVGEBatchJob()
{
   // Default constructor
   cpu_scaling_factor = 10;
// Bonjour,
//
// Le scaling factor n'est pas accessible par la commande qstat. La normalisation de la
// puissance de calcul en HS06 a �t� ajout�e � la main par les administrateurs du batch
// du centre de calcul.
//
// La ferme �tant assez uniforme, vous pouvez consid�rer que ce facteur est de 10 pour
// l'ensemble des machines. Ainsi en divisant le temps cpu en HS06 par 10 vous obtenez
// approximativement le temps cpu consomm� (ligne cpu time: 815/36000  dans fichier
// toto.o689578). Ce sont les m�mes secondes que vous demandez au qsub.
//
// Cette approche est approximative mais pour l'instant il n'y a pas mieux.
//
// Cordialement.
//
// Pascal Calvat
// --
// CC-IN2P3 User Support
}

KVGEBatchJob::~KVGEBatchJob()
{
   // Destructor
}

void KVGEBatchJob::DeleteJob()
{
   // delete this job:
   //    qdel [jobid]
   gSystem->Exec(Form("qdel %d", GetJobID()));
}

void KVGEBatchJob::AlterResources(TString r)
{
   // call qalter on job with given resource list
   gSystem->Exec(Form("qalter -l %s %d", r.Data(), GetJobID()));
}
