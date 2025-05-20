#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- WiFi 和 MQTT 配置 ---
const char* ssid         = "Man";
const char* password     = "hjnb0404";
const char* mqttServer   = "d82876d880.st1.iotda-device.cn-north-4.myhuaweicloud.com";
const int   mqttPort     = 1883;
const char* mqttClientId = "67f74c6f2902516e867003b8_66666666_0_0_2025041005";
const char* mqttUser     = "67f74c6f2902516e867003b8_66666666";
const char* mqttPassword = "53d46fc2ee51d7e7e5b91ce3542b5886fd2b6b3bd3748cedd363f8fcda3bc668";
#define deviceId "67f74c6f2902516e867003b8_66666666"
#define RELAY_PIN 5  // D1 (NodeMCU开发板实际GPIO5)

#define SHADOW_GET_REQUEST_TOPIC_PREFIX "$oc/devices/" deviceId "/sys/shadow/get/request_id="
#define SHADOW_GET_RESPONSE_TOPIC_PREFIX "$oc/devices/" deviceId "/sys/shadow/get/response/request_id="
#define PROPERTIES_REPORT_TOPIC "$oc/devices/" deviceId "/sys/properties/report"

WiFiClient espClient;
PubSubClient client(espClient);

bool kValue = false;
unsigned long lastShadowRequestTime = 0;
const unsigned long shadowRequestInterval = 2000;
unsigned long lastPrintTime = 0;

// ---- WiFi 连接 ----
void setup_wifi() {
  delay(10);
  Serial.println("[WIFI] Connecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected!");
}

// ---- 控制继电器 ----
void setRelay(bool on) {
  // 低电平吸合，k=1吸合（开门），k=0断开（关门）
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  Serial.printf("[继电器] %s（k=%d，实际IO=%s）\n",
    on ? "吸合(开门)" : "断开(关门)",
    on ? 1 : 0,
    on ? "LOW" : "HIGH"
  );
}

// ---- 上报属性到云端 ----
void publishData() {
  StaticJsonDocument<256> doc;
  JsonArray services = doc.createNestedArray("services");
  JsonObject service = services.createNestedObject();
  service["service_id"] = "door";
  JsonObject properties = service.createNestedObject("properties");
  properties["k"] = kValue;
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);
  client.publish(PROPERTIES_REPORT_TOPIC, jsonBuffer);
  Serial.print("[MQTT] 上报 reported.k: ");
  Serial.println(kValue ? "true" : "false");
}

// ---- shadow 响应回调 ----
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[DEBUG] 收到MQTT消息：");
  Serial.print(topic);
  Serial.print(" ： ");
  for (unsigned int i = 0; i < length; ++i) Serial.write(payload[i]);
  Serial.println();

  String topicStr = String(topic);
  if (topicStr.indexOf("/shadow/get/response/") != -1) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.print("[MQTT] 解析错误: ");
      Serial.println(error.c_str());
      return;
    }
    if (doc.containsKey("shadow")) {
      JsonArray shadowArray = doc["shadow"].as<JsonArray>();
      for (JsonVariant v : shadowArray) {
        if (v["service_id"] && strcmp(v["service_id"], "door") == 0) {
          if (v.containsKey("reported") && v["reported"].containsKey("properties")) {
            JsonObject props = v["reported"]["properties"].as<JsonObject>();
            if (props.containsKey("k")) {
              bool repK = false;
              if (props["k"].is<bool>()) repK = props["k"].as<bool>();
              else if (props["k"].is<int>()) repK = props["k"].as<int>() != 0;
              kValue = repK;
              setRelay(kValue); // 强制同步
              Serial.printf("[MQTT] setRelay(k=%d)\n", kValue ? 1 : 0);
            }
          }
          break;
        }
      }
    }
  }
}

// ---- MQTT 连接/重连 ----
void reconnect() {
  while (!client.connected()) {
    Serial.print("[MQTT] 尝试连接...");
    if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
      String shadowResponseTopic = String(SHADOW_GET_RESPONSE_TOPIC_PREFIX) + "+";
      Serial.print("[MQTT] 订阅主题：");
      Serial.println(shadowResponseTopic);
      client.subscribe(shadowResponseTopic.c_str());
      Serial.println("[MQTT] 连接成功并已订阅 shadow 响应");
    } else {
      Serial.print("[MQTT] 连接失败, 状态: ");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// ---- 初始化 ----
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // 上电第一时间断开
  delay(300);
  kValue = false; 
  setRelay(kValue); // 再保证断开一次

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

// ---- 主循环 ----
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 定时主动拉取shadow
  unsigned long now = millis();
  if (now - lastShadowRequestTime >= shadowRequestInterval) {
    String requestId = String(now); // 保证唯一
    String requestTopic = String(SHADOW_GET_REQUEST_TOPIC_PREFIX) + requestId;
    client.publish(requestTopic.c_str(), "");
    lastShadowRequestTime = now;
    Serial.println("[MQTT] 主动请求设备影子...");
  }

  // 串口接收STM32本地指令（如有）
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar == '0' || inChar == '1') {
      bool localK = (inChar == '1');
      kValue = localK;
      setRelay(kValue);
      publishData();
      Serial.printf("[本地] 收到STM32指令, k=%d\n", localK ? 1 : 0);
    }
  }

  // 定时打印状态调试
  if (now - lastPrintTime >= 2000) {
    Serial.printf("[调试] 当前k=%d\n", kValue ? 1 : 0);
    lastPrintTime = now;
  }
}
