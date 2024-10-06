# 项目名称: BALA2Fire-BleRemoteControl
### 项目简介:
- 本项目是基于M5Stack MiniJoyC和M5Stack BALA2Fire的BLE遥控器项目，通过M5Stack MiniJoyC的摇杆控制小车的前进、后退、左转、右转等动作。
### IDE:
- [Arduino IDE](https://www.arduino.cc/en/software)
### 硬件商品地址:
- [M5Stack MiniJoyC](https://docs.m5stack.com/zh_CN/hat/MiniJoyC)
- [M5Stack BALA2Fire](https://docs.m5stack.com/zh_CN/app/bala2fire)
- [M5StickC PLUS2](https://docs.m5stack.com/zh_CN/core/M5StickC%20PLUS2)

### 目录结构:
- BleRemoteControl: 遥控器Arduino项目文件夹
- BALA2: BALA2Fire小车Arduino项目文件夹

### 使用方法:
1. 下载并安装Arduino IDE
2. 下载本项目
3. 打开Arduino IDE，打开BleRemoteControl文件夹，编译并上传到M5StickC PLUS2
4. 打开Arduino IDE，打开BALA2文件夹，编译并上传到M5Stack BALA2Fire
5. 打开M5Stack MiniJoyC，重启M5Stack BALA2Fire,自动链接成功
6. 摇杆控制小车的前进、后退、左转、右转等动作
7. 摇杆4个方向都有12个档位，可以根据实际情况调整

### 存在问题:
- 重连问题, 先按下M5StickC PLUS2的BtnA按钮，再重启M5Stack BALA2Fire
- 基于简单的速度偏移量来实现的遥控，并未参与pid的计算特殊情况不够灵活，看起来bala1小车的代码库里面有直接的控制方法,后续bala1代码改进