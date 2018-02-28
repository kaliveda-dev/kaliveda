/*****************************************************************************
 *                                                                           *
 *           Function  : acq_swap_buf                                        *
 *           Objet     : swappe les buffers acquisition en fonction de       *
 *                       l'entete                                            *
 *                                                                           *
 *           Entree    : Buff -- Buffer de lecture des donnees.              *
 *                       Size -- Taille du buffer de lecture.                *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_UNKBUF si type de buffer inconnu       *
 *                              - ACQ_UNKMSB si MSB/LSB non determine        *
 *
 *          Modifications :
 *             B. Raine le 23 oct 98
 *               Test automatique de l'architecture avec fct T_LSB
 *
 *             F. Saillant le 11 juin 1999
 *               Ajout buffers de type EBYEDAT (EXOGAM)
 *
 *             F. Saillant le 22 juillet 1999
 *               Utilisation du numero magique pour EBYEDAT
 *
 *             B. Raine le 13 decembre 2002
 *               Ajout buffers de type ENDRUN
 *                                                                           *
 ****************************************************************************/

#include <string.h>
#include <stdio.h>
#include "gan_tape_param.h"   /* Fichier de parametres modifiables.    */
#include "gan_tape_general.h"
#include "GanTape/GEN_TYPE.H"
#include "GanTape/gan_acq_buf.h"
#include "gan_acq_swap_buf.h"

#include "gan_tape_erreur.h"

/* Prototypes des routines internes au package */

static int T_LSB(void);
static int SwapNumBuf(in2p3_buffer_struct* Buff, int Size);
static int SwapEvent(in2p3_buffer_struct* Buff, int Size);
static int SwapJbus(in2p3_buffer_struct* Buff, int Size);
static int SwapScaler(in2p3_buffer_struct* Buff, int Size);
static int SwapUnknown(in2p3_buffer_struct* Buff, int Size);
static int SwapNone(in2p3_buffer_struct* Buff, int Size);
static int SwapEbyedat(in2p3_buffer_struct* Buff, int Size);
static int SwapEventdbToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapEventdbSwapToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapEventctToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapEventctSwapToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapJbusToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapJbusSwapToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapScalerToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapScalerSwapToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapStatusSwapToHost(in2p3_buffer_struct* Buff, int Size);
static int SwapEbyedatToHost(in2p3_buffer_struct* Buff, int Size);

/* Definition des routines de traitement en fonction du type de buffer */

typedef struct {
   char Header[9];
   int (*SwapFct)(in2p3_buffer_struct*, int);
} HdBuffer;

static HdBuffer HdBufList[] = {
   FILEH_Id,    SwapNumBuf,
   EVENTH_Id,   SwapNumBuf,
   PARAM_Id,    SwapNumBuf,
   COMMENT_Id,  SwapNumBuf,
   EVENTDB_Id,  SwapEvent,
   EVENTCT_Id,  SwapEvent,
   JBUS_Id,     SwapJbus,
   SCALER_Id,   SwapScaler,
   EBYEDAT_Id,  SwapEbyedat,
   ENDRUN_Id,   SwapNumBuf,
   "",          SwapUnknown
};


#define MSB 1
#define LSB 2

#ifdef Machine_LSB
#undef Machine_LSB
#endif

#define Machine_LSB T_LSB()

static int OrdreOctets = 0;   /* Non defini a priori */
static int OnSwap;            /* Booleen mis a jour au debut de acq_swap_buf
                                 en fonction du contexte */
typedef struct mot32 {
   unsigned char Byte1;
   unsigned char Byte2;
   unsigned char Byte3;
   unsigned char Byte4;
} SW_MOT32;

typedef struct mot16 {
   unsigned char Byte1;
   unsigned char Byte2;
} SW_MOT16;

typedef struct doubleword {
   unsigned short Word1;
   unsigned short Word2;
} SW_DOUBLEWORD;

/* Retourne TRUE si architecture LSB */
static int T_LSB(void)
{
   int LsbFlag;
   union {
      short int s;
      long int l;
   } b;

   b.l = 0;
   b.s = 1;
   if (b.l == b.s)
      LsbFlag = TRUE; /* LSB car short = long */
   else
      LsbFlag = FALSE; /* MSB car byte inverses */

   return (LsbFlag);
}

