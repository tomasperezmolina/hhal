
extern "C" __global__ 
void saxpy_1(float a, float *x, float *out, size_t n) {
  size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
  if (tid < n) {
    out[tid] = a * x[tid];
  }
}
