/*   s_evctrl:c -- creation date : 15 sep 89  last update : 13 janvier 1999
Copyright Acquisition group, GANIL Caen, France
module name   : s_evctrl.c   version  : 05 de s_evctrl
library    : ACQ$GAN:LIB

author     : Bruno RAINE


  function : set control event from brut event

synopsis : s_evctrl(evtbuf, evtctrl, evtstr, pCtrlForm)
    ou si FRONTAL defini
     s_evctrl(evtbuf, evtctrl, evtstr, pCtrlForm, pfacqvar)
     INT16 *evtbuf;   event buffer address
     INT16 *evtctrl;  control event buffer address
     INT16 *evtstr;   event structure address
                  INT32 *pCtrlForm; control event format address
                                   (see GanTape/STR_EVT.H for valid formats):
                                    EVCT_FIX -> format fixe (0)
                                    EVCT_VAR -> format variable (1)
                                                (un label par parametre)
                  PFACQVAR *pfacqvar;    (si FRONTAL defini )

Autre point d'entree, a la place de s_evctrl :
     s_evctdbg   si compilation avec la macro EVCTDBG definie.
     Lorsque le programme est compile avec cette option, il
     imprime le detail de l'evenement brut en meme temps qu'il
     construit l'evenement de control.

input   :

output  : rend GR_OK si Ok
         erreur > 0 sinon

example : Call from a C program :
                 ctrlform = EVCT_FIX;
                 status = s_evctrl(evtbuf, evctrl, evtstr, &ctrlform);

          Call from a Fortran program :
                 ctrlform = 0
                 status = s_evctrl(evtbuf, evctrl, evtstr, ctrlform)


       modifications :
# 21 nov 91 - B. Raine
            Prise en compte des labels sur 14 bits avec overflow sur bit 15
            pour codeurs VXI d'INDRA
       # 14 sep 92 - B. Raine
            Utilisation d'une variable locale pour pointeur courant sur buffer
            structure evenement
            Remplacer <= par < dans boucle for d'init du buffer de control
       # 15 sep 92 - B. Raine
            Remplacer les variables INT16 definissant les tailles des
            structures en mots de 16 bits par des #define.
       # 16 nov 92 - B. Raine
            Possibilite de creer des evenements de control de taille variable.
            Chaque parametre est associe a son label et forme donc toujours un
            mot de 32 bits forme des deux mots de 16 bits LABEL/PARAMETRE.
            Un argument supplementaire est ajoute a l'appel pour definir le
            format de l'evenement de control desire.
       # fevrier 1994 Bruno PIQUET
     for compilation under Open VMS-AXP, and ANSI-C
       # 15 juillet 94 - B. Raine
            On passe pfacqvar en argument pour prendre en compte les options
            eventuelles  de debug
            si S_DEBUG, on imprime l'evt en erreur
            si S_ACQCTRL, on n'appelle pas tr_err
       # 18 juillet 94 - B. Raine
            test de compatibilite entre wc et longueur ss-evt
       # 1er aout 96 - B. Raine
     Compilation conditionnelle pour traitement des erreurs si FRONTAL
     FRONTAL est postionne a 1 si PDOS
       # 19 aout 96 - B. Raine
            Prise en compte coupleur externe type memoire MEMEXT
# 02 dec 96 - B. Raine
     Restructuration pour execution multiplateforme
# 17 dec 96 - B. Raine
     Introduction de la macro EVCTDBG pour compilation avec les ordres
            de print et definition du point d'entree s_evctdbg au lieu de
     s_evctrl.
# 9 janvier 97 - B. Raine
     Modification des formats de print pour evctdbg
       # 12 mars 1997 - B. Raine
            int s_evctrl et int err
# 13 janvier 99 - B. Raine
            init correct de strfct

*/

#ifdef PDOS
#define FRONTAL 1
#endif

#ifdef PDOS
#include <stdio.h>
#include "GEN_TYPE:H"   /* general typedef definitions */
#include "ERR$GAN:H" /* GANIL errors definitions */
#include "STR_EVT:H" /* structure of event description */
#include "EQUIPDES:H"   /* equipment descriptors */
#include "MDACQDES:H"
#else
#include <stdio.h>
#include "GanTape/GEN_TYPE.H"   /* general typedef definitions */
#if defined ( __unix ) || ( __unix__) || (__APPLE__) || (__MACH__)
#include "ERR_GAN.H" /* GANIL errors definitions */
#else
/*#include "ERR$GAN.H" GANIL errors definitions */
#error "should not have come here"
#endif
#include "GanTape/STR_EVT.H" /* structure of event description */
#include "EQUIPDES.H"   /* equipment descriptors */
#endif

