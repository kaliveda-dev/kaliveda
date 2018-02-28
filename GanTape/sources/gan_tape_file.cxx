/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_FILE.c                                     *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  15 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Fonctions de gestion des bandes sous VMS ou U~~X.   *
 *                       avec swap des donnees si buffer acquisition GANIL   *
 *                       et options de swap positionnees dans param.h        *
 *                                                                           *
 *       Fonctions    :  open, close, read, write, skip.                     *
 *                                                                           *
 *       Modifications :                                                     *
 *            - 23 sep 98 B. Raine                                           *
 *               dans fonction  acq_mt_open_c , si VMS, on remplace l'option *
 *               fop=rwo par fop=pos pour ne pas rembobiner la bande a       *
 *               chaque ouverture                                          *
 *                                                                           *
 *****************************************************************************/


#define __GAN_TAPE_FILE_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gan_tape_file.h"
#include "gan_tape_test.h"
#include "gan_tape_general.h"
#include "gan_tape_param.h"
#include "GanTape/GEN_TYPE.H"
#include "GanTape/gan_acq_buf.h"
#include "gan_acq_swap_buf.h"

#include "gan_tape_erreur.h"

#include <unistd.h>

#if defined  ( __VMS ) || ( VMS )

#include <unixio.h>        /* Fichiers include specifiques a DECC */
#include <file.h>

#include <iodef.h>
#include <libdef.h>
#include <lib$routines.h>
#include <ssdef.h>

extern int sys$assign();
extern int sys$dassgn();
extern int sys$qiow();


#elif defined ( __unix__ ) || ( __unix )               /***** Cas de UNIX *****/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <fcntl.h>


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/

/* Compilation using RFIO or Xrootd to read remote data files */
#if defined (CCIN2P3_RFIO)
#include "shift.h"
#endif
#if defined (CCIN2P3_XRD)
#include "XrdPosix.hh"
#endif

/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_open_c                                       *
 *           Objet     : Ouverture d'un fichier sur bande ou sur disque.     *
 *                       !!! Pour une bande, le device doit etre alloue et   *
 *                       monte.                                              *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'une chaine *
 *                       de caracteres.                                      *
 *                       RWMode     -- Ouverture en lecture ou ecriture.     *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_NOTMOUNT si le device n'est pas monte. *
 *                              - ACQ_ERRPARAM si un des parametres d'entree *
 *                                est errone.                                *
 *                       Lun    -- Pointeur sur le descripteur de fichier    *
 *                              ouvert.                                      *
 *           A la suite d'un appel a acq_mt_open_c, le champ Is_protected    *
 * de gan_tape_desc est mis a jour et indique alors si l'on peut        *
 * ecrire des donnees sur la bande.               *
 *                              *
 *****************************************************************************/

