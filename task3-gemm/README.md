# Main idea
初步思路是使用Cuda来加速这个矩阵。
* Linux C++与Cuda混合编程：https://blog.csdn.net/baishuiniyaonulia/article/details/120675337
* Cuda C++ demo:https://blog.csdn.net/qq_40283970/article/details/105893038

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


* 也许使用访存和有关C++知识来写这道题，可以参考如下链接:
  * https://blog.csdn.net/artorias123/article/details/86527456
  * https://www.delftstack.com/zh/howto/cpp/matrix-multiplication-in-cpp/
  * https://www.google.com/search?q=%E7%9F%A9%E9%98%B5%E7%9B%B8%E4%B9%98%E4%BC%98%E5%8C%96C%2B%2B&oq=%E7%9F%A9%E9%98%B5%E7%9B%B8%E4%B9%98%E4%BC%98%E5%8C%96C%2B%2B&aqs=chrome..69i57j35i39l2j0i512j46i199i465i512j0i512l2j46i199i465i512j0i512l2.10761j0j7&sourceid=chrome&ie=UTF-8

  * 关于openmp的一些简单了解：https://blog.csdn.net/dcrmg/article/details/53862448
  * 

## 2022.5.9
当前一个较为简单的思路为将b的4列元素来先放到tr里面然后使用寄存器变量来负责相加，接着使用a的4行元素依次来进行相乘，把b的这4列元素用到极致就结束就行了。

当然这里也就是每次都能计算出结果的一个4×4的矩阵答案出来，我们同样可以来进行尝试8×8等这些会不会更快。

反正可以整除

笑死按照上面方法优化反而变慢了：
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

