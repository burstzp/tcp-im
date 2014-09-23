mobile2loc
==========

手机号归属地 <br>

共享内存，占用3M内存，1G虚拟机，3.6个/s, 单个最小0.0001s左右，最长0.00017s <br>

phpize <br>
./configure <br>
make && make install <br>


php.ini <br>

[mobile2loc]  <br>
extension=mobile2loc.so <br>
mobile2loc.filename="/path/mobile2loc.dat"<br>



