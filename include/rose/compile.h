#ifndef __ROSE_COMPILE_H__
#define __ROSE_COMPILE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

rsexp r_compile (RState* state, rsexp program);

R_END_DECLS

#endif  //  __ROSE_COMPILE_H__