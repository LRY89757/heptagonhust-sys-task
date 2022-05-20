#include <iostream>
#include <mmintrin.h>
#include <xmmintrin.h>  // SSE
#include <pmmintrin.h>  // SSE2
#include <emmintrin.h>  // SSE3
#include <immintrin.h>  // AVX
#include <omp.h>
// g++ -mavx -mavx2 -mfma -msse -msse2 -msse3 -mavx512bw -mavx512vl -mavx512f -mavx512cd -mavx512dq fmadd.cpp && ./a.out
using namespace std;

int main(int argc, char const*argv[])
{

    // load the data
    int i;
    // int * aligned_int = (int*)aligned_alloc(32, 8*8*sizeof(int));
    int * aligned_int = (int*)aligned_alloc(64, 8*8*sizeof(int));
    for(i=0;i<16;i++)
        aligned_int[i] = i+1;
    // __m256i int_vec = _mm256_load_si256((const __m256i*)aligned_int);
    // int* i_v = (int*) &int_vec;
    // printf("int:\t\t%d, %d, %d, %d, %d, %d, %d, %d\n", i_v[0], i_v[1], i_v[2], i_v[3], i_v[4], i_v[5], i_v[6], i_v[7]);

    __m512i int_vec512 = _mm512_load_si512((const __m512i*)aligned_int);
    __m512i int_vec512_0 = _mm512_load_si512((const __m512i*)aligned_int);

    __m512i add = int_vec512 * int_vec512_0;

    // int *i_512 = (int*)&int_vec512;
    int *i_512 = (int*)&add;
    // cout<<"int512:\t\t";
    cout<<"add512:\t\t";
    for(int i = 0;i<16;i++)
        cout<<i_512[i]<<",";
    cout<<'\n';

    __m512i result[16] = {_mm512_set1_epi32(0)};
    cout<<"sizeof(result)"<<sizeof(result)<<"\n";
    int *i512 = (int *)&result[0];
    int *i51 = (int*)&result[8];
    cout<<"int51:\t\t";
    for(int i = 0;i<16;i++)
        cout<<i51[i]<<" ";







    __m256i int_256_vec_0 = _mm256_set1_epi32(3);
    __m256i int_256_vec_1 = _mm256_set1_epi32(4);
    __m256i int_256_vec_2 = _mm256_set1_epi32(4);

    // 注意对比下区别
    __m256i int_256_result1 = _mm256_mul_epi32(int_256_vec_0, int_256_vec_1);
    __m256i int_256_result2 = _mm256_mullo_epi32(int_256_vec_0, int_256_vec_1);
    

    long long* in = (long long*)&int_256_result1;
    int* inn = (int*)&int_256_result2;
    int* in1 = (int*)&int_256_vec_0;
    cout<<"long long:\t:"<<in[0]<<"\t"<<in[1]<<"\t"<<in[2]<<"\t"<<in[3]<<"\n";
    cout<<"int:\t\t:"<<inn[0]<<"\t"<<inn[1]<<"\t"<<inn[2]<<"\t"<<inn[3]<<"\n";
    cout<<"int:\t\t:"<<in1[0]<<"\t"<<in1[1]<<"\t"<<in1[2]<<"\t"<<in1[3]<<"\n";

    __m512i a = _mm512_set1_epi32(8);
    __m512i b = _mm512_set1_epi32(8);
    __m512i c = _mm512_set1_epi32(0);
    for(int i = 0;i<7;i++)
        c += _mm512_mullo_epi32(a, b);
    cout<<"int:\t\t";
    inn = (int*)&c;
    #pragma parallel for
    for(int i = 0;i<16;i++)
        cout<<inn[i]<<'\t';
    cout<<"\n";


    // Single-precision multiply and add with 128-bit vectors (FMA)
    __m128 float_128_vec_0 = _mm_set1_ps(8.0);
    __m128 float_128_vec_1 = _mm_set1_ps(20.0);
    __m128 float_128_vec_2 = _mm_set1_ps(2.0);

    __m128 float_128_result = _mm_fmadd_ps(float_128_vec_0, float_128_vec_1, float_128_vec_2);

    float* flo = (float*) &float_128_result;
    printf("float:\t\t%f, %f, %f, %f\n", flo[0], flo[1], flo[2], flo[3]);

    // Double-precision multiply and add with 128-bit vectors (FMA)
    __m128d double_128_vec_0 = _mm_set1_pd(8.0);
    __m128d double_128_vec_1 = _mm_set1_pd(20.0);
    __m128d double_128_vec_2 = _mm_set1_pd(2.0);

    __m128d double_128_result = _mm_fmadd_pd(double_128_vec_0, double_128_vec_1, double_128_vec_2);

    double* dou = (double*) &double_128_result;
    printf("double:\t\t%lf, %lf\n", dou[0], dou[1]);

    // Single-precision multiply and add with 256-bit vectors (FMA)
    __m256 float_256_vec_0 = _mm256_set1_ps(8.0);
    __m256 float_256_vec_1 = _mm256_set1_ps(20.0);
    __m256 float_256_vec_2 = _mm256_set1_ps(2.0);

    __m256 float_256_result = _mm256_fmadd_ps(float_256_vec_0, float_256_vec_1, float_256_vec_2);

    flo = (float*) &float_256_result;
    printf("float:\t\t%f, %f, %f, %f, %f, %f, %f, %f\n", flo[0], flo[1], flo[2], flo[3], flo[4], flo[5], flo[6], flo[7]);

    // Double-precision multiply and add with 256-bit vectors (FMA)
    __m256d double_256_vec_0 = _mm256_set1_pd(8.0);
    __m256d double_256_vec_1 = _mm256_set1_pd(20.0);
    __m256d double_256_vec_2 = _mm256_set1_pd(2.0);

    __m256d double_256_result = _mm256_fmadd_pd(double_256_vec_0, double_256_vec_1, double_256_vec_2);

    dou = (double*) &double_256_result;
    printf("double:\t\t%lf, %lf, %lf, %lf\n", dou[0], dou[1], dou[2], dou[3]);

    // Single-precision multiply and add the lowest element with 128-bit vectors (FMA)
    __m128 float_128_low_result = _mm_fmadd_ss(float_128_vec_0, float_128_vec_1, float_128_vec_2);

    flo = (float*) &float_128_low_result;
    printf("float:\t\t%f, %f, %f, %f\n", flo[0], flo[1], flo[2], flo[3]);

    // Double-precision multiply and add the lowest element with 128-bit vectors (FMA)
    __m128d double_128_low_result = _mm_fmadd_sd(double_128_vec_0, double_128_vec_1, double_128_vec_2);

    dou = (double*) &double_128_low_result;
    printf("double:\t\t%lf, %lf\n", dou[0], dou[1]);


    return 0;
}


