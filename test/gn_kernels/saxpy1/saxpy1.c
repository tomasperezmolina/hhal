#include "dev/mango_hn.h"
#include "dev/debug.h"
#include <stdlib.h>

#pragma mango_kernel
void kernel_function(int a, float *x, float *out, int n) {
    for (int i=0; i<n; i++) {
        out[i] = a * x[i];
    }
}