#define CAM_COUPL CPUFAMI<<8  /* ca devrait etre CAMFAMI, mais cette famille
               est deja utilisee pour les CBD et de plus, 
                                   le CPU controllant le CBD est de CPUFAMI
                                   .......a revoir...........                */
#ifdef EVCTDBG
#define FCTNAME s_evctdbg
#else
#define FCTNAME s_evctrl
#endif

#ifdef __STDC__
#ifdef FRONTAL

int FCTNAME(INT16* evtbuf, INT16* evtctrl,
            char*   evtstr, int*   pCtrlForm,
            PFACQVAR* pfacqvar)

#else

int FCTNAME(INT16* evtbuf, INT16* evtctrl,
            char*   evtstr, int*   pCtrlForm)

#endif
#else
#ifdef FRONTAL
int FCTNAME(evtbuf, evtctrl, evtstr, pCtrlForm, pfacqvar)
INT16* evtbuf;  /* event buffer address */
INT16* evtctrl; /* control event buffer address */
char*  evtstr;  /* event structure address */
int*   pCtrlForm; /* control event format address */
PFACQVAR* pfacqvar;
#else
int FCTNAME(evtbuf, evtctrl, evtstr, pCtrlForm)
INT16* evtbuf;  /* event buffer address */
INT16* evtctrl; /* control event buffer address */
char*  evtstr;  /* event structure address */
int*   pCtrlForm; /* control event format address */
#endif
#endif
{
   EVENT* event;
   SS_EVENT* ss_evt;
   CTRL_EVENT* ct_evt;
   SEV_FCT* sev_fct;     /* Pointeur courant sur fonction structure evt */
   SEV_FCT* SevFctCpl;   /* Pointeur sur structure coupleur courant */
   SEV_FIX* sev_fix;
   SEV_COUPL* sev_cpl;
   SEV_FERA* sev_fera;
   INT16* evtend, *ssevend, *ctend; /* adresse fin evt ou ss_evt */
   INT16* evtcour; /* pointeur courant dans le buffer evt brut*/
   INT16* ctstart; /* pointeur debut zone parametres dans evt de ctrl */
   INT16* ctcour; /* pointeur courant dans evt ctrl */
   INT16 nbcoupl;  /* nombre de coupleurs traites */
   INT16 strfct;  /* code courant de la table structure evt */
   INT16* ferstart; /* debut table conversion vsn_sa->numero de parametre */
   INT16* fercour; /* pointeur courant dans table de conversion */
   INT16 wc, vsn, vsn_sa, typ_fera, shft_sa, msk;
   static INT16 ctsiz16 = sizeof(CTRL_EVENT) / 2 - 1;
   static INT16 lg_fera[2] = {16, 8}; /* nb max de sa en fct du type de codeur */
   static INT16 nbit_sa[2] = {4, 3};  /* nb bits sa en fct type de module */
   static INT16 nbit_par[2] = {11, 12}; /* nb bits param en fct type de module */
   static INT16 msk_par[2] = {0x7ff, 0xfff}; /* mask param en fct type de module */
   register INT16 param_vsn, param_sa, indparam;
   int  i, cont, typcpl;
   int err;
   unsigned long  errl;
   UNSINT16 label;
   SEV_STR_HD*  SevStrHd;  /* Pointeur sur debut buffer structure */
   int ctrlform;   /* Control event format */


   SevStrHd = (SEV_STR_HD*)evtstr;  /* Pointeur sur debut buffer structure */
   event = (EVENT*)evtbuf;       /* on pointe sur l'entete de l'evt */
   evtend = evtbuf + event->ev_len; /* adresse fin de l'evt */
   err = GR_OK;

   /*printf("\nentree s_evctrl, err=0x%x, GR_OK=0x%x\n", err, GR_OK);    */

   /* on pointe sur l'entete de l'evt de control */
   ct_evt = (CTRL_EVENT*)evtctrl;

   ct_evt->ct_nb = event->ev_nb;      /* numero de l'evt */

   ctrlform = *pCtrlForm;
   switch (ctrlform) {
      case EVCT_FIX :
         /*lg evt ctrl = entete + nb par */
         ct_evt->ct_len = (INT16)SevStrHd->NbParam + ctsiz16; /*lg evt ctrl =
                                                            entete + nb par */
         ctend = evtctrl + ct_evt->ct_len; /* adresse fin evt control */

         /*raz ctrl evt*/
         for (ctcour = (INT16*)(ct_evt->ct_stat) ;
               ctcour < ctend ; *ctcour++ = -1);
         break;

      case EVCT_VAR :
         for (i = 0; i < 8; i++)
            ct_evt->ct_stat[i] = 0xff;
         break;

      default :
         err = R_EVCTFORM;   /* mauvais format evenement de control */
         printf("\nsortie s_evctrl mauvais format evenement de control\n");
         printf("    err=0x%x\n", err);
         return (err);       /* Sortie en erreur */
   }

   ct_evt->ct_stat[0] = (event->ev_s1 >> 8) & 0x80; /* on prend uniquement le
                                                       bit erreur du status */

   ctstart = &ct_evt->ct_par - 1; /* se placer en fin d' entete evt de ctrl */

   ctcour = ctstart + 1;

   sev_fct = SevStrHd->sev_next;

   nbcoupl = 0;
   evtcour = (INT16*)&event->ev_ss ;  /* se placer sur l'entete du ss_evt */
   cont = TRUE;

   do { /* boucler sur les coupleurs */

      do {
         /* 1 */ /* boucler sur les codes fonctions pour chaque coupleur */

         /* B. Raine le 13 jan 99, init correct de strfct */
         strfct = sev_fct->sev_code;

         if (evtcour <= evtend) {
            /* 1.1 */
            switch (sev_fct->sev_code) { /* switch fct du code struct evt a traiter */
               /* 1.1.1 */
               case EV_COUPL :
                  SevFctCpl = sev_fct; /* On memorise le pointeur de coupleur */
                  sev_cpl = &SevFctCpl->sev_par.sev_cpl;
                  ss_evt = (SS_EVENT*)evtcour;
                  typcpl = sev_cpl->cpl_typ;
#ifdef EVCTDBG
                  printf("\nheader sous-evenement: ");
                  printf("wc = %d, evt_num = %d, id = 0x%x, status = 0x%x",
                         ss_evt->ss_len, (UNSINT16)ss_evt->ss_nb,
                         (UNSINT16)ss_evt->ss_s1_id, (UNSINT16)ss_evt->ss_s2);
#endif
                  if ((ss_evt->ss_s1_id & 0xff00) != typcpl) {
                     err = R_SEVID; /* erreur sur l'ident de l'interface */
                     errl = sev_cpl->cpl_typ;
                     cont = FALSE;
                  }
                  else {
                     nbcoupl++; /* incrementer le nombre de coupleurs traites */
                     if ((ct_evt->ct_stat[nbcoupl] = ss_evt->ss_s2) < 0) {
                        /* set bit erreur dans ct_stat[0] si erreur */
                        ct_evt->ct_stat[0] = ct_evt->ct_stat || 0x80;
                        err = R_NOEVT;
                        cont = FALSE; /*  aucun traitement pour l'instant */
                     }
                     else {
                        ssevend = evtcour + ss_evt->ss_len;
                        evtcour = &ss_evt->ss_par;
                        if (typcpl == CAM_COUPL) {
                           sev_fct = (SEV_FCT*)&sev_cpl->cpl_deblam[ss_evt->ss_s2];
                        }
                        else {
                           sev_fct = (SEV_FCT*)&sev_cpl->cpl_deblam[0];
                        }
                     }
                  }
                  break;

               case EV_FIX :
                  sev_fix = &sev_fct->sev_par.sev_fix;
#ifdef EVCTDBG
                  printf("\n  partie fixe: ");
#endif
                  switch (ctrlform) {
                     case EVCT_FIX :
                        for (i = 0, ctcour = ctstart + sev_fix->fix_start;
                              i < sev_fix->fix_len;
                              i++) {
#ifdef EVCTDBG
                           if ((i % 8) == 0)
                              printf("\n     ");
                           printf(" 0x%04.04x |", (UNSINT16)*evtcour);
#endif
                           *ctcour++ = *evtcour++;
                        }
                        break;

                     case EVCT_VAR :
                        for (i = 0, ctcour = ctstart + sev_fix->fix_start;
                              i < sev_fix->fix_len;) {
#ifdef EVCTDBG
                           if ((i % 8) == 0)
                              printf("\n     ");
                           printf(" 0x%04.04x |", (UNSINT16)*evtcour);
#endif
                           *ctcour++ = ++i;
                           *ctcour++ = *evtcour++;
                        }
                        break;
                  }
                  break;

               case EV_VAR :
#ifdef EVCTDBG
                  printf("\n   partie variable: ");
                  i = 0;
#endif
                  switch (ctrlform) {
                     case EVCT_FIX :
                        while (evtcour < ssevend) { /* vider la fin du ss-evt */
                           label = *evtcour++ & 0x3fff;
#ifdef EVCTDBG
                           if ((i++ % 5) == 0)
                              printf("\n     ");
                           printf(" %4d, 0x%04.04x |", label, (UNSINT16)*evtcour);
#endif
                           ctcour = ctstart + label;
                           *ctcour = *evtcour++;
                        }
                        break;

                     case EVCT_VAR :
                        while (evtcour < ssevend) { /* vider la fin du ss-evt */
#ifdef EVCTDBG
                           label = *evtcour & 0x3fff;
#endif
                           *ctcour++ = *evtcour++ & 0x3fff;
#ifdef EVCTDBG
                           if ((i++ % 5) == 0)
                              printf("\n     ");
                           printf(" %4d, 0x%04.04x |", label, (UNSINT16)*evtcour);
#endif
                           *ctcour++ = *evtcour++;
                        }
                        break;
                  }
                  break;

               case EV_MEMEXT:
                  /* Le traitement etant tres similaire au FERA, on reutilise les
                     meme sequences et les memes variables */
                  ferstart  = sev_fct->sev_par.sev_fera.fer_label;
#ifdef EVCTDBG
                  i = 0;
#endif
                  while ((evtcour < ssevend) && cont) {
                     /* On se place dans la table de correpondance des labels */
#ifdef EVCTDBG
                     if ((i++ % 5) == 0)
                        printf("\n     ");
                     printf(" %4d, 0x%04.04x |",
                            (UNSINT16)*evtcour, (UNSINT16) * (evtcour + 1));
#endif
                     fercour = ferstart + *evtcour++;
                     if ((indparam = *fercour) != 0) {
                        /* Ok ce label est connu */
                        switch (ctrlform) {
                           case EVCT_FIX :
                              ctcour = ctstart + indparam;
                              *ctcour = *evtcour++;
                              break;

                           case EVCT_VAR :
                              *ctcour++ = indparam;
                              *ctcour++ = *evtcour++;
                              break;
                        }
                     }
                     else {
                        /* entree non initialisee dans table de conversion */
#ifdef EVCTDBG
                        printf("\n entree %d non initialisee dans table de conversion"
                               , (UNSINT16) * (evtcour - 1));
#endif

                        err = R_MEXTLAB;
                        errl = (unsigned long)indparam;
                        errl = errl << 16;
                        errl = errl + (unsigned short) * (evtcour - 1);
                        *evtcour++;
                        cont = FALSE;
                     }
                  } /* fin du while */
                  break;

               case EV_FERA :
                  ferstart  = sev_fct->sev_par.sev_fera.fer_label;
                  while ((evtcour < ssevend) && cont) {
                     /*EV_FERA1*/
                     if ((param_vsn = *evtcour++) < 0) {
                        /*EV_FERA1.1*/
                        typ_fera = (param_vsn >> 8) & 7;

                        /* si wc=0, wc=8 ou 16 */
                        if ((wc = ((param_vsn >> 11) & 0XF)) == 0)
                           wc = lg_fera[typ_fera];

                        vsn = (param_vsn & 0xff) << nbit_sa[typ_fera];
                        shft_sa = nbit_par[typ_fera];
                        msk = msk_par[typ_fera];
#ifdef EVCTDBG
                        printf("\n  param_vsn = 0x%4.4x => wc = %d(0x%x), vsn = %d(0x%x)",
                               (UNSINT16)param_vsn, wc, wc,
                               (param_vsn & 0xff), (param_vsn & 0xff));
#endif
                        /* ----------------------------------------------------------------- */
                        /* Modif du 18 juillet 94 (B. Raine) */
                        if ((evtcour + wc) > ssevend) {
                           /*EV_FERA1.1.1*//* longueur ss-evt incompatible avec wc */
                           err = R_VSNLEN;
                           errl = param_vsn;
                           cont = FALSE;
                        }/*EV_FERA1.1.1*/
                        else
                           /* ----------------------------------------------------------------- */
                           for (i = 0; (i < wc) && cont; i++) {
                              /*EV_FERA1.1.2*/
                              if ((param_sa = *evtcour++) >= 0) {
                                 /*EV_FERA1.1.2.1*/
#ifdef EVCTDBG
                                 if ((i % 8) == 0)
                                    printf("\n     ");
                                 printf(" 0x%04.04x |", param_sa);
#endif
                                 vsn_sa = (param_sa >> shft_sa) + vsn;
                                 fercour = ferstart + vsn_sa;
                                 if ((indparam = *fercour) != 0) {
                                    /*EV_FERA1.1.2.1.1*/
                                    switch (ctrlform) {
                                       case EVCT_FIX :
                                          ctcour = ctstart + indparam;
                                          *ctcour = param_sa & msk;
                                          break;

                                       case EVCT_VAR :
                                          *ctcour++ = indparam;
                                          *ctcour++ = param_sa & msk;
                                          break;
                                    }
                                 } /*EV_FERA1.1.2.1.1*/
                                 else {
                                    /* entree VSN_SA non init. dans table de conversion */
                                    err = R_VSNSA;
                                    errl = param_vsn;
                                    errl = errl << 16;
                                    errl = errl + param_sa;
                                    cont = FALSE;
                                    printf("\nentree VSN_SA non init. dans table de conversion vsn_sa = 0x%x, indparam = 0x%x", vsn_sa, indparam);
                                 }
                              } /*EV_FERA1.1.2.1*/ /* end if param_sa */
                              else {
                                 /* mauvaise sequence VSN/SA ,
                                    on a un VSN a la place d'un SA */
                                 err = R_NOSA;
                                 errl = param_vsn;
                                 errl = errl << 16;
                                 errl = errl + param_sa;
                                 cont = FALSE;
                                 printf("\nmauvaise sequence VSN/SA, on a un VSN a la place d'un SA");
#ifdef EVCTDBG
                                 printf("\n ERREUR => data = 0x%04.04x", param_sa);
#endif

                              }
                           } /*EV_FERA1.1.2*/ /* end for i */
                     } /*EV_FERA1.1*/ /* end if param_vsn */
                     else {
                        /* mauvaise sequence VSN/SA,
                           on n'a pas un VSN la ou on l'attend */
                        err = R_NOVSN;
                        errl = param_vsn;
                        cont = FALSE;
                        printf("\nmauvaise sequence VSN/SA, on n'a pas un VSN la ou on l'attend");
#ifdef EVCTDBG
                        printf("\n ERREUR => data = 0x%04.04x", param_vsn);
#endif
                     }
                  } /*EV_FERA1*/ /* end while */
                  break;

               case EV_FIN :
                  break;

               default :
                  err = R_SEVCODE;
                  errl = strfct;
                  cont = FALSE;
                  printf("\ncode fonction structure evenement inconnu: ");
                  printf("evtstr=0x%lx, sev_fct=0x%lx, code=0x%x",
                         evtstr, sev_fct, sev_fct->sev_code);
#ifdef PDOS
                  xdmp(64, evtstr);
#endif
                  break;

            } /*1.1.1*/ /* end switch */
         } /*1.1*/

         else {
            /*1.2*/
            if ((strfct != EV_VAR) && (strfct != EV_FERA)) {
               /*1.2.1*/
               err = R_SEVBAD; /* longueur evt et structure evt incompatibles */
               errl = strfct;
            } /*1.2.1*/
            cont = FALSE;
         } /*1.2*/

      }/*1*/ while ((sev_fct = sev_fct->sev_next) != (SEV_FCT*)0);

   }
   while ((sev_fct = SevFctCpl->sev_next) != 0 && cont);

   if (err != GR_OK) {
      /* ----------------------------------------------------------------- */
      /* Modif du 15 juillet 94 et 1er aout 96 (B. Raine) */
#ifdef FRONTAL
      /* Si on s'execute sur le frontal VME, on conditionne les affichages */
      if ((pfacqvar->acq_stat & S_DEBUG) != 0) {
         /* si DEBUG, on imprime l'erreur */
#endif
         /* ----------------------------------------------------------------- */
#ifdef EVCTDBG
         printf("\n\nevtcour = 0x%lx", evtcour);
#endif
         printf("\nErreur 0x%x dans reconstruction evt (ss-prg %s),errl=0x%lx\n"
                , err, "FCTNAME", errl);
         /* ----------------------------------------------------------------- */
         /* Modif du 15 juillet 94  et 1er aout 96 (B. Raine) */
#ifdef FRONTAL
         printf("dump ss_evt en erreur:\n");
         xdmp(200, ss_evt);
      }
      if ((pfacqvar->acq_stat & S_ACQCTRL) == 0)
         /* si pas flag ACQCTRL, on appelle tr_err */
         tr_errl(err, ss_evt->ss_s1_id, (long)errl);
#endif
      /* ----------------------------------------------------------------- */
   }

   if (ctrlform == EVCT_VAR) {
      /* maj longueur evt ctrl */
      ct_evt->ct_len = ctcour - (INT16*)ct_evt;
   }

   /* printf("\nsortie s_evctrl, err=0x%x, GR_OK=0x%x\n", err, GR_OK);  */
   return (err);

}
