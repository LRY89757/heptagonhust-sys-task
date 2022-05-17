# Main idea
初步思路是使用Cuda来加速这个矩阵。
* Linux C++与Cuda混合编程：https://blog.csdn.net/baishuiniyaonulia/article/details/120675337
* Cuda C++ demo:https://blog.csdn.net/qq_40283970/article/details/105893038
* Cuda矩阵乘法优化：https://juejin.cn/post/7008002811279441927
* tvm矩阵相乘优化：https://github.com/Orion34-lanbo/tvm-batch-matmul-example
后续想法还是利用缓存来做，但是GPU利用Cuda还是要做，但是先按照题目要求学学openmp还有mips这些再说

Cuda优化的过程文档看[这里](./READMECuda.md)

* 也许使用访存和有关C++知识来写这道题，可以参考如下链接:
  * https://blog.csdn.net/artorias123/article/details/86527456
  * https://www.delftstack.com/zh/howto/cpp/matrix-multiplication-in-cpp/
  * https://www.google.com/search?q=%E7%9F%A9%E9%98%B5%E7%9B%B8%E4%B9%98%E4%BC%98%E5%8C%96C%2B%2B&oq=%E7%9F%A9%E9%98%B5%E7%9B%B8%E4%B9%98%E4%BC%98%E5%8C%96C%2B%2B&aqs=chrome..69i57j35i39l2j0i512j46i199i465i512j0i512l2j46i199i465i512j0i512l2.10761j0j7&sourceid=chrome&ie=UTF-8

* 关于openmp的一些简单了解：https://blog.csdn.net/dcrmg/article/details/53862448

别人写的矩阵相乘实例：
  * https://github.com/NJU-TJL/OpenMP-MPI_Labs/tree/main/Lab01
  * https://github.com/attractivechaos/matmul
  * 看到了一个快速转置N×N的矩阵的代码：https://github.com/shines77/fast_matmult/blob/master/src/fast_matmult/fast_matmult.cpp（235行走起）
  * 访存优化：https://developer.aliyun.com/article/120543
  * **1k Star**how to optimize gemm:https://github.com/flame/how-to-optimize-gemm
    * 有一说一这个确实很赞
  * 如何对矩阵相乘进行优化, 主要从这之中学到了非常多的名词与一些专业术语的解释：https://renzibei.com/2021/06/30/optimize-gemm/
  * 矩阵优化常见思路：https://blog.csdn.net/wwxy1995/article/details/114762108
  * 

  

原始效果：
```sh
(base) lry@ubuntu:~/projects/recruitment-2022-spring/task3-gemm$ make run
./gemm
Running, dataset: size 256
time spent: 106430us
Passed, dataset: size 256

Running, dataset: size 512
^[[Dtime spent: 1.61292e+06us
Passed, dataset: size 512

Running, dataset: size 1024
time spent: 2.04408e+07us
Passed, dataset: size 1024

Running, dataset: size 2048
time spent: 2.85959e+08us
Passed, dataset: size 2048
```


 * 

## 2022.5.9
当前一个较为简单的思路为将b的4列元素来先放到tr里面然后使用寄存器变量来负责相加，接着使用a的4行元素依次来进行相乘，把b的这4列元素用到极致就结束就行了。

当然这里也就是每次都能计算出结果的一个4×4的矩阵答案出来，我们同样可以来进行尝试8×8等这些会不会更快。

反正可以整除

笑死按照上面方法优化对于512的数据反而变慢了：
```sh
lry@ubuntu ~/p/r/task3-gemm (main)> make run
./gemm
Running, dataset: size 256
time spent: 112962us
Passed, dataset: size 256

Running, dataset: size 512
time spent: 867932us
Passed, dataset: size 512

Running, dataset: size 1024
time spent: 6.94441e+06us
Passed, dataset: size 1024

Running, dataset: size 2048
time spent: 5.48987e+07us
Passed, dataset: size 2048
```


目前又看到一种新的优化方案https://blog.csdn.net/gaoxueyi551/article/details/104595957

其实就是利用到了访存改变了一点循环顺序而已。不过这种方法无法对于大矩阵带来极致的提升。

