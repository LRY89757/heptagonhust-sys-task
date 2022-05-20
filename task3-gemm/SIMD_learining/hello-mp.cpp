#include<iostream>
#include<omp.h>
#define NUM_THREADS 8

// g++ -fopenmp intergral.cpp &&  ./a.out

int main(){
    omp_set_num_threads(8);  // 可以用来确定使用多少个线程
    #pragma omp parallel
    {
        int ID = omp_get_thread_num();
        std::cout<<"hello "<<ID<<'\n';
        std::cout<<"hello again "<<ID<<'\n';
    }
    return 0;
}
/*
hello hello hello hello hello 026
hello again 2
hello 8
hello again 8
hello 9
hello again 9
hello 12
hello again 12
hello 15
hello again 15
hello 19
hello again 19
18
hello again 18
hello 11
hello again 11
hello 13
hello 5
hello again 5
hello 1
hello 4
hello again 4
hello again 13

hello again 0
hello 17
hello again 17
hello 16
3
hello again 3

hello again 6
hello again 16
hello 10
hello again 10
hello 7
hello again 7
hello 14
hello again 14
hello again 1
*/