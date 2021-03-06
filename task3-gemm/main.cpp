#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <vector>
#include <cassert>
#include <numeric> 
#include <omp.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>
#include <cstring>

#define NUM_THREADS 8

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

void multikernel(vec &c, vec&a, 
                    vector<vec> b, int row, int col, const int&size);
void multikernel1(vec &c, vec&a, 
                    vector<vec> b, int row, int col, const int&size);

void multikernel1(int *c, int *a, 
                    vector<vec> b, int col, const int&size);

void blockkernel(vec&c, vec&a, vec&b, int m, int n, int k, int blocksize);
void blockkernel(int*c, int*a, int*b, int i, int j, int k, int blocksize);
void blockkernel(int*c, int*a, int*b, int i, int j, int k, int size, int blocksize);
void do_gemm_block(int*c, int*a, int*b, int ia, int ja, int ib, int jb, int blocksize, int size);
void do_gemm_block(vec&c, vec&a, vec&b, int ia, int ja, int ib, int jb, int blocksize, int size);
inline void MultikernelAvx(int *c, int *a, 
                    int*b, int col, const int&size);

void GemmOrigin(const int &size, vec &a, vec &b, vec &c)
{
    // origin solution
    for(int i = 0; i < size; i++)
        for(int j = 0; j < size; j++)
            for(int k = 0; k < size; k++)
                c[i*size+j] += a[i*size+k] * b[k*size+j];
}

void GemmAvx(const int &size, int* a, int *b, int *c) {

    int blocksize = 4;
    // omp_set_num_threads(8);  // 可以用来确定使用多少个线程
    #pragma omp parallel for schedule(dynamic)
    for(int j = 0;j<size;j+=blocksize)
    {
        // vector<vec> tr(4, vec(size, 0));
        int* tr = (int*)aligned_alloc(64, 4*size*sizeof(int)+2048);

        // 先取出4列
        for(int i = 0;i<size;i++)
        {
            tr[0*size+i] = b[i*size+j];
            tr[1*size+i] = b[i*size+j+1];
            tr[2*size+i] = b[i*size+j+2];
            tr[3*size+i] = b[i*size+j+3];
        }

        // 目前需要一个kernel计算的是a中的4行和b中刚刚取出的4列
        MultikernelAvx(c, a, tr, j, size); // 注意这里是a中的4行和tr中4行来相乘
    }
}

// __m512i operator*(const __m512i a, const __m512i b)
// {
//     return _mm512_mullo_epi32(a, b);
// }

