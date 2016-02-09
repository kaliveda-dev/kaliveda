/* acq_ebyedat_get_next_event.c -- creation : 9 jun 99 last update : 11 mar 03
   Copyright Acquisition group, GANIL Caen, France
   module name : acq_ganil_get_next_event.c version   : 0
   library     :

   author      : B. Raine

   function : get next event in a EBYEDAT buffer

   synopsis : acq_ebyedat_get_next_event(char * Buffer,
                                         short int ** EvtAddr,
                     int  * EvtNum,
                     int    EvtFormat);

   input :
      char *Buffer
          pointer on the buffer containing events

      int ** EvtAddr
          Address of a pointer to return event address

      int EvtNum
          -1 if first event of the buffer is asked

      int EvtFormat
          EVCT_FIX  if ask for a GANIL fix format control event
                     ==> No implemented in this version
          EVCT_VAR  if ask for a GANIL variable format control event
          -1        if ask only for a pointer on the event without
                    any transformation

   output   :
      int ** EvtAddr
         points on the current event; 0 if no more event in the buffer

      int * EVtNum
         returns current event number; -1 if no more event in the buffer

      return ACQ_OK if good event returned
        ACQ_ENDOFBUFFER  if no more event in the buffer
                  ACQ_BADEVENTFORM if unknown event format
        ACQ_ERRDATABUF   if error in data format
        ACQ_INVARG       if bad argument:
                            EvtFormat
                  EvtNum != -1 and new buffer

        ACQ_INVDATABUF   if no EBYEDAT buffer
   example :

   modifications :

     B. Raine le 7 juillet 99
         - verification de coherence entre longueur donnee dans l'entete
           et dernier token du buffer
         - verification sub-event token exogam
     B. Raine fev 2003
         - mise au point pour plusieurs sub-events
     B. Raine le 11 mars 03
         - add acquiris numeriseur
     B. Raine le 27 mai 2003
         - +2 on LastToken to accept TIARA events
         - print correct LastToken in case of error
     B. Raine le 14 octobre 2005
         - add TPC chamber from CENBG
     B. Raine le 17 oct 2005
         - add error details for BAD_EVTFORM

*/
#include <stdio.h>
#include <string.h>

#include "GEN_TYPE.H"   /* GANIL general typedef */
#include "STR_EVT.H"
#include "gan_acq_buf.h"
#include "acq_ebyedat_get_next_event.h"
#include "gan_tape_erreur.h"

/* local prototypes */
static void dump(char* p, int nb);

/* Provisoirement, on definit les event tokens reconnus pour aller vite */
#define ExogamEventToken 0xff60
#define EndDataBlockToken 0xff00

#define TPC_NbChannels 384
#define TPC_Separateur 0xefff

#define EventMaxLen 0x4000

int acq_ebyedat_get_next_event(UNSINT16* Buffer,
                               UNSINT16** EvtAddr,
                               int*   EvtNum,
                               int    EvtFormat)
{
   static UNSINT16* NextEvent = 0;  /* Pointer on start of next event */

   return (acq_ebyedat_get_next_event_r(Buffer, EvtAddr, EvtNum, EvtFormat, &NextEvent));
}

/* interface pour le fortran */
int acq_ebyedat_get_next_event_(UNSINT16* Buffer,
                                UNSINT16** EvtAddr,
                                int*   EvtNum,
                                int    EvtFormat)
{
   return (acq_ebyedat_get_next_event(Buffer, EvtAddr, EvtNum, EvtFormat));
}


