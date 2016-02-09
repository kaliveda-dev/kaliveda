/*****************************************************************************
 *                                                                           *
 * Fichier     : gan_acq_buf.h                                      *
 *                                                                           *
 * Auteur      : Bruno Raine                                        *
 * Date     : 03 dec 96                                          *
 * Modifie le  : 21 juillet 1999                                    *
 *                        F. Saillant: mot magique dans buffer GANIL new gene*
 *                        14 juin 1999                                       *
 *                        B. Piquet: introduction de #pragma pour resoudre   *
 *                        les pb de non alignements dans les structures      *
 *                        11 juin 1999                                       *
 *                        F. Saillant: ajout nouveaux types de buffers       *
 *                        18 mars 1999                                       *
 *                        B. Piquet: recadrage de l'entete JBUS pour swap    *
 *                        B. Raine le 13 decembre 2002
 *                          Ajout buffers de type ENDRUN
 *                        B. Raine 29 janvier 2003
                            ifdef cplusplus
             maj GANIL_BUF_HD
 *                                                                           *
 * Objet    : Description des buffers acquisition GANIL          *
 *      Remarques       : Ce fichier necessite la definition des macros      *
 *                        UNSINT16 et UNSINT32 qui sont definies en          *
 *                        standard dans GEN_TYPE.H.                          *
 *                        Donc sauf contre-indication, inclure GEN_TYPE.H    *
 *                        dans votre programme pour utilise ce .h            *
 *                                                                           *
 ****************************************************************************/
#ifndef __GanAcqBuf_H
#define __GanAcqBuf_H


#ifdef __cplusplus
extern "C" {
#endif

/* Definition des entetes de buffer */
#define FILEH_Id        " FILEH  "
#define EVENTH_Id       " EVENTH "
#define PARAM_Id        " PARAM  "
#define COMMENT_Id      " COMMENT"
#define EVENTDB_Id      " EVENTDB"
#define EVENTDB_SWAP_Id "E EVTNBD"
#define EVENTCT_Id      " EVENTCT"
#define EVENTCT_SWAP_Id "E EVTNTC"
#define JBUS_Id         "  JBUS  "
#define JBUS_SWAP_Id    "  BJSU  "
#define SCALER_Id       " SCALER "
#define SCALER_SWAP_Id  "S ACEL R"
#define STATUS_Id       " VMESTAT"
#define STATUS_SWAP_Id  "V EMTSTA"
#define EBYEDAT_Id      " EBYEDAT"
#define RAWDT32_Id      " RAWDT32"
#define CONFIG_Id       " CONFIG "
#define INFODAT_Id      " INFODAT"
#define ENDRUN_Id       " ENDRUN "

/* Definitions generales */
#define IN2P3_MIN_STORE_BUF_SIZE 4096
#define IN2P3_MAX_STORE_BUF_SIZE 16384
#define GANIL_MAX_STORE_BUF_SIZE 32764


#define IN2P3_HEADER_LEN   12
#define SCALE_HEADER_LEN   20
#define SCALE_STRUCT_LEN   32

#define TAILLEBUFHEAD   400

#define NB_MAX_SHORT    (IN2P3_MAX_STORE_BUF_SIZE - IN2P3_HEADER_LEN) / 2
#define NB_MAX_CHAR  (IN2P3_MAX_STORE_BUF_SIZE - IN2P3_HEADER_LEN)
#define NB_MAX_CHANNEL  (IN2P3_MAX_STORE_BUF_SIZE - ( IN2P3_HEADER_LEN +  \
                         SCALE_HEADER_LEN)) / SCALE_STRUCT_LEN
#define NB_MAX_JBUS  (IN2P3_MAX_STORE_BUF_SIZE - ( IN2P3_HEADER_LEN +  \
                      SCALE_HEADER_LEN)) / 2

#define EVCT_FIX 0L
#define EVCT_VAR 1L

/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment save
#pragma nomember_alignment
#endif

/* Definition des buffers  */

/***********************************************************************/
/* Structure en-tete d'un buffer Acquisition GANIL Nouvelle Generation */
/***********************************************************************/

typedef struct _GANIL_BUF_HD {
   char ident[8];             /* identificateur ASCII      */
   unsigned int num;          /* numero de buffer          */
   unsigned int magic;        /* mot magique (0x22061999)  */
   unsigned short source_id;  /* source ident              */
   unsigned short dest_id;    /* destination ident         */
   unsigned short stream;     /* numero de flux            */
   unsigned short eventcount; /* nombre d'evenements       */
   unsigned int checksum;     /* checksum                  */
   unsigned int length;       /* nombre de mots de 16 bits */

} GANIL_BUF_HD;

#define GANIL_BUF_HD_SIZE 32 /* Taille d'un en-tete de buffer GANIL Nouvelle Generation */

#define GANIL_BUF_END_SIZE 32

#define MAGIC_NUMBER 0x22061999

/********************************************/
/* Structure en-tete d'un evenement EBYEDAT */
/********************************************/

typedef struct _EBYEDAT_EVENT_HD {
   unsigned short StartToken; /* Token de debut d'un evt                     */
   unsigned short length;     /* longueur de l'evt (nbre de mots de 16 bits) */
   unsigned short status;     /* mot de status                               */
   unsigned short num_upper;  /* numero de l'evt : mot 16 bits de poids fort */
   unsigned short num_lower;  /* numero de l'evt : mot 16 bits de poids faibl*/

} EBYEDAT_EVENT_HD;

#define EBYEDAT_EVENT_HD_SIZE 10 /* Taille d'un en-tete d'evt EBYEDAT */

