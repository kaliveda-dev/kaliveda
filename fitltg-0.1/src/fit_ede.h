#ifndef FIT_EDE_LOADED

/*
 *                   **************************************
 *                   * Wrapper C-Fortran pour fit_ede_f.f *
 *                   **************************************
 */
/*
 *
 *                 ***********************************
 *                 *     Fit of E-DE functional      *
 *                 *  Laurent TASSAN-GOT  17/03/2001 *
 *                 ***********************************
 *
 *  Three basicn routines are available :
 * globede_c: user-friend routine in which only the data have to be supplied.
 *            The routine estimates good initial values, reasonnable limits,
 *            automatically from the data and manages the whole fit procedure
 *  fitede_c: more specific but flexible routine in which initial values
 *            have to be supplied and also limits of the parameters and
 *            scaling factors. This routine performs a fit in the given
 *            conditions, it is called several times by globede. It can be
 *            invoked directly by the user in case of specific difficult cases,
 *            otherwise globede is much simpler and easier to handle.
 *   fede_c : function which returns the value of the functional (energy loss)
 *            for a given Z, A and energy (or light). Subroutines globede or
 *            fitede should have been called first to load the parameters.
 *
 * At least 1 (Z,A) pair must contain 2 points.
 */

/*
 *---------------------------------------------------------------------
 *
 *                   ******************
 *                   *  Automatic fit *
 *                   ******************
 *
 *     globede_c(int npts,const float *zd,const float *ad,const float *xd,
 *         const float *yd,int ixt,int ih,float *par,int *istate)
 *
 * ARGUMENTS :
 *  npts : number of data points really used in zd, ad, xd, yd
 *   zd  : array of Z's (npts values)
 *   ad  : array of A's (npts values)
 *   xd  : array of E's (npts values)
 *   yd  : array of DE's (npts values)
 *  ixt  :  =0->basic functional       <>0->extended functional
 *      * For the basic nformula :
 *        yy = ((g*E)**(mu+1)+lambda**(mu+1)*Z**2*A**mu)**(1/(mu+1))-g*E + pdy
 *      * For the extended formula :
 *        yy = ((g*E)**(mu+nu+1)+(lambda*Z**alpha*A**beta)**(mu+nu+1)+
 *                 xi*A**mu*(g*E)**nu)**(1/(mu+mu+1))-g*E + pdy
 *
 *      *  If ih=0  no non-linear light response : E=xx-pdx
 *      *  If ih<>0 non-linear light response included :
 *          E = sqrt(h**2+2*rho*h*(1+log(1+h/rho)))
 *         rho=eta*Z**2*A    and   h=xx-pdx
 *   ih  :  =0->no non-linear light response
 *             <>0->non-linear light response included
 *   par : address where the fit parameters will be written, the number of
 *          returned values depend on ixt and ih :
 *             the numbers and orders of parameters are :
 *             ixt=0  ih=0   5 parameters: lambda, mu, g, pdx, pdy
 *             ixt=0  ih<>0  6 parameters: lambda, mu, g, pdx, pdy, eta
 *             ixt<>0 ih=0   9 parameters: lambda, alpha, beta, mu, nu,
 *                                        xi, g, pdx, pdy
 *             ixt<>0 ih<>0 10 parameters: lambda, alpha, beta, mu, nu,
 *                                        xi, g, pdx, pdy, eta
 * istate : address where the status of parameters again constraints will be
 *          written. The number of returned values is the same as for par
 *                =0 -> free parameter
 *                =1 -> parameter constrained by the lower limit
 *                =2 -> parameter constrained by the upper limit
 *                =3 -> constant parameter (bl(i)=bu(i)=par(i))
 * FUNCTION VALUE : return code
 *             0 -> convergence reached
 *             1 -> convergence reached, but not well marked minimum
 *             2 -> too many iterations, convergence not reached
 *            -1 -> no identification line with at least 2 points
 *            -2 -> too few data points
 *            -3 -> addressing problem between Fortran and C
 */