int acq_mt_open_c(gan_tape_desc* DeviceName, mode_r_w RWMode, int* Lun)
{


   int Status, Mode;
   char ChaineRet [MAX_CARACTERES];

#if defined ( __VMS ) || defined ( VMS )
   Status = acq_dev_is_mount_c(*DeviceName);
   if (Status == ACQ_OK || Status == ACQ_ISNOTATAPE)  {
#endif

      if (RWMode == o_read) {
         Mode = O_RDONLY;    /* Ouverture en Lecture seule ou en     */
         Status = ACQ_OK;
      } else if (RWMode == o_write) {
         /* Une Ouverture en WRITE ne peut se faire que si la bande le permet */
         Status = acq_dev_is_wr_protect_c(*DeviceName);
         if (Status == ACQ_DEVWRITLOCK) {
            DeviceName->Is_protected = true;
            Status = ACQ_DEVWRITLOCK;
         } else if (Status == ACQ_ISNOTATAPE || Status == ACQ_OK) {
            DeviceName->Is_protected = false;
            Status = ACQ_OK;
         }
         Mode = O_WRONLY;    /* ecriture seule...                    */
      } else Status = ACQ_ERRPARAM;

      if (Status == ACQ_OK) {

         /* Les parametres d'ouverture d'un fichier ne sont pas portables depuis VMS */
#if defined ( __VMS ) || defined ( VMS )
         Status = acq_real_dev_name_c(*DeviceName, ChaineRet);
         if (Status == ACQ_OK)
            /* B. Raine le 22/9/98  remplace fop=rwo (option rewind)
               par fop=pos pour ouvrir la bande en position courante */
            Status = open(ChaineRet, Mode, 666, "bls=16384", "ctx=rec", "fop=pos",
                          "mrs=32764");
         else if (Status == ACQ_ISNOTATAPE)
            Status = open(DeviceName->DevName, Mode, 666, "bls=16384", "ctx=rec",
                          "fop=rwo", "mrs=32764");
         else return (Status);

#elif defined ( __unix__ ) || ( __unix )
         Status = open(DeviceName->DevName, Mode);


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


         if (!(Status < 0)) {
            *Lun = Status;      /* Renvoie la valeur du LUN */
            Status = ACQ_OK;
         }


#if defined ( __VMS ) || defined ( VMS )
      } else Status = ACQ_NOTMOUNT;
#endif

   }
   DeviceName->Lun = *Lun;
   return (Status);

}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_close_c                                      *
 *           Objet     : Fermeture d'un fichier sur bande ou sur disque.     *
 *                       !!! Pour une bande, le device doit etre alloue et   *
 *                       monte.                                              *
 *                                                                           *
 *           Entree    : Lun  -- Descripteur du fichier a fermer             *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ERRPARAM si le lun d'entree est errone *
 *                                ou s'il y a un probleme de fermeture.      *
 *                                                                           *
 *****************************************************************************/

int acq_mt_close_c(gan_tape_desc DeviceName)
{


   int Status;

   Status = close(DeviceName.Lun);

   if (Status < 0)
      Status = ACQ_ERRPARAM;
   else Status = ACQ_OK;

   return (Status);
}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_read_c                                       *
 *           Objet     : Lecture par blocs sur un disque ou une bande.       *
 *                       !!! Le fichier doit etre ouvert.                    *
 *                       !!! Pour une bande, le device doit etre alloue et   *
 *                       monte.                                              *
 *                                                                           *
 *           Entree    : Lun    -- Descripteur du fichier a lire.            *
 *                       Buffer -- Buffer de stockage des donnees lues.      *
 *                       Taille -- Taille du block a lire.                   *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ERRPARAM si un parametre d'entree est  *
 *                                errone.                                    *
 *                              - ACQ_ENDOFFILE si la fin du fichier est     *
 *                                atteinte.                                  *
 *                              - ACQ_ERRDURREAD si erreur de lecture.       *
 *                              - ACQ_UNKBUF si type de buffer inconnu       *
 *                              - ACQ_UNKMSB si MSB/LSB non determine        *
 *                                           pour autoswap (cf param.h)      *
 *                                                                           *
 *                       Taille -- Pointeur sur le nombre de caracteres lus. *
 *                                                                           *
 *****************************************************************************/

int acq_mt_read_c(int Lun, char* Buffer, int* Taille)
{

   int Status;

   if (*Taille > 0) {
      Status = read(Lun, Buffer, *Taille);

      *Taille = Status; /* Pour retourner nombre de caracteres lus */
      if (Status > 0)
         Status = acq_swap_buf((in2p3_buffer_struct*)Buffer, *Taille);

      else if (Status == 0)
         Status = ACQ_ENDOFFILE;
      else Status = ACQ_ERRDURREAD;;
   } else Status = ACQ_ERRPARAM;

   return (Status);

}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_write_c                                      *
 *           Objet     : Ecriture par blocs sur un disque ou une bande.      *
 *                       !!! Le fichier doit etre ouvert.                    *
 *                       !!! Pour une bande, le device doit etre alloue et   *
 *                       monte.                                              *
 *                                                                           *
 *           Entree    : Lun    -- Descripteur du fichier a lire.            *
 *                       Buffer -- Buffer de stockage des donnees a ecrire.  *
 *                       Taille -- Taille du block a lire.                   *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ERRPARAM si un parametre d'entree est  *
 *                                errone.                                    *
 *                              - ACQ_ENDOFFILE si la fin du fichier est     *
 *                                atteinte.                                  *
 *                                                                           *
 *****************************************************************************/

