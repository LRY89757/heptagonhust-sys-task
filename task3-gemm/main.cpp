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


}
void multikernel1(vec &c, vec&a, 
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
        assert(c[i] == res_i);
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

    for(int i = 0; i < nelems; i++) {
        file_a >> a[i];
    }
    for(int i = 0; i < nelems; i++) {
        file_b >> b[i];
    }

    PRINT_TIME(
       Gemm(size, a, b, c);
    );
    
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