/*
 *                   ******************
 *                   *  Specific fit  *
 *                   ******************
 *
 *     fit_ede_c(int npts,const float *zd,const float *ad,const float *xd,
 *         const float *yd,int ixt,int ih,const float *sc,const float *bl,
 *         const float *bu,float *par,int *istate)
 *
 * ARGUMENTS :
 *  npts : number of data points really used in zd, ad, xd, yd
 *   zd  : array of Z's (npts values)
 *   ad  : array of A's (npts values)
 *   xd  : array of E's (npts values)
 *   yd  : array of DE's (npts values)
 *  ixt  :  =0->basic functional       <>0->extended functional
 *      * For the basic nformula :
 *        yy = ((g*E)**(mu+1)+lambda**(mu+1)*Z**2*A**mu)**(1/(mu+1))-g*E + pdy
 *      * For the extended formula :
 *        yy = ((g*E)**(mu+nu+1)+(lambda*Z**alpha*A**beta)**(mu+nu+1)+
 *                 xi*A**mu*(g*E)**nu)**(1/(mu+mu+1))-g*E + pdy
 *
 *      *  If ih=0  no non-linear light response : E=xx-pdx
 *      *  If ih<>0 non-linear light response included :
 *          E = sqrt(h**2+2*rho*h*(1+log(1+h/rho)))
 *         rho=eta*Z**2*A    and   h=xx-pdx
 *   ih  :  =0->no non-linear light response
 *             <>0->non-linear light response included
 *   sc  : array of size equal to the number of parameters : scaling
 *             factors of parameters. If sc[0]=0. the initial values
 *             of parameters will be taken as scaling factors, this is
 *             recommended if no initial value is null.
 *    bl : array of the lower limits of parameters
 *    bu : array of the upper limits of parameters
 *             if bl[i]=bu[i]=par[i] the parameter i will be constrained to
 *             stay fixed along the fit procedure, for example pdx and pdy
 *             can be fixed in this way, if in the data A is taken dependent
 *             of Z, beta should be constant, and if only one Z is present
 *             alpha should be constant.
 *   par : address where the fit parameters will be written, the number of
 *          returned values depend on ixt and ih :
 *             the numbers and orders of parameters are :
 *             ixt=0  ih=0   5 parameters: lambda, mu, g, pdx, pdy
 *             ixt=0  ih<>0  6 parameters: lambda, mu, g, pdx, pdy, eta
 *             ixt<>0 ih=0   9 parameters: lambda, alpha, beta, mu, nu,
 *                                        xi, g, pdx, pdy
 *             ixt<>0 ih<>0 10 parameters: lambda, alpha, beta, mu, nu,
 *                                        xi, g, pdx, pdy, eta
 * istate : address where the status of parameters again constraints will be
 *          written. The number of returned values is the same as for par
 *                =0 -> free parameter
 *                =1 -> parameter constrained by the lower limit
 *                =2 -> parameter constrained by the upper limit
 *                =3 -> constant parameter (bl[i]=bu[i]=par[i])
 * FUNCTION VALUE : return code
 *             0 -> convergence reached
 *             1 -> convergence reached, but not well marked minimum
 *             2 -> too many iterations, convergence not reached
 *            11 -> initial values inconsistent with limits :
 *                    bu < bl   or    par < bl   or   par > bu
 *            -3 -> addressing problem between Fortran and C
 */
/*
 *                   *************************************
 *                   *  DE value given by the functional *
 *                   *************************************
 *       fede_c(float Z,float A,float E)
 * ARGUMENT
 *    Z  : Z value
 *    A  : A value
 *    E  : E value
 * FUNCTION VALUE : value of DE from the functional
 *     This call is only valid after having called  fitede or globede
 */
#ifdef __cplusplus
extern "C" {
#endif
extern int globede_c(int npts, const float* zd, const float* ad, const float* xd,
                     const float* yd, int ixt, int ih, float* par, int* istate);
extern float fede(float z, float a, float xxx);
#ifdef __cplusplus
}
#endif
#define FIT_EDE_LOADED
#endif
