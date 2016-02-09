/*   BUF_EVT:H -- last update :    creation date : 30 sep 94
Copyright Acquisition group, GANIL Caen, France
author  : B. Raine

  contents : description d'un buffer d'acquisition

*/

#ifdef __cplusplus
extern "C" {
#endif

/* Description des buffers d'acquisition */

typedef struct Acq_Hd_Buf {
   char AcqHdBuf[8];      /* header du buffer */
   UNSINT32 AcqNumBuf;    /* Numero du buffer */
} ACQHDBUF;

typedef struct End_Evt_Buf {
   UNSINT16 LastEvent[2];   /* deux int16 reserves pour marquer le dernier
                               evenement ( longuer et numero = 0 ) */
   UNSINT32 Checksum;       /* Checksum du buffer, compteur d'evt compris */
   UNSINT16 NbEvt;          /* Nombre d'evenements contenus dans le buffer
                               (dans le cas d'un buffer evt) */
   char DRB32Stat[24];      /* Reserve pour le status du protocol DRB32 */
} ACQENDBUF;


#define HDBUFLEN        sizeof(ACQHDBUF) /* longueur entete buffer en bytes */
#define FINBUFLEN       sizeof(ACQENDBUF) /* longueur fin buffer en bytes */

/* Definition des entetes de buffers en fonction de la machine */
/* A REVOIR dans l'avenir car c'est peu-etre la routine de swap qui
   doit etre specialisee */

#ifndef vms
/* Definition des differents entetes de buffer */
#define EVENTDB         "E EVTNBD"      /* bloc evenements bruts */
#define EVENTDF         "E EVTNFD"      /* bloc evenements filtres */
#define EVENTCT         "E EVTNTC"      /* bloc evenements control */
#define JBUS_BUFF       "JB    US"      /* bloc JBUS */
#define SCAL_BUFF       "ACS  REL"      /* bloc SCALER */
#endif

#ifdef vms
/* Definition des differents entetes de buffer */
#define EVENTDB         " EVENTDB"      /* bloc evenements bruts */
#define EVENTDF         " EVENTDF"      /* bloc evenements filtres */
#define EVENTCT         " EVENTCT"      /* bloc evenements control */
#define JBUS_BUFF       "  JBUS  "      /* bloc JBUS */
#define SCAL_BUFF       " SCALER "      /* bloc SCALER */
#endif
#ifdef __cplusplus
}
#endif
