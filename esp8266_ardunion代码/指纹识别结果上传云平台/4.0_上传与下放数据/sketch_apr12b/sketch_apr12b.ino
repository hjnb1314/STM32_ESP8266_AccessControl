//4.0
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== 请根据实际情况确认下列信息 =====
const char* ssid           = "Man";
const char* password       = "hjnb0404";
const char* mqttServer     = "d82876d880.st1.iotda-device.cn-north-4.myhuaweicloud.com";  // 或使用实际 IP 地址
const int   mqttPort       = 1883;
const char* mqttClientId   = "67f74c6f2902516e867003b8_66666666_0_0_2025041005";
const char* mqttUser       = "67f74c6f2902516e867003b8_66666666";
const char* mqttPassword   = "53d46fc2ee51d7e7e5b91ce3542b5886fd2b6b3bd3748cedd363f8fcda3bc668";

#define deviceId "67f74c6f2902516e867003b8_66666666"

// 设备影子获取请求与响应主题前缀定义
#define SHADOW_GET_REQUEST_TOPIC_PREFIX "$oc/devices/" deviceId "/sys/shadow/get/request_id="
#define SHADOW_GET_RESPONSE_TOPIC_PREFIX "$oc/devices/" deviceId "/sys/shadow/get/response/request_id="

// 设备属性上报主题（用于 STM32 按键数据上传云平台时使用）
#define PROPERTIES_REPORT_TOPIC "$oc/devices/" deviceId "/sys/properties/report"
// =================================================

// 创建 WiFi 与 MQTT 客户端实例
WiFiClient espClient;
PubSubClient client(espClient);

// 全局变量，用于保存设备影子中 "k" 属性的状态（取自云平台数据或 STM32 下传数据）
bool kValue = false;

// 定时发送影子获取请求的变量与时间间隔（单位：毫秒）
unsigned long lastShadowRequestTime = 0;
const unsigned long shadowRequestInterval = 2000;  // 每 10 秒请求一次影子数据

// 用于控制周期性打印状态的变量（用于调试）
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 2000;  // 每 5 秒打印一次当前状态

// WiFi连接函数
void setup_wifi() {
  delay(10);
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  
  //Serial.println();
  //Serial.println("WiFi connected");
  //Serial.print("IP Address: ");
  //Serial.println(WiFi.localIP());
}

// MQTT消息回调函数，处理平台返回的设备影子响应消息
void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  
  // 判断是否为设备影子响应消息
  if (topicStr.startsWith(SHADOW_GET_RESPONSE_TOPIC_PREFIX)) {
    //Serial.print("Received device shadow response [");
    //Serial.print(topic);
    //Serial.print("]: ");
    for (unsigned int i = 0; i < length; i++) {
      //Serial.print((char)payload[i]);
    }
    //Serial.println();
    
    // 解析平台返回的 JSON 数据  
    // 示例响应格式如下：
    // {
    //   "shadow": [
    //      {
    //         "desired": {"properties": {},"event_time":"20250410T073200Z"},
    //         "reported": {"properties":{"k":true},"event_time":"20250412T125118Z"},
    //         "version":310,
    //         "service_id":"door"
    //      },
    //      {
    //         "desired": {"properties":null,"event_time":null},
    //         "reported": {"properties":{"k":65},"event_time":"20250410T111608Z"},
    //         "version":1,
    //         "service_id":"test"
    //      }
    //   ],
    //   "object_device_id": "67f74c6f2902516e867003b8_66666666"
    // }
    //
    // 我们在这里针对 service_id 为 "door" 的项提取 reported.properties.k
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      //Serial.print("Failed to parse device shadow JSON: ");
      //Serial.println(error.c_str());
      return;
    }
    
    // 检查是否存在 shadow 数组
    if (doc.containsKey("shadow")) {
      JsonArray shadowArray = doc["shadow"].as<JsonArray>();
      bool found = false;
      // 遍历数组，找 service_id=="door" 的项
      for (JsonVariant v : shadowArray) {
        if (v.containsKey("service_id")) {
          const char* serviceId = v["service_id"];
          if (strcmp(serviceId, "door") == 0) {
            // 找到 door 项，读取 reported 部分的 k 属性
            if (v.containsKey("reported")) {
              JsonObject reported = v["reported"].as<JsonObject>();
              if (reported.containsKey("properties")) {
                JsonObject properties = reported["properties"].as<JsonObject>();
                if (properties.containsKey("k")) {
                  // k属性可能为布尔型或者数字，依据实际情况判断（本示例以布尔型为主）
                  // 如果 k 为 boolean，直接读取；如果是数值，则判断非0为 true
                  JsonVariant kVar = properties["k"];
                  bool newK;
                  if (kVar.is<bool>()) {
                    newK = kVar.as<bool>();
                  } else if (kVar.is<int>()) {
                    newK = (kVar.as<int>() != 0);
                  } else {
                    newK = false;
                  }
                  //Serial.print("Extracted k value from shadow (service door): ");
                  //Serial.println(newK ? "true" : "false");
                  kValue = newK;
                  // 下发给 STM32，通过串口发送 '1' 或 '0'
                  //Serial.write(kValue ? '1' : '0');
                  //Serial.print(" Downlink to STM32: ");
                  //Serial.println(kValue ? "1" : "0");
                  found = true;
                  break;
                }
              }
            }
          }
        }
      }
      if (!found) {
        //Serial.println("No 'k' property found in shadow response for service 'door'.");
      }
    } else {
      //Serial.println("Shadow response JSON does not contain 'shadow' array.");
    }
    return;
  }
  
  // 若收到非影子响应的消息，可在此添加其它处理逻辑
  //Serial.print("Received unrelated MQTT message [");
  //Serial.print(topic);
  //Serial.println("]");
}

