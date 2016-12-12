#ifndef	 __GEN_TYPE_LOADED
#define	 __GEN_TYPE_LOADED	1

/*
**	GEN_TYPE:H -- last update : 12 aout 93	creation date : 13 feb 87
**	Copyright Acquisition group, GANIL Caen, France
**	author	: Herve POSTEC
**
**	contents : General "typedef" operations . 
**		   General constants .
**
**	mod.: char   STRING256[256];	  	W. Scheuing; 07-1992
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __vms
/* definition des descripteurs standards VMS, voir
   detail dans sys$library:descrip.h */
#include descrip	

/* descripteur de chaine de caractere pour interfacage avec un programme
   fortran (voir routine string_for_descrip.c pour retour d'un tableau de
   type character vers un programme appelant fortran et fichier
   SYS$LIBRARY:DESCRIP.H pour detail de la structure.
*/
typedef struct dsc$descriptor_d DESCRIP_D;
#endif

#define D_			*
typedef char                    BYTE;
typedef short int               INT16;
typedef unsigned short int      UNSINT16;
#ifdef PDOS
typedef long  int               INT32;
typedef unsigned long  int      UNSINT32;
#else
typedef int                     INT32;
typedef unsigned int            UNSINT32;
#endif
typedef char			STRING256[256];

#define TRUE	1	/* "true" */
#define FALSE	0	/* "false" */

#ifdef __cplusplus
}
#endif

/* END OF GEN_TYPE.H */
#endif
