// Option: --use-experimental-features=free-function-queries
__global__ void test(int i) {
  // Start
  __syncthreads_count(i /*int*/);
  // End
}