// MQTT重连函数，断线时重连并订阅设备影子响应主题
void reconnect() {
  while (!client.connected()) {
    //Serial.println("Attempting MQTT connection...");
    if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
      //Serial.println("MQTT connected.");
      // 订阅平台返回设备影子响应消息（使用单级通配符匹配 request_id 部分）
      String shadowResponseTopic = String(SHADOW_GET_RESPONSE_TOPIC_PREFIX) + "+";
      client.subscribe(shadowResponseTopic.c_str());
      //Serial.print("Subscribed to shadow response topic: ");
      //Serial.println(shadowResponseTopic);
    } else {
      //Serial.print("MQTT connection failed, state=");
      //Serial.println(client.state());
      delay(5000);
    }
  }
}

// 上传属性数据到云平台（上报 kValue，通常由 STM32下发数据后调用）
void publishData() {
  StaticJsonDocument<256> doc;
  JsonArray services = doc.createNestedArray("services");
  JsonObject service = services.createNestedObject();
  service["service_id"] = "door";
  JsonObject properties = service.createNestedObject("properties");
  properties["k"] = kValue;
  
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);
  
  if (client.publish(PROPERTIES_REPORT_TOPIC, jsonBuffer)) {
    //Serial.println("Data published successfully to cloud.");
  } else {
    //Serial.println("Failed to publish data to cloud.");
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  // 如需使用模拟读取，可配置 A0 引脚（ESP8266 默认 A0 已配置为模拟输入）
  // pinMode(A0, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // 检查并处理来自 STM32 的按键数据（通过串口接收）
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar == '0' || inChar == '1') {
      // 将 STM32 下传的数据更新到 kValue 中
      kValue = (inChar == '1');
      //Serial.print("Received from STM32: ");
      //Serial.print(kValue ? "true" : "false");
      //Serial.write(kValue ? '1' : '0');
      // 上报云平台
      publishData();
      delay(100);  // 避免连续触发
    }
  }
  
  // 定时向平台发布设备影子获取请求
  unsigned long now = millis();
  if (now - lastShadowRequestTime >= shadowRequestInterval) {
    // 使用当前 millis() 值作为 request_id（你也可改为其它唯一标识）
    String requestId = String(now);
    String requestTopic = String(SHADOW_GET_REQUEST_TOPIC_PREFIX) + requestId;
    
    // 根据平台要求，payload 为空字符串即可
    if (client.publish(requestTopic.c_str(), "")) {
      //Serial.print("Published shadow get request, topic: ");
      //Serial.println(requestTopic);
    } else {
      //Serial.print("Failed to publish shadow get request, topic: ");
      //Serial.println(requestTopic);
    }
    lastShadowRequestTime = now;
  }
  
  // 周期性打印当前 kValue 状态用于调试
  if (now - lastPrintTime >= printInterval) {
    //Serial.print("Current k value: ");
    //Serial.println(kValue ? "true" : "false");
    Serial.write(kValue ? '1' : '0');
    lastPrintTime = now;
  }
}