int acq_ebyedat_get_next_event_r(UNSINT16* Buffer,
                                 UNSINT16** EvtAddr,
                                 int*   EvtNum,
                                 int    EvtFormat,
                                 UNSINT16** NextEvent)
{
   static UNSINT16 CtrlEventBuf[EventMaxLen];
   static CTRL_EVENT* CtrlEvent = (CTRL_EVENT*)CtrlEventBuf;

   int err = ACQ_OK;

   GANIL_BUF_HD*        BufHd;
   EBYEDAT_EVENT_HD*    EventHd;
   EBYEDAT_SUBEVENT_HD* SubEventHd;
   EBYEDAT_SUBEVENT_HD_20* SubEventHd_20;
   UNSINT16*            CtrlEventData;  /* Current pointer in CtrlEvent */
   UNSINT16*            EventData;      /* Current pointer in Event */
   UNSINT16*            NextSubEvent;

   UNSINT16*        BufEnd;     /* Pointer on end of valid data in buffer */
   UNSINT16*        LastToken;   /* Theorical Pointer on End of block token */
   UNSINT16*        CurEvent = 0;   /* Pointer on start of current event */
   char* p;

   if (*EvtNum == -1) {
      /* New buffer, verify its type */
      /*
      if ( strncmp((char *)Buffer, EBYEDAT_Id, 8) != 0 )
      */
      if (strncmp((char*)Buffer + 1, "EBYEDAT", 7) != 0)
         err = ACQ_INVDATABUF;
      else
         *NextEvent = Buffer + GANIL_BUF_HD_SIZE / 2;
   }

   if (err == ACQ_OK) {
      /* Get current event pointer */
      if (*NextEvent == 0) {
         err = ACQ_INVARG; /* New buffer and EvtNum != -1 */
         printf("acq_ebyedat_get_next_event: New buffer and EvtNum != -1 \n");
      } else {
         /* verify that nextevent doesn't point out of the buffer */
         BufHd = (GANIL_BUF_HD*)Buffer;
         BufEnd = Buffer + GANIL_BUF_HD_SIZE / 2 + BufHd->length;
         //    LastToken = BufEnd - 2; /* Theorical Pointer on End of block token */
         LastToken = BufEnd; /* Theorical Pointer on End of block token
             B. Raine 27 may 03
                  Not -2 in case of TIARA, because length does
             not include End Data Block Token
               */
         if (*NextEvent > LastToken) {
            err = ACQ_ERRDATABUF; /* Bad end of buffer */
            printf("acq_ebyedat_get_next_event : NextEvent points after LastToken:\n   Buffer = 0x%lx, NextEvent = 0x%lx, LastToken = 0x%lx\n",
                   Buffer, *NextEvent, LastToken);
         } else {
            CurEvent = *NextEvent;
            CtrlEventData = (UNSINT16*)&CtrlEvent->ct_par;
            EventHd = (EBYEDAT_EVENT_HD*)CurEvent;
         }
      }
   }

   if (err == ACQ_OK) {
      /* Get current event */

      switch (EventHd->StartToken) {
         /* Test Start Event Token */

         case ExogamEventToken :
            if (EventHd->length == 0) {
               /* No more events in buffer */
               err = ACQ_ENDOFBUFFER;
               /* Test commente tant que
               actual length of data in the block incorrect
               if ( LastToken != CurEvent )
               {
                 err = ACQ_ERRDATABUF;
                 printf("acq_ebyedat_get_next_event : Bad end of buffer:\n   Buffer = 0x%lx, LastToken = 0x%lx, CurEvent = 0x%lx\n",
                 Buffer, LastToken, CurEvent );
               }
                   */
            } else {
               *NextEvent = CurEvent + EventHd->length;
               if (*NextEvent > LastToken) {
                  err = ACQ_ERRDATABUF; /* Bad end of buffer */
                  printf("acq_ebyedat_get_next_event :\n");
                  printf("   Event length too long regarding the actual length of data in the block\n");
                  printf("    => NextEvent points after LastToken:\n");
                  printf("       Buffer = 0x%lx, NextEvent = 0x%lx, LastToken = 0x%lx\n",
                         Buffer, *NextEvent, LastToken);

                  printf("\n  -> Current event dump");
                  dump((char*)CurEvent, (EventHd->length * 2) + 20);
               }
            }

            if (err == ACQ_OK) {
               *EvtNum = EventHd->num_lower;

               /* dummy variable used in case EVCT_VAR */
               char* tata;

               switch (EvtFormat) {
                  case -1: /* Return pointer on current event in source buffer */
                     *EvtAddr = CurEvent;
                     break;

                  case EVCT_VAR :
                     CtrlEvent->ct_nb =  EventHd->num_lower & 0x7fff;

                     // B. Raine 20 fev 03
                     /*(char *)SubEventHd = (char *)EventHd + EBYEDAT_EVENT_HD_SIZE;*/

                     p = (char*)EventHd + EBYEDAT_EVENT_HD_SIZE;

                     // (char *)NextSubEvent = (char *)EventHd + EBYEDAT_EVENT_HD_SIZE;

                     NextSubEvent = (UNSINT16*)p;

                     do { /* scan the sub events */
                        SubEventHd = (EBYEDAT_SUBEVENT_HD*)NextSubEvent;
                        NextSubEvent = SubEventHd->length + (UNSINT16*)SubEventHd;
                        if (NextSubEvent > *NextEvent) {
                           err = ACQ_ERRDATABUF;
                           printf("acq_ebyedat_get_next_event : NextSubEvent (0X%x) != *NextEvent (0X%x)\n",
                                  NextSubEvent, *NextEvent);
                        } else {
                           /* Copy subevent */
                           int SubEvtFormat;
                           SubEvtFormat = SubEventHd->StartToken & 0Xf;

                           /* Sequence de traitement inutile car on ne sait pas quoi faire
                              de ces informations
                              NbSubEvtNumWords = (SubEventHd->StartToken >> 4) & 0x3;
                              NbStatusWords = (SubEventHd->StartToken >> 6) & 0x3;
                              NbClockWords = (SubEventHd->StartToken >> 8) & 0x3;
                           */

                           switch (SubEvtFormat) {
                              case 0x0001: /* ExoGam 32 bit labelled data item */
                                 p = (char*)SubEventHd + EBYEDAT_SUBEVENT_HD_SIZE;

                                 //  (char *)EventData = (char *)SubEventHd + EBYEDAT_SUBEVENT_HD_SIZE;

                                 EventData = (UNSINT16*)p;

                                 /* Pour corriger erreur 1 mot de trop dans sub-event
                                    on ajoute 1 si longueur impaire */
                                 if (SubEventHd->length & 1)
                                    EventData++;
                                 while (EventData < NextSubEvent) {
                                    /* mask 2 label upper bits */
                                    *CtrlEventData++ = *EventData++ & 0x3fff;
                                    *CtrlEventData++ = *EventData++;
                                 }
                                 break;

                              case 0x0004: /* TPC */
                                 /*
                                   L'entete du ss-evt est constitue de
                                      Start Sub-Event Token
                                 Sub-Event Length
                                 0 a 3 Status Words pour aligner les donnees sur frontiere de 64 bits

                                   Structure des donn�s :

                                   - LabelMode (1 si groupe, 0 si pas groupe) 1 mot de 16 bits
                                   - LabelBase                                1 mot de 16 bits
                                   - Numero local d'evenement                 1 mot de 32 bits
                                   - 2 blocs de donn�s de 384 voies correspondant aux 2 plans anode (A) et cathode(K)


                                   Structure d'un bloc de donn�s de 384 voies:

                                   - TIME chan0                       1 mot de 16 bits
                                   - ENERGY chan0                     1 mot de 16 bits
                                   -......
                                   - TIME chan383                     1 mot de 16 bits
                                   - ENERGY chan383                   1 mot de 16 bits
                                   - Separateur                       1 mot de 16 bits
                                  */
                                 {
                                    int LabelMode, LabelStep, LabelBase, CurLabel;
                                    int* TPC_EndData;
                                    unsigned int TPC_EventNb;
                                    int NbStatusWords, i, j;
                                    NbStatusWords = (SubEventHd->StartToken >> 6) & 0x3;
                                    EventData = (UNSINT16*)SubEventHd + 2 + NbStatusWords;
                                    TPC_EndData = (int*)((UNSINT16*)SubEventHd + SubEventHd->length);
                                    LabelMode = *EventData++;
                                    LabelBase = CurLabel = *EventData++;
                                    TPC_EventNb = (*EventData++ << 16) + *EventData++;
                                    if (LabelMode == 1)   // groupe
                                       LabelStep = 256;
                                    else
                                       LabelStep = 1;

                                    /* Data Cathode */
                                    while ((*EventData != TPC_Separateur) && (EventData < (UNSINT16*)TPC_EndData)) {
                                       *CtrlEventData++ = CurLabel;
                                       CurLabel += LabelStep;
                                       *CtrlEventData++ = *EventData++;
                                    }

                                    /* Data Anode */
                                    if (LabelMode == 1)   // groupe
                                       CurLabel = LabelBase + 12;    // A REVOIR
                                    //CurLabel = LabelBase + (TPC_NbChannels * 2)/64; // a verifier
                                    else
                                       CurLabel = LabelBase + TPC_NbChannels * 2;

                                    while ((*EventData != TPC_Separateur) && (EventData < (UNSINT16*)TPC_EndData)) {
                                       *CtrlEventData++ = CurLabel;
                                       CurLabel += LabelStep;
                                       *CtrlEventData++ = *EventData++;
                                    }
                                 }
                                 break;

                              case 0x0008: /* Acquiris numeriser */
                                 /*
                                   L'entete du ss-evt est constitue de
                                      Start Sub-Event Token
                                 Sub-Event Length
                                 0 a 3 Status Words pour aligner les donnees sur frontiere de 64 bits

                                   Structure du bloc des donn�s :

                                   - nombre de voies                   1 mot de 16 bits
                                   - 0 �8 vecteurs


                                   Structure d'un vecteur :

                                   - num�o de voie                    1 mot de 16 bits
                                   - nombre de samples de la voie      1 mot de 16 bits
                                   - gain                              4 mot de 16 bits
                                   - offset                            4 mot de 16 bits
                                   - TStampLow                         2 mots de 16 bits
                                   - TStampHigh                        2 mots de 16 bits
                                   - HorPos                            4 mots de 16 bits
                                   - SampTime                          4 mots de 16 bits
                                   - donn�s �hantillons 8 bits

                                   le champ "num�o de voie" represente le label de la voie,
                                   plus exactement m�e le label des �hantillons. Ce label
                                   constitue une base pour les labels des informations
                                   suppl�entaires de la voie que sont le nombre de samples,
                                   le gain, l'offset, etc...
                                   ATTENTION NE FONCTIONNE PAS EN MODE GROUPE => A REVOIR!
                                  */
                                 {
                                    int NbStatusWords, NbVoies, i;
                                    NbStatusWords = (SubEventHd->StartToken >> 6) & 0x3;
                                    EventData = (UNSINT16*)SubEventHd + 2 + NbStatusWords;
                                    NbVoies = *EventData++;
                                    for (i = 0; i < NbVoies; i++) {
                                       /* Recopier chaque vecteur */
                                       int LabelVoie, CurLabel, NbSamples, n;
                                       LabelVoie = *EventData++;
                                       CurLabel = LabelVoie + 1;
                                       *CtrlEventData++ = CurLabel++;
                                       NbSamples = *EventData;
                                       *CtrlEventData++ = *EventData++; // Nb samples
                                       for (n = 0; n < 20; n++) {
                                          /* get infos */
                                          *CtrlEventData++ = CurLabel++;
                                          *CtrlEventData++ = *EventData++;
                                       }
                                       for (n = 0; n < NbSamples / 2; n++) {
                                          /* get 8 bits samples */
                                          *CtrlEventData++ = LabelVoie;
                                          *CtrlEventData++ = *EventData >> 8;
                                          *CtrlEventData++ = LabelVoie;
                                          *CtrlEventData++ = *EventData++ & 0Xff;
                                       }
                                    }
                                 }
                                 break;

                              default :
                                 err = ACQ_BADEVENTFORM;
                                 printf("acq_ebyedat_get_next_event : SubEvent StartToken 0x%x not treated\n",
                                        SubEventHd->StartToken);
                           }
                        }
                     } while ((NextSubEvent < *NextEvent) && (err == ACQ_OK));

                     CtrlEvent->ct_len = CtrlEventData - (UNSINT16*)CtrlEvent;
                     *EvtAddr = CtrlEventBuf;
                     break;

                  default:
                     err = ACQ_INVARG;
                     printf("acq_ebyedat_get_next_event : Bad EvtFormat argument \n");

               } /* End of switch EvtFormat of ExogamEventToken */
            }
            break;

         case EndDataBlockToken :
            /* Provisoire pour mauvais format evt */
            if (EventHd->length == 0) {
               /* No more events in buffer */
               err = ACQ_ENDOFBUFFER;
            } else {
               printf("acq_ebyedat_get_next_event : EndDataBlockToken with EventHd->length=%d != 0\n", EventHd->length);
               err = ACQ_BADEVENTFORM;
            }
            break;

         default : {
               printf("acq_ebyedat_get_next_event : Bad event Token with EventHd->StartToken = 0x%x\n", EventHd->StartToken);
               err = ACQ_BADEVENTFORM;
            }

      } /* End of switch EventToken */
   } /* End of verify Start Event token */

   if (err != ACQ_OK) {
      *EvtNum = -1;
      *EvtAddr = 0;
      *NextEvent = 0;
   }

   return (err);

} /* End of acq_ebyedat_get_next_event */

static void dump(char* p, int nb)
{
   int i, j, k;
   unsigned short* pShort;
   unsigned char* pChar;
   j = nb / 16;
   pShort = (unsigned short*)p;
   pChar = (unsigned char*)p;
   for (i = 0; i < j; i++) {
      printf("\n%8x:", pChar);
      for (k = 0; k < 8; k++)
         printf(" %04x", *pShort++);
      printf("    ");
      for (k = 0; k < 16; k++) {
         if ((*pChar >= 32) && (*pChar < 127))
            printf("%c", *pChar);
         else
            printf(".");
         pChar++;
      }
   }
   printf("\n\r");
}
