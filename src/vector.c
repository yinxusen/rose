#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/vector.h"

#include <assert.h>
#include <stdarg.h>

typedef struct RVector RVector;

struct RVector {
    R_OBJECT_HEADER
    rsize  length;
    rsexp* data;
};

#define vector_from_sexp(obj)   (r_cast (RVector*, (obj)))
#define vector_to_sexp(vector)  (r_cast (rsexp, (vector)))

typedef RWriteFunc ROutputFunc;

static rsexp vector_output (RState* state,
                            rsexp port,
                            rsexp obj,
                            ROutputFunc output_fn)
{
    rsize i;
    rsize length;

    length = r_uint_from_sexp (r_vector_length (obj));
    ensure (r_port_puts (state, port, "#("));

    if (length) {
        ensure (output_fn (state, port, r_vector_ref (state, obj, 0)));

        for (i = 1u; i < length; ++i) {
            ensure (r_port_puts (state, port, " "));
            ensure (output_fn (state, port, r_vector_ref (state, obj, i)));
        }
    }

    ensure (r_port_puts (state, port, ")"));

    return R_UNSPECIFIED;
}

static rsexp vector_write (RState* state, rsexp port, rsexp obj)
{
    return vector_output (state, port, obj, r_port_write);
}

static rsexp vector_display (RState* state, rsexp port, rsexp obj)
{
    return vector_output (state, port, obj, r_port_display);
}

static void vector_finalize (RState* state, RObject* obj)
{
    RVector* v = r_cast (RVector*, obj);
    r_free (state, v->data);
}

static rsexp vvector (RState* state, rsize k, va_list args)
{
    rsize i;
    rsexp res = r_vector_new (state, k, R_UNDEFINED);

    for (i = 0; i < k; ++i)
        r_vector_set_x (state, res, i, va_arg (args, rsexp));

    return res;
}

static rbool vector_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    rsize  k;
    rsize  lhs_len;
    rsize  rhs_len;
    rsexp* lhs_data;
    rsexp* rhs_data;

    if (!r_vector_p (lhs) || !r_vector_p (rhs))
        return FALSE;

    lhs_len = vector_from_sexp (lhs)->length;
    rhs_len = vector_from_sexp (rhs)->length;

    if (lhs_len != rhs_len)
        return FALSE;

    lhs_data = vector_from_sexp (lhs)->data;
    rhs_data = vector_from_sexp (rhs)->data;

    for (k = 0; k < lhs_len; ++k)
        if (!r_equal_p (state, lhs_data [k], rhs_data [k]))
            return FALSE;

    return TRUE;
}

static rbool check_index_overflow (RState* state, rsexp vec, rsize k)
{
    rsize length = r_uint_from_sexp (r_vector_length (vec));

    if (k >= length) {
        r_error_code (state, R_ERR_INDEX_OVERFLOW);
        return FALSE;
    }

    return TRUE;
}

static void vector_mark (RState* state, rsexp obj)
{
    rsize i;
    rsize length = r_uint_from_sexp (r_vector_length (obj));

    for (i = 0; i < length; ++i)
        r_gc_mark (state, r_vector_ref (state, obj, i));
}

void init_vector_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = sizeof (RVector);
    type.name         = "vector";
    type.ops.write    = vector_write;
    type.ops.display  = vector_display;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = vector_equal_p;
    type.ops.mark     = vector_mark;
    type.ops.finalize = vector_finalize;

    init_builtin_type (state, R_TAG_VECTOR, &type);
}

rsexp r_vector_new (RState* state, rsize k, rsexp fill)
{
    rsize i;
    RVector* res = r_object_new (state, RVector, R_TAG_VECTOR);

    if (!res)
        return R_FAILURE;

    res->length = k;
    res->data   = k ? r_new_array (state, rsexp, k) : NULL;

    if (k && !res->data) {
        r_free (state, res);
        return R_FAILURE;
    }

    for (i = 0; i < k; ++i)
        res->data [i] = fill;

    return vector_to_sexp (res);
}

rsexp r_vector (RState* state, rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = vvector (state, k, args);
    va_end (args);

    return res;
}

rbool r_vector_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_VECTOR;
}

rsexp r_vector_ref (RState* state, rsexp vector, rsize k)
{
    return check_index_overflow (state, vector, k)
           ? vector_from_sexp (vector)->data [k]
           : R_FAILURE;
}

rsexp r_vector_set_x (RState* state, rsexp vector, rsize k, rsexp obj)
{
    if (!check_index_overflow (state, vector, k))
        return R_FAILURE;

    vector_from_sexp (vector)->data [k] = obj;

    return R_TRUE;
}

rsexp r_vector_length (rsexp vector)
{
    return r_uint_to_sexp (vector_from_sexp (vector)->length);
}

rsexp r_list_to_vector (RState* state, rsexp list)
{
    rsexp res;
    rsize i;

    res = r_length (state, list);

    if (r_failure_p (res))
        goto exit;

    res = r_vector_new (state,
                        r_uint_from_sexp (res),
                        R_UNDEFINED);

    if (r_failure_p (res))
        goto exit;

    for (i = 0; !r_null_p (list); ++i) {
        r_vector_set_x (state, res, i, r_car (list));
        list = r_cdr (list);
    }

exit:
    return res;
}

rsexp r_vector_to_list (RState* state, rsexp vector)
{
    rsize i;
    rsexp res;
    rsize length = r_uint_from_sexp (r_vector_length (vector));

    r_gc_scope_open (state);

    for (i = length, res = R_NULL; i > 0; --i) {
        res = r_cons (state, r_vector_ref (state, vector, i - 1), res);

        if (r_failure_p (res)) {
            res = R_FAILURE;
            goto exit;
        }
    }

exit:
    r_gc_scope_close_and_protect (state, res);

    return res;
}
