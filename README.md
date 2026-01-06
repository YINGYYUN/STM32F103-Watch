# STM32F103-Watch
## HDU低年级专项-STM32手表

---

#### 项目参考
- 【STM32手表教程（全网最详细!)】https://www.bilibili.com/video/BV1CoGuzEEeN?p=2&vd_source=befff269c2e0ef1583e2528659770521
- 教程Github链接：https://github.com/BQT0411/STM32-WATCH

---

#### 项目成员
- `YXZ`: [@YINGYYUN](https://github.com/YINGYYUN)
- `WZ`: [@z15888286525-ux](https://github.com/z15888286525-ux)
- `LSY`: [@huaxiren6](https://github.com/huaxiren6)

#### 测试账号
- `YXZ`: [@YUNYYING](https://github.com/YUNYYING)

---

### 配置声明

#### [RTC]
- `低速外部时钟LSE(32.768kHz)`: 时钟源

#### [TIM定时器]
- `TIM2`: 定时器定时中断(1ms)

#### [LED]
- `PB15`: GND
- `3V3 `: VCC

#### [OLED]
- 通信方式：软件I2C
- `PB8 `：SCK(SCL)
- `PB9 `：SDA(SDA)

#### [按键]
- 使用TIM2 定时器定时中断(1ms) 扫描
- `PB1`: 上键
- `PA6`: 下键
- `PA4`: 确认键

---

### 工程日志
- 2026.1.4.15:14(YXZ):基于 江协V2.0四针脚OLED模板代码 进行项目初始化
- 2026.1.4.22:45(YXZ):（测试性质的递交）LED模块
- 2026.1.6.1:14(YXZ):
    完善了gitignore;
    引入RTV并完成时钟UI;
    OLED 12X24的ASCII字符的取模,部分汉字16X16的取模;
    TIM2 定时器定时中断(1ms);
    引入江协全功能非阻塞式按键（与教程不同）;
- 2026.1.6.18:13(YXZ):发现gitignore仍然有问题，进行了递交更改