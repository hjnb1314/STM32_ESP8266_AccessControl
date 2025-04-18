#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== 请确认下列信息与你的实际情况一致 =====
const char* ssid = "Man";
const char* password = "hjnb0404";
const char* mqttServer = "d82876d880.st1.iotda-device.cn-north-4.myhuaweicloud.com";  // 或者使用IP地址
const int   mqttPort = 1883;
const char* mqttClientId = "67f74c6f2902516e867003b8_66666666_0_0_2025041005";
const char* mqttUser = "67f74c6f2902516e867003b8_66666666";
const char* mqttPassword = "53d46fc2ee51d7e7e5b91ce3542b5886fd2b6b3bd3748cedd363f8fcda3bc668";

#define deviceId "67f74c6f2902516e867003b8_66666666"

// 主题定义
// 上报属性使用的主题（未在本示例中使用，但可扩展使用）
#define PROPERTIES_REPORT_TOPIC "$oc/devices/" deviceId "/sys/properties/report"

// 设备属性获取请求及响应主题前缀
#define PROPERTIES_GET_TOPIC_PREFIX "$oc/devices/" deviceId "/sys/properties/get/request_id="
#define PROPERTIES_GET_RESPONSE_TOPIC_PREFIX "$oc/devices/" deviceId "/sys/properties/get/response/request_id="

// 设备命令相关主题（订阅平台下发的命令，包括通配所有命令）
#define COMMANDS_TOPIC "$oc/devices/" deviceId "/sys/commands/#"
// 命令响应发布主题（参考 AT 命令示例，此处发布空payload）
#define COMMANDS_RESPONSE_TOPIC "$oc/devices/" deviceId "/sys/commands/response/request_id=4c0fc52f-12b3-4ccd-ab53-818f9d3923be"

// =================================================

WiFiClient espClient;
PubSubClient client(espClient);

// 设备属性变量（例如：kValue，通常由 STM32下传，本示例直接在代码中维护）
bool kValue = false;

// 调试时周期性打印的时间控制变量
unsigned long lastCheckTime = 0;
const unsigned long publishInterval = 5000; // 每5秒打印一次状态

// 模拟传感器数据读取函数（示例：读取 A0 引脚模拟值）
int readSensorData() {
  int sensorValue = analogRead(A0);
  return sensorValue;
}

// MQTT消息回调函数（处理属性查询、命令下发等）
void callback(char* topic, byte* payload, unsigned int length) {
  // 打印收到的主题及消息内容
  Serial.print("MQTT Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicStr = String(topic);
  
  // ----------------------------
  // 处理属性查询请求
  // 主题格式：$oc/devices/{deviceId}/sys/properties/get/request_id=xxx
  String getPrefix = String(PROPERTIES_GET_TOPIC_PREFIX);
  if (topicStr.startsWith(getPrefix)) {
    // 提取 request_id（请求标识）
    String requestId = topicStr.substring(getPrefix.length());
    Serial.print("Property get request received, request_id: ");
    Serial.println(requestId);
    
    // 读取真实数据：示例中读取传感器数据
    int sensorValue = readSensorData();
    
    // 构造 JSON 响应，将 kValue 与传感器数据上报
    StaticJsonDocument<128> responseDoc;
    responseDoc["k"] = kValue;
    responseDoc["sensor"] = sensorValue;
    
    char jsonBuffer[128];
    serializeJson(responseDoc, jsonBuffer);
    
    // 拼接响应主题：$oc/devices/{deviceId}/sys/properties/get/response/request_id=xxx
    String responseTopic = String(PROPERTIES_GET_RESPONSE_TOPIC_PREFIX) + requestId;
    if (client.publish(responseTopic.c_str(), jsonBuffer)) {
      Serial.println("Published property get response successfully.");
    } else {
      Serial.println("Failed to publish property get response.");
    }
    return;
  }
  
  // ----------------------------
  // 处理设备命令（平台下发的命令）
  // 主题格式为：$oc/devices/{deviceId}/sys/commands/...
  if (topicStr.indexOf("/sys/commands/") > 0) {
    // 可选：如果平台下发的命令是 JSON 格式，可解析进一步处理
    StaticJsonDocument<128> cmdDoc;
    DeserializationError error = deserializeJson(cmdDoc, payload, length);
    if (!error) {
      if (cmdDoc.containsKey("cmd")) {
        Serial.print("Command received: ");
        Serial.println(cmdDoc["cmd"].as<const char*>());
      }
    } else {
      Serial.println("Command payload is not valid JSON.");
    }
    
    // 发送命令响应到平台（参考AT命令示例，payload为空）
    if (client.publish(COMMANDS_RESPONSE_TOPIC, "")) {
      Serial.println("Published command response successfully.");
    } else {
      Serial.println("Failed to publish command response.");
    }
    return;
  }
  
  // 其他消息可在此扩展
}

// 重连函数：断线重连及订阅相关主题
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
      Serial.println("MQTT connected.");
      
      // 订阅属性查询主题（使用单级通配符匹配 request_id）
      String propertiesGetTopic = String(PROPERTIES_GET_TOPIC_PREFIX) + "+";
      client.subscribe(propertiesGetTopic.c_str());
      Serial.print("Subscribed to properties get topic: ");
      Serial.println(propertiesGetTopic);
      
      // 订阅平台下发的所有命令消息
      client.subscribe(COMMANDS_TOPIC);
      Serial.print("Subscribed to commands topic: ");
      Serial.println(COMMANDS_TOPIC);
      
    } else {
      Serial.print("MQTT connection failed, state=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// WiFi连接函数
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  // 如果需要读取模拟数据，确保A0引脚配置正确
  pinMode(A0, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // 调试输出当前 kValue 状态，每 publishInterval 时间打印一次
  if (millis() - lastCheckTime >= publishInterval) {
    Serial.print("Current k value: ");
    Serial.println(kValue ? "true" : "false");
    lastCheckTime = millis();
  }
  
  // 此处可以添加其他代码，例如处理来自STM32的串口数据更新 kValue 的逻辑
}