笑死这么简单的方法反而有用：
```sh
lry@ubuntu ~/p/r/task3-gemm (main) [2]> make run
./gemm
Running, dataset: size 256
time spent: 84643us
Passed, dataset: size 256

Running, dataset: size 512
time spent: 643813us
Passed, dataset: size 512

Running, dataset: size 1024
time spent: 5.2408e+06us
Passed, dataset: size 1024

Running, dataset: size 2048
time spent: 4.17518e+07us
Passed, dataset: size 2048
```

又认真看了看输出，原来确实有优化，少看了数量级，但是优化仅仅5，6倍，这个确实不太好，理想状态下至少优化十几倍才比较理想才好。况且方法一被方法二完爆，方法二还更简单，看来有必要尝试着提升下多线程openmp的相关方向以及MIPS了。
同时访存这部分优化还不够极致。

访存


### 一些简单笔记
看了看GitHub一个简单的矩阵相乘优化项目https://github.com/flame/how-to-optimize-gemm
这里简单介绍下仓库的内容：

首先是单次计算目标矩阵4个元素，同时要尽量减少函数调用的开销，也就是过程尽量写在一个函数中，同时单次计算4个元素的时候将4个元素的计算放到同一个循环中。
同时将频繁访问赋值的元素都放到寄存器中：
```c++
  for ( p=0; p<k; p++ ){
    a_0p_reg = A( 0, p );

    c_00_reg += a_0p_reg * B( p, 0 );     
    c_01_reg += a_0p_reg * B( p, 1 );     
    c_02_reg += a_0p_reg * B( p, 2 );     
    c_03_reg += a_0p_reg * B( p, 3 );     
  }
```
而后由于这里选择指针访问而非数组访问：
```c++
  for ( p=0; p<k; p++ ){
    a_0p_reg = A( 0, p );

    c_00_reg += a_0p_reg * *bp0_pntr++;
    c_01_reg += a_0p_reg * *bp1_pntr++;
    c_02_reg += a_0p_reg * *bp2_pntr++;
    c_03_reg += a_0p_reg * *bp3_pntr++;
  }
```
再后来就是单次选择进行将计算循环减少4倍，个人认为这里的有利之处在于这样的话寄存器的优势会体现的更加明显。

而后便是间接寻址来代替每次指针更新的开销，也就是使用`*(bp3_pntr+1)`,`*(bp3_pntr+2)`这类间接寻址来代替`*(bp3_pntr++)`
*不过作者这里尝试后发现并没有提升，应该是编译器自动优化了*

而后换成了4×4的矩阵计算，也即单次计算16个元素。
然后把之前用过的存在寄存器、指针代替数组访问等都用上。
后续优化为由于此时计算的为4×4的这样一个矩阵，所以我们这里行和列的都要复用4次，所以可以将他们都使用寄存器来存储。另外就是当时单个循环体内是依此累加第一行的结果、第二行的结果现在改为单次累加一二行，三四行(就换了下代码顺序)。虽然这个优化不是非常多，但是对后续优化有帮助。后续作者将使用向量操作来优化。



偶然看到的这类运算可以优化的点：
> 有几个优化点，一个是对外层循环做tile，利用访问内存的局部性尽量多利用缓存中现有的数据进行计算，通常对i和j做blocking，第二是对外层循环并行化，第三是内层循环可以向量化，硬件通常提供了fma，也就是y=a*b+c的指令，最后在向量化的基础上还可以unroll，减少循环次数
* https://blog.csdn.net/wwxy1995/article/details/114762108


我个人的想法是要将访存端的优化优化到极致，然后去追求SIMD以及omp等操作指令。
关键在于如何根据cache来进行访存的极致优化。


目前利用访存加速了20倍：
```sh
./gemm
Running, dataset: size 256
time spent: 33847us
Passed, dataset: size 256

Running, dataset: size 512
time spent: 259867us
Passed, dataset: size 512

Running, dataset: size 1024
time spent: 1.86889e+06us
Passed, dataset: size 1024

Running, dataset: size 2048
time spent: 1.43576e+07us
Passed, dataset: size 2048
```
目前用的方法如下：
* 每次读取b中的4列，然后存到tr[size][4]中，
* 使用a中4行4行的取出来乘以b中的4列，结果使用`register`来存储
* 由于每个循环最内部计算的相当于4×4的矩阵，所以这里同样使用一个临时变量来存储需要乘的8个变量，这样一来计算访存比达到16/8=2/1，当然这样一来还有优化空间，访存比个人认为还有必要提高
* 另外矩阵分块cache blocking还有待优化。
