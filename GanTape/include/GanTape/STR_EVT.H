/*	STR_EVT.H -- creation date : 15 sep 89   last update : 26 nov 97
	Copyright Acquisition group, GANIL Caen, France
	author	: Bruno RAINE

 	contents : GANIL data acquisition events structures description

	List of the files to be included before the present one :
	These files must necessary be included in the application .

	 GEN_TYPE.H
	
         Modifications :
          -B. Raine le 9 fev 95   Compatibilite PDOS 3.3/4.1
          -B. Raine le 19 aout 96
		Ajout coupleur externe de type memoire MEMEXT
          -F. Saillant le 22 aout 96
                Ajout extension pour fichier d'Install
          -B. Raine le 2 dec 96
	        Restructuration pour execution multiplateforme
                on remplce tous les INT16 non indispensables par des int
	  -B. Raine le 26 nov 97
		Introduction des codes EV_CTRFIX et EV_CTRVAR pour le fichier
		structure evenements dans le cas ou on stocke des evenements 
		de control

*/

#ifdef __cplusplus
extern "C" {
#endif
/* definition des extensions standards aux noms de fichiers d'acquisition */
#define DSK_CVC "8"		/* Disque contenant les fichiers d'action */
#define EXT_CAM "CVC"
#define EXT_STR "STR"
#define EXT_FILT "FIL"
#define EXT_INST "INS"          /* Extension pour fichier d'Install       */

/* Definition des codes de format d'evenments de control. Ces codes sont
   utilises dans l'appel a s_evctrl pour indiquer le type d'evenemnet de
   control desire */ 
#define EVCT_FIX 0L  /* evenement de control de taille fixe               */
                     /* A l'indice i de l'evt correspond le parametre i   */
#define EVCT_VAR 1L  /* evenement de control de taille variable           */
                     /* Chaque parametre est compose de 2 mot de 16 bits: */
                     /* LABEL / PARAMETRE                                 */

#define MAX_LAM 16    /* nombre max de lam's dans un meme evt camac */

/* La table en memoire de description d'un evenement est une table de mots de
   16 bits ayant le format suivant :

      - le 1er mot contient le nombre total de parametres

      - code fct de description de l'evt, suivi d'arguments dont le nombre
        est fonction de la valeur du code
          les codes valides actuellement sont :                             */

#define EV_DEBLAM 0  /* code debut lam dans un ss-evt camac */
#define EV_FIX 1     /* code partie fixe de l'evenement */
#define EV_VAR 2     /* code partie variable de l'evenement */
#define EV_COUPL 3   /* code definissant le coupleur ayant acquis les donnees 
			qui suivent */
#define EV_FERA 4    /* code definissant le debut d'une table de correspondance
                        VSN_SA/Numero de parametres */
#define EV_MEMEXT 5  /* code definissant le debut d'une table de correspondance
                        Labels coupleur externe memoire/Numero de parametres */
#define EV_FIN -1    /* code fin descriptif evenement */

/* Codes utilises pour le fichier structure ecrit sur le support magnetique
   lorsqu'on stocke des evenements de controle plutot que des bruts */
#define EV_CTRFIX 10 /* Les evts de controle sont de type EVCT_FIX */
#define EV_CTRVAR 11 /* Les evts de controle sont de type EVCT_VAR */

/*================Fin description table de structure evenement===============*/

typedef struct { /* description des arguments du code EV_FIX */
	int fix_start;
	int fix_len;
	} SEV_FIX;

typedef struct sev_coupl { /* description des argumenets du code EV_COUPL */
	int cpl_typ;
        struct sev_fct *cpl_next;   /* pointeur sur prochain coupleur a traiter */
	struct sev_fct *cpl_deblam[MAX_LAM];  /* table des adresses de structure en
                                       fct du numero de lam dans le ss-evt */
	} SEV_COUPL;

typedef struct { /* description des arguments du code EV_FERA */
	int fer_maxad;  /* VSN_SA max pour ce coupleur */
        INT16 *fer_label; /* pointeur sur tables correspondance labels/vsn_sa */
	} SEV_FERA;

typedef SEV_FERA SEV_MEMEXT; /* arguments code EV_MEMEXT identiques a FERA */

typedef struct sev_fct{ /* structure des codes fct de description d'un evt */
        struct sev_fct *sev_next;
	int sev_code;
	union {
	  SEV_FIX sev_fix;
	  SEV_COUPL sev_cpl;
	  SEV_FERA sev_fera;
	  } sev_par;
	} SEV_FCT;

typedef struct sev_str_hd{ /* description debut de la memoire structure evt */
	int    NbParam;   /* Nombre total de parametres */
        struct sev_fct *sev_next;
        } SEV_STR_HD;
    

typedef struct { /* format de l'entete d'un evt ou d'un ss-evt brut */
	INT16 ss_len;	/* event length in 16-bit words */
	INT16 ss_nb;	/* event number */
	INT16 ss_s1_id;	/* event status 1, ident de l'interface */
	INT16 ss_s2;	/* event status 2 */
        INT16 ss_par;   /* 1st parameter of sub-event */
	} SS_EVENT;

typedef struct { /* format de l'entete d'un evt ou d'un ss-evt brut */
	INT16 ev_len;	/* event length in 16-bit words */
	INT16 ev_nb;	/* event number */
	INT16 ev_s1;	/* event status 1 */
	INT16 ev_s2;	/* event status 2 */
        SS_EVENT ev_ss;   /* 1st sub-event of event */
	} EVENT;

typedef struct { /* format de l'entete d'un evt DE CONTROL */
	INT16 ct_len;	/* event length in 16-bit words */
	INT16 ct_nb;	/* event number */
	char ct_stat[8]; /* event status */
			/* le 1er octet contient le status general, le bit de
			   de poids fort indiquant une erreur sur un des
			   coupleurs;
			   les 7 octets suivants sont le reflet du status des
			   7 coupleurs possibles. */
        INT16 ct_par;  /* 1st parameter of control event */
	} CTRL_EVENT;

#ifdef __cplusplus
}
#endif
