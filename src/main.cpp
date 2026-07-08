/**
 * @file main.cpp
 * @brief UDP控制的LED花海律动 - ESP32 PlatformIO项目
 *
 * 功能说明：
 * - 10朵白光LED花朵，螺旋排列
 * - 中心花亮度由UDP传输数据的大小控制
 * - 其余花朵按螺旋方式，跟随中心花的变化而变化（带延迟传播）
 *
 * 硬件要求：
 * - ESP32开发板
 * - 10个白光LED（带驱动电路）
 * - 连接引脚见 flower_config.h
 *
 * 网络要求：
 * - WiFi连接
 * - UDP数据包发送到指定端口
 * - 数据包大小 (0-1024字节) 映射到中心花亮度 (0-255)
 */

#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "flower_config.h"

// ============ 全局变量 ============
Flower flowers[NUM_FLOWERS];
unsigned long startTime;

// WiFi和UDP相关变量
WiFiUDP udp;
unsigned int udpPort = UDP_PORT;
char udpPacketBuffer[UDP_PACKET_SIZE_MAX + 1];
uint8_t centerBrightness = BRIGHTNESS_MIN;  // 中心花当前亮度
uint8_t targetCenterBrightness = BRIGHTNESS_MIN;  // 目标中心花亮度

// 亮度历史记录（用于螺旋传播）
#define BRIGHTNESS_HISTORY_SIZE 100
uint8_t brightnessHistory[BRIGHTNESS_HISTORY_SIZE];
int historyIndex = 0;
unsigned long lastHistoryUpdate = 0;

// ============ 初始化 ============
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("UDP Controlled Flower System Starting...");
    
    startTime = millis();
    
    // 初始化花朵位置
    initFlowerPositions();
    
    // 初始化LED引脚
    for (int i = 0; i < NUM_FLOWERS; i++) {
        pinMode(LED_PINS[i], OUTPUT);
        analogWrite(LED_PINS[i], 0);
    }
    
    // 初始化亮度历史
    for (int i = 0; i < BRIGHTNESS_HISTORY_SIZE; i++) {
        brightnessHistory[i] = BRIGHTNESS_MIN;
    }
    
    // 连接WiFi
    connectToWiFi();
    
    // 启动UDP监听
    udp.begin(udpPort);
    Serial.print("UDP listening on port: ");
    Serial.println(udpPort);
    
    Serial.println("System Ready!");
}

// ============ 连接WiFi ============
void connectToWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed! Running in offline mode.");
    }
}

// ============ 主循环 ============
void loop() {
    float t = (millis() - startTime) / 1000.0;
    
    // 处理UDP数据包
    handleUDP();
    
    // 平滑更新中心花亮度
    updateCenterBrightnessSmooth();
    
    // 更新亮度历史（用于螺旋传播）
    updateBrightnessHistory();
    
    // 更新每朵花的亮度
    for (int i = 0; i < NUM_FLOWERS; i++) {
        flowers[i].brightness = calculateFlowerBrightness(i, t);
        analogWrite(LED_PINS[i], flowers[i].brightness);
    }
    
    // 调试输出（可选）
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == 'd') {
            printDebugInfo(t);
        }
    }
    
    delay(FRAME_DELAY);  // 控制帧率
}

// ============ 处理UDP数据包 ============
void handleUDP() {
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        // 读取数据包
        int len = udp.read(udpPacketBuffer, UDP_PACKET_SIZE_MAX);
        if (len > 0) {
            udpPacketBuffer[len] = '\0';
            
            // 根据数据包大小计算中心花亮度
            // 数据包大小范围: 0-1024 字节 -> 亮度: BRIGHTNESS_MIN-BRIGHTNESS_MAX
            targetCenterBrightness = map(len, 0, UDP_PACKET_SIZE_MAX, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
            
            Serial.print("UDP packet received, size: ");
            Serial.print(len);
            Serial.print(" -> center brightness: ");
            Serial.println(targetCenterBrightness);
        }
    }
}

// ============ 平滑更新中心花亮度 ============
void updateCenterBrightnessSmooth() {
    // 使用指数平滑，让亮度变化更柔和
    centerBrightness = (uint8_t)(centerBrightness * (1.0f - BRIGHTNESS_SMOOTHING) +
                                  targetCenterBrightness * BRIGHTNESS_SMOOTHING);
}

// ============ 更新亮度历史 ============
void updateBrightnessHistory() {
    unsigned long now = millis();
    // 每10ms记录一次亮度历史
    if (now - lastHistoryUpdate >= 10) {
        brightnessHistory[historyIndex] = centerBrightness;
        historyIndex = (historyIndex + 1) % BRIGHTNESS_HISTORY_SIZE;
        lastHistoryUpdate = now;
    }
}

