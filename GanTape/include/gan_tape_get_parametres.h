/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_GET_PARAMETRES.h                           *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  12 mars 1997                                        *
 *                                                                           *
 *       Objet        :  Informations necessaires pour la recuperation des   *
 *       parametres en ligne de commande, en variables d'envt *
 *       ou par defaut. A utiliser aves la fonction acq_get_  *
 *       param_env ( .. ).
 *      Modifications :            *
 *           B. raine 22 avril 04,
 *             on remplace le type bool par BOOL pour compatibilte C/C++
 *             dans tous les environnements ( a cause du CC de Solaris)
 *                                                                           *
 *****************************************************************************/

#ifndef __GET_PARAMETERS_H
#define __GET_PARAMETERS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gan_tape_param.h"
#include "gan_tape_general.h"



typedef enum PARAM_ID { device_id, ctrlform_id, autoswbuf_id, swbufon_id,
                        density_id, bufsize_id, evbsize_id, evcsize_id,
                        lun_id
                      } param_id;

typedef enum TYPE_ID { entier, caract, logic } type_id;


typedef struct PARAM_STRUCT {

   param_id Num_param;
   const char  Nom_param[MAX_CARACTERES];
   type_id     Typ_param;
   int      Val_defaut;

} param_struct;


typedef union VAL_RET {
   int   Val_INT;
   char  Val_CAR[MAX_CARACTERES];
   BOOL   Val_BOL;
   /*      enum BOOL Val_BOL; */
} val_ret;



/* Prototype de fonction locale pour la recuperation des arguments de la
 ligne de commande. */
int acq_get_param_env(param_id, val_ret*, int, char**);


#ifdef __cplusplus
}
#endif
#endif

