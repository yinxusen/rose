#include "detail/sexp.h"
#include "rose/state.h"
#include "rose/port.h"

void r_write (rsexp port, rsexp obj)
{
    RState* state = r_port_get_state (port);
    RType* type = r_sexp_get_type (state, obj);

    if (type)
        (type->write_fn) (port, obj);
}

void r_display (rsexp port, rsexp obj)
{
    RState* state = r_port_get_state (port);
    RType* type = r_sexp_get_type (state, obj);

    if (type)
        (type->display_fn) (port, obj);
}
