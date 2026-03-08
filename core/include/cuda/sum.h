#pragma once

/**
 * @file cuda_vector_sum.h
 * @brief Compute element-wise vector addition on the GPU using CUDA.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Computes the element-wise sum of two arrays in parallel on the GPU.
 *
 * <p>This function dispatches a CUDA kernel to compute the vector addition
 * {@code C = A + B} element-wise across all {@code n} elements:
 *
 * <pre>
 *     C[i] = A[i] + B[i],  for i = 0, 1, ..., n-1
 * </pre>
 *
 * <p>Input arrays are copied from host to device memory before computation,
 * and the result is copied back to the host upon completion.
 *
 * @param A host pointer to the first input array (length {@code n})
 * @param B host pointer to the second input array (length {@code n})
 * @param C host pointer to the output array (length {@code n}); must be
 *          pre-allocated by the caller
 * @param n the number of elements in each array
 */
void cuda_vector_sum(const float *A, const float *B, float *C, int n);

#ifdef __cplusplus
}
#endif