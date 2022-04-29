#include <iostream>
#include <thread>

using namespace std;

int g_count = 0;

int print(int n, char c)
{
    for (int i = 0; i < n; ++i)
    {
        std ::cout << c;
        g_count++;
    }
    std ::cout << '\n';

    std ::cout << "count=" << g_count << std ::endl;
    return 0;
}

int main()
{
    std ::thread t1(print, 10, 'A');
    std ::thread t2(print, 5, 'B');
    t1.join();
    t2.join();

    return 0;
}
// 如果没上锁的话,可能造成不预期的输出,如下count=5A所示,t2 执行绪的g_count 还没来得及印完\n,另一个执行绪t1 已经开始抢着印了。
// 另外补充一下,t1 与t2 谁先执行并没有一定谁先谁后,每次执行的结果都有可能不同。