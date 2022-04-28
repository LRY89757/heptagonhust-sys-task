#include <iostream>
#include <thread>
#include <chrono>
// g++ thread3.cpp -o thread3 -pthread && ./thread3

using namespace std;

void f1(int n)
{
    for (int i = 0; i < 5; ++i)
    {
	cout << "=====Thread:" << n << "======" << endl;
	this_thread::sleep_for(chrono::microseconds(10));
    }
}

void f2(int& n)
{
    for (int i = 0; i < 5; ++i)
    {
	cout << "thread two executing" << endl;
	++n;
	this_thread::sleep_for(chrono::microseconds(10));
    }
}

int main()
{
    int n = 0;
    thread t1;    //这是一个空的线程对象，还不是一个线程
    thread t2(f1, n + 1);
    thread t3(f2, ref(n));
    thread t4(move(t3));    //t3不再是一个线程
    t2.join();
    t4.join();
    cout << "the result n is:" << n << endl;

    // system("pause");
// system("pause");  这是windows的老用法，但是linux shell 并没有pause命令
// 所以可以用下面命令代替:
system("read -p 'Press Enter to continue...' var");
    return 0;
}