#ifdef __CINT__
#include "RVersion.h"
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ namespace KVImpactParameters;
#pragma link C++ class KVImpactParameters::gamma_kernel+;
#pragma link C++ class KVImpactParameters::NBD_kernel+;
#pragma link C++ class KVImpactParameters::BD_kernel+;
#pragma link C++ class KVImpactParameters::rogly_fitting_function<3>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<3>,KVImpactParameters::gamma_kernel>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<3>,KVImpactParameters::NBD_kernel>+;
#pragma link C++ class KVImpactParameters::rogly_fitting_function<4>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<4>,KVImpactParameters::gamma_kernel>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<4>,KVImpactParameters::NBD_kernel>+;
#pragma link C++ class KVImpactParameters::algebraic_fitting_function+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::algebraic_fitting_function,KVImpactParameters::gamma_kernel>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::algebraic_fitting_function,KVImpactParameters::NBD_kernel>+;
#pragma link C++ class KVImpactParameters::algebraic_fitting_function_binomial+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::algebraic_fitting_function,KVImpactParameters::BD_kernel>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::algebraic_fitting_function_binomial,KVImpactParameters::BD_kernel>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<3>,KVImpactParameters::BD_kernel>+;
#pragma link C++ class KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<4>,KVImpactParameters::BD_kernel>+;
#pragma link C++ class KVImpactParameters::cavata_prescription+;
#pragma link C++ class KVImpactParameters::impact_parameter_distribution+;
#pragma link C++ class KVImpactParameters::participant_spectator_model+;
#endif
//1  #theta       2.83131e-01   4.10621e-04   2.81740e-07   2.55060e-02
//2  k_{max}      1.23852e+02   1.87011e-01   4.63611e-08  -1.04322e-02
//3  a_{1}        1.22529e+00   3.30652e-03   5.68359e-05   7.35488e-06
//4  a_{2}       -5.55274e-01   1.26092e-02   2.16503e-07  -9.38952e-03
//5  a_{3}        2.05869e+00   2.00417e-02   1.01175e-07  -2.93556e-04
//6  a_{4}       -4.87507e-01   1.15649e-02   3.79981e-08  -5.21483e-03
