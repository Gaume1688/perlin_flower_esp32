# UDP控制LED花海律动 - ESP32 PlatformIO项目

## 项目简介

本项目实现了一个UDP控制的LED花海律动系统。10个白光LED按螺旋排列，中心花亮度由UDP数据包大小控制，外围花朵的亮度变化从中心向外传播，形成波浪效果。

## 效果特点

- **UDP控制**：中心花亮度由UDP数据包大小（0-1023字节）实时控制
- **螺旋排列**：使用黄金角(137.5°)排列，模拟自然界向日葵种子的分布
- **波浪传播**：亮度变化从中心向外围延迟传播，形成视觉上的波浪效果
- **动态呼吸**：呼吸效果强度随UDP值变化，数据越大呼吸效果越弱

## 项目结构

```
perlin_flower_esp32/
├── platformio.ini       # PlatformIO配置文件
├── index.html           # Web可视化界面（模拟UDP控制）
├── src/
│   └── main.cpp         # 主程序
├── include/
│   └── flower_config.h  # 配置头文件
└── README.md            # 项目文档
```

## 硬件需求

| 组件 | 数量 | 说明 |
|------|------|------|
| ESP32开发板 | 1 | 任意ESP32型号 |
| 白光LED | 10 | 带驱动电路 |
| 驱动电路 | 10 | MOSFET或LED驱动模块 |
| 限流电阻 | 10 | 根据LED规格选择 |
| 电源 | 1 | 5V/足够电流 |

### 引脚连接

| 花朵 | GPIO | 说明 |
|------|------|------|
| 花0 (中心) | GPIO 5 | 中心花 |
| 花1 | GPIO 18 | |
| 花2 | GPIO 19 | |
| 花3 | GPIO 21 | |
| 花4 | GPIO 22 | |
| 花5 | GPIO 23 | |
| 花6 | GPIO 25 | |
| 花7 | GPIO 26 | |
| 花8 | GPIO 27 | |
| 花9 | GPIO 32 | |

> **注意**：GPIO 0, 2, 6-11 不建议使用，与内置Flash相关。

## 开发环境设置

### PlatformIO 安装

1. 安装 [VSCode](https://code.visualstudio.com/)
2. 在VSCode扩展市场搜索并安装 "PlatformIO IDE"
3. 等待PlatformIO初始化完成（首次需要下载依赖）

### 项目编译上传

1. 用VSCode打开本项目文件夹
2. PlatformIO会自动识别 `platformio.ini` 并配置项目
3. 点击底部状态栏的 ✓ 按钮编译，或按 `Ctrl+Alt+B`
4. 点击 → 按钮上传，或按 `Ctrl+Alt+U`
5. 打开串口监视器：点击 🔌 按钮，或按 `Ctrl+Alt+S`

### WiFi配置

修改 [`include/flower_config.h`](include/flower_config.h) 中的WiFi配置：

```c
#define WIFI_SSID "Your_SSID"
#define WIFI_PASSWORD "Your_PASSWORD"
```

## UDP控制协议

| 参数 | 值 |
|------|-----|
| 端口 | 4210 |
| 数据包大小范围 | 0-1023 字节 |
| 映射亮度范围 | 20-255 |

发送UDP数据包到ESP32的IP地址和端口4210，数据包大小会被映射为中心花的亮度值。

## Web可视化界面

打开 [`index.html`](index.html) 可以在浏览器中模拟UDP控制效果：

- 滑块调节模拟UDP数据包大小
- 实时可视化花朵亮度和螺旋排列
- 自动演示模式
- 可调螺旋传播速度和亮度平滑系数

## 参数调优

所有可调参数都在 [`include/flower_config.h`](include/flower_config.h) 中定义：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `UDP_PACKET_SIZE_MAX` | 1023 | UDP数据包最大字节数 |
| `SPIRAL_B` | 1.5 | 螺旋间距系数 |
| `GOLDEN_ANGLE` | 2.399 | 黄金角（弧度）≈137.5° |
| `SPIRAL_DELAY_FACTOR` | 0.15 | 螺旋传播延迟系数（秒/距离单位） |
| `BRIGHTNESS_SMOOTHING` | 0.3 | 亮度平滑系数 (0-1) |
| `BREATH_PERIOD` | 3.0 | 呼吸周期（秒） |
| `BREATH_MAX_INTENSITY` | 0.8 | 呼吸最大强度 (0-1) |
| `BREATH_PHASE_OFFSET` | 0.5 | 每朵花相位偏移（弧度） |
| `BRIGHTNESS_MIN` | 20 | 最小亮度 |
| `BRIGHTNESS_MAX` | 255 | 最大亮度 |

## 调试

串口波特率：115200

发送字符 `d` 可打印调试信息：
```
=== Debug Info ===
Time: 10.50s
Center brightness: 128
Target center brightness: 130
WiFi status: Connected
Flower 0: brightness=128, distance=0.00
Flower 1: brightness=125, distance=3.60
...
================
```

## 算法说明

### 螺旋坐标计算

使用阿基米德螺旋公式：
```
θ(i) = i × 137.5°  (黄金角)
r(i) = b × θ(i)    (螺旋半径)
x(i) = r × cos(θ)
y(i) = r × sin(θ)
```

### 亮度平滑

使用指数平滑算法，让亮度变化更柔和：
```
centerBrightness = centerBrightness × (1 - α) + targetBrightness × α
```
其中 α 为平滑系数 `BRIGHTNESS_SMOOTHING`。

### 螺旋传播

使用亮度历史记录实现延迟传播：
- 每10ms记录一次中心花亮度到历史数组
- 外围花朵根据距离从历史记录中读取延迟后的亮度
- 距离越远，延迟越大

```
delayIndex = distance × SPIRAL_DELAY_FACTOR × 100
delayedBrightness = history[historyIndex - delayIndex]
```

### 呼吸效果

呼吸强度随UDP值动态变化：
```
brightnessRatio = centerBrightness / BRIGHTNESS_MAX
breathIntensity = BREATH_MAX_INTENSITY × (1 - brightnessRatio)
breathFactor = 1 - breathIntensity + breathValue × breathIntensity
```

- UDP值越大，呼吸效果越弱
- UDP值达到最大时，呼吸效果完全消失

### 综合亮度计算

```
最终亮度 = 基础亮度 × 呼吸因子
```

其中：
- 中心花：基础亮度 = 平滑后的UDP控制亮度
- 外围花：基础亮度 = 延迟后的中心花亮度 × 距离衰减

## 扩展建议

1. **更多花朵**：修改 `NUM_FLOWERS` 和 `LED_PINS` 数组
2. **RGB效果**：替换白光LED为WS2812B，添加颜色控制
3. **音乐同步**：添加麦克风模块，将声音强度映射到UDP数据包大小
4. **多点控制**：添加多个UDP控制源，实现更复杂的交互效果

## 许可证

本项目为开源项目，可自由使用和修改。

## 联系方式

如有问题或建议，欢迎反馈。
