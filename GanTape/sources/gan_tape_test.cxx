/*****************************************************************************
 *****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_TEST.c                                     *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  22 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                       Modifie en mars 1997 pour implementation SUN et HP  *
 *                                                                           *
 *       Objet        :  Realisation des fonctions de test pour la gestion   *
 *                    des bandes : acq_dev_is_tape, get_dev_type,            *
 *                    create_descriptor                                      *
 *                                                                           *
 *****************************************************************************
 *****************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gan_tape_test.h"
#include "gan_tape_general.h"

#include "gan_tape_erreur.h"


#if defined ( __unix__ ) || defined ( __unix )  /***** Cas de UNIX *****/
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mtio.h>

/* Pour tests CC sous solaris B. Raine le 21/4/04
//extern int ioctl (int __fildes, int __cmd, struct mtget *);
//extern int close(int);
*/

#ifdef __osf__
#include <io/common/devio.h> /* Pour DEVGET (fonctions de 'status' de bande) */
#endif


#elif defined ( __VMS ) || defined ( VMS )       /***** Cas du VMS ******/

#include <descrip.h>            /*  Fichiers necessaires pour l'utilisation */
#include <dcdef.h>
#include <devdef.h>
#include <dvidef.h>             /* des librairies de VMS                    */
#include <lib$routines.h>
#include <ssdef.h>

#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_dev_is_tape_c                                   *
 *           Objet     : Teste si un nom correspond a une bande .            *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur sous VMS et sous UNIX.                  *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - ACQ_ERRPARAM si le fichier n'existe pas.   *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/


int acq_dev_is_tape_c(gan_tape_desc DeviceName)
{

   int Status;

#if defined ( __VMS ) || defined ( VMS )

   int Value;
   DESC Desc_In;

   Desc_In = create_descriptor(DeviceName.DevName);
   Status = lib$getdvi(&DVI$_DEVCLASS, 0, &Desc_In, &Value, 0, 0);

   if (Status == SS$_NORMAL)

      if (Value == DC$_TAPE)
         Status = ACQ_OK;
      else Status = ACQ_ISNOTATAPE;

   else if (Status == SS$_NOSUCHDEV)
      Status = ACQ_ERRPARAM;


#elif defined ( __unix__ ) || defined ( __unix )


#ifdef __osf__  /* differences entre OSF et SUN */
   struct devget Devget;

   Status = ioctl(DeviceName.Lun, DEVIOCGET, &Devget);
   if (Status != -1)
      if (Devget.category  == DEV_TAPE)

#elif defined ( __sun__ ) || defined ( __sun ) || defined ( __hpux ) || defined ( __linux )

   struct mtget Mtget;
   Status = ioctl(DeviceName.Lun, MTIOCGET, &Mtget);
   if (Status != -1)
      if (Mtget.mt_type != 0)

#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/

         Status = ACQ_OK;
      else Status = ACQ_ISNOTATAPE;
   else Status = ACQ_ERRPARAM;
#endif

   return (Status);
}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_get_tape_type_c                                 *
 *           Objet     : Retourne le type de device correspondant.           *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur sous VMS et sous UNIX.                  *
 *                       ChaineRetour -- Chaine de retour du nom de device   *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/


int acq_get_tape_type_c(gan_tape_desc DeviceName, char* ChaineRetour)
{


   int Status;

#if defined ( __VMS ) || defined ( VMS )

   int Leng;
   DESC Desc_In, Desc_Out;

   Desc_In  = create_descriptor(DeviceName.DevName);
   Desc_Out = create_descriptor(ChaineRetour);
   /* Regle un proble d'assignation de taille dans le descripteur    */
   Desc_Out.dsc$w_length = DVI_TAILLE;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {
      Status = lib$getdvi(&DVI$_MEDIA_NAME, 0, &Desc_In, 0, &Desc_Out, &Leng);

      if (Status == SS$_NORMAL)   {
         int Index = 0;       /* Ajoute un caractere de fin de chaine */
         while (ChaineRetour[Index++] != ' ') {}
         ChaineRetour[--Index] = '\0';
         Status = ACQ_OK;
      }
   } else Status = ACQ_ISNOTATAPE;

   /* Cette fonctionnalite n'est pas utilisee sous UNIX ... */
#elif defined ( __osf__ )

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      struct devget Devget;

      Status = ioctl(DeviceName.Lun, DEVIOCGET, &Devget);
      if (Status != -1) {
         strcpy(ChaineRetour, Devget.device);
         Status = ACQ_OK;
      } else Status = ACQ_ERRPARAM;
   } else Status = ACQ_ISNOTATAPE;


#else

   strcpy(ChaineRetour, "INCONNU ");
   Status = ACQ_OK;

#endif

   return (Status);

}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_real_dev_name_c                                 *
 *           Objet     : Retourne le nom physique ( noeud + device ).        *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur sous VMS et sous UNIX.                  *
 *                       ChaineRetour -- Chaine de retour du nom physique.   *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/


int acq_real_dev_name_c(gan_tape_desc DeviceName, char* ChaineRetour)
{


   int Status;

#if defined ( __VMS ) || defined ( VMS )

   bool PP;
   int Leng;
   DESC Desc_In, Desc_Out;

   Desc_In  = create_descriptor(DeviceName.DevName);
   Desc_Out = create_descriptor(ChaineRetour);
   Desc_Out.dsc$w_length = DVI_TAILLE;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {
      Status = lib$getdvi(&DVI$_DEVNAM, 0, &Desc_In, 0, &Desc_Out, &Leng);

      if (Status == SS$_NORMAL)   {
         int Index = 0;       /* Ajoute un caractere de fin de chaine */
         while (ChaineRetour[Index] != ' ') {
            if (ChaineRetour[Index] == ':') PP = true;
            else PP = false;
            Index++;
         }
         if (PP == true)
            ChaineRetour[Index] = '\0';
         else {
            ChaineRetour[Index] = ':';
            ChaineRetour[Index + 1] = '\0';
         }

         Status = ACQ_OK;
      }
   } else Status = ACQ_ISNOTATAPE;

#else

   strcpy(ChaineRetour, DeviceName.DevName);
   Status = ACQ_OK;

#endif
   return (Status);

}