void SwapDoubleWord(UNSINT32* Buf, int NbOctets)
{
   SW_DOUBLEWORD Temp, *DW;
   int i, N;

   N = NbOctets / 4;

   for (i = 0, DW = (SW_DOUBLEWORD*)Buf; i < N; i++, DW++) {
      Temp.Word1 = DW->Word2;
      Temp.Word2 = DW->Word1;
      *DW = Temp;
   }
}

void SwapInt32(UNSINT32* Buf, int NbOctets)
{
   SW_MOT32 Temp, *Mot32;
   int i, NbMots;

   NbMots = NbOctets / 4;

   for (i = 0, Mot32 = (SW_MOT32*)Buf; i < NbMots; i++, Mot32++) {
      Temp.Byte1 = Mot32->Byte4;
      Temp.Byte2 = Mot32->Byte3;
      Temp.Byte3 = Mot32->Byte2;
      Temp.Byte4 = Mot32->Byte1;
      *Mot32 = Temp;
   }
}

void SwapInt16(UNSINT16* Buf, int NbOctets)
{
   SW_MOT16 Temp, *Mot16;
   int i, NbMots;

   NbMots = NbOctets / 2;

   for (i = 0, Mot16 = (SW_MOT16*)Buf; i < NbMots; i++, Mot16++) {
      Temp.Byte1 = Mot16->Byte2;
      Temp.Byte2 = Mot16->Byte1;
      *Mot16 = Temp;
   }
}

/* Swap du numero de buffer */
static int SwapNumBuf(in2p3_buffer_struct* Buff, int Size)
{
   if (OnSwap)
      SwapInt32(&Buff->les_donnees.Count, 4);

   return (ACQ_OK);
}

static int SwapEvent(in2p3_buffer_struct* Buff, int Size)
{
   SwapNumBuf(Buff, Size);
   SwapInt16(Buff->les_donnees.cas.Buff, Size - IN2P3_HEADER_LEN);

   return (ACQ_OK);
}

static int SwapJbus(in2p3_buffer_struct* Buff, int Size)
{
   SwapNumBuf(Buff, Size);
   SwapInt32(&Buff->les_donnees.cas.scale.Length, SCALE_HEADER_LEN);
   SwapInt16(Buff->les_donnees.cas.scale.jbus_scale.Info_jbus,
             Size - IN2P3_HEADER_LEN - SCALE_HEADER_LEN);

   return (ACQ_OK);
}

static int SwapScaler(in2p3_buffer_struct* Buff, int Size)
{
   SwapNumBuf(Buff, Size);
   SwapInt32(&Buff->les_donnees.cas.scale.Length,
             Size - IN2P3_HEADER_LEN);

   return (ACQ_OK);
}

/* Type de buffer inconnu */
static int SwapUnknown(in2p3_buffer_struct* Buff, int Size)
{
   char Header[9];
   Header[8] = '\0';
   strncpy(Header, Buff->les_donnees.Ident, 8);
   printf(" Unknow Header : <%s>\n", Header);
   return (ACQ_UNKBUF);
}

/* Recupere l'information MSB/LSB dans bloc FILEH et positionne OnSwap */
static int SwTraiteFileh(in2p3_buffer_struct* Buff)
{
   int err = ACQ_OK;

   extern bool AutoswBuf, Swbufon; /* Valeurs extraites en ligne de commande */
   /* les valeurs sont issues de exemple_relecture.    */

   if (strncmp(Buff->les_donnees.cas.Buf_fileh.OrdreOctets, " MSB", 4) == 0)
      OrdreOctets = MSB;
   else if (strncmp(Buff->les_donnees.cas.Buf_fileh.OrdreOctets, " LSB", 4) == 0)
      OrdreOctets = LSB;
   else
      err = ACQ_UNKMSB;

   if ((err == ACQ_OK) && AutoswBuf) {
      if ((OrdreOctets == LSB) && Machine_LSB)
         OnSwap = FALSE;
      else
         OnSwap = TRUE;
   }

   return (err);
}

