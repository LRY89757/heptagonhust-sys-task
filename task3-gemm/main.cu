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
__global__ void Gemm(const int &size, int *a, int *b, int *c)
{
    int tx = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = blockIdx.y * blockDim.y + threadIdx.y;
    int M = size, N = size, K=size;
    if(ty < M && tx < N) {
        int C = 0;
        for(int i = 0; i < K; ++i){
            C += a[ty * K + i] * b[i * N + tx];  // 1 FMA, 2 load, 计算访存比太低
        }
        c[ty * N + tx] = C;
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
        assert(c[i] == res_i);
    }
    file_result.close();
}

// c = a * b
void Benchmark(const int &size)
{
    int dev = 0;
    cudaSetDevice(dev);
    
    const int nelems = size * size;
    const string a_path(data_path + to_string(size) + "/a");
    const string b_path(data_path + to_string(size) + "/b");
    const string result_path(data_path + to_string(size) + "/result");
    ifstream file_a(a_path);
    ifstream file_b(b_path);

    vec a(nelems, 0);
    vec b(nelems, 0);
    vec c(nelems, 0);
    int *a_h, *b_h, *c_h;
    a_h = (int*)malloc(nelems * sizeof(int));
    b_h = (int*)malloc(nelems * sizeof(int));
    c_h = (int*)malloc(nelems * sizeof(int));

    for (int i = 0; i < nelems; i++)
    {
        file_a >> a[i];
        a_h[i] = a[i];
    }
    for (int i = 0; i < nelems; i++)
    {
        file_b >> b[i];
        b_h[i] = b[i];
    }


    int *a_d, *b_d, *c_d;
    cudaMalloc((void**)&a_d, size*size*sizeof(int));
    cudaMalloc((void**)&b_d, size*size*sizeof(int));
    cudaMalloc((void**)&c_d, size*size*sizeof(int));

    //将host拷贝到device
    cudaMemcpy((void*)a_d, (void*)a_h, nelems*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy((void*)b_d, (void*)b_h, nelems*sizeof(int), cudaMemcpyHostToDevice);


    dim3 BlockSize(32, 32);
    dim3 GridSize((size+32-1)/32, (size+32-1)/32);
    // PRINT_TIME(
        // Gemm<<<GridSize, BlockSize>>>(size, a_d, a_d, c_d););
    Gemm<<<GridSize, BlockSize>>>(size, a_d, a_d, c_d);

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

    file_a.close();
    file_b.close();
}

int main()
{
    for (auto size : scale)
    {
        cout << "Running, dataset: size " << size << endl;
        Benchmark(size);
        cout << "Passed, dataset: size " << size << endl;
        cout << endl;
    }
    return 0;
}