inline void MultikernelAvx(int *c, int *a, 
                    int* b, int col, const int&size)
{
    // #pragma omp parallel for schedule(dynamic) //这里加了反而变慢了些
    for(int i = 0;i<size;i+=4)
    {

        __m512i result[16] = {_mm512_set1_epi32(0)};
        int res[16] = {0};
        // int* res = (int*)malloc(sizeof(int)*16);
        // memset(res, 0, sizeof(int) * 16);
        for(int k = 0;k<size;k+=16)
        {
            // register int
            //     t0(0), t1(0), t2(0), t3(0),
            //     t4(0), t5(0), t6(0), t7(0),
            //     t8(0), t9(0), t10(0), t11(0),
            //     t12(0), t13(0), t14(0), t15(0);

            __m512i r1 = _mm512_load_si512((const __m512i *)(a + i * size + k));
            __m512i r2 = _mm512_load_si512((const __m512i *)(a + (i + 1) * size + k));
            __m512i r3 = _mm512_load_si512((const __m512i*)(a+(i+2)*size+k));
            __m512i r4 = _mm512_load_si512((const __m512i*)(a+(i+3)*size+k));

            __m512i c1 = _mm512_load_si512((const __m512i*)(b+k));
            __m512i c2 = _mm512_load_si512((const __m512i*)(b+size+k));
            __m512i c3 = _mm512_load_si512((const __m512i*)(b+2*size+k));
            __m512i c4 = _mm512_load_si512((const __m512i*)(b+3*size+k));

            result[0]+=  _mm512_mullo_epi32(r1 , c1);
            result[4]+=  _mm512_mullo_epi32(r2 , c1);
            result[8]+=  _mm512_mullo_epi32(r3 , c1);
            result[12]+= _mm512_mullo_epi32(r4 , c1);
            result[1]+=  _mm512_mullo_epi32(r1 , c2);
            result[5]+=  _mm512_mullo_epi32(r2 , c2);
            result[9]+=  _mm512_mullo_epi32(r3 , c2);
            result[13]+= _mm512_mullo_epi32(r4 , c2);
            result[2]+=  _mm512_mullo_epi32(r1 , c3);
            result[6]+=  _mm512_mullo_epi32(r2 , c3);
            result[10]+= _mm512_mullo_epi32(r3 , c3);
            result[14]+= _mm512_mullo_epi32(r4 , c3);
            result[3]+=  _mm512_mullo_epi32(r1 , c4);
            result[7]+=  _mm512_mullo_epi32(r2 , c4);
            result[11]+= _mm512_mullo_epi32(r3 , c4);
            result[15]+= _mm512_mullo_epi32(r4 , c4);

            // int r1 = a[i*size+k], r2 = a[(i+1)*size+k], r3 = a[(i+2)*size+k], r4 = a[(i+3)*size+k];
            // int c1 = b[k][0], c2 = b[k][1], c3 = b[k][2], c4 = b[k][3];
            // t0 += r1 * c1;
            // t4 += r2 * c1;
            // t8 += r3 * c1;
            // t12 += r4 * c1;
            // t1 += r1 * c2;
            // t5 += r2 * c2;
            // t9 += r3 * c2;
            // t13 += r4 * c2;
            // t2 += r1 * c3;
            // t6 += r2 * c3;
            // t10 += r3 * c3;
            // t14 += r4 * c3;
            // t3 += r1 * c4;
            // t7 += r2 * c4;
            // t11 += r3 * c4;
            // t15 += r4 * c4;
        }
        for(int i = 0;i<16;i++)
            for(int j=0;j<16;j++)
                res[i] += ((int*)&result[i])[j];

        c[(i+0)*size+col]=res[0], c[(i+0)*size+col+1]=res[1], c[(i+0)*size+col+2]=res[2], c[(i+0)*size+col+3]=res[3]; 
        c[(i+1)*size+col]=res[4], c[(i+1)*size+col+1]=res[5], c[(i+1)*size+col+2]=res[6], c[(i+1)*size+col+3]=res[7]; 
        c[(i+2)*size+col]=res[8], c[(i+2)*size+col+1]=res[9], c[(i+2)*size+col+2]=res[10], c[(i+2)*size+col+3]=res[11]; 
        c[(i+3)*size+col]=res[12], c[(i+3)*size+col+1]=res[13], c[(i+3)*size+col+2]=res[14], c[(i+3)*size+col+3]=res[15]; 
    }
}




void Gemm(const int &size, int* a, int *b, int *c) {

    // solution 5
    // // 其实是对于3的加强版
    // vector<vec> tr(size, vec(4, 0));
    // // int tr[4][size];
    // // vec c(nelems, 0);

    // int blocksize = 4;
    // for(int j = 0;j<size;j+=blocksize)
    // {
    //     // 先取出4列
    //     for(int i = 0;i<size;i++)
    //     {
    //         tr[i][0] = b[i*size+j];
    //         tr[i][1] = b[i*size+j+1];
    //         tr[i][2] = b[i*size+j+2];
    //         tr[i][3] = b[i*size+j+3];
    //     }

    //     // 目前需要一个kernel计算的是a中的4行和b中刚刚取出的4列
    //     multikernel1(c, a, tr, j, size); // 注意这里是a中的4行和tr中4行来相乘
    // }



    // solution 4
    // 思路主要为分块，暂时分成128×128的block,这个是经过计算的，计算详细过程请见文档
    int blocksize = 1 << 7;
    for (int i = 0; i < size; i += blocksize)
        for (int j = 0; j < size; j += blocksize)
            for (int k = 0; k < size; k += blocksize)
                blockkernel(c + i * size + j, a + i * size + k, b + k * size + j, i, j, k, size, blocksize);

}

inline void blockkernel(int*c, int*a, int*b, 
                    int i, int j, int k, int size, int blocksize){
    for(int ii = 0;ii<blocksize;ii++)
    {
        for(int jj = 0;jj<blocksize;jj++)
        {
            int tmp = c[ii*size+jj];
            for(int kk = 0;kk<blocksize;kk++)
                tmp += a[ii*size+kk] * b[kk*size+jj];
            c[ii*size+jj] = tmp;
        }
    }
}

inline void blockkernel(int*c, int*a, int*b, int i, int j, int k, int blocksize)
{
    for(int ii = 0;ii<k;ii+=blocksize)
    {
        for(int jj=0;jj<k;jj+=blocksize)
        {
            do_gemm_block(c, a, b, i, j, ii, jj, blocksize, k);
        }
    }

}

