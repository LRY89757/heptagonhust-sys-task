#include <iostream>
#include <cuda_runtime.h>

// nvcc naive_1.cu -o naive  && sudo nvprof ./naive
__global__ void matrixMul(const float *A, const float *B, float *C,
                          int M, int N, int K)
{
    int tx = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = blockIdx.y * blockDim.y + threadIdx.y;
    if (ty < M && tx < N)
    {
        float c = 0;
        for (int i = 0; i < K; ++i)
        {
            c += A[ty * K + i] * B[i * N + tx]; // 1 FMA, 2 load, 计算访存比太低
        }
        C[ty * N + tx] = c;
    }
}

__global__ void matrixMul0(const float *A, const float *B, float *C,
                           int M, int N, int K)
{
    int tx = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = blockIdx.y * blockDim.y + threadIdx.y;
    if (ty < M && tx < N)
    {
        for (int i = 0; i < K; ++i)
        {
            C[ty * N + tx] += A[ty * K + i] * B[i * N + tx];
        }
    }
}

__global__ void matrixMul1(const float *A, const float *B, float *C,
                           int M, int N, int K)
{
    int tx = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = blockIdx.y * blockDim.y + threadIdx.y;
    if (ty < M && tx < N)
    {
        float c = 0;
        for (int i = 0; i < K; ++i)
        {
            c += A[tx * K + i] * B[i * N + ty];
        }
        C[tx * N + ty] = c;
    }
}

// __global__ void matrixMul2(const float *A, const float *B, float *C,
//                            int M, int N, int K)
// {
//     const int tx = threadIdx.x;
//     const int ty = threadIdx.y;
//     const int bx = blockIdx.x;
//     const int by = blockIdx.y;

//     int BLOCK = blockDim.x;
//     float *begin_a = (float*)A + by * BLOCK;
//     float *begin_b = (float*)B + bx * BLOCK;
//     float *end_a = begin_a + K;

//     float sum = 0.f;
//     for(float *a_ptr = begin_a, *b_ptr=begin_b;a_ptr<end_a;
//         a_ptr += BLOCK, b_ptr += BLOCK * n){

//         }

// }

// template <int BLOCK>
// __global__ void sgemm(int m, int n, int k, float *a, int lda, float *b, int ldb,
//   float *c, int ldc) {
__global__ void sgemm(const float *a, const float *b, float *c, int m, int n, int k)
{
    int BLOCK = blockDim.x;
    int _m = blockIdx.x * BLOCK + threadIdx.x;
    int _n = blockIdx.y * BLOCK + threadIdx.y;
    if (_m < m && _n < n)
    {
        float sum = 0.f;
        for (int i = 0; i < k; ++i)
        {
            // sum += a[_m * k + i] * b[i * n + _n];
            sum += a[_n * k + i] * b[i * n + _m]; // 神奇了，就这行命令和上面那行命令一对比，居然可以快这么多
        }
        // c[_m * n + _n] = sum;
        c[_n * n + _m] = sum;
    }
}

template <int BLOCK>
// __global__ void sgemm1(int m, int n, int k, float *a, int lda, float *b, int ldb,
//   float *c, int ldc) {
__global__ void sgemm1(float *a, float *b, float *c, int m, int n, int k)
{
    const int tx = threadIdx.x;
    const int ty = threadIdx.y;
    const int bx = blockIdx.x;
    const int by = blockIdx.y;

    float *begin_a = a + by * BLOCK * k;
    float *begin_b = b + bx * BLOCK;
    float *end_a = begin_a + k;

    float sum = 0.f;
    for (float *a_ptr = begin_a, *b_ptr = begin_b; a_ptr < end_a;
         a_ptr += BLOCK, b_ptr += BLOCK * n)
    {
        __shared__ float ashare[BLOCK][BLOCK];
        __shared__ float bshare[BLOCK][BLOCK];

        ashare[ty][tx] = a_ptr[ty * k + tx];
        bshare[ty][tx] = b_ptr[ty * n + tx];
        __syncthreads();

#pragma unroll
        for (int kk = 0; kk < BLOCK; ++kk)
        {
            sum += ashare[ty][kk] * bshare[kk][tx];
        }
        __syncthreads();
    }

    c[(BLOCK * by + ty) * n + BLOCK * bx + tx] = sum;
}

// Host kernel Device Grid Block Thread
int main(int argc, char **argv)
{
    int dev = 0;
    cudaSetDevice(dev);

    // 单个block最多可以调用多少个thread
    cudaDeviceProp prop;
    // int devCnt = 0;
    cudaGetDeviceProperties(&prop, 0);
    fprintf(stdout, "%d", prop.maxThreadsPerBlock);

    // int x1 = 8, x2=1<<18, x3=1024;
    // m, k, n
    int x1 = 1 << 11, x2 = x1, x3 = x1;

    //申请host内存
    float *a_h, *b_h, *dst_h;
    a_h = (float *)malloc(x1 * x2 * sizeof(float));
    b_h = (float *)malloc(x3 * x2 * sizeof(float));
    dst_h = (float *)malloc(x1 * x3 * sizeof(float));

    for (int i = 0; i < x1 * x2; ++i)
        a_h[i] = 1.0;

    for (int i = 0; i < x3 * x2; ++i)
        b_h[i] = 2.0;

    // 申请device内存
    float *a_d, *b_d, *dst_d;
    cudaMalloc((void **)&a_d, x1 * x2 * sizeof(float));
    cudaMalloc((void **)&b_d, x3 * x2 * sizeof(float));
    cudaMalloc((void **)&dst_d, x1 * x3 * sizeof(float));

    //将host拷贝到device
    cudaMemcpy((void *)a_d, (void *)a_h, x1 * x2 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy((void *)b_d, (void *)b_h, x3 * x2 * sizeof(float), cudaMemcpyHostToDevice);

    // dim3 grid(x1);
    // dim3 block(x3);
    dim3 blockSize(32, 32);
    dim3 gridSize((x1 + blockSize.x - 1) / blockSize.x,
                  (x3 + blockSize.y - 1) / blockSize.y);

    // Matrix_mul<<<grid, block>>>(a_d, b_d, dst_d, x1, x3, x2);
    // sgemm<<<gridSize, blockSize>>>(a_d, b_d, dst_d, x1, x3, x2);
    matrixMul0<<<gridSize, blockSize>>>(a_d, b_d, dst_d, x1, x3, x2);
    matrixMul<<<gridSize, blockSize>>>(a_d, b_d, dst_d, x1, x3, x2);
    matrixMul1<<<gridSize, blockSize>>>(a_d, b_d, dst_d, x1, x3, x2);
    // sgemm1<32><<<gridSize, blockSize>>>(a_d, b_d, dst_d, x1, x3, x2);

    // cudaDeviceSynchronize();
    // cudaMemcpy((void*)a_h, (void*)a_d, x1)
    cudaMemcpy((void *)dst_h, (void *)dst_d, x1 * x3 * sizeof(float), cudaMemcpyDeviceToHost);

    // 检查执行结果
    float maxError = 0.0;
    for (int i = 0; i < x1 * x3; ++i)
        maxError = fmax(maxError, fabs(dst_h[i] - 2 * x1));
    std::cout << "\n最大误差: " << maxError << std::endl;

    free(a_h);
    free(b_h);
    free(dst_h);

    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(dst_d);

    cudaDeviceReset();
    return 0;
}
