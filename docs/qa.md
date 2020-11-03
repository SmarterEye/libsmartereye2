## Q&A

Q: 为什么我在虚拟机下使用SDK，无法连接上设备？

A: 由于硬件传输速率要求，请尽量使用USB3.0接口。另外，虚拟机因大多存在USB驱动兼容性问题，不建议使用。

---

Q: 在Linux(Ubuntu)系统下，无法连接设备，libusb_open返回-3？

A: 
```
在Linux平台下，加载USB设备驱动，可能需要root权限。
为了在连接设备时顺利加载设备驱动，需要把双目相机的USB规则添加到系统配置里面：
* copy 'scripts/99-se4d.rules' into '/etc/udev/rules.d/'
* reboot system
```
