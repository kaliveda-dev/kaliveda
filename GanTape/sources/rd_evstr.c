/*   rd_evstr:C -- creation date : 15 sep 89  last update : 26 nov 97
Copyright Acquisition group, GANIL Caen, France
module name   : rd_evstr:C   version  : 06
library    : RAINE:LIB

author     : Bruno RAINE

  function : read event structure file and put it in memory

synopsis : int rd_evstr(sev_filnam, sev_memadd, sev_memlen)
     char *sev_filnam;      evt struct file name
        char *sev_memadd; evt struct. add. in memory
     int   sev_memlen; length in bytes of evt str. mem.

input   : sev_filnam -> nom du fichier contenant la structure evenement
    sev_memadd -> adresse de la zone memoire devant contenir
        la structure evenement
    sev_memlen -> nombre de bytes de la zone structure evt

return  : R_OK (= 0)si structure evenement bien initialisee
    R_SEVFIL  si probleme d'acces au fichier structure
    R_SEVMEM  si zone memoire structure trop petite
       R_SEVCODE si code fct inconnu dans fichier structure evt
    R_SEVSTRUCT si mauvais format de fichier structure

example :

       modifications   :
   fevrier 1994 Bruno PIQUET
  for compilation under Open VMS-AXP, and ANSI-C
          12 aout 96 B. Raine
               remplacement R_OK par GR_OK
  modification des includes
   19 aout 96 B. Raine
  Ajout du case EV_MEMEXT pour traitement table des
  labels coupleur externe
   02 dec 96 - B. Raine
        Restructuration pour execution multiplateforme
   26 nov 97 - B. Raine
        Ajout des codes EV_CTRFIX et EV_CTRVAR
*/
#include <stdio.h>
#ifdef PDOS
#include "GEN_MAC:H"
#include "GEN_TYPE:H"   /* general typedef definitions */
#include "ERR$GAN:H" /* GANIL errors definitions */
#include "STR_EVT:H" /* structure of events definitions */
#else
#ifdef PDOS
#include "GEN_MAC.H"
#endif
#include "GEN_TYPE.H"   /* general typedef definitions */
#if defined ( __unix ) || ( __unix__)
#include "ERR_GAN.H" /* GANIL errors definitions */
#else
/*#include "ERR$GAN.H" GANIL errors definitions */
#error "should not have come here"
#endif
#include "STR_EVT.H" /* structure of events definitions */
#endif

#define LENLINE 132

/* Cadrer valeur sur une frontiere 0x10 pour eviter les "unaligned address" */
#ifdef __STDC__
static unsigned long Cadrer(unsigned long Valeur)
#else
static unsigned long Cadrer(Valeur)
unsigned long Valeur;
#endif
{
   if (Valeur % 0x10) {
      Valeur = (Valeur & ~0xfl); /* en 2 instruction a cause d'un bug */
      Valeur = Valeur + 0x10;    /* du compilateur MRI                */
   }

   return (Valeur);

}

#ifdef __STDC__
int rd_evstr(char* sev_filnam, char* sev_memadd, int sev_memlen)
#else
int rd_evstr(sev_filnam, sev_memadd, sev_memlen)
char* sev_filnam;      /* evt struct file name */
char* sev_memadd;      /* evt struct. add. in memory */
int   sev_memlen;      /* length in bytes of evt str. mem. */
#endif
{
   /*char *fgets();*/
   char buf[LENLINE];
   FILE* fd;
#ifndef __STDC__
   FILE* fopen();
#endif
   int err;
   char* sev_memend;

   int cont, code, strsiz;
   SEV_FCT* sev_fct;    /* Pointeur courant sur structure evenement */
   SEV_FCT* SevFctPrev; /* Pointeur sur code precedent dans structure evenement */
   SEV_FCT* SevFctCpl;  /* Pointeur sur descripteur coupleur courant */
   SEV_STR_HD* SevStrHd; /* Pointeur sur le debut de la memoire structure evt */

   char coupleur[8];
   int nbvsnsa, maxvsnsa, vsn_sa, par_num;
   INT16* par_adr, *start_par, *end_par;
   SEV_COUPL* hdcpl;
   int ind_lam, i;
   INT16* pt16;
   int* ptInt;  /* Pointeur temporaire sur un int */

   err = GR_OK;
   cont = TRUE;

   if ((fd = fopen(sev_filnam, "r")) == NULL)
      err = R_SEVFIL;   /* probleme d'acces au fichier */

   else {
      /* adr fin mem. str. evt */
      sev_memend = sev_memadd + sev_memlen - 4;
      if (fgets(buf, LENLINE, fd) == NULL)
         err = R_SEVFIL;

      else {
         /* a cause du vax, le premier record peut etre un line feed */
         if (buf[0] == 0xa) {
            if (fgets(buf, LENLINE, fd) == NULL)
               err = R_SEVFIL;
         }
      }

      if (err == GR_OK) {
         SevStrHd = (SEV_STR_HD*)sev_memadd;
         sscanf(buf, "%d", &SevStrHd->NbParam);   /* Nombre total de parametres */
         sev_memadd += sizeof(SEV_STR_HD);
         sev_memadd = (char*)Cadrer((unsigned long)sev_memadd);
         SevStrHd->sev_next = (SEV_FCT*)sev_memadd;

         /* Quelques initialisations avant de boucler */
         hdcpl = 0;

         SevFctPrev = (SEV_FCT*)0;
         SevFctCpl = (SEV_FCT*)0;

         sev_fct = (SEV_FCT*)sev_memadd;
         sev_fct->sev_next = (SEV_FCT*)0;
         sev_fct->sev_code = EV_FIN;

         while (fgets(buf, LENLINE, fd) != NULL && cont) {
            if (sev_memadd  + sizeof(SEV_FCT) < sev_memend) {
               /* ok, on a assez de place pour le code suivant*/

               sscanf(buf, "%d", &code);

               switch (code) {
                  case EV_CTRFIX:
                  case EV_CTRVAR:
                     if (SevFctCpl != (SEV_FCT*)0) {
                        /* Ce code doit etre le premier rencontre */
                        err = R_SEVSTRUCT; /* Mauvaise structure du fichier struct */
                        cont = FALSE;
                     } else {
                        sev_fct->sev_code = code;
                        SevFctPrev = sev_fct;
                        sev_fct++;
                     }
                     break;

                  case EV_COUPL:
                     if (SevFctCpl != (SEV_FCT*)0) {
                        /* maj du chainage dans coupleur precedent */
                        hdcpl->cpl_next = sev_fct;
                        SevFctCpl->sev_next = sev_fct;
                     }
                     sscanf(buf, "%d %s %d", &sev_fct->sev_code, coupleur,
                            &sev_fct->sev_par.sev_cpl.cpl_typ);
                     /* cadrer ident du couleur sur byte de poids fort */
                     sev_fct->sev_par.sev_cpl.cpl_typ =
                        sev_fct->sev_par.sev_cpl.cpl_typ << 8;
                     /* sauver adresse coupleur courant */
                     hdcpl = &sev_fct->sev_par.sev_cpl;
                     SevFctCpl = sev_fct++;
                     sev_fct = (SEV_FCT*)Cadrer((unsigned long)sev_fct);
                     /* entree 0 = prochain descripteur, les autres sont mises a 0 */
                     /* lam 0 utilise si pas de code DEBLAM */
                     hdcpl->cpl_deblam[0] = sev_fct;
                     for (i = 1; i <= MAX_LAM; hdcpl->cpl_deblam[i] = 0, i++);
                     hdcpl->cpl_next = 0;
                     SevFctPrev = (SEV_FCT*)0;
                     break;

                  case EV_DEBLAM :
                     sscanf(buf, "%d %d", &code, &ind_lam);
                     /* maj entree table des lams, on ajoute 1 a l'adresse courante
                        a cause du EV_FIN mis entre les structures pour chaque lam */
                     hdcpl->cpl_deblam[ind_lam] = sev_fct;
                     SevFctPrev = (SEV_FCT*)0;
                     break;

                  case EV_FIX :
                     sscanf(buf, "%d %d %d", &sev_fct->sev_code,
                            &sev_fct->sev_par.sev_fix.fix_start,
                            &sev_fct->sev_par.sev_fix.fix_len);

                     /* Maj chainage precedent et adr. prochain descripteur */
                     if (SevFctPrev != (SEV_FCT*)0)
                        SevFctPrev->sev_next = sev_fct;
                     SevFctPrev = sev_fct;
                     sev_fct++;
                     break;

                  case EV_VAR :
                     sscanf(buf, "%d", &sev_fct->sev_code);

                     /* Maj chainage precedent et adr. prochain descripteur */
                     if (SevFctPrev != (SEV_FCT*)0)
                        SevFctPrev->sev_next = sev_fct;
                     SevFctPrev = sev_fct;
                     sev_fct++;
                     break;

                  case EV_MEMEXT:
                  /* La table de correspondance pour memoire de type coupleur
                     externe est du meme type qu'une table FERA, donc meme
                     traitement  */
                  case EV_FERA :
                     sscanf(buf, "%d %d %d", &sev_fct->sev_code, &maxvsnsa, &nbvsnsa);
                     strsiz = sizeof(SEV_FCT) + maxvsnsa + 1;
                     if (sev_memadd + strsiz + 1 < sev_memend) {
                        sev_fct->sev_par.sev_fera.fer_maxad = maxvsnsa;
                        /* Maj chainage precedent et adr. prochain descripteur */
                        if (SevFctPrev != (SEV_FCT*)0)
                           SevFctPrev->sev_next = sev_fct;
                        SevFctPrev = sev_fct;
                        sev_fct++;
                        SevFctPrev->sev_par.sev_fera.fer_label = (INT16*)sev_fct;
                        start_par = (INT16*)sev_fct;
                        end_par = start_par + maxvsnsa + 1;
                        for (par_adr = start_par; par_adr < end_par; *par_adr++ = 0);
                        do {
                           if (fgets(buf, LENLINE, fd) != NULL) {
                              sscanf(buf, "%d %d", &vsn_sa, &par_num);
                              par_adr = start_par + vsn_sa;
                              *par_adr = par_num;
                           } else {
                              err = R_SEVEOF;  /* fin de fichier non attendue */
                              cont = FALSE;
                           }
                           nbvsnsa--;
                        } while ((nbvsnsa > 0) && cont);

                        sev_fct = (SEV_FCT*)end_par;
                     }

                     else {
                        err = R_SEVMEM;   /* buffer structure evenement trop petit */
                        cont = FALSE;
                     }
                     break;

                  default :
                     printf("\nrd_evstr: code %d inconnu dans fichier structure"
                            , code);
                     err = R_SEVCODE;    /* code inconnu */
                     cont = FALSE;
               }
            } else {
               err = R_SEVMEM;  /* buffer structure evenement trop petit */
               cont = FALSE;
            }

            sev_fct = (SEV_FCT*)Cadrer((unsigned long)sev_fct);
            sev_memadd = (char*)sev_fct;
            sev_fct->sev_next = (SEV_FCT*)0;
            sev_fct->sev_code = EV_FIN;

         } /* end while */
      }
      fclose(fd);
   }
   if (err != GR_OK)
      printf("\ncode erreur sortie rd_evstr (en hexa) = %x", err);

   return (err);
}
