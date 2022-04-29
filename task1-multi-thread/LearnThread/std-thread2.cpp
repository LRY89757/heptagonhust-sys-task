// g++ std-thread2.cpp -o a.out -std=c++11 -pthread
#include <iostream>
#include <thread>

class AA
{
public:
    void a1() { std ::cout << "a1\n"; }

    void a2(int n) { std ::cout << "a2 " << n << "\n"; }

    void start()
    {
        std ::thread t1(&AA::a1, this);
        std ::thread t2(&AA::a2, this, 10);

        t1.join();
        t2.join();
    }
};

int main()
{
    AA a;
    a.start();

    return 0;
}