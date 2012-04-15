#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

r_sexp  sexp_cons      (r_sexp car,
                        r_sexp cdr);
r_sexp  sexp_car       (r_sexp sexp);
r_sexp  sexp_cdr       (r_sexp sexp);
r_sexp  sexp_set_car_x (r_sexp pair,
                        r_sexp sexp);
r_sexp  sexp_set_cdr_x (r_sexp pair,
                        r_sexp sexp);
r_sexp  sexp_reverse   (r_sexp list);
r_sexp  sexp_append    (r_sexp list,
                        r_sexp value);

#endif  //  __ROSE_PAIR_H__
