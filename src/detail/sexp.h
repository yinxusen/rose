#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef struct RObject   RObject;
typedef struct RTypeInfo RTypeInfo;

#define R_OBJECT_HEADER\
        RTypeTag type_tag : 5;\
        ruint    gc_color : 2;\
        RObject* gc_next;

typedef rbool (*REqvPred)     (RState*, rsexp, rsexp);
typedef rbool (*REqualPred)   (RState*, rsexp, rsexp);
typedef rsexp (*RWriteFunc)   (RState*, rsexp, rsexp);
typedef rsexp (*RDisplayFunc) (RState*, rsexp, rsexp);
typedef void  (*RGcMarkFunc)  (RState*, rsexp);
typedef void  (*RObjDestruct) (RState*, RObject*);

struct RObject {
    R_OBJECT_HEADER
};

struct RTypeInfo {
    rsize size;
    char* name;

    struct {
        RWriteFunc   write;
        RDisplayFunc display;
        REqvPred     eqv_p;
        REqualPred   equal_p;
        RGcMarkFunc  mark;
        RObjDestruct destruct;
    }
    ops;
};

ruint      r_type_tag     (rsexp    obj);
RTypeInfo* r_type_info    (RState*  state,
                           rsexp    obj);
RObject*   r_object_alloc (RState*  state,
                           RTypeTag type_tag);

#define r_object_new(state, type, tag)\
        (r_cast (type*, r_object_alloc (state, tag)))

#endif  /* __ROSE_DETAIL_SEXP_H__ */
