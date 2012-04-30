#ifndef __ROSE_READ_H__
#define __ROSE_READ_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

rsexp sexp_read_boolean        (FILE*     input,
                                RContext* context);
rsexp sexp_read_symbol         (FILE*     input,
                                RContext* context);
rsexp sexp_read_string         (FILE*     input,
                                RContext* context);
rsexp sexp_read_simple_datum   (FILE*     input,
                                RContext* context);
rsexp sexp_read_list           (FILE*     input,
                                RContext* context);
rsexp sexp_read_compound_datum (FILE*     input,
                                RContext* context);
rsexp sexp_read_datum          (FILE*     input,
                                RContext* context);

#endif  //  __ROSE_READ_H__
