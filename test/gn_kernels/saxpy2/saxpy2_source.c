#include "dev/mango_hn.h"
#include "dev/debug.h"
#include <stdlib.h>

#pragma mango_gen_entrypoint

#pragma mango_kernel
void kernel_function(float *x, float *y, float *out, int n) {
    for (int i=0; i<n; i++) {
	    out[i] = x[i] + y[i];
    }
}

