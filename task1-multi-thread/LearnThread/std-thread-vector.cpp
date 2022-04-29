// g++ std-thread-vector.cpp -o a.out -std=c++11 -pthread
#include <iostream>
#include <thread>
#include <vector>

void foo(int n) { std ::cout << "foo() " << n << std ::endl; }

int main()
{
    std ::vector<std ::thread> threads;
    int thread_num = 8;

    for (int i = 0; i < thread_num; i++)
    {
        threads.push_back(std ::thread(foo, i));
    }

    for (int i = 0; i < thread_num; i++)
    {
        threads[i].join();
    }

    std ::cout << "main() exit.\n";

    return 0;
}