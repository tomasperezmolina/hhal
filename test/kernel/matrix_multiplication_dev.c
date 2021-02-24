/* 
 *
 * Test application. This application
 * will launch a kernel in PEAK. The kernel will just
 * compute a matrix multiplication. The size of the matrix 
 * and the matrices pointers will be passed as parameters
 *
 * After executing the kernel, the application shows
 * the resulting output matrix
 *
*/
#include "dev/mango_hn.h"
#include "dev/debug.h"
#include <stdlib.h>

#pragma mango_kernel
void kernel_function(int *A, int *B, int *C, int rows, int cols, mango_event_t e) {

	printf("[Kernel] Rows: %d Cols: %d\n", rows, cols);
	for (int r=0;r<rows;r++) {
		for (int c=0;c<cols;c++) {
			int v = 0;
			for (int p=0;p<rows;p++) {
				v = v + A[r * cols + p] * B[p * cols + c];
			}
			C[r * cols + c] = v;
		}
	}

	printf("[Kernel] Waiting for buffer event\n");
	mango_wait(&e, 2);

	printf("[Kernel] matrix C result:\n");
	for (int r=0;r<rows;r++) {
		for (int c=0;c<cols;c++) {
			printf("%d ", C[r * cols + c]);
		}
		printf("\n");
	}

	mango_write_synchronization(&e, 1);

	return;
}

/*
 * This main function performs the, init, close and argument decoding
 * functions. These should be performed in the kernel itself, but could be
 * easily wrapped in an auto-generated code.
 */
/*int main(int argc, char **argv){
	printf("test_peak_kernel_app\n");
	int *A = (int *)strtol(argv[5],NULL,16);
	int *B = (int *)strtol(argv[6],NULL,16);
	int *C = (int *)strtol(argv[7],NULL,16);
	mango_event_t EV;
	EV.vaddr = 0x5e000058; // (uint32_t *)strtol(argv[8],NULL,16);
	int rows = (int)strtol(argv[8],NULL,16);
	int cols = (int)strtol(argv[9],NULL,16);
	mango_init(argv);
	printf("Reader name: %s\n", argv[0]);
	printf("A address: %p\n", A);
	printf("B address: %p\n", B);
	printf("C address: %p\n", C);
	printf("EV address: %p\n", EV.vaddr);
	printf("rows: %d\n", rows);
	printf("cols: %d\n", cols);
	kernel_function(A, B, C, rows, cols);
	printf("Kernel completed\n");

	mango_read_synchronization(&EV);
	mango_write_synchronization(&EV, 1);

	mango_close(15);
}
*/
