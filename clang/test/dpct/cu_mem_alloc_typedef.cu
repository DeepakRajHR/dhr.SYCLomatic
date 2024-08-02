// UNSUPPORTED: system-linux
// RUN: dpct --format-range=none -out-root %T/cu_mem_alloc_typedef %s --cuda-include-path="%cuda-path/include" -- -std=c++14 -x cuda --cuda-host-only
// RUN: FileCheck %s --match-full-lines --input-file %T/cu_mem_alloc_typedef/cu_mem_alloc_typedef.dp.cpp
#include <cstdint>
#include <cuda.h>

typedef uint64_t CUdeviceptr;

void foo(CUdeviceptr ptr) {
  ptr = cuMemAlloc(&ptr, 1024);
}
