#include <dayan_kernel.h>

__device__ int cross_tu_helper(int value);

__kernel__ void linked_kernel(int *output, int value) {
  *output = cross_tu_helper(value);
}

DAYAN_KERNEL_DESC(linked_kernel, 1, 0, 32768, 1, 0, 0, 0);