int acq_mt_write_c(int Lun, char* Buffer, int Taille)
{

   int Status;

   Status = write(Lun, Buffer, Taille);
   if (Status > 0)
      Status = ACQ_OK;
   else if (Status == 0)
      Status = ACQ_ENDOFFILE;
   else Status = ACQ_ERRPARAM;
   return (Status);
}



/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_rewind_c                                     *
 *           Objet     : Reembobine une bande.                               *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur utilisable sous VMS et UNIX.            *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si ce n'est pas une bande.  *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_mt_rewind_c(gan_tape_desc DeviceName)
{

   int Status;

#if defined ( __VMS ) || defined ( VMS )


   int Channel, Ass_Status;
   DESC Descr_In;
   short int Iosb[4];

   Descr_In = create_descriptor(DeviceName.DevName);

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      /* Assignation d'un canal de communication      */
      Ass_Status = sys$assign(&Descr_In, &Channel, 0, 0, 0);
      if (Ass_Status == SS$_NORMAL) {
         /* Appel de REWIND   */
         Status = sys$qiow(0, Channel, IO$_REWIND, Iosb,
                           0, 0, 0, 0, 0, 0, 0, 0);
         if (Status == SS$_NORMAL && Iosb[0] == SS$_NORMAL)
            Status = ACQ_OK;
         else Status = Iosb[0];

         Ass_Status = sys$dassgn(Channel);

      }
      /* Deassigne le canal   */
      if (Ass_Status != SS$_NORMAL) Status = Ass_Status;

   } else Status = ACQ_ISNOTATAPE;

#elif defined ( __unix__ ) || ( __unix ) /* Coherence a verifier entre les differents UniX  */

   struct mtop Control;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Control.mt_op    = MTREW;
      Control.mt_count = 1; /* Nombre d'actions a effectuer */
      Status = ioctl(DeviceName.Lun, MTIOCTOP, &Control);
      if (Status != -1)
         Status = ACQ_OK;
   } else Status = ACQ_ISNOTATAPE;


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


   /**** On traite maintenant le cas d'un fichier disque
   ****/
   if (Status == ACQ_ISNOTATAPE) {
      Status = lseek(DeviceName.Lun, 0, SEEK_SET);
      if (Status < 0)
         Status = ACQ_ERRPARAM;
      else Status = ACQ_OK;
   }

   return (Status);
}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_skip_file_c                                  *
 *           Objet     : Saute 'n' fichiers sur une bande.                   *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur utilisable sous VMS et UNIX.            *
 *                       NombreSkip -- Nombre de fichiers a passer.          *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ENDOFTAPE si fin de bande atteinte.    *
 *                              - ACQ_ISNOTATAPE si c'est un disque.         *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_mt_skip_file_c(gan_tape_desc DeviceName, int NombreSkip)
{

   int Status;


#if defined ( __VMS ) || defined ( VMS )

   int Ass_Status, Channel;
   DESC Descr_In;
   short int Iosb[4];

   Descr_In = create_descriptor(DeviceName.DevName);

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      if (NombreSkip < 0) NombreSkip--;   /* Pour faire un skip de */
      /* n fichiers vers le BOT, il faut passer n+1 Tape_Mark.     */

      Ass_Status = sys$assign(&Descr_In, &Channel, 0, 0, 0);

      if (Ass_Status == SS$_NORMAL) {
         printf("\n>>>Skipping %d files on disk %s ...", NombreSkip,
                DeviceName.DevName);
         Status = sys$qiow(0, Channel, IO$_SKIPFILE, Iosb,
                           0, 0, NombreSkip, 0, 0, 0, 0, 0);
         if (Status == SS$_NORMAL && Iosb[0] == SS$_NORMAL) {

            Status = ACQ_OK;

            if (NombreSkip < 0 && (Iosb[1] + NombreSkip) == 0) {
               int Tape_Mark = 1;  /* Pour passer outre le tape mark */

               Status = sys$qiow(0, Channel, IO$_SKIPRECORD, Iosb,
                                 0, 0, Tape_Mark, 0, 0, 0, 0, 0);
               if (Status == SS$_NORMAL && Iosb[0] == SS$_NORMAL)
                  Status = ACQ_OK;
               else Status = Iosb[0];
            }
         } else Status = Iosb[0];

         Ass_Status = sys$dassgn(Channel);
         if (Ass_Status != SS$_NORMAL)
            Status = Ass_Status;
      }

      else Status = Ass_Status;

      if (Status == SS$_ENDOFTAPE)
         Status = ACQ_ENDOFTAPE;
      if (Status == SS$_ENDOFFILE)
         Status = ACQ_OK;

   } else Status = ACQ_ISNOTATAPE;


