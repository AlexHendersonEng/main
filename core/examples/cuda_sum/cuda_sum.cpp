#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

#include "cuda/sum.h"

// CPU implementation of vector sum for correctness verification and benchmarking
static void cpu_vector_sum(const float *A, const float *B, float *C, int n) {
    for (int i = 0; i < n; ++i) {
        C[i] = A[i] + B[i];
    }
}

// Utility to measure execution time of a function in milliseconds
template <typename Fn>
static double time_ms(Fn &&fn) {
    auto t0 = std::chrono::high_resolution_clock::now();
    fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// Verify that the results from CPU and GPU match within a specified tolerance
static bool verify(const float *ref, const float *got, int n,
                   float tol = 1e-5f) {
    for (int i = 0; i < n; ++i) {
        if (std::fabs(ref[i] - got[i]) > tol) {
            std::cerr << "  Mismatch at [" << i << "]: ref=" << ref[i]
                      << " got=" << got[i] << "\n";
            return false;
        }
    }
    return true;
}

int main() {
    // Run for several sizes so the comparison is meaningful
    const std::vector<int> sizes = {1 << 10,   // 1 K
                                    1 << 16,   // 64 K
                                    1 << 20,   // 1 M
                                    1 << 24,   // 16 M
                                    1 << 28};  // 256 M

    constexpr int kRuns = 5; // averaged runs per size

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\n"
              << std::setw(10) << "N"
              << std::setw(14) << "CPU (ms)"
              << std::setw(14) << "CUDA (ms)"
              << std::setw(12) << "Speedup"
              << std::setw(10) << "Match?"
              << "\n"
              << std::string(60, '-') << "\n";

    for (int N : sizes) {
        std::vector<float> A(N), B(N), C_cpu(N), C_gpu(N);

        // Fill with reproducible pseudo-random values
        for (int i = 0; i < N; ++i) {
            A[i] = static_cast<float>(std::rand()) / RAND_MAX;
            B[i] = static_cast<float>(std::rand()) / RAND_MAX;
        }

        // Benchmark CPU
        double cpu_total = 0.0;
        for (int r = 0; r < kRuns; ++r) {
            cpu_total += time_ms([&] {
                cpu_vector_sum(A.data(), B.data(), C_cpu.data(), N);
            });
        }
        const double cpu_avg = cpu_total / kRuns;

        // Benchmark CUDA
        // Warm-up: first CUDA launch includes driver init overhead
        cuda_vector_sum(A.data(), B.data(), C_gpu.data(), N);

        double gpu_total = 0.0;
        for (int r = 0; r < kRuns; ++r) {
            gpu_total += time_ms([&] {
                cuda_vector_sum(A.data(), B.data(), C_gpu.data(), N);
            });
        }
        const double gpu_avg = gpu_total / kRuns;

        // Verify
        bool match = verify(C_cpu.data(), C_gpu.data(), N);

        std::cout << std::setw(10) << N
                  << std::setw(14) << cpu_avg
                  << std::setw(14) << gpu_avg
                  << std::setw(11) << (cpu_avg / gpu_avg) << "x"
                  << std::setw(10) << (match ? "YES" : "NO")
                  << "\n";
    }

    std::cout << "\nNote: CUDA timings include host<->device memory transfers.\n\n";
    return 0;
}
