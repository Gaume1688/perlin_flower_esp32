# 柏林噪声LED花海律动 - ESP32 PlatformIO项目

## 项目简介

本项目使用柏林噪声(Perlin Noise)算法驱动10个白光LED，创造出自然、永不重复的律动效果。花朵按螺旋排列，中心花直接由噪声驱动，外围花朵的亮度变化从中心向外传播，形成类似"风吹过花海"的波浪效果。

## 效果特点

- **自然律动**：柏林噪声产生连续、平滑、永不重复的亮度变化
- **螺旋排列**：使用黄金角(137.5°)排列，模拟自然界向日葵种子的分布
- **波浪传播**：亮度变化从中心向外围传播，形成视觉上的波浪效果
- **呼吸效果**：全局缓慢的亮度起伏，增加生命感

## 项目结构

```
perlin_flower_esp32/
├── platformio.ini       # PlatformIO配置文件
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

### 依赖库

项目依赖已在 `platformio.ini` 中配置：
- FastLED ^3.5.0

PlatformIO会自动下载所需库，无需手动安装。

## 参数调优

所有可调参数都在 [`include/flower_config.h`](include/flower_config.h) 中定义：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `NOISE_SCALE` | 500 | 噪声变化速度，值越大变化越快 |
| `PROPAGATION_SPEED` | 2.0 | 波浪传播速度 |
| `WAVE_SPEED` | 3.0 | 波浪速度 |
| `WAVE_LENGTH` | 5.0 | 波浪空间周期 |
| `WAVE_MIX` | 0.3 | 波浪混合比例 (0-1) |
| `BREATH_PERIOD` | 4.0 | 呼吸周期（秒） |
| `BREATH_DEPTH` | 0.2 | 呼吸深度 (0-1) |
| `BRIGHTNESS_MIN` | 20 | 最小亮度 |
| `BRIGHTNESS_MAX` | 255 | 最大亮度 |

## 调试

串口波特率：115200

发送字符 `d` 可打印调试信息：
```
=== Debug Info ===
Time: 10.50s
Flower 0: brightness=128, distance=0.00
Flower 1: brightness=135, distance=3.60
...
================
```

## 算法说明

### 柏林噪声 (Perlin Noise)

柏林噪声是一种梯度噪声，产生连续、平滑的随机值。与纯随机数不同，柏林噪声的变化是渐进的，非常适合模拟自然现象。

FastLED 库提供 `inoise8()` 函数，输入3D坐标，返回 -128 到 127 的噪声值。

### 螺旋坐标计算

使用阿基米德螺旋公式：
```
θ(i) = i × 137.5°  (黄金角)
r(i) = b × θ(i)    (螺旋半径)
x(i) = r × cos(θ)
y(i) = r × sin(θ)
```

### 传播效果

外围花朵的亮度变化基于中心花的延迟版本：
```
delayedTime = currentTime - distance / propagationSpeed
```

### 综合亮度计算

```
最终亮度 = (噪声亮度 × 0.7 + 波浪亮度 × 0.3) × 呼吸因子
```

## 扩展建议

1. **更多花朵**：修改 `NUM_FLOWERS` 和 `LED_PINS` 数组
2. **RGB效果**：替换白光LED为WS2812B，使用FastLED的颜色功能
3. **音乐同步**：添加麦克风模块，将声音强度映射到亮度
4. **无线控制**：使用ESP32的WiFi/蓝牙功能，添加手机APP控制

## 许可证

本项目为开源项目，可自由使用和修改。

## 联系方式

如有问题或建议，欢迎反馈。
