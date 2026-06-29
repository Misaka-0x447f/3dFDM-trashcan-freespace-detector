# 3D 打印废料桶剩余空间探测器

ESP8266 + HC-SR04 超声测距 + WS2812B-8 灯条 + SSD1306 OLED，测量 3D 打印冲刷废料桶（"竹屎"）的剩余空间百分比。

## 硬件接线

| 外设 | 引脚 | ESP8266 GPIO | NodeMCU 丝印 |
|---|---|---|---|
| HC-SR04 | Trig | GPIO5 | D1 |
| HC-SR04 | Echo | GPIO4 | D2 |
| WS2812B-8 | DIN | GPIO13 | D7 |
| SSD1306 OLED | SCL | **GPIO12** | **D5** |
| SSD1306 OLED | SDA | **GPIO14** | **D6** |

外设供电全部走 3V3。HC-SR04 是宽压版，3.3V 工作正常；WS2812B 厂商标 3.3~5V 宽压；OLED 本身就 3V3。USB 供电，限灯条亮度到 80/255 避免 AMS1117 发热。

## 硬件笔记（必读）

### NodeMCU 兼容板的 D5/D6 GPIO 映射跟标准丝印反过来

- 这块板：D5 = GPIO12 / D6 = GPIO14
- 标准 Amica/LoLin NodeMCU：D5 = GPIO14 / D6 = GPIO12

i2c_scan 实测确认：STANDARD 映射扫不到设备，SWAPPED 映射扫到 SSD1306 @ 0x3C。
所以 `platformio.ini` 必须启用 `-DI2C_PINMAP_SWAPPED`（默认已配好）。
未来再加任何 I2C 外设，按 SCL=GPIO12 / SDA=GPIO14 算。
HC-SR04 和 WS2812 用的是常规 GPIO（D1/D2/D7），跟标准映射一致，仅 D5/D6 反了。

### 0.96" 黄蓝双色 SSD1306 屏的物理分区

这块 OLED 是**物理双色**面板，颜色由 y 坐标决定，跟绘制内容无关：

| y 行 | 颜色 | 高度 |
|---|---|---|
| 0..15 | 黄 | 16 px |
| 16..17 附近 | 不发光黑条 | ~2 px |
| 18..63 | 蓝 | 46 px |

任何跨越 y=16 的图形会被一分二色，且被黑条切断。`OledView` 已按此约束布局：
当前 `U8G2_R2` 翻转 180° 安装，黄区显示距离 `xx.x cm`、蓝区显示大百分比 + `%`，
都靠右对齐，且严格不跨黑条。重新调布局时务必尊重这道分界。

## 编译与烧录

```bash
# 主固件
pio run -e nodemcuv2 -t upload

# 一次性 I2C 探针（若怀疑接线/换板）
pio run -e i2c_scan -t upload
pio device monitor -b 115200   # 看到 found 0x3C 即可
```

## 标定

接好硬件并固定探头位置后，串口监视 115200，记下两个状态的 `filt=X.Xcm`：

1. **桶空**（桶底无料）→ 回填 `include/config.h` 的 `D_EMPTY_CM`
2. **桶满**（你认为该清理那条线）→ 回填 `D_FULL_CM`

注意 HC-SR04 盲区 2cm，`D_FULL_CM` 留 ≥3cm 余量。

## 架构

- `lib/Ultrasonic` — HC-SR04 触发 + pulseIn 25ms 超时 + 中位数滤波（窗口 5）
- `lib/LevelMapper` — 距离 → 剩余空间% （EMA α=0.3 + 死区滞回 2%）
- `lib/LedBar` — WS2812B-8 渐变填充，剩余多绿 / 中黄 / 少红，边界灯按余数调暗
- `lib/OledView` — U8g2 SW_I2C 驱动 SSD1306，明确传 SCL/SDA 引脚（规避 Wire 默认引脚被库内覆盖的坑）
- `src/main.cpp` — millis 调度：采样 → mapper.feed → 渲染
- `src/i2c_scan.cpp` — 独立 env，一次性 I2C 扫描探针
