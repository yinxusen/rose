#ifndef __ROSE_DETAIL_PORT_H__
#define __ROSE_DETAIL_PORT_H__

#include "detail/sexp.h"
#include "rose/context.h"
#include "rose/port.h"

#include <stdio.h>

enum {
    INPUT_PORT,
    OUTPUT_PORT,
};

struct RPort {
    RType*    type;
    RContext* context;
    FILE*     stream;
    rint      mode;
    rsexp     name;
};

#define PORT_FROM_SEXP(obj) (*((RPort*) (obj)))
#define PORT_TO_SEXP(port)  ((rsexp) (port))
#define PORT_TO_FILE(obj)   ((FILE*) (PORT_FROM_SEXP (obj).stream))

#endif  //  __ROSE_DETAIL_PORT_H__