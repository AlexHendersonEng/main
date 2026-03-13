#include "cuda/sum.h"
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>

// Throws a std::runtime_error if `err` indicates a CUDA failure, prefixing
// the error string with `context` to identify the call site.
static void check(cudaError_t err, const char *context) {
    if (err != cudaSuccess) {
        throw std::runtime_error(std::string(context) +
                                 ": " + cudaGetErrorString(err));
    }
}

// Wraps a CUDA API call with check(), using the stringified call expression
// as the context label for any resulting error message.
#define CUDA_CHECK(call) check((call), #call)

// Each thread computes one element of C. Threads whose global index exceeds
// the array length are masked out to handle non-multiple-of-blockDim inputs.
__global__ void kernel_vector_sum(const float *A, const float *B,
                                  float *C, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        C[idx] = A[idx] + B[idx];
    }
}

void cuda_vector_sum(const float *A, const float *B, float *C, int n) {
    const size_t bytes = n * sizeof(float);

    // Allocate device memory for both inputs and the output.
    float *d_A, *d_B, *d_C;
    CUDA_CHECK(cudaMalloc(&d_A, bytes));
    CUDA_CHECK(cudaMalloc(&d_B, bytes));
    CUDA_CHECK(cudaMalloc(&d_C, bytes));

    // Copy input arrays from host to device.
    CUDA_CHECK(cudaMemcpy(d_A, A, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_B, B, bytes, cudaMemcpyHostToDevice));

    // Launch one thread per element, grouped into fixed-size blocks of 256.
    // The grid is padded to cover any remainder when n is not a multiple of kThreads.
    constexpr int kThreads = 256;
    const int blocks = (n + kThreads - 1) / kThreads;
    kernel_vector_sum<<<blocks, kThreads>>>(d_A, d_B, d_C, n);

    // Check for kernel launch errors, then block until the kernel completes.
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    // Copy the result back to the caller's host buffer.
    CUDA_CHECK(cudaMemcpy(C, d_C, bytes, cudaMemcpyDeviceToHost));

    // Release all device allocations.
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}
