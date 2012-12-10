#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

rsexp r_port_read (RState* state,
                   rsexp   port);
rsexp r_read      (RState* state);

R_END_DECLS

#endif  /* __ROSE_READER_H__ */
