/*
 *              *****************************************************
 *              * Exemple de programme de fit de fonctionnelle E-DE *
 *              *****************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fit_ede.h>

/* Parametres de fit */
#define MXPAR 10
static float par[MXPAR] ;
static int istate[MXPAR] ;
/* Tableaux des data */
#define MXPTS 200
static float zd[MXPTS], ad[MXPTS], xd[MXPTS], yd[MXPTS] ;
/* Chaine multipurpose */
#define LNSTR 512
static char line[LNSTR] ;

float geta(float Z, int ityp) ;

int main(int argc, char* argv[])
{
   if (argc != 2) { /* argc should be 2 for correct execution */
      /* We print argv[0] assuming it is the program name */
      printf("usage: %s [path to ChioSi_new.dat]", argv[0]);
      exit(EXIT_FAILURE);
   }
   printf("sizeof(double)=%lu\n", sizeof(double));

   int i, iz, ia, np, npts ;
   float aa, xx, yy ;
   FILE* fd ;

   /* Ouverture fichier de donnees */
   fd = fopen(argv[1], "r") ;
   if (fd == NULL) {
      printf("Fichier de data %s introuvable\n", argv[1]) ;
      exit(EXIT_FAILURE);
   }
   /* Chargement des data */
   npts = 0 ;
   char* fgets_result;
   for (i = 0 ; i < 11 ; i++) fgets_result = fgets(line, LNSTR, fd) ;
   if (fgets_result) {
      ;
   }
   while (1) {
      if (fgets(line, LNSTR, fd) == NULL) break ;
      if (fgets(line, LNSTR, fd) == NULL) break ;
      if (fgets(line, LNSTR, fd) == NULL) break ;
      sscanf(line, "%d", &iz) ;
      aa = geta((float)iz, 1) ;
      ia = (int)(aa + 0.5) ;
      if (fgets(line, LNSTR, fd) == NULL) break ;
      sscanf(line, "%d", &np) ;
      for (i = 0 ; i < np ; i++) {
         if (fgets(line, LNSTR, fd) == NULL) break ;
         sscanf(line, "%f %f", &xx, &yy) ;
         if (npts < MXPTS) {
            zd[npts] = iz ;
            ad[npts] = ia ;
            xd[npts] = xx ;
            yd[npts] = yy ;
         }
         npts++ ;
      }
   }
   fclose(fd) ;
   printf("npts=%d\n", npts) ;
   if (npts > MXPTS) {
      printf("Attention : %d > %d\n", npts, MXPTS) ;
      return 1 ;
   }
   fflush(stdout) ;
   /* Fit fonctionnelle */
   i = globede_c(npts, zd, ad, xd, yd, 1, 0, par, istate) ;
   printf("irc_fit=%d\n", i) ;
   fflush(stdout) ;
   if (i != 0 && i != 1) return 1 ;
   /* Calcul et affichage valeur de la fonctionnelle pour chque point */
   for (i = 0 ; i < npts ; i++)
      printf(" %3d  %3d   %6.1f   %6.1f   %6.1f\n", (int)(zd[i] + 0.5),
             (int)(ad[i] + 0.5), xd[i], yd[i], fede(zd[i], ad[i], xd[i])) ;
   return EXIT_SUCCESS ;
}

/*
 *                  ********************************
 *                  * CALCUL DU A STABLE OU RESIDU *
 *                  ********************************
 *     float geta(float Z,int ityp)
 * ARGUMENTS :
 *   Z   :  Z du noyau
 *  ityp : 0 -> A stable     <>0 -> A residu
 * RETOUR FONCTION : A du stable ou du residu
 */
float geta(float Z, int ityp)
{
   float F0 = 1.98, F1 = 0.0155, RS1 = 71.35, RS2 = 0.00021353, RS3 = 0.03047, RS4 = -1.087;
   float EPS = 0.001;
   float a, fxp, da, daa, zc, f11, drv, den, as ;
   int   nit ;

   fxp = 2. / 3. ;
   a = Z * 2. ;
   daa = da = EPS * 2. ;
   f11 = F1 * fxp ;
   nit = 0 ;
   while (daa > EPS) {
      nit++ ;
      as = pow((double)a, (double)fxp) ;
      den = F0 + F1 * as ;
      zc = a / den ;
      drv = (1. - f11 * as / den) / den ;
      if (ityp) {
         if (a > RS1) {
            zc = zc + RS3 * a + RS4 ;
            drv = drv + RS3 ;
         } else {
            zc = zc + RS2 * a * a ;
            drv = drv + RS2 * a * 2.;
         }
      }
      daa = da = (Z - zc) / drv ;
      if (daa < 0.) daa = -daa ;
      a = a + da ;
   }
   return a ;
}



