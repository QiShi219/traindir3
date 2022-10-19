/*
 * File: F_ACC_VT.c
 *
 * MATLAB Coder version            : 4.2
 * C/C++ source code generated on  : 09-Mar-2021 11:05:36
 */

/* Include Files */
#include <math.h>
#include "F_ACC_VT.h"

/* Function Definitions */

/*
 * Arguments    : double v0
 *                double v1
 * Return Type  : double
 */
double F_ACC_VT(double v0, double v1)
{
  double T_section_tmp;
  double b_T_section_tmp;
  double c_T_section_tmp;
  T_section_tmp = 8.96471830954024E-5 * (v1 / 3.6) + -0.0117587963763836;
  b_T_section_tmp = 8.96471830954024E-5 * (v0 / 3.6) + -0.0117587963763836;
  b_T_section_tmp = 5.7868364767017337E+6 * log(fabs((b_T_section_tmp -
    1.72805988907079E-7) / (b_T_section_tmp + 1.72805988907079E-7)));
  c_T_section_tmp = 282303.85559623461 * atan(9.88694045867616E-5 * (v1 / 3.6) /
    7.08456494785003E-6 + -1826.7472485419662);
  return ((5.7868364767017337E+6 * log(fabs((T_section_tmp - 1.72805988907079E-7)
             / (T_section_tmp + 1.72805988907079E-7))) - b_T_section_tmp) *
          (double)(v1 <= 167.0) + (c_T_section_tmp - 282303.85559623461 * atan
           (9.88694045867616E-5 * (v0 / 3.6) / 7.08456494785003E-6 +
            -1826.7472485419662)) * (double)(v0 >= 167.0)) +
    (((263.15224529976996 - b_T_section_tmp) + c_T_section_tmp) -
     -443202.4895194039) * (double)((v0 < 167.0) && (v1 > 167.0));
}

/*
 * File trailer for F_ACC_VT.c
 *
 * [EOF]
 */
