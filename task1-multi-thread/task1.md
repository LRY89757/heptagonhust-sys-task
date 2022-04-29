# Main idea

使用`std::thread`来并行运行`mandelbrot set`的生成迭代过程，而mandelbrot set是一个迭代生成的图形集合。

或许重要的并不是所谓的分型图怎么画而重要的是怎么使用多线程来操作。另外重要的是如何分开让每个线程同时来进行编程。


通过我但对于mandelbrot set生成过程的了解与分析，完成这个集合的多线程程实现应该不可以修改x0,y0,x1,y1这些参数，而应该改变的是startRow和numRows等参数来完成多线程的实现。

更加精确的研究发现，也就是将传入的参数中的height分成8份或是其他的份数就行。


## 2022.4.29号

目前开了8个线程来同时计算相应的结果，但是目前加速没有6倍，只有3.74倍左右，但是跑通了相应的代码，发现原来这里面循环是开了7个进程，然后刚开始的时候调用了workerThreadStart调用了第0号进程，而我刚开始将`workerThreadStart(&args[0])`这行代码注释掉了（~~真是自作聪明~~），接下来打算计算一下每个线程花费的时间来加速代码。

```shell
lry@ubuntu ~/p/r/task1-multi-thread (main) [SIGABRT]> make
/bin/mkdir -p objs/
g++ -m64 mandelbrotThread.cpp -I../common -Iobjs/ -O3 -std=c++11 -Wall -fPIC -c -o objs/mandelbrotThread.o
g++ -m64 -I../common -Iobjs/ -O3 -std=c++11 -Wall -fPIC -o mandelbrot objs/main.o objs/mandelbrotSerial.o objs/mandelbrotThread.o objs/ppm.o -lm -lpthread
lry@ubuntu ~/p/r/task1-multi-thread (main)> ./mandelbrot --view 1
[mandelbrot serial]:            [368.757] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [98.581] ms
Wrote image file mandelbrot-thread.ppm
                                (3.74x speedup from 8 threads)
```

