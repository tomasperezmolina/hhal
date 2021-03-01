/* Test kernel. 
 * Performs x2 scaling of a frame described as an array of bytes (Y*X*RGB)
 */
#include "dev/mango_hn.h"
#include "dev/debug.h"
#include <stdlib.h>

#pragma mango_kernel
void scale_kernel_function(mango_event_t e, uint8_t *out, uint8_t *in, int X, int Y){
    printf("DEV: Scale kernel running... \n");
	int X2=X*2;
	int Y2=Y*2;
	for(int x=0; x<X2; x++)
		for(int y=0; y<Y2; y++)
			for(int c=0; c<3; c++){
				out[y*X2*3+x*3+c]=in[y/2*X*3+x/2*3+c];
			}
    mango_write_synchronization(&e, 1);
	printf("DEV: Scale kernel finished\n");
}

