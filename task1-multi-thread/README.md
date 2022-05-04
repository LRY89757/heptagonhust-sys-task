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

```sh
lry@ubuntu ~/p/r/task1-multi-thread (main)> make
/bin/mkdir -p objs/
g++ -m64 mandelbrotThread.cpp -I../common -Iobjs/ -O3 -std=c++11 -Wall -fPIC -c -o objs/mandelbrotThread.o
g++ -m64 -I../common -Iobjs/ -O3 -std=c++11 -Wall -fPIC -o mandelbrot objs/main.o objs/mandelbrotSerial.o objs/mandelbrotThread.o objs/ppm.o -lm -lpthread
lry@ubuntu ~/p/r/task1-multi-thread (main)> ./mandelbrot --view 1
[mandelbrot serial]:            [370.022] ms
Wrote image file mandelbrot-serial.ppm
the thread 0 cost time is [13.554] ms
the thread 1 cost time is [56.647] ms
the thread 2 cost time is [88.658] ms
the thread 3 cost time is [106.280] ms
the thread 4 cost time is [108.130] ms
the thread 5 cost time is [80.206] ms
the thread 6 cost time is [48.519] ms
the thread 7 cost time is [19.872] ms
[mandelbrot thread]:            [101.716] ms
Wrote image file mandelbrot-thread.ppm
                                (3.64x speedup from 8 threads)
```

目前发现果然中间高度的线程花费了大多数时间然而0，1，6，7线程几乎没怎么耗时，这个确实是一个瓶颈。
现如今打算根据以上的时间比例，来优化高度(也就是代码中实现一个固定的数组来告诉每个线程要计算的高度，):
目前的计算时间比为：
13:56:88:106:108:80:48:19
=
15:60:90:110:110:80:50:20
=
3:12:18:22:22:16:10:4(和为107)

107/8=13.675

对应下来：
0: 0->150*1.8=270  
1: 150*(1.8-2.6)
2: 150*(2.6-3.3)
3: 150*(3.3-4.0)
4: 150*(4.0-4.7)
5: 150*(4.7-5.4)
6: 150*(5.4-6.2)
7: 150*(6.2-8.0)

0 270 390 495 600 705 810 930 1200



照以上的划分标准，目前已经能够加速到5.03倍：
```sh
lry@ubuntu ~/p/r/task1-multi-thread (main)> ./mandelbrot --view 1
[mandelbrot serial]:            [369.970] ms
Wrote image file mandelbrot-serial.ppm
the thread 0 cost time is [40.645] ms
the thread 1 cost time is [55.590] ms
the thread 2 cost time is [68.482] ms
the thread 3 cost time is [79.670] ms
the thread 4 cost time is [79.293] ms
the thread 5 cost time is [73.837] ms
the thread 6 cost time is [61.428] ms
the thread 7 cost time is [39.544] ms
[mandelbrot thread]:            [73.531] ms
Wrote image file mandelbrot-thread.ppm
                                (5.03x speedup from 8 threads)
```



刚刚又看了看样本的分配同样还是不均匀，但是好了一些，同时需要考虑的是即使这么做会有效果，还有第二个分型需要去考虑计算速度，所以这个还是需要进一步的思考优化。

## 2022/4/30  Finished!
参考了有关文档的解释以及有关资料，目前最新的进展已经完美加速两个进程了!
大致思路其实很简单，因为相邻像素得亮度是均匀的，所以计算的开销也是类似的，所以就严格将图像块每个部分都细分为8份进行运算即可，这样一来就非常easy了：
```sh
lry@ubuntu ~/p/r/task1-multi-thread (main)> ./mandelbrot --view 1
[mandelbrot serial]:            [408.423] ms
Wrote image file mandelbrot-serial.ppm
the thread 0 cost time is [51.934] ms
the thread 1 cost time is [56.738] ms
the thread 2 cost time is [52.428] ms
the thread 3 cost time is [56.523] ms
the thread 4 cost time is [53.968] ms
the thread 5 cost time is [53.042] ms
the thread 6 cost time is [52.898] ms
the thread 7 cost time is [55.334] ms
[mandelbrot thread]:            [60.264] ms
Wrote image file mandelbrot-thread.ppm
                                (6.78x speedup from 8 threads)
lry@ubuntu ~/p/r/task1-multi-thread (main)> ./mandelbrot --view 2
[mandelbrot serial]:            [255.144] ms
Wrote image file mandelbrot-serial.ppm
the thread 0 cost time is [32.793] ms
the thread 1 cost time is [33.909] ms
the thread 2 cost time is [33.029] ms
the thread 3 cost time is [33.572] ms
the thread 4 cost time is [35.237] ms
the thread 5 cost time is [36.303] ms
the thread 6 cost time is [32.252] ms
the thread 7 cost time is [36.353] ms
[mandelbrot thread]:            [40.296] ms
Wrote image file mandelbrot-thread.ppm
                                (6.33x speedup from 8 threads)
```

完结撒花！



