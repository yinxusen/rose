#include "detail/context.h"
#include "detail/sexp.h"
#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/writer.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

struct RPair {
    rsexp car;
    rsexp cdr;
};

#define PAIR_TO_SEXP(ptr) (((rsexp) (ptr)) | R_PAIR_TAG)
#define SEXP_TO_PAIR(obj) ((RPair*) ((obj) & (~R_PAIR_TAG)))

typedef void (*ROutputFunction) (rsexp, rsexp, RContext*);

static inline rsexp r_reverse_internal (rsexp list, rsexp acc)
{
    return r_null_p (list)
           ? acc
           : r_reverse_internal (r_cdr (list),
                                r_cons (r_car (list), acc));
}

static void output_cdr (rsexp           port,
                        rsexp           obj,
                        ROutputFunction output_fn,
                        RContext*       context)
{
    if (r_pair_p (obj)) {
        r_port_puts (port, " ");
        output_fn (port, r_car (obj), context);
        output_cdr (port, r_cdr (obj), output_fn, context);
    }
    else if (!r_null_p (obj)) {
        r_port_puts (port, " . ");
        output_fn (port, obj, context);
    }
}

static void output_pair (rsexp           port,
                         rsexp           obj,
                         ROutputFunction output_fn,
                         RContext*       context)
{
    assert (r_pair_p (obj));

    r_port_puts (port, "(");
    output_fn (port, r_car (obj), context);
    output_cdr (port, r_cdr (obj), output_fn, context);
    r_port_puts (port, ")");
}

rsexp r_cons (rsexp car, rsexp cdr)
{
    RPair* pair;

    pair = GC_NEW (RPair);
    pair->car = car;
    pair->cdr = cdr;

    return PAIR_TO_SEXP (pair);
}

rsexp r_car (rsexp obj)
{
    assert (r_pair_p (obj));
    return SEXP_TO_PAIR (obj)->car;
}

rsexp r_cdr (rsexp obj)
{
    assert (r_pair_p (obj));
    return SEXP_TO_PAIR (obj)->cdr;
}

rsexp r_set_car_x (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    SEXP_TO_PAIR (pair)->car = obj;
    return R_UNSPECIFIED;
}

rsexp r_set_cdr_x (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    SEXP_TO_PAIR (pair)->cdr = obj;
    return R_UNSPECIFIED;
}

rboolean r_pair_equal_p (rsexp lhs, rsexp rhs)
{
    return r_pair_p (lhs) &&
           r_pair_p (rhs) &&
           r_equal_p (r_car (lhs), r_car (rhs)) &&
           r_equal_p (r_cdr (lhs), r_cdr (rhs));
}

rsexp r_reverse (rsexp list)
{
    assert (r_null_p (list) || r_pair_p (list));
    return r_reverse_internal (list, R_NULL);
}

rsexp r_append_x (rsexp list, rsexp obj)
{
    rsexp tail;

    if (r_null_p (list))
        return obj;

    assert (r_pair_p (list));

    for (tail = list; r_pair_p (r_cdr (tail)); )
        tail = r_cdr (tail);

    r_set_cdr_x (tail, obj);

    return list;
}

rboolean r_list_p (rsexp obj)
{
    return r_null_p (obj) ||
           (r_pair_p (obj) && r_list_p (r_cdr (obj)));
}

rsexp r_list (rsize count, ...)
{
    va_list args;
    rsize i;

    va_start (args, count);

    rsexp res = R_NULL;
    for (i = 0; i < count; ++i)
        res = r_cons (va_arg (args, rsexp), res);

    va_end (args);

    return r_reverse (res);
}

rsize r_length (rsexp list)
{
    return r_null_p (list) ? 0 : 1 + r_length (r_cdr (list));
}

void r_pair_write (rsexp port, rsexp obj, RContext* context)
{
    output_pair (port, obj, r_write, context);
}

void r_pair_display (rsexp port, rsexp obj, RContext* context)
{
    output_pair (port, obj, r_display, context);
}

void r_register_pair_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = sizeof (RPair);
    type->name       = "pair";
    type->write_fn   = r_pair_write;
    type->display_fn = r_pair_display;

    context->tc3_types [R_PAIR_TAG] = type;
}
