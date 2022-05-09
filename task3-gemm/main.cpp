#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <vector>
#include <cassert>

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

void Gemm(const int &size, vec &a, vec &b, vec &c) {
    // for(int i = 0; i < size; i++)
    //     for(int j = 0; j < size; j++)
    //         for(int k = 0; k < size; k++)
    //             c[i*size+j] += a[i*size+k] * b[k*size+j];

    // vec tr[4];
    vector<vec> tr(4, vec(size, 0));
    // int tr[4][size];
    // vec c(nelems, 0);

    // for(int i = 0;i<size;i+=4)
    // {
    //     for(int j = 0;j<size;j+=4)
    //     {
    //         ;
    //     }
    // }
    for(int j = 0;j<size;j+=4)
    {
        // 先取出4列
        for(int i = 0;i<size;i++)
        {
            tr[0][i] = b[i*size+j];
            tr[1][i] = b[i*size+j+1];
            tr[2][i] = b[i*size+j+2];
            tr[3][i] = b[i*size+j+3];
        }

        // 目前需要一个kernel计算的是a中的4行和b中刚刚取出的4列
        for(int k = 0;k<size;k+=4)
        // for循环来遍历了所有的a中的4行
        {
            multikernel(c, a, tr, k, j, size); // 注意这里是a中的4行和tr中4行来相乘
        }

    }

}

void multikernel(vec &c, vec&a, 
                    vector<vec> b, int row, int col, const int&size)
{
    register int t[4][4] = {0};  // t[r][c]负责存第r行乘以对应列的答案

    for(int i = 0;i<size;i++)
    {
        for(int r=0;r<4;r++)
        {
            for(int c=0;c<4;c++)
            {
               t[r][c] += a[((row+r)*size) + i] * b[c][i];
            }
        }
        // for(int idx = 0;idx<16;idx++)
        // {
        //     t[idx] += 
        // }
    }

    for(int r=0;r<4;r++)
        for(int cc = 0;cc<4;cc++)
            c[((row+r)*size)+col+cc] = t[r][cc];

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