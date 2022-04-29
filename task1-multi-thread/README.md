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


看了看可以优化的地方，当前我使用的是8个线程，每个线程恒定计算1200/8=150高度的块，然后经过计时发现确实会有一些问题，因为这样分配的话图形本身按照高度分是不均匀的，最上面的部分和最下面的部分对应线程计算起来比较快，但是其他线程计算起来就比较慢，计算的时间对比如下(5次)：
```shell
lry@ubuntu ~/p/r/task1-multi-thread (main)> ./mandelbrot --view 1
[mandelbrot serial]:            [369.226] ms
Wrote image file mandelbrot-serial.ppm
the thread 0 cost time is [6.075] ms
the thread 7 cost time is [19.220] ms
the thread 1 cost time is [38.152] ms
the thread 6 cost time is [56.823] ms
the thread 2 cost time is [85.779] ms
the thread 5 cost time is [101.298] ms
the thread 3 cost time is [115.834] ms
the thread 4 cost time is [116.928] ms
the thread 7 cost time is [15.709] ms
the thread 0 cost time is [19.871] ms
the thread 6 cost time is [52.994] ms
the thread 5 cost time is [70.359] ms
the thread 1 cost time is [71.212] ms
the thread 4 cost time is [104.420] ms
the thread 2 cost time is [106.728] ms
the thread 3 cost time is [114.976] ms
the thread 0 cost time is [8.359] ms
the thread 7 cost time is [18.413] ms
the thread 1 cost time is [48.157] ms
the thread 6 cost time is [49.016] ms
the thread 5 cost time is [91.930] ms
the thread 2 cost time is [98.787] ms
the thread 4 cost time is [99.190] ms
the thread 3 cost time is [113.434] ms
the thread 0 cost time is [6.653] ms
the thread 7 cost time is [7.813] ms
the thread 1 cost time is [40.324] ms
the thread 6 cost time is [51.750] ms
the thread 5 cost time is [73.239] ms
the thread 2 cost time is [82.452] ms
the thread 3 cost time is [98.786] ms
the thread 4 cost time is [101.288] ms
the thread 0 cost time is [19.368] ms
the thread 7 cost time is [20.738] ms
the thread 6 cost time is [33.937] ms
the thread 1 cost time is [52.120] ms
the thread 2 cost time is [66.939] ms
the thread 5 cost time is [73.324] ms
the thread 3 cost time is [115.438] ms
the thread 4 cost time is [117.707] ms
[mandelbrot thread]:            [101.477] ms
Wrote image file mandelbrot-thread.ppm
                                (3.64x speedup from 8 threads)
```

所以这里目前一个我能够想到的思路就是分块的时候按照面积来分好，确定每个线程要计算的高度参数，这个看来还是要理解代码了，不过这个感觉也可以手算出来，只不过这样一来就必须要理解相应的`mandelbrot set`有关详细计算过程了。



