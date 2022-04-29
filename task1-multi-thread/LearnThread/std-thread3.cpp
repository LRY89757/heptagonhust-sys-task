#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void foo()
{
    this_thread::sleep_for(chrono::milliseconds(200));
    cout << "foo"<<endl;
}

int main()
{
    std ::thread t1(foo);
    cout << "main 1"<<endl;
    t1.join();  // 等待t1执行完才继续执行主程序
    cout << "main 2"<<endl;
    return 0;
}
