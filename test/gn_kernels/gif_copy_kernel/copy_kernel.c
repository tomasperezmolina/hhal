/* Test kernel. 
 * Performs copy of a frame described as an array of bytes (Y*X*RGB)
 */
#include "dev/mango_hn.h"
#include "dev/debug.h"
#include <stdlib.h>


#pragma mango_kernel
void copy_kernel_function(mango_event_t e, uint8_t *out, uint8_t *in, int X, int Y){
    printf("DEV: Copy kernel waiting... \n");
    mango_wait(&e, 1);
    printf("DEV: Copy kernel running... \n");
	for(int x=0; x<X; x++)
		for(int y=0; y<Y; y++)
			for(int c=0; c<3; c++){
				out[y*X*3+x*3+c] = in[y*X*3+x*3+c];
            }
    mango_write_synchronization(&e, 2);
    printf("DEV: Copy kernel finished\n");
}

