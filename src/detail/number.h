#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "detail/sexp.h"
#include "rose/gmp.h"
#include "rose/number.h"

R_BEGIN_DECLS

typedef struct {
    R_OBJECT_HEADER
    mpq_t real;
    mpq_t imag;
}
RFixnum;

typedef struct {
    R_OBJECT_HEADER
    double real;
    double imag;
}
RFlonum;

#define fixnum_to_sexp(fixnum)  ((rsexp) (fixnum))
#define fixnum_from_sexp(obj)   ((RFixnum*) (obj))

#define flonum_to_sexp(flonum)  ((rsexp) (flonum))
#define flonum_from_sexp(obj)   ((RFlonum*) (obj))

#define fixnum_real(n) (fixnum_from_sexp (n)->real)
#define fixnum_imag(n) (fixnum_from_sexp (n)->imag)

#define flonum_real(n) (flonum_from_sexp (n)->real)
#define flonum_imag(n) (flonum_from_sexp (n)->imag)

#define fixnum_real(n) (fixnum_from_sexp (n)->real)
#define fixnum_imag(n) (fixnum_from_sexp (n)->imag)

R_END_DECLS

#endif /* __ROSE_DETAIL_NUMBER_H__ */