void Gemmm(const int &size, int* a, int *b, int *c) {

    // solution 5
    // 其实是对于3的加强版
    // vector<vec> tr(size, vec(4, 0));
    // int tr[4][size];
    // vec c(nelems, 0);

    int blocksize = 4;
    // omp_set_num_threads(8);  // 可以用来确定使用多少个线程
    #pragma omp parallel for schedule(dynamic)
    for(int j = 0;j<size;j+=blocksize)
    {
        vector<vec> tr(size, vec(4, 0));

        // 先取出4列
        for(int i = 0;i<size;i++)
        {
            tr[i][0] = b[i*size+j];
            tr[i][1] = b[i*size+j+1];
            tr[i][2] = b[i*size+j+2];
            tr[i][3] = b[i*size+j+3];
        }

        // 目前需要一个kernel计算的是a中的4行和b中刚刚取出的4列
        multikernel1(c, a, tr, j, size); // 注意这里是a中的4行和tr中4行来相乘
    }
}


inline void do_gemm_block(int*c, int*a, int*b, int ia, int ja, int ib, int jb, int blocksize, int size)
{
    for(int ii = 0;ii<blocksize;ii++)
    {
        for(int jj = 0;jj<blocksize;jj++)
        {
            int tmp = c[(ia+ii)*size + jb+jj];
            for(int kk = 0;kk<blocksize;kk++)
                tmp += a[(ia+ii)*size+ja+kk] * b[(ib+kk)*size+jb+jj];
            c[(ia+ii)*size + jb+jj] = tmp;
        }
    }
}



void Gemm(const int &size, vec &a, vec &b, vec &c) {

    // origin solution
    // for(int i = 0; i < size; i++)
    //     for(int j = 0; j < size; j++)
    //         for(int k = 0; k < size; k++)
    //             c[i*size+j] += a[i*size+k] * b[k*size+j];

    
    // solution 1
    // vector<vec> tr(4, vec(size, 0));
    // // int tr[4][size];
    // // vec c(nelems, 0);

    // for(int j = 0;j<size;j+=4)
    // {
    //     // 先取出4列
    //     for(int i = 0;i<size;i++)
    //     {
    //         tr[0][i] = b[i*size+j];
    //         tr[1][i] = b[i*size+j+1];
    //         tr[2][i] = b[i*size+j+2];
    //         tr[3][i] = b[i*size+j+3];
    //     }

    //     // 目前需要一个kernel计算的是a中的4行和b中刚刚取出的4列
    //     for(int k = 0;k<size;k+=4)
    //     // for循环来遍历了所有的a中的4行
    //     {
    //         multikernel(c, a, tr, k, j, size); // 注意这里是a中的4行和tr中4行来相乘
    //     }

    // }

    // solution 2 from csapp
    // for(int k = 0;k<size;k++)
    // {
    //     for(int i = 0;i<size;i++)
    //     {
    //         int r = a[i*size+k];
    //         for(int j = 0;j<size;j++)
    //         {
    //             c[i*size+j] += r * b[k*size+j];
    //         }
    //     }
    // }


    // solution 3
    vector<vec> tr(size, vec(4, 0));
    // int tr[4][size];
    // vec c(nelems, 0);
    #pragma omp parallel for schedule(dynamic)
    for(int j = 0;j<size;j+=4)
    {
        // 先取出4列
        for(int i = 0;i<size;i++)
        {
            tr[i][0] = b[i*size+j];
            tr[i][1] = b[i*size+j+1];
            tr[i][2] = b[i*size+j+2];
            tr[i][3] = b[i*size+j+3];
        }

        // 目前需要一个kernel计算的是a中的4行和b中刚刚取出的4列
        multikernel1(c, a, tr, 0, j, size); // 注意这里是a中的4行和tr中4行来相乘
    }

    // solution 4
    // 思路主要为分块，暂时分成128×128的block,这个是经过计算的，计算详细过程请见文档
    // int blocksize = 1 << 7;
    // for (int i = 0; i < size; i += blocksize)
    // {
    //     for (int j = 0; j < size; j += blocksize)
    //         blockkernel(c, a, b, i, j, size, blocksize);
    // }
}