/* Swappe les buffers d'acquisition GANIL en fonction de leur type et des
   macros d'options definies (en principe dans gan_acq_param.h) */
int acq_swap_buf(in2p3_buffer_struct* Buff, int Size)
{

   extern bool AutoswBuf, Swbufon; /* Valeurs extraites en ligne de commande    */

   char Header[9];
   int i, err = ACQ_OK;
   HdBuffer* HdBuf;

   if (AutoswBuf || Swbufon) {
      strncpy(Header, Buff->les_donnees.Ident, 8);
      Header[8] = '\0';     /* Pour delimiter la 'string' */

      if (AutoswBuf) {
         if (strcmp(Header, FILEH_Id) == 0)
            err = SwTraiteFileh(Buff);
         else if (OrdreOctets == 0)
            err = ACQ_UNKMSB;
      } else if (Swbufon)
         OnSwap = TRUE;
      else
         OnSwap = FALSE;

      if ((err == ACQ_OK) && OnSwap) {

         for (HdBuf = HdBufList;
               (HdBuf->Header[0] != '\0') &&
               (strcmp(Header + 1, HdBuf->Header + 1) != 0);
               HdBuf++);

         err = HdBuf->SwapFct(Buff, Size);
      }
   }

   return (err);
}

static int SwapNone(in2p3_buffer_struct* Buff, int Size)
{
   return ACQ_OK;
}

/***************************************************************************************/
/* Transforme un buffer pour qu'il soit correct pour la machine sur laquelle on tourne */
/* uniquement si le buffer vient du reseau                                             */
/***************************************************************************************/

int SwapBufToHost(char* Buff, int Size)
{
   static HdBuffer HdList[] = {
      { EBYEDAT_Id,     SwapEbyedatToHost     },
      { JBUS_Id,        SwapJbusToHost        },
      { JBUS_SWAP_Id,   SwapJbusSwapToHost    },
      { SCALER_Id,      SwapScalerToHost      },
      { SCALER_SWAP_Id, SwapScalerSwapToHost  },
      { STATUS_SWAP_Id, SwapStatusSwapToHost  },
      { EVENTDB_Id,     SwapEventdbToHost     },
      { EVENTDB_SWAP_Id, SwapEventdbSwapToHost },
      { EVENTCT_Id,     SwapEventctToHost     },
      { EVENTCT_SWAP_Id, SwapEventctSwapToHost },
      { "",             SwapNone              }
   };

   char Header[9];
   HdBuffer* Hd;
   int err;

   strncpy(Header, Buff, 8);
   Header[8] = '\0';     /* Pour delimiter la 'string' */

   for (Hd = HdList; (Hd->Header[0] != '\0') && (strcmp(Header + 1, Hd->Header + 1)); Hd++) ;

   err = Hd->SwapFct((in2p3_buffer_struct*)Buff, Size);

   return (err);
}

static int SwapEventdbToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      /* inversion des 4 octets du numero de buffer */
      SwapInt32(&Buff->les_donnees.Count, 4);

      SwapInt16(Buff->les_donnees.cas.Buff, Size - IN2P3_HEADER_LEN);
   }

   return (ACQ_OK);
}

static int SwapEventdbSwapToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      SwapInt16((UNSINT16*)Buff, Size);
   } else { /* machine big-endian */
      /* swappe Byte/Byte l'ident ASCII */
      SwapInt16((UNSINT16*)Buff, 8);

      /* swappe Word/Word le numero de buffer */
      SwapDoubleWord(&Buff->les_donnees.Count, 4);
   }

   return (ACQ_OK);
}

static int SwapEventctToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      /* inversion des 4 octets du numero de buffer */
      SwapInt32(&Buff->les_donnees.Count, 4);

      SwapInt16(Buff->les_donnees.cas.Buff, Size - IN2P3_HEADER_LEN);
   }

   return (ACQ_OK);
}

static int SwapEventctSwapToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      SwapInt16((UNSINT16*)Buff, Size);
   } else { /* machine big-endian */
      /* swappe Byte/Byte l'ident ASCII */
      SwapInt16((UNSINT16*)Buff, 8);

      /* swappe Word/Word le numero de buffer */
      SwapDoubleWord(&Buff->les_donnees.Count, 4);
   }

   return (ACQ_OK);
}

