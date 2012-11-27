#include "detail/hash.h"
#include "detail/sexp.h"
#include "rose/env.h"
#include "rose/error.h"
#include "rose/memory.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"

#include <assert.h>

struct REnv{
    R_OBJECT_HEADER
    rsexp       parent;
    RHashTable* bindings;
};

#define ENV_FROM_SEXP(obj)  (r_cast (REnv*, (obj)))
#define ENV_TO_SEXP(env)    (r_cast (rsexp, (env)))

static void set_parent_frame_x (rsexp env, rsexp parent)
{
    ENV_FROM_SEXP (env)->parent = parent;
}

static rsexp get_parent_frame (rsexp env)
{
    return ENV_FROM_SEXP (env)->parent;
}

static void write_env (RState* state, rsexp port, rsexp obj)
{
    r_port_printf (state, port, "#<%s>", r_type_info (state, obj)->name);
}

static void destruct_env (RState* state, RObject* obj)
{
    REnv* env = r_cast (REnv*, obj);
    r_hash_table_free (env->bindings);
}

RTypeInfo* init_env_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (REnv);
    type->name         = "environment";
    type->ops.write    = write_env;
    type->ops.display  = write_env;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_env;

    return type;
}

static rsexp frame_lookup (rsexp frame, rsexp var)
{
    rpointer val = r_hash_table_get (ENV_FROM_SEXP (frame)->bindings,
                                     r_cast (rconstpointer, var));

    return val ? (rsexp) val : R_UNDEFINED;
}

rbool r_env_p (rsexp obj)
{
    return r_type_tag (obj) == R_ENV_TAG;
}

rsexp r_env_new (RState* state)
{
    REnv* res = r_object_new (state, REnv, R_ENV_TAG);

    res->parent = R_UNDEFINED;
    res->bindings = r_hash_table_new (NULL, NULL);

    if (!res->bindings)
        r_error (state, "Failed");

    return ENV_TO_SEXP (res);
}

rsexp r_env_extend (RState* state, rsexp parent, rsexp vars, rsexp vals)
{
    rsexp env = r_env_new (state);

    while (!r_null_p (vars)) {
        r_env_define (env, r_car (vars), r_car (vals));
        vars = r_cdr (vars);
        vals = r_cdr (vals);
    }

    assert (r_null_p (vals));
    set_parent_frame_x (env, parent);

    return env;
}

rsexp r_env_lookup (rsexp env, rsexp var)
{
    rsexp frame;
    rsexp val;

    assert (r_env_p (env));

    for (frame = env; !r_undefined_p (frame); ) {
        val = frame_lookup (frame, var);

        if (r_undefined_p (val))
            frame = get_parent_frame (frame);
    }

    return val;
}

void r_env_define (rsexp env, rsexp var, rsexp val)
{
    assert (r_env_p (env));

    r_hash_table_put (ENV_FROM_SEXP (env)->bindings,
                      (rpointer) var,
                      (rpointer) val);
}

void r_env_set_x (rsexp env, rsexp var, rsexp val)
{
    assert (r_env_p (env));

    r_hash_table_put (ENV_FROM_SEXP (env)->bindings,
                      r_cast (rpointer, var),
                      r_cast (rpointer, val));
}
