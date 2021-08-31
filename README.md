# CacheSim
This version so far:

- Testing for 1 request pending from each requestor
- shared cache controller cannot process req and data at the same time
- response arbiter must be updated to capture the responses that coming from other cores (remember discussion with Rodolfo)


sudo ./waf --enable-gcov --run "scratch/MultiCoreSimulator --CfgFile=./src/MultiCoreSim/model/test.xml --BMsPath=/home/rmirosan/Dropbox/Papers/RTSS2021/dev/BM --LogFileGenEnable=1" >log