static int SwapJbusToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      /* inversion des 4 octets du numero de buffer et des 5 mots de 32 bits suivants */
      SwapInt32(&Buff->les_donnees.Count, 24);

      SwapInt16(Buff->les_donnees.cas.scale.jbus_scale.Info_jbus,
                Size - IN2P3_HEADER_LEN - SCALE_HEADER_LEN);
   }

   return (ACQ_OK);
}

static int SwapJbusSwapToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      SwapInt16((UNSINT16*)Buff, Size);
   } else { /* machine big-endian */
      /* swappe Byte/Byte l'ident ASCII */
      SwapInt16((UNSINT16*)Buff, 8);

      /* swappe Word/Word le numero de buffer */
      SwapDoubleWord(&Buff->les_donnees.Count, 4);

      /* swappe Word/Word l'entete de type SCALE */
      SwapDoubleWord(&Buff->les_donnees.cas.scale.Length, SCALE_HEADER_LEN);
   }

   return (ACQ_OK);
}

static int SwapScalerToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      /* inverse les 4 octets du numero de buffer et ce qui suit sauf le checksum */
      SwapInt32(&Buff->les_donnees.Count, (Size - IN2P3_HEADER_LEN - 26));

      /* swappe Byte/Byte du checksum et du nombre d'evts */
      SwapInt16(((UNSINT16*)Buff + ((Size - 30) / 2)), 6);
   }

   return (ACQ_OK);
}

static int SwapScalerSwapToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      SwapInt16((UNSINT16*)Buff, Size);
   } else { /* machine big-endian */
      /* swappe Byte/Byte l'ident ASCII */
      SwapInt16((UNSINT16*)Buff, 8);

      /* swappe Word/Word le numero de buffer et ce qui suit sauf le checksum */
      SwapDoubleWord(&Buff->les_donnees.Count, (Size - IN2P3_HEADER_LEN - 26));
   }

   return (ACQ_OK);
}

static int SwapStatusSwapToHost(in2p3_buffer_struct* Buff, int Size)
{
   if (Machine_LSB) { /* machine little-endian */
      SwapInt16((UNSINT16*)Buff, Size);
   } else { /* machine big-endian */
      /* swappe Byte/Byte l'ident ASCII */
      SwapInt16((UNSINT16*)Buff, 8);

      /* swappe Word/Word le numero de buffer */
      SwapDoubleWord(&Buff->les_donnees.Count, 4);

      /* swappe Word/Word le champ "longueur utile du buffer" */
      SwapDoubleWord(&Buff->les_donnees.cas.status.Length, 4);

      /* swappe Byte/Byte le nom du frontal VME */
      SwapInt16((UNSINT16*)Buff->les_donnees.cas.status.VmeName, 16);
   }

   return (ACQ_OK);
}

static int SwapEbyedat(in2p3_buffer_struct* Buff, int Size)
{
   GANIL_BUF_HD* bufhd;                 /* Header du buffer */

   bufhd = (GANIL_BUF_HD*)Buff;

   /* swappe le numero de buffer */
   SwapInt32((UNSINT32*)&bufhd->num, 8);

   /* swappe le numero de flux et le nombre d'evts */
   SwapInt16((UNSINT16*)&bufhd->stream, 4);

   /* swappe le checksum 32 bits et la longueur utile du buffer */
   SwapInt32((UNSINT32*)&bufhd->checksum, 8);

   /* Swappe le reste du buffer */
   SwapInt16((UNSINT16*)((char*)Buff + GANIL_BUF_HD_SIZE), Size - GANIL_BUF_HD_SIZE);

   return (ACQ_OK);
}

static int SwapEbyedatToHost(in2p3_buffer_struct* Buff, int Size)
{
   int err;
   GANIL_BUF_HD* bufhd;  /* Header du buffer */

   bufhd = (GANIL_BUF_HD*)Buff;

   if (bufhd->magic != MAGIC_NUMBER) {
      err = SwapEbyedat(Buff, Size);
   }

   return (ACQ_OK);
}
