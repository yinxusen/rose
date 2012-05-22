#include "cell.h"
#include "opaque.h"

#define SEXP_TO_OPAQUE(obj) R_CELL_VALUE (obj).opaque

rboolean r_opaque_p (rsexp obj)
{
    return r_cell_p (obj) &&
           r_cell_get_type (obj) == SEXP_OPAQUE;
}

rsexp r_opaque_new (rpointer opaque)
{
    R_SEXP_NEW (res, SEXP_OPAQUE);
    SEXP_TO_OPAQUE (res) = opaque;
    return res;
}

void r_opaque_set (rsexp obj, rpointer opaque)
{
    SEXP_TO_OPAQUE (obj) = opaque;
}

rpointer r_opaque_get (rsexp obj)
{
    return SEXP_TO_OPAQUE (obj);
}
