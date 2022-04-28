#include<iostream>
#include<thread>

using namespace std;

void func1(void)
{
    printf("Thread::hello world!\n");
}

int main()
{
    thread t1(func1);
    t1.join(); // 这一行似乎不能没有。
    return 0;
}