#elif defined ( __unix__ ) || ( __unix )

   struct mtop Control;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      if (NombreSkip < 0) {                 /* Ligne 388 */
         Control.mt_count = -NombreSkip;
         Control.mt_op     = MTBSF;
      } else {
         Control.mt_count = NombreSkip;
         Control.mt_op     = MTFSF;
      }
      Status = ioctl(DeviceName.Lun, MTIOCTOP, &Control);
      if (Status != -1)
         Status = ACQ_OK;
      else Status = ACQ_ENDOFTAPE;
   } else Status = ACQ_ISNOTATAPE;


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


   return (Status);
}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_skip_block_c                                 *
 *           Objet     : Saute 'n' blocs sur une bande.                      *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur utilisable sous VMS et UNIX.            *
 *                       NombreSkip -- Nombre de blocs a passer.             *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ENDOFFILE si fin de fichier atteinte.  *
 *                              - ACQ_ISNOTATAPE si c'est un disque.         *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_mt_skip_block_c(gan_tape_desc DeviceName, int NombreSkip)
{

   int Status;

   int BufSize = BUFSIZE;

#if defined ( __VMS ) || defined ( VMS )

   DESC Descr_In;
   int Ass_Status, Channel;
   short Iosb[4];
   bool Continue = true;

   Descr_In = create_descriptor(DeviceName.DevName);

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Ass_Status = sys$assign(&Descr_In, &Channel, 0, 0, 0);
      if (Ass_Status == SS$_NORMAL) {

         int Loc_Skip = NombreSkip ;

         Status = ACQ_OK;
         while (Continue) {

            if (Loc_Skip < MIN_SKIP_BLOCK ||
                  Loc_Skip > MAX_SKIP_BLOCK) {

               /* Si le nombre de total de blocs a passer est > 32767,  */
               /* skip passe par des valeurs intermediaires qui sont   */
               /* determines a la ligne suivante en fonction du signe.  */
               int Tempo_Skip = Loc_Skip < 0 ? MIN_SKIP_BLOCK :
                                MAX_SKIP_BLOCK;
               Continue = true;
               Loc_Skip = Loc_Skip - Tempo_Skip; /* Skip intermediaire */
               NombreSkip = Tempo_Skip;
            } else {
               NombreSkip = Loc_Skip;
               Continue = false;
            }
            printf("\n>>>skipping %d blocks on disk %s ...", NombreSkip
                   , DeviceName.DevName);
            Status = sys$qiow(0, Channel, IO$_SKIPRECORD, Iosb,
                              0, 0, NombreSkip, 0, 0, 0, 0, 0);

            if (Status == SS$_NORMAL && Iosb[0] == SS$_NORMAL)
               Status = ACQ_OK;
            else Status = Iosb[0];

            if (Iosb[0] == SS$_ENDOFFILE) Continue = false;

         }

         Ass_Status = sys$dassgn(Channel);
         if (Ass_Status != SS$_NORMAL)
            Status = Ass_Status;
      }

      else Status = Ass_Status;

      if (Status == SS$_ENDOFFILE)
         Status = ACQ_ENDOFFILE;
   }

   else Status = ACQ_ISNOTATAPE; /* C'est un disque */