/*************************************************/
/* Structure en-tete d'un sous-evenement EBYEDAT */
/*************************************************/

typedef struct _EBYEDAT_SUBEVENT_HD {
   unsigned short StartToken; /* Token de debut d'un sous-evt       */
   unsigned short length;     /* longueur (nbre de mots de 16 bits) */

} EBYEDAT_SUBEVENT_HD;

#define EBYEDAT_SUBEVENT_HD_SIZE 4 /* Taille d'un en-tete de sous-evt EBYEDAT */

/********************************************/
/* Structure en-tete d'un evenement EBYEDAT StartEventToken = 0xff00 */
/********************************************/

typedef struct _EBYEDAT_EVENT_HD_0 {
   unsigned short StartToken; /* Token de debut d'un evt                     */
   unsigned short length;     /* longueur de l'evt (nbre de mots de 16 bits) */

} EBYEDAT_EVENT_HD_0;

#define EBYEDAT_EVENT_HD_SIZE_0 4 /* Taille d'un en-tete d'evt EBYEDAT */

/*************************************************/
/* Structure en-tete d'un sous-evenement EBYEDAT StartSubeventToken = 0x0020*/
/*************************************************/

typedef struct _EBYEDAT_SUBEVENT_HD_20 {
   unsigned short StartToken; /* Token de debut d'un sous-evt       */
   unsigned short length;     /* longueur (nbre de mots de 16 bits) */
   unsigned short num_upper;  /* numero de l'evt                    */
   unsigned short num_lower;  /* numero de l'evt                    */

} EBYEDAT_SUBEVENT_HD_20;

#define EBYEDAT_SUBEVENT_HD_SIZE_20 8 /* Taille d'un en-tete de sous-evt EBYEDAT */

typedef struct SCALE_STRUCT {
   UNSINT32 Label;
   INT32 Status;
   UNSINT32 Count;
   UNSINT32 Freq;
   UNSINT32 Tics;
   UNSINT32 Reserve[3];
} scale_struct;

typedef struct FILEH_STRUCT {
   char Ident[12];         /* Identificateur Norme ' IN2P3-PN   ' */
   char Version[4];
   char Labo[16];          /* Nom du labo                         */
   char Machine[16];       /* Nom de la machine                   */
   char Application[16];   /* Nom de l'application                */
   char Reserve1[16];
   char TailleBlocs[8];    /* Taille fixe des blocs en octets     */
   char OrdreOctets[4];    /* ' MSB' ou ' LSB'                    */
   char Date[20];          /* ' JJ-MMM-AA HH/MM/SS '              */
   char Reserve2[48];
} fileh_struct;

typedef struct EVENTH_STRUCT {
   char TypeEvent[8];      /* ' WORDC  ' ou ' FFFF   '                  */
   char FormatDonnees[8];  /* ' INT*2  ' ou ' INT*4  '                  */
   char FormatWC[8];       /* ' INT*2  ' pour un word count sur 15 bits */
   char Date[20];          /* ' JJ-MMM-AA HH/MM/SS '                    */
   char NomRun[16];        /* Nom du run                                */
   char NumeroRun[8];      /* Numero du run                             */
   char Reserve1[4];
   char TypeStruct[8];     /* Type de structure ' GANIL '               */
   char Commentaire[80];
   char NbBlocStrEvt[4];   /* Nombre de blocs ' COMMENT' contenant la
                             structure de l'evenement                  */
} eventh_struct;

typedef struct INFO_ENDRUN_STRUCT {
   union {
      struct i {
         char label[19];
         char info[12];
         char eos;
      } i;
      char line[32];
   } u;
} INFO_ENDRUN_STRUCT;

typedef struct ENDRUN_STRUCT {
   INFO_ENDRUN_STRUCT date;
   INFO_ENDRUN_STRUCT run_number;
   INFO_ENDRUN_STRUCT nb_events;
   INFO_ENDRUN_STRUCT nb_buffers;
   INFO_ENDRUN_STRUCT nb_buffers_ontape;
} endrun_struct;

typedef union IN2P3_BUFFER_STRUCT {

   char Buffer[IN2P3_MAX_STORE_BUF_SIZE]; /* Buffer de lecture     */
   struct LES_DONNEES {  /* Debut de la structure de donnees IN2P3 */

      char Ident[8];
      UNSINT32  Count;
      union CAS {
         struct STATUS   {
            UNSINT32 Length; /* Nb bytes utiles suivant ce mot */
            char VmeName[16]; /* Nom du frontal VME */
         } status;
         UNSINT16 Buff[NB_MAX_SHORT];   /* Buffer standard  */
         char     Buf_param[NB_MAX_CHAR];  /* Buffer parametre */
         char        Buf_ascii[TAILLEBUFHEAD];/* Buffer d'entete  */
         fileh_struct    Buf_fileh; /* Buffer d'entete 'FILEH'  */
         eventh_struct   Buf_eventh;/* Buffer d'entete 'EVENTH' */
         struct SCALE   {
            UNSINT32 Length; /* Nb bytes utils suivant ce mot */
            UNSINT32 Nb_channel;
            UNSINT32 Acq_status;
            UNSINT32 Reserve[2];
            union JBUS_SCALE {
               scale_struct UnScale[NB_MAX_CHANNEL];
               UNSINT16 Info_jbus[NB_MAX_JBUS];
            } jbus_scale;
         } scale;
         endrun_struct  Buf_endrun; /* Buffer de fin de run ' ENDRUN ' */
      } cas;
   } les_donnees;
} in2p3_buffer_struct;

/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment restore
#endif

#ifdef __cplusplus
}
#endif

#endif
