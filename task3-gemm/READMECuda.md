# how to optimize GEMM
## Preface
如何优化矩阵相乘，除了传统的CPU端访存以及各类的Openmp、sse指令集优化，我们还可以使用Cuda来加速，这类就涉及到GPU如何处理这些事情了，我们参考了大量的文档：
  
* matmul的两个输入输出的shape为[m, k] [k, n]，许多dev讲的`split_k`便是讲的中间的k。
* 许多下标计算都有32这个数，这个之前学Cuda的时侯似乎见过:指的是物理层的一块核心SM的一个线程束所含线程的数目，为32个。
* 注意threadidx.xyz这三个维度仅仅是软件层的抽象，并非物理层的执行方式，所以我们开发的时候只使用threadidx.x,另外yz两项等于1就好
```cuda
// 可以查看一个Block到底可以用多少个thread
cudaDeviceProp prop;
cudaGetDeviceProperties(&prop, 0);
fprintf(stdout, "%d", prop.maxThreadsPerBlock);
```
* 关于grid/block最大是多少这个问题也很关键，因为如果MNK比较大的时候，对于CPU来说就会出现reg不够导致save load切换问题，但是所幸的是：`maxGridSize = {2147483647, 65535, 65535}`，对于GPU来说这个完全够

## Reference
Refer:
* OpenBlas gemm: https://zhuanlan.zhihu.com/p/65436463
* Cuda优化gemm：https://zhuanlan.zhihu.com/p/478846788
* 天元优化指南：https://zhuanlan.zhihu.com/p/410278370
* 更加进阶版的优化方案：https://zhuanlan.zhihu.com/p/441146275
* 拓展版神经网络卷积算子实现原理：https://zhuanlan.zhihu.com/p/372973726
* Nvidia Cuda C Programming:https://docs.nvidia.com/cuda/cuda-c-programming-guide/
* 优化矩阵相乘论文Anatomy of High-Performance Matrix Multiplication: https://www.cs.utexas.edu/users/pingali/CS378/2008sp/papers/gotoPaper.pdf
* 矩阵相乘优化以及在卷积上的应用：https://www.51cto.com/article/616486.html
* 一个关于cuda加速的专栏：https://zhuanlan.zhihu.com/p/435908830（这是第一篇矩阵加速文章)
  * 这是专栏链接：https://www.zhihu.com/column/c_1437330196193640448

## GPU设备
Detected 3 CUDA Capable device(s)
Device 0:"GeForce RTX 2080 Ti"
  CUDA Driver Version / Runtime Version         11.2  /  10.1
  CUDA Capability Major/Minor version number:   7.5
  Total amount of global memory:                10.76 MBytes (4843621399236972288 bytes)
  GPU Clock rate:                               1650 MHz (1.65 GHz)
  Memory Bus width:                             352-bits
  L2 Cache Size:                            	5767168 bytes
  Max Texture Dimension Size (x,y,z)            1D=(131072),2D=(131072,65536),3D=(16384,16384,16384)
  Max Layered Texture Size (dim) x layers       1D=(32768) x 2048,2D=(32768,32768) x 2048
  Total amount of constant memory               65536 bytes
  Total amount of shared memory per block:      49152 bytes
  Total number of registers available per block:65536
  Wrap size:                                    32
  Maximun number of thread per multiprocesser:  1024
  Maximun number of thread per block:           1024
  Maximun size of each dimension of a block:    1024 x 1024 x 64
  Maximun size of each dimension of a grid:     2147483647 x 65535 x 65535
  Maximu memory pitch                           2147483647 bytes

SM的数量:68
每个线程块的共享内存大小:48 KB
每个线程块的最大线程数:1024
每个EM的最大线程数:1024
每个SM的最大线程束数:32


## naive impletmention
```c
__global__ void matrixMul(const float *A, const float *B, float *C, 
                          int M, int N, int K) {
    int tx = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = blockIdx.y * blockDim.y + threadIdx.y;
    if(ty < M && tx < N) {
        float c = 0;
        for(int i = 0; i < K; ++i){
            c += A[ty * K + i] * B[i * N + tx];  // 1 FMA, 2 load, 计算访存比太低
        }
        C[ty * N + tx] = c;
    }
}
```
这种方式经过了两个优化：
* `c`的引入，这个可以带来一半时间的减少，
* 同时如果我们更换一种访问方式，仅仅将其中if语句块内的`tx`,`ty`互换，我们就能够得到慢20倍的结果。
  * 这个原理个人理解是GPU物理层缓存的实现导致的，主要在于`A[ty*K+i]`的访问，如果换成`A[tx*K+i]`的话会导致频繁更新GPU某个BLOCKS内的`Shared Memory`内的cache造成访问变慢。这里带一张关于Block， 的内部存储图：
  ![](https://pic2.zhimg.com/80/v2-6456af75530956da6bc5bab7418ff9e5_1440w.jpg)

但是以上方法还是一个非常普通的优化实现，主要问题在于`c += A[ty * K + i] * B[i * N + tx];`这行语句有着2个load，1次FMA累乘加运算，计算访存比太低，同时需要频繁访问内存.
同时此时的计算读取的是GPU的Global Memory，此时读取代价很大，通常来说需要几百个时钟周期(cycle)，而进行一次FMA通常只需要几个cycle就可以，所以我们可以选择将Global Memory搬到Shared Memory上，这个Shared Memory是SM中的on-chip-momory, 每个block内线程共享。同时提高计算访存比就可以:
![](https://pic1.zhimg.com/80/v2-055721191f1e4b05fcf0ca95fe1d5320_1440w.jpg)

## Optimize 1

* `#progma unroll`:https://cloud.tencent.com/developer/article/1173287
* 

