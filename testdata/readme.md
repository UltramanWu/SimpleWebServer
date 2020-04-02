### 虚拟机测试结果
使用10000个客户端和2000个客户端进行60s长连接测试。
* 10000客户端下连接成功数比2000客户端下少，推测由于是使用同一主机测试，webbench建立了大量进程，导致硬件资源有限，服务器处理速度下降；
* 服务器端，较少连接数时，主线程的负载比较小，几乎不占用CPU，但是在上万连接的情况下，主线程开始大量占用CPU资源。
* 使用线程追踪工具strace追踪主线程事件调用出现EMFILE错误，主要问题在于打开文件数超出了系统本身限制的数量，而acceptfd一直没有处理，因此就会一直调用，
  使主线程一直在循环。
* 解决办法：使用ulimit -n修改open file大小
