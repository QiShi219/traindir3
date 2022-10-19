/*
 * File: F_ACC_VS.c
 *
 * MATLAB Coder version            : 4.2
 * C/C++ source code generated on  : 08-Mar-2021 19:50:44
 */

/* Include Files */
#include <math.h>
#include "F_ACC_VS.h"

/* Function Definitions */

/*
 * Arguments    : double v0
 *                double v1
 * Return Type  : double
 */
double F_ACC_VS(double v0, double v1)
{
  double a;
  double b_a;
  double c_a;
  double d_a;
  double e_a;
  double f_a;
  double S_section_tmp;
  double b_S_section_tmp;
  double c_S_section_tmp;
  double d_S_section_tmp;
  double e_S_section_tmp;

  /*  b^2 <4ac */
  a = v1 / 3.6;
  b_a = v0 / 3.6;

  /*  b^2 >4ac */
  c_a = v1 / 3.6;
  d_a = v0 / 3.6;
  e_a = v0 / 3.6;
  f_a = v1 / 3.6;
  S_section_tmp = 8.96471830954024E-5 * (v1 / 3.6) + -0.0117587963763836;
  b_S_section_tmp = 8.96471830954024E-5 * (v0 / 3.6) + -0.0117587963763836;
  c_S_section_tmp = -0.0117587963763836 * (v0 / 3.6);
  d_S_section_tmp = -0.0129417095256019 * (v1 / 3.6);
  b_S_section_tmp = 5.7868364767017337E+6 * log(fabs((b_S_section_tmp -
    1.72805988907079E-7) / (b_S_section_tmp + 1.72805988907079E-7)));
  e_S_section_tmp = 282303.85559623461 * atan(9.88694045867616E-5 * (v1 / 3.6) /
    7.08456494785003E-6 + -1826.7472485419662);
  return (((11154.840179816933 * log(fabs((4.48235915477012E-5 * (c_a * c_a) +
    -0.0117587963763836 * (v1 / 3.6)) + 0.77118592808568343)) -
            11154.840179816933 * log(fabs((4.48235915477012E-5 * (d_a * d_a) +
    c_S_section_tmp) + 0.77118592808568343))) - -131.16749428556955 *
           (5.7868364767017337E+6 * log(fabs((S_section_tmp -
    1.72805988907079E-7) / (S_section_tmp + 1.72805988907079E-7))) -
            b_S_section_tmp)) * (double)(v1 <= 167.0) + ((10114.352404362489 *
            log((4.94347022933808E-5 * (a * a) + d_S_section_tmp) +
                0.847015799964379) - 10114.352404362489 * log
            ((4.94347022933808E-5 * (b_a * b_a) + -0.0129417095256019 * (v0 /
    3.6)) + 0.847015799964379)) - -130.89701085683251 * (e_S_section_tmp -
            282303.85559623461 * atan(9.88694045867616E-5 * (v0 / 3.6) /
             7.08456494785003E-6 + -1826.7472485419662))) * (double)(v0 >= 167.0))
    + ((((-12634.970662493488 - 11154.840179816933 * log(fabs
           ((4.48235915477012E-5 * (e_a * e_a) + c_S_section_tmp) +
            0.77118592808568343))) + 10114.352404362489 * log(fabs
          ((4.94347022933808E-5 * (f_a * f_a) + d_S_section_tmp) +
           0.847015799964379))) - -10530.678269895116) - -130.89701085683251 *
       (((263.15224529976996 - b_S_section_tmp) + e_S_section_tmp) -
        -443202.4895194039)) * (double)((v0 < 167.0) && (v1 > 167.0));
}

/*
 * File trailer for F_ACC_VS.c
 *
 * [EOF]
 */
