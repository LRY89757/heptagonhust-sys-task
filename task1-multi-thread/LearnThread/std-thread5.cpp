// 原因是因为在std::thread 的参数传递方式为传值，要传参考的话需要透过std::ref来辅助达成，所以程式就会写成这样，myfunc 与myfunc2 的参数传递方式不同，可以看看这两者之间的差异，
#include <iostream>
#include <thread> 
void myfunc ( int & n) { std :: cout << "myfunc n=" << n << "\n" ;     n+= 10 ; }

void myfunc2(int n)
{
    std ::cout << "myfunc n=" << n << "\n";
    n += 10;
}
int main()
{
    int n1 = 5;
    std ::thread t1(myfunc, std ::ref(n1));  // 传引用ref()
    t1.join();
    std ::cout << "main n1=" << n1 << "\n";
    int n2 = 5;
    std ::thread t2(myfunc2, n2);
    t2.join();
    std ::cout << "main n2=" << n2 << "\n";

    return 0;
}