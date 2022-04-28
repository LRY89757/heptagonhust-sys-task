#include <iostream>
#include <thread>
// g++ thread1.cpp -o thread1 -pthread && ./thread1

void myfunc()
{
    std ::cout << "myfunc\n"; // do something ... }
}
int main()
{
    std ::thread t1(myfunc);
    t1.join();
    return 0;
}