/*****************************************************************************
 *                                                                           *
 *           Function  : acq_dev_is_alloc_c                                  *
 *           Objet     : Teste si le device est alloue.                      *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur sous VMS et sous UNIX.                  *
 *                       sous UNIX.                                          *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si le device est alloue.            *
 *                              - ACQ_NOTALLOC dans le cas d'un test negatif.*
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_dev_is_alloc_c(gan_tape_desc DeviceName)
{

   int Status;

#if defined ( __VMS ) || defined ( VMS )

   int Value;
   DESC Descr_In;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Descr_In = create_descriptor(DeviceName.DevName);
      Status = lib$getdvi(&DVI$_DEVCHAR, 0, &Descr_In, &Value, 0, 0);

      if (Status == SS$_NORMAL)

         if ((Value & DEV$M_ALL) != DEV$M_ALL)
            Status = ACQ_NOTALLOC;
         else Status = ACQ_OK;

   } else Status = ACQ_ISNOTATAPE;

#else

   /* Fonction transparente sous UNIX     */
   Status = ACQ_OK;

#endif

   return Status;

}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_dev_is_mount_c                                  *
 *           Objet     : Teste si le device est monte.                       *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur sous VMS et sous UNIX.                  *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si le device est monte.             *
 *                              - ACQ_NOTALLOC dans le cas d'un test negatif.*
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_dev_is_mount_c(gan_tape_desc DeviceName)
{

   int Status;

#if defined ( __VMS ) || defined ( VMS )

   int Value;
   DESC Descr_In;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Descr_In = create_descriptor(DeviceName.DevName);
      Status = lib$getdvi(&DVI$_DEVCHAR, 0, &Descr_In, &Value, 0, 0);

      if (Status == SS$_NORMAL)

         if ((Value & DEV$M_MNT) != DEV$M_MNT)
            Status = ACQ_NOTMOUNT;
         else Status = ACQ_OK;

   }

   else Status = ACQ_ISNOTATAPE;

#else

   /* Fonction transparente sous UNIX     */
   Status = ACQ_OK;

#endif

   return Status;

}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_dev_is_wr_protect_c                             *
 *           Objet     : Teste si le device est protege en ecriture.         *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'un         *
 *                       descripteur sous VMS et sous UNIX.                  *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si le device est Read/Write.        *
 *                              - ACQ_DEVWRITLOCK si protege en ecriture.    *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_dev_is_wr_protect_c(gan_tape_desc DeviceName)
{

   int Status;

#if defined ( __VMS ) || defined ( vms )

   int Value;
   DESC Descr_In;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Descr_In = create_descriptor(DeviceName.DevName);
      Status = lib$getdvi(&DVI$_DEVCHAR, 0, &Descr_In, &Value, 0, 0);

      if (Status == SS$_NORMAL)

         if ((Value & DEV$M_SWL) == DEV$M_SWL)
            Status = ACQ_DEVWRITLOCK;
         else Status = ACQ_OK;

   } else Status = ACQ_ISNOTATAPE;


#elif defined ( __osf__ )

   struct devget Devget;

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {
      Status = ioctl(DeviceName.Lun, DEVIOCGET, &Devget);
      if (Status != -1)
         if ((Devget.stat) & WR_PROTECT  == WR_PROTECT)
            Status = ACQ_DEVWRITLOCK;
         else Status = ACQ_OK;
      else Status = ACQ_ERRPARAM;
   } else Status = ACQ_ISNOTATAPE;

#else

   int LunTemp;
   Status = open(DeviceName.DevName, O_RDONLY);
   if (!(Status <= 0)) {   /* Le fichier ou la bande existe bien */
      Status = close(Status);
      Status = open(DeviceName.DevName, O_WRONLY);
      if (!(Status <= 0)) {  /* Non protege en ecriture */
         Status = close(Status);
         Status = ACQ_OK;
      } else Status = ACQ_DEVWRITLOCK;
   }

#endif

   return Status;

}



/*****************************************************************************
 *                                                                           *
 *           Function  : create_descriptor                                   *
 *           Objet     : Retourne un descripteur de caracteres pour utiliser *
 *                       les fonctions systeme de VMS.                       *
 *                       -- Specifique a VMS --                              *
 *                                                                           *
 *           Entree    : Name   -- Chaine de caracteres allouee au           *
 *                       descripteur.                                        *
 *                                                                           *
 *           Retour    : Descripteur cree.                                   *
 *                                                                           *
 *****************************************************************************/



#if defined ( __VMS ) || defined ( VMS )
/* Pour le passage par descripteur sur fonctions VMS SYS$ ou LIB$ */

DESC create_descriptor(char* Name)
{

   DESC Tempo;

   Tempo.dsc$w_length  = strlen(Name);
   Tempo.dsc$a_pointer = Name;
   Tempo.dsc$b_class   = DSC$K_CLASS_S;
   Tempo.dsc$b_dtype   = DSC$K_DTYPE_T;

   return (Tempo);
}

#endif


/******************************** END  ***************************************/
