#ifndef __ROSE_DETAIL_NUMBER_READER_H__
#define __ROSE_DETAIL_NUMBER_READER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/types.h"

typedef struct RNumberReader RNumberReader;

struct RNumberReader {
    RState*       state;

    rconstcstring begin;
    rconstcstring end;
    rconstcstring pos;

    rtribool      exact;
    rtribool      decimal;
    ruint         radix;
};

void  r_number_reader_init (RState*        state,
                            RNumberReader* reader);
rsexp r_number_read        (RNumberReader* reader,
                            rconstcstring  text);

#endif  /* __ROSE_DETAIL_NUMBER_READER_H__ */
