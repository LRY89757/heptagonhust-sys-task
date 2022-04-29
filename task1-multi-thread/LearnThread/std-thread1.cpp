// g++ std-thread1.cpp -o a.out -std=c++11 -pthread
#include <iostream>
#include <thread>

void foo() { std ::cout << "foo\n"; }

void bar(int x) { std ::cout << "bar\n"; }

int main()
{
    std ::thread t1(foo);                                            // 建立一个新执行绪且执行foo 函式
    std ::thread t2(bar, 0);                                         // 建立一个新执行绪且执行bar 函式
    std ::cout << "main, foo and bar now execute concurrently...\n"; // synchronize threads

    std ::cout << "sleep 1s\n";
    std ::this_thread::sleep_for(std ::chrono::seconds(1));

    std ::cout << "join t1\n";
    t1.join(); // 等待t1 执行绪结束
    std ::cout << "join t2\n";
    t2.join(); // 等待t2 执行绪结束

    std ::cout << "foo and bar completed.\n";

    return 0;
}