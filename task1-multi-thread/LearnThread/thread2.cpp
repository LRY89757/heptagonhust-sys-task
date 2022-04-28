#include <iostream>
#include <thread>
#include <string>

using namespace std;
// g++ thread2.cpp -o thread2 -pthread && ./thread2

void thread_one()
{
    puts("hello");
}

void thread_two(int num, string& str)
{
    cout << "num:" << num << ",name:" << str << endl;
}

int main(int argc, char* argv[])
{
    thread tt(thread_one);
    tt.join();
    string str = "lry";
    thread yy(thread_two, 88, ref(str));
    //这里要注意是以引用的方式调用参数
    yy.detach();

// system("pause");  这是windows的老用法，但是linux shell 并没有pause命令
// 所以可以用下面命令代替:
system("read -p 'Press Enter to continue...' var");
    return 0;
}