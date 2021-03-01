/* Test kernel. 
 * Performs simple smooth of a frame described as an array of bytes (Y*X*RGB)
 */
#include "dev/mango_hn.h"
#include "dev/debug.h"
#include <stdlib.h>

#pragma mango_kernel
void smooth_kernel_function(mango_event_t e, uint8_t *out, uint8_t *in, int X, int Y){
    printf("DEV: Smooth kernel waiting... \n");
    mango_wait(&e, 2);
    printf("DEV: Smooth kernel running... \n");
	for(int x=1; x<X-1; x++)
		for(int y=1; y<Y-1; y++)
			for(int c=0; c<3; c++){
				out[y*X*3+x*3+c]=
					(in[y*X*3+x*3+c]+
					in[y*X*3+(x-1)*3+c]+
					in[y*X*3+(x+1)*3+c]+
					in[(y-1)*X*3+x*3+c]+
					in[(y+1)*X*3+x*3+c]) / 5;
			}
	fprintf(stderr, "End smooth kernel %d %d %d\n", out[10*X*3+10*3+0], out[10*X*3+10*3+1], out[10*X*3+10*3+2]);
	fflush(stderr);
}

