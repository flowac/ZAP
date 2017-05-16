// BenchCon.h

#ifndef __BENCH_CON_H
#define __BENCH_CON_H

#include <stdio.h>

#include "CreateCoder.h"
#include "Property.h"

HRESULT BenchCon(DECL_EXTERNAL_CODECS_LOC_VARS
    const CObjectVector<CProperty> &props, UInt32 numIterations, FILE *f);

#endif
