/**
 * @file flower_config.h
 * @brief 花海律动系统配置文件
 *
 * 包含所有可调参数和硬件配置
 *
 * 算法说明：
 * - 中心花亮度由UDP传输数据的大小控制
 * - 其余花朵按螺旋方式，跟随中心花的变化而变化（带延迟传播）
 */

#ifndef FLOWER_CONFIG_H
#define FLOWER_CONFIG_H

#include <Arduino.h>

// ============ 硬件配置 ============

// 花朵数量
#define NUM_FLOWERS 10

// 中心花索引（通常是第一朵）
#define CENTER_FLOWER 0

// LED引脚配置 (ESP32 GPIO)
// 注意：避免使用 GPIO 0, 2, 6-11 (内置Flash相关)
// 推荐使用: 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
const int LED_PINS[NUM_FLOWERS] = {
    5,   // 花0 - 中心花
    18,  // 花1
    19,  // 花2
    21,  // 花3
    22,  // 花4
    23,  // 花5
    25,  // 花6
    26,  // 花7
    27,  // 花8
    32   // 花9
};

// 帧率控制
#define FRAME_DELAY 20  // 毫秒 (50 FPS)

// ============ WiFi和UDP配置 ============

// WiFi配置（根据实际情况修改）
#define WIFI_SSID "Your_SSID"
#define WIFI_PASSWORD "Your_PASSWORD"

// UDP端口
#define UDP_PORT 4210

// UDP数据包大小（用于控制中心花亮度）
// 数据包大小范围: 0-1024 字节，映射到亮度 0-255
#define UDP_PACKET_SIZE_MAX 1024

// ============ 螺旋排列参数 ============

// 螺旋间距系数（控制花朵之间的距离）
#define SPIRAL_B 1.5f

// 黄金角（弧度）= 137.5°
// 这是自然界中常见的排列角度，如向日葵种子
#define GOLDEN_ANGLE 2.399f

// ============ 数据结构 ============

/**
 * @brief 花朵结构体
 */
struct Flower {
    int index;       // 花朵索引
    float x, y;      // 笛卡尔坐标
    float distance;  // 到中心距离
    float angle;     // 角度（弧度）
    uint8_t brightness; // 当前亮度
};

// ============ 亮度参数 ============

// 亮度范围
#define BRIGHTNESS_MIN 20   // 最小亮度（不完全熄灭）
#define BRIGHTNESS_MAX 255  // 最大亮度

// ============ 螺旋传播参数 ============

// 螺旋传播速度（距离单位/秒）
// 值越大，波浪传播越快
#define SPIRAL_PROPAGATION_SPEED 3.0f

// 螺旋传播延迟系数（每单位距离的延迟秒数）
#define SPIRAL_DELAY_FACTOR 0.15f

// ============ 平滑参数 ============

// 亮度平滑系数 (0.0 - 1.0)
// 值越大，响应越快；值越小，越平滑
#define BRIGHTNESS_SMOOTHING 0.3f

// ============ 呼吸效果参数 ============

// 呼吸周期（秒）
#define BREATH_PERIOD 3.0f

// 呼吸效果基础强度 (0.0 - 1.0)
// 这是UDP数据为0时的最小呼吸强度
#define BREATH_BASE_INTENSITY 0.15f

// 呼吸效果最大强度 (0.0 - 1.0)
// 这是UDP数据最大时的呼吸强度
#define BREATH_MAX_INTENSITY 0.8f

// 每朵花的相位偏移（弧度）
// 让每朵花有不同的呼吸节奏，形成波浪感
#define BREATH_PHASE_OFFSET 0.5f

// ============ 函数声明 ============

// 初始化花朵位置
void initFlowerPositions();

// 计算单朵花的亮度
uint8_t calculateFlowerBrightness(int index, float t);

// 更新中心花亮度（由UDP数据控制）
void updateCenterBrightness(uint8_t newBrightness);

// 获取中心花当前亮度
uint8_t getCenterBrightness();

// 调试信息输出
void printDebugInfo(float t);

#endif // FLOWER_CONFIG_H
