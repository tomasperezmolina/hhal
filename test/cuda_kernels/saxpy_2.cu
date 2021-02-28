
extern "C" __global__ 
void saxpy_2(float *x, float *y, float *out, size_t n) {
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid < n) {
    out[tid] = x[tid] + y[tid];
  }
}
