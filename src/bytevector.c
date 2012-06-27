#include "detail/sexp.h"
#include "rose/bytevector.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/writer.h"

#include <assert.h>
#include <gc/gc.h>

struct RBytevector {
    RType* type;
    rsize  length;
    rbyte* data;
};

#define BYTEVECTOR_FROM_SEXP(obj)   (*((RBytevector*) (obj)))
#define BYTEVECTOR_TO_SEXP(bytevec) ((rsexp) (bytevec))

static void r_bytevector_write (rsexp port, rsexp obj)
{
    rsize i;
    RBytevector* vec = &BYTEVECTOR_FROM_SEXP (obj);

    r_port_puts (port, "#u8(");

    if (vec->length > 0u) {
        r_write (port, r_int_to_sexp (vec->data [0u]));

        for (i = 1u; i < vec->length; ++i)
            r_format (port, " ~s", r_int_to_sexp (vec->data [i]));
    }

    r_write_char (port, ')');
}

static RType* r_bytevector_type_info ()
{
    static RType* type = NULL;

    if (!type) {
        type = GC_NEW (RType);

        type->cell_size  = sizeof (RBytevector);
        type->name       = "bytevector";
        type->write_fn   = r_bytevector_write;
        type->display_fn = r_bytevector_write;
    }

    return type;
}

rsexp r_bytevector_new (rsize k)
{
    RBytevector* res = GC_NEW (RBytevector);

    res->type   = r_bytevector_type_info ();
    res->length = k;
    res->data   = k ? GC_MALLOC_ATOMIC (k * sizeof (rbyte)) : NULL;

    return BYTEVECTOR_TO_SEXP (res);
}

rboolean r_bytevector_p (rsexp obj)
{
    return r_cell_p (obj) &&
           (R_CELL_TYPE (obj) == r_bytevector_type_info ());
}

rsexp r_make_bytevector (rsize k, rbyte fill)
{
    rsexp  res;
    rbyte* data;
    rsize  i;

    res  = r_bytevector_new (k);
    data = BYTEVECTOR_FROM_SEXP (res).data;

    for (i = 0; i < k; ++i)
        data [i] = fill;

    return res;
}

rsexp r_bytevector_set_x (rsexp obj, rsize k, rbyte byte)
{
    assert (r_bytevector_p (obj));
    assert (BYTEVECTOR_FROM_SEXP (obj).length > k);

    BYTEVECTOR_FROM_SEXP (obj).data [k] = byte;

    return R_UNSPECIFIED;
}

rsexp r_list_to_bytevector (rsexp list)
{
    rsize length = r_length (list);
    rsexp res = r_make_bytevector (length, R_UNSPECIFIED);
    rbyte byte;
    rsize k;

    for (k = 0; k < length; ++k) {
        byte = (rbyte) r_int_from_sexp (r_car (list));
        r_bytevector_set_x (res, k, byte);
        list = r_cdr (list);
    }

    return res;
}