// ============ 初始化花朵螺旋位置 ============
void initFlowerPositions() {
    Serial.println("Initializing flower positions...");
    
    for (int i = 0; i < NUM_FLOWERS; i++) {
        // 黄金角螺旋排列
        float theta = i * GOLDEN_ANGLE;
        float r = SPIRAL_B * theta;
        
        flowers[i].index = i;
        flowers[i].angle = theta;
        flowers[i].distance = r;
        flowers[i].x = r * cos(theta);
        flowers[i].y = r * sin(theta);
        flowers[i].brightness = BRIGHTNESS_MIN;
        
        Serial.print("Flower ");
        Serial.print(i);
        Serial.print(": pos=(");
        Serial.print(flowers[i].x, 2);
        Serial.print(", ");
        Serial.print(flowers[i].y, 2);
        Serial.print("), dist=");
        Serial.println(flowers[i].distance, 2);
    }
}

// ============ 获取呼吸因子 ============
// 根据UDP数据（中心花亮度）计算呼吸强度
// UDP值越大，呼吸效果越弱；UDP值达到最大时，无呼吸效果
float getBreathFactor(int index, float t) {
    // 根据中心花亮度计算呼吸强度
    // 中心花亮度越高，呼吸效果越弱
    float brightnessRatio = (float)centerBrightness / (float)BRIGHTNESS_MAX;
    
    // 呼吸强度 = 最大强度 * (1 - 亮度比例)
    // 当亮度为0时，呼吸强度最大；当亮度最大时，呼吸强度为0
    float breathIntensity = BREATH_MAX_INTENSITY * (1.0f - brightnessRatio);
    
    // 计算呼吸相位（每朵花有不同的相位偏移）
    float phase = (t / BREATH_PERIOD) * TWO_PI + index * BREATH_PHASE_OFFSET;
    
    // 返回呼吸因子 (0.0 - 1.0)
    // sin输出范围 -1 到 1，转换为 0 到 1
    float breathValue = (sin(phase) + 1.0f) / 2.0f;
    
    // 根据呼吸强度调整呼吸幅度
    // 最终呼吸因子范围: (1 - breathIntensity) 到 1.0
    return 1.0f - breathIntensity + breathValue * breathIntensity;
}

// ============ 计算单朵花的亮度 ============
uint8_t calculateFlowerBrightness(int index, float t) {
    uint8_t baseBrightness;
    
    if (index == CENTER_FLOWER) {
        // 中心花：直接使用UDP控制的亮度
        baseBrightness = centerBrightness;
    } else {
        // 其他花：按螺旋方式跟随中心花变化（带延迟）
        // 距离越远，延迟越大
        
        float distance = flowers[index].distance;
        
        // 计算延迟（以历史记录的索引为单位）
        // 每单位距离对应 SPIRAL_DELAY_FACTOR 秒的延迟
        // 历史记录每10ms更新一次，所以1秒 = 100个索引
        int delayIndex = (int)(distance * SPIRAL_DELAY_FACTOR * 100);
        
        // 从历史记录中获取延迟后的亮度
        int delayedIndex = ((historyIndex - 1 - delayIndex) % BRIGHTNESS_HISTORY_SIZE + BRIGHTNESS_HISTORY_SIZE) % BRIGHTNESS_HISTORY_SIZE;
        
        // 获取延迟后的中心花亮度
        uint8_t delayedBrightness = brightnessHistory[delayedIndex];
        
        // 可以添加一些衰减，让远处的花亮度略低
        float attenuation = 1.0f - (distance / (NUM_FLOWERS * SPIRAL_B * GOLDEN_ANGLE)) * 0.3f;
        baseBrightness = (uint8_t)(delayedBrightness * attenuation);
    }
    
    // 应用呼吸效果
    // UDP值越大，呼吸效果越弱；UDP值达到最大时，呼吸效果消失
    float breathFactor = getBreathFactor(index, t);
    uint8_t brightness = (uint8_t)(baseBrightness * breathFactor);
    
    // 限制范围
    return constrain(brightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
}

// ============ 调试信息输出 ============
void printDebugInfo(float t) {
    Serial.println("\n=== Debug Info ===");
    Serial.print("Time: ");
    Serial.print(t, 2);
    Serial.println("s");
    Serial.print("Center brightness: ");
    Serial.println(centerBrightness);
    Serial.print("Target center brightness: ");
    Serial.println(targetCenterBrightness);
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    
    for (int i = 0; i < NUM_FLOWERS; i++) {
        Serial.print("Flower ");
        Serial.print(i);
        Serial.print(": brightness=");
        Serial.print(flowers[i].brightness);
        Serial.print(", distance=");
        Serial.println(flowers[i].distance, 2);
    }
    Serial.println("================\n");
}
