#include <cuda_profiler_api.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <vector>
#include <cassert>
#include <numeric> 
#include <cuda_runtime.h>

// nvcc main.cu -o cumain && sudo nvprof ./cumain

#define PRINT_TIME(code) do { \
    auto start = system_clock::now(); \
    code \
    auto end   = system_clock::now(); \
    auto duration = duration_cast<microseconds>(end - start); \
    cout << "time spent: " << double(duration.count()) << "us" << endl; \
} while(0)

using namespace std;

using namespace chrono;

using vec = vector<int>; 

const int scale[] = {256, 512, 1024, 2048};
const string data_path("./data/");

// __global__ void Gemm(const int &size, vec &a, vec &b, vec &c)
__global__ void Gemm(const int &size, const int *a, const int *b, int *c)
{
    int tx = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = blockIdx.y * blockDim.y + threadIdx.y;
    int M = size, N = size, K=size;
    if(ty < M && tx < N) {
        int ans = 0;
        for(int i = 0; i < K; ++i){
            ans += a[ty * K + i] * b[i * N + tx];  // 1 FMA, 2 load, 计算访存比太低
        }
        c[ty * N + tx] = ans;
        
        // c[1] = C;
        // C = 1;
    }
}

void CheckResult(const vec &c, const string &result_path)
{
    ifstream file_result(result_path);
    int nelems = c.size();
    float res_i;
    for (int i = 0; i < nelems; i++)
    {
        file_result >> res_i;
        // assert(c[i] == res_i);
    }
    file_result.close();
}

// c = a * b
void Benchmark(const int &size)
{


    const int nelems = size * size;
    const string a_path(data_path + to_string(size) + "/a");
    const string b_path(data_path + to_string(size) + "/b");
    const string result_path(data_path + to_string(size) + "/result");
    ifstream file_a(a_path);
    ifstream file_b(b_path);

    // vec a(nelems, 0);
    // vec b(nelems, 0);
    vec c(nelems, 0);
    int *a_h, *b_h, *c_h;
    a_h = (int*)malloc(nelems * sizeof(int));
    b_h = (int*)malloc(nelems * sizeof(int));
    c_h = (int*)malloc(nelems * sizeof(int));

    int *a_d, *b_d, *c_d;
    cudaMalloc((void**)&a_d, nelems*sizeof(int));
    cudaMalloc((void**)&b_d, nelems*sizeof(int));
    cudaMalloc((void**)&c_d, nelems*sizeof(int));

    for (int i = 0; i < nelems; i++)
    {
        file_a >> a_h[i];
        // a_h[i] = a[i];
    }
    for (int i = 0; i < nelems; i++)
    {
        file_b >> b_h[i];
        // b_h[i] = b[i];
        // c_h[i] = 0;
    }

    //将host拷贝到device
    cudaMemcpy((void*)a_d, (void*)a_h, nelems*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy((void*)b_d, (void*)b_h, nelems*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy((void*)c_d, (void*)c_h, nelems*sizeof(int), cudaMemcpyHostToDevice);

    // cout<<nelems<<" "<<sizeof(c_d)<<"\n";
    // cout<<sizeof(a_h)<<"\n";


    dim3 BlockSize(32, 32);
    dim3 GridSize((size+BlockSize.x-1)/BlockSize.x, (size+BlockSize.y-1)/BlockSize.y);
    // PRINT_TIME(
        // Gemm<<<GridSize, BlockSize>>>(size, a_d, a_d, c_d););
    Gemm<<<GridSize, BlockSize>>>(size, a_d, b_d, c_d);

    cudaMemcpy((void*)c_h, (void*)c_d, nelems*sizeof(int), cudaMemcpyDeviceToHost);

    for(int i = 0;i<nelems;i++)
        c[i] = c_h[i];

    CheckResult(c, result_path);

    free(a_h);
    free(b_h);
    free(c_h);

    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(c_d);

    // cudaDeviceReset();
    cudaError_t err = cudaGetLastError(); // add
    if (err != cudaSuccess)
        std::cout << "CUDA error: " << cudaGetErrorString(err) << std::endl; // add
    cudaProfilerStop();

    file_a.close();
    file_b.close();
}

int main()
{
    int dev = 1;
    cudaSetDevice(dev);

    for (auto size : scale)
    {
        cout << "Running, dataset: size " << size << endl;
        Benchmark(size);
        cout << "Passed, dataset: size " << size << endl;
        cout << endl;
    }
    return 0;
}