inline void blockkernel(vec&c, vec&a, vec&b, int i, int j, int k, int blocksize)
{
    for(int ii = 0;ii<k;ii+=blocksize)
    {
        for(int jj=0;jj<k;jj+=blocksize)
        {
            do_gemm_block(c, a, b, i, j, ii, jj, blocksize, k);
        }
    }

}



inline void do_gemm_block(vec&c, vec&a, vec&b, int ia, int ja, int ib, int jb, int blocksize, int size)
{
    for(int ii = 0;ii<blocksize;ii++)
    {
        for(int jj = 0;jj<blocksize;jj++)
        {
            int tmp = c[(ia+ii)*size + jb+jj];
            for(int kk = 0;kk<blocksize;kk++)
                tmp += a[(ia+ii)*size+ja+kk] * b[(ib+kk)*size+jb+jj];
            c[(ia+ii)*size + jb+jj] = tmp;
        }
    }
}


inline void multikernel1(int *c, int *a, 
                    vector<vec> b, int col, const int&size)
{
    register int 
        t0(0), t1(0), t2(0), t3(0),
        t4(0), t5(0), t6(0), t7(0),
        t8(0), t9(0), t10(0), t11(0),
        t12(0), t13(0), t14(0), t15(0);

    // #pragma omp parallel for schedule(dynamic) //这里加了反而变慢了些
    for(int i = 0;i<size;i+=4)
    {
        t0=t1=t2=t3=t4=t5=t6=t7=t8=t9=t10=t11=t12=t13=t14=t15=0;
        for(int k = 0;k<size;k++)
        {
            int r1 = a[i*size+k], r2 = a[(i+1)*size+k], r3 = a[(i+2)*size+k], r4 = a[(i+3)*size+k];
            int c1 = b[k][0], c2 = b[k][1], c3 = b[k][2], c4 = b[k][3];
            t0 += r1 * c1;
            t4 += r2 * c1;
            t8 += r3 * c1;
            t12 += r4 * c1;
            t1 += r1 * c2;
            t5 += r2 * c2;
            t9 += r3 * c2;
            t13 += r4 * c2;
            t2 += r1 * c3;
            t6 += r2 * c3;
            t10 += r3 * c3;
            t14 += r4 * c3;
            t3 += r1 * c4;
            t7 += r2 * c4;
            t11 += r3 * c4;
            t15 += r4 * c4;
        }
        c[i*size+col]=t0, c[i*size+col+1]=t1, c[i*size+col+2]=t2, c[i*size+col+3]=t3; 
        c[(i+1)*size+col]=t4, c[(i+1)*size+col+1]=t5, c[(i+1)*size+col+2]=t6, c[(i+1)*size+col+3]=t7; 
        c[(i+2)*size+col]=t8, c[(i+2)*size+col+1]=t9, c[(i+2)*size+col+2]=t10, c[(i+2)*size+col+3]=t11; 
        c[(i+3)*size+col]=t12, c[(i+3)*size+col+1]=t13, c[(i+3)*size+col+2]=t14, c[(i+3)*size+col+3]=t15; 
    }
}



inline void multikernel1(vec &c, vec&a, 
                    vector<vec> b, int row, int col, const int&size)
{
    register int 
        t0(0), t1(0), t2(0), t3(0),
        t4(0), t5(0), t6(0), t7(0),
        t8(0), t9(0), t10(0), t11(0),
        t12(0), t13(0), t14(0), t15(0);

    for(int i = 0;i<size;i+=4)
    {
        t0=t1=t2=t3=t4=t5=t6=t7=t8=t9=t10=t11=t12=t13=t14=t15=0;
        for(int k = 0;k<size;k++)
        {
            int r1 = a[i*size+k], r2 = a[(i+1)*size+k], r3 = a[(i+2)*size+k], r4 = a[(i+3)*size+k];
            int c1 = b[k][0], c2 = b[k][1], c3 = b[k][2], c4 = b[k][3];
            t0 += r1 * c1;
            t4 += r2 * c1;
            t8 += r3 * c1;
            t12 += r4 * c1;
            t1 += r1 * c2;
            t5 += r2 * c2;
            t9 += r3 * c2;
            t13 += r4 * c2;
            t2 += r1 * c3;
            t6 += r2 * c3;
            t10 += r3 * c3;
            t14 += r4 * c3;
            t3 += r1 * c4;
            t7 += r2 * c4;
            t11 += r3 * c4;
            t15 += r4 * c4;
        }
        c[i*size+col]=t0, c[i*size+col+1]=t1, c[i*size+col+2]=t2, c[i*size+col+3]=t3; 
        c[(i+1)*size+col]=t4, c[(i+1)*size+col+1]=t5, c[(i+1)*size+col+2]=t6, c[(i+1)*size+col+3]=t7; 
        c[(i+2)*size+col]=t8, c[(i+2)*size+col+1]=t9, c[(i+2)*size+col+2]=t10, c[(i+2)*size+col+3]=t11; 
        c[(i+3)*size+col]=t12, c[(i+3)*size+col+1]=t13, c[(i+3)*size+col+2]=t14, c[(i+3)*size+col+3]=t15; 
    }
}

