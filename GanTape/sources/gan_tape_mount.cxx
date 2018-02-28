/*****************************************************************************
 *****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_MOUNT.c                                    *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  22 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Fonctions de gestion des bandes sous VMS ou U~~X.   *
 *       Fonctions    :  Mount, DisMount                                     *
 *       Modifications :                                                     *
 *          B. Raine le 28 janvier 98                                        *
 *    Dans acq_mount, on test tous les TZ en meme temps            *
 *                                                                           *
 *****************************************************************************
 *****************************************************************************/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "GanTape/GEN_TYPE.H"
#include "gan_tape_mount.h"
#include "gan_tape_test.h"
#include "gan_tape_general.h"
#include "gan_tape_param.h"
#include "GanTape/gan_acq_buf.h"

#include "gan_tape_erreur.h"

#if defined ( __VMS ) || defined ( VMS )

#include <dmtdef.h>
#include <lib$routines.h>
#include <mntdef.h>
#include <ssdef.h>

extern int sys$mount();
extern int sys$dismou();

#endif

/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_mount_c                                      *
 *           Objet     : Montage d'un device alloue au precessus appelant.   *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'une chaine *
 *                       de caracteres sous VMS et sous UNIX.                *
 *                       Densit     -- Densite du device.                    *
 *                       BlockSize  -- Taillel de blocs voulue.              *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - ACQ_NOTALLOC si le device est alloue a un  *
 *                                autre processus ou n'est pas alloue.       *
 *                              - ACQ_ALREADYMOUNT si le device est deja     *
 *                                monte par un autre processus.              *
 *                              - ACQ_ERRPARAM si un des parametres d'entree *
 *                                est errone.                                *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/


int acq_mt_mount_c(gan_tape_desc DeviceName, int Densit, int BlocSize)
{

   int Status;

#if defined ( __VMS ) || defined ( VMS )


   bool Un_TZ;
   int Flag[2];
   item_list Itm_Lst[5];
   char Nom[MAX_CARACTERES];

   Status = acq_dev_is_tape_c(DeviceName);
   if (Status == ACQ_ISNOTATAPE)
      Status = ACQ_ISNOTATAPE;   /***  Ce device est un disque  ***/

   else if (Status  == ACQ_ERRPARAM)
      Status = ACQ_ERRPARAM;

   else if (acq_dev_is_alloc_c(DeviceName) != ACQ_OK)
      Status = ACQ_NOTALLOC;

   else if (acq_dev_is_mount_c(DeviceName) == ACQ_OK)
      Status = ACQ_ALREADYMOUNT;

   else if ((Densit != MIN_DENSIT && Densit != MED_DENSIT &&
             Densit != MAX_DENSIT) && (BlocSize < IN2P3_MIN_STORE_BUF_SIZE ||
                                       BlocSize > IN2P3_MAX_STORE_BUF_SIZE ||
                                       BlocSize > GANIL_MAX_STORE_BUF_SIZE))

      Status = ACQ_ERRPARAM;

   else {
      Status = acq_get_tape_type_c(DeviceName, Nom);

      if (Status == ACQ_OK) {
         /* On test tous les TZ8x en meme temps (B. Raine 28/01/98) */
         Un_TZ = strncmp(Nom, "TZ8", 3);       /* Test si TZ8x  */
         if (Un_TZ != 0) Un_TZ = false;
         else Un_TZ = true;

         Itm_Lst[0].Item_Code = MNT$_DEVNAM;
         Itm_Lst[0].Buf_Len  = strlen(DeviceName.DevName);
         Itm_Lst[0].Buf_Adr  = DeviceName.DevName;

         Flag[0] = MNT$M_FOREIGN | MNT$M_NOASSIST |
                   MNT$M_TAPE_DATA_WRITE;
         if (Un_TZ == true)
            Flag[1] = MNT2$M_COMPACTION;
         else    Flag[1] = MNT2$M_NOCOMPACTION;

         Itm_Lst[1].Item_Code = MNT$_FLAGS; /*  Passage des flags  */
         Itm_Lst[1].Buf_Len  = sizeof(Flag);
         Itm_Lst[1].Buf_Adr  = &Flag;

         Itm_Lst[2].Item_Code = MNT$_BLOCKSIZE; /*  Taille des blocs  */
         Itm_Lst[2].Buf_Len  = sizeof(BlocSize);
         Itm_Lst[2].Buf_Adr  = &BlocSize;

         if (Un_TZ == true) {
            Itm_Lst[3].Item_Code = 0; /* Fin de liste */
            Itm_Lst[3].Buf_Len  = 0;
         } else {
            Itm_Lst[3].Item_Code = MNT$_DENSITY;
            Itm_Lst[3].Buf_Len  = sizeof(Densit);
            Itm_Lst[3].Buf_Adr  = &Densit;
            Itm_Lst[4].Item_Code = 0; /* Fin de liste */
            Itm_Lst[4].Buf_Len  = 0;
         }

         Status = sys$mount(&Itm_Lst);
         if (Status == SS$_NORMAL)
            Status = ACQ_OK;
      }
   }

#elif defined ( __unix__ ) || ( __unix )

   Status = ACQ_OK;


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/

   return (Status);
}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_dismount_c                                   *
 *           Objet     : Demontage d'un device alloue au precessus appelant. *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'une chaine *
 *                       de caracteres sous VMS et sous UNIX.                *
 *                       Option     -- Parametre indiquant si il faut        *
 *                       decharger ou non la bande au demontage.             *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - ACQ_ERRPARAM si Option != 1 ou != 2.       *
 *                              - ACQ_NOTMOUNT si le device n'est pas monte. *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/


int acq_mt_dismount_c(gan_tape_desc DeviceName, int Option)
{


   int Status;

#if defined ( __VMS ) || defined ( VMS )


   DESC Descri_In;
   int Flag;

   Descri_In = create_descriptor(DeviceName.DevName);

   if (Option == 1)      Flag = DMT$M_UNLOAD;
   else if (Option == 2)  Flag = DMT$M_NOUNLOAD;

   if (Option != 1 && Option != 2) Status = ACQ_ERRPARAM;
   else if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {

      Status = sys$dismou(&Descri_In, Flag);
      if (Status == SS$_NORMAL) Status = ACQ_OK;
      else if (Status == SS$_DEVNOTMOUNT) Status = ACQ_NOTMOUNT;
   } else Status = ACQ_ISNOTATAPE;


#endif

#if defined ( __unix__ ) || ( __unix )

   Status = ACQ_OK;

#endif


   return (Status);
}


/********************************* END **************************************/