#elif defined ( __unix__ ) || ( __unix )

   struct mtop Control;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      if (NombreSkip < 0) {
         Control.mt_count = -NombreSkip;
         Control.mt_op     = MTBSR;
      } else {
         Control.mt_count = NombreSkip;
         Control.mt_op     = MTFSR;
      }
      Status = ioctl(DeviceName.Lun, MTIOCTOP, &Control);
      if (Status != -1)
         Status = ACQ_OK;
      else Status = ACQ_ENDOFFILE;
   } else Status = ACQ_ISNOTATAPE;


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/

   if (Status == ACQ_ISNOTATAPE) {
      Status = lseek(DeviceName.Lun, BufSize * NombreSkip, SEEK_CUR);
      if (Status < 0)
         Status = ACQ_ERRPARAM;
      else Status = ACQ_OK;
   }

   return (Status);
}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_skip_to_eot_c                                *
 *           Objet     : Deroule un bande jusqu'a la fin et renvoi le        *
 *                       nombre de fichiers passes.                          *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur utilisable sous VMS et UNIX.            *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si c'est un disque.         *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                       NombreSkip -- Pointeur sur le nombre de fichiers    *
 *                                     passes.                               *
 *                                                                           *
 *****************************************************************************/

int acq_mt_skip_to_eot_c(gan_tape_desc DeviceName, int* NombreSkip)
{

   int Status = 0;

#if defined ( __VMS ) || defined ( VMS )

   int Ass_Status, Channel;
   DESC Descr_In;
   short Iosb[4];

   Descr_In = create_descriptor(DeviceName.DevName);

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Ass_Status = sys$assign(&Descr_In, &Channel, 0, 0, 0);
      if (Ass_Status == SS$_NORMAL) {

         Status = ACQ_OK;
         *NombreSkip = MAX_SKIP_FILE;
         Status = sys$qiow(0, Channel, IO$_SKIPFILE, Iosb,
                           0, 0, NombreSkip, 0, 0, 0, 0, 0);

         if (Status == SS$_NORMAL)
            if (Iosb[0] == SS$_ENDOFVOLUME) {
               Status = ACQ_OK;
               *NombreSkip = Iosb[1];
            } else {
               Status = Iosb[0];
               *NombreSkip = -1;
            } else *NombreSkip = -1;

         Ass_Status = sys$dassgn(Channel);
         if (Ass_Status != SS$_NORMAL)
            Status = Ass_Status;

      }

      else Status = Ass_Status;
   } else Status = ACQ_ISNOTATAPE;


#elif defined ( __unix__ ) || ( __unix )

   struct mtop Control;
   int NbrTempo = 0;

   *NombreSkip = 0;
   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Control.mt_count = 1;
      Control.mt_op    = MTFSF;
      while (Status != - 1) {   /* Atteint la fin de bande    */
         Status = ioctl(DeviceName.Lun, MTIOCTOP, &Control);
         if (Status != -1)
            NbrTempo++;
      }

      NbrTempo --;   /* Pour supprimer le double 'Tape Mark' de fin  */
      Status = ACQ_OK;
      *NombreSkip = NbrTempo;
   } else Status = ACQ_ISNOTATAPE;

#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


   return (Status);
}


/******************************** END  ***************************************/