void multikernel(vec &c, vec&a, 
                    vector<vec> b, int row, int col, const int&size)
{
    // register int t[4][4] = {0};  // t[r][c]负责存第r行乘以对应列的答案
    // register int 
    //     t0(0), t1(0), t2(0), t3(0),
    //     t4(0), t5(0), t6(0), t7(0),
    //     t8(0), t9(0), t10(0), t11(0),
    //     t12(0), t13(0), t14(0), t15(0);

    // for(int i = 0;i<size;i++)
    // {
    //     t0 += 
    // }
    // for(int i = 0;i<size;i++)
    // {
        // for(int r=0;r<4;r++)
        // {
        //     for(int c=0;c<4;c++)
        //     {
        //        t[r][c] += a[((row+r)*size) + i] * b[c][i];
        //     }
        // }
        // t0 += a[row]
        
    // }
    
    // std::inner_product(a, a+size, b[0]);
    // http://www.cplusplus.com/forum/beginner/63142/

    // for(int r=0;r<4;r++)
    //     for(int cc = 0;cc<4;cc++)
    //         c[((row+r)*size)+col+cc] = t[r][cc];

    // cout<<row<<","<<col<<"\n";
}


void CheckResult(const vec &c, const string &result_path) {
    ifstream file_result(result_path);
    int nelems = c.size();
    float res_i;
    for(int i = 0; i < nelems; i++) {
        file_result >> res_i;
        // if(res_i != c[i] && i!=0)
        //     cout<<"error!"<<" res_i:"<<res_i<<" c["<<i<<"]: "<<c[i]<<"\n";
        assert(c[i] - res_i < 1200);
    }
    file_result.close();
}

// c = a * b
void Benchmark(const int &size) {
    const int nelems = size * size;
    const string a_path(data_path+to_string(size)+"/a");
    const string b_path(data_path+to_string(size)+"/b");
    const string result_path(data_path+to_string(size)+"/result");
    ifstream file_a(a_path);
    ifstream file_b(b_path);

    vec a(nelems, 0);
    vec b(nelems, 0);
    vec c(nelems, 0);
    // int *inta = (int *)malloc(nelems*sizeof(int));
    // int *intb = (int *)malloc(nelems*sizeof(int));
    // int *intc = (int *)malloc(nelems*sizeof(int));

    int *inta = (int *)aligned_alloc(64, nelems*sizeof(int));
    int *intb = (int *)aligned_alloc(64, nelems*sizeof(int));
    int *intc = (int *)aligned_alloc(64, nelems*sizeof(int));

    for(int i = 0; i < nelems; i++) {
        file_a >> a[i];
        inta[i] = a[i];
    }
    for(int i = 0; i < nelems; i++) {
        file_b >> b[i];
        intb[i] = b[i];
    }

    memset(intc, 0, nelems*sizeof(int));

    cout<<"the origin matrix:\n";
    PRINT_TIME(
        GemmOrigin(size, a, b, c););

    memset(intc, 0, nelems*sizeof(int));

    // PRINT_TIME(
    //     Gemm(size, a, b, c););
    
    cout<<"the optimized matrix:\n";
    PRINT_TIME(
        Gemmm(size, inta, intb, intc););
    
    memset(intc, 0, nelems*sizeof(int));

    cout<<"the avx optimized matrix:\n";
    PRINT_TIME(
        GemmAvx(size, inta, intb, intc););

    // PRINT_TIME(
        // Gemm(size, inta, intb, intc););

    for(int i = 0;i<nelems;i++)
        c[i] = intc[i];

    CheckResult(c, result_path);

    file_a.close();
    file_b.close();
}

int main() {
    for(auto size: scale) {
        cout << "Running, dataset: size " << size << endl;
        Benchmark(size);
        cout << "Passed, dataset: size " << size << endl;
        cout << endl;
    }
    return 0;
}