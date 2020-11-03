### Dependence

* SDK代码基于CMake构建，请使用CMake配置并编译SDK；
* 编译器需要支持C++11标准；
* 示例代码依赖OpenCV，如需使用，需要自行安装OpenCV库。

由于硬件传输速率要求，请尽量使用USB3.0接口。另外，虚拟机因大多存在USB驱动兼容性问题，不建议使用。

### Build

#### Linux平台

在Linux平台下，加载USB设备驱动，可能需要root权限。
为了在连接设备时顺利加载设备驱动，需要把双目相机的USB规则添加到系统配置里面：
* copy 'scripts/99-se4d.rules' into '/etc/udev/rules.d/'
* reboot system

完成之后，重启设备就可以不通过root权限顺利连接设备。

编译SDK源码
```shell script
git clone https://github.com/SmarterEye/libsmartereye2.git
cd libsmartereye2
mkdir build
cmake ..
make -j
```

#### Windows平台

在Windows下，连接设备会自动加载设备驱动。

通过Visual Studio编译SDK源码

1. 使用cmake-gui打开sdk目录，设置build目录;
2. 点击configure，并指定编译器和平台类型；
3. 点击generate生成vs工程；
4. 打开build目录下的sln工程进行编译。
