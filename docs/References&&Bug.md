## 参考链接(自制版)
* 2021秋季招新试题：https://github.com/ikitsuchi/kmeans
* 别人写的答案：https://github.com/PKUFlyingPig/asst1/
* 多线程编程：https://shengyu7697.github.io/std-thread/
* 多线程编程：https://zhuanlan.zhihu.com/p/389140084
* 多线程编程(详细版):https://blog.csdn.net/sevenjoin/article/details/82187127
* `std::mutex`:https://shengyu7697.github.io/std-mutex/



## 一些遇到的问题和BUG

* vscode ssh连接远端服务器上无法使用`display`命令，因为查询不到端口：
  * https://blog.csdn.net/weixin_42569673/article/details/111611554
  * https://icepoint666.github.io/2019/02/24/ssh-remote/#%E7%99%BB%E5%BD%95ssh%E5%90%8E%E4%BD%BFserver%E7%AB%AF%E7%9A%84%E5%9B%BE%E5%BD%A2%E7%95%8C%E9%9D%A2%E6%98%BE%E7%A4%BA%E5%88%B0client%E7%AB%AF
  * 

* mandelbrot set
  * 简单介绍：https://zhuanlan.zhihu.com/p/52303089
  * 使用python画了一个：https://zhuanlan.zhihu.com/p/98166561
  * 还可以继续查，这个并不够充分。

* `std::thread`
  * https://zhuanlan.zhihu.com/p/389140084
  * std::thread编译出错：https://blog.csdn.net/sixdaycoder/article/details/81910816
    * 加一个`-pthread`就行
  * 

* 一行命令将所有可执行文件加到gitignore(注意包括.sh文件):
  * https://stackoverflow.com/questions/8237645/how-to-add-linux-executable-files-to-gitignore
  ```shell
  for f in $(find . -perm /111 -type f | grep -v '.git' | sed 's#^./##' | sort -u); do grep -q "$f" .gitignore || echo "$f" >> .gitignore ; done
  ```
