// g++ std-thread-array.cpp -o a.out -std=c++11 -pthread
#include <iostream>
#include <thread>

// 从打印的输出可以看出实际上各个进程是同步进行的，因为打印输出是同步的。


void foo(int n) { std ::cout << "foo() " << n <<std::endl; }

int main()
{
    int thread_num = 8;

    std ::thread threads[thread_num];

    for (int i = 0; i < thread_num; i++)
    {
        threads[i] = std ::thread(foo, i);
    }

    for (int i = 0; i < thread_num; i++)
    {
        threads[i].join();
    }

    std ::cout << "main() exit.\n";

    return 0;
}


