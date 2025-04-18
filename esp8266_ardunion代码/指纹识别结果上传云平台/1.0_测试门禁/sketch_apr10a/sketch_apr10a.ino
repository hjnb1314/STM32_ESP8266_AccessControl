//1.0

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Man";
const char* password = "hjnb0404";
const char* mqttServer = "d82876d880.st1.iotda-device.cn-north-4.myhuaweicloud.com"; // 也可以使用 117.78.5.125
const int mqttPort = 1883;
const char* mqttClientId = "67f74c6f2902516e867003b8_66666666_0_0_2025041005";
const char* mqttUser = "67f74c6f2902516e867003b8_66666666";
const char* mqttPassword = "53d46fc2ee51d7e7e5b91ce3542b5886fd2b6b3bd3748cedd363f8fcda3bc668";

#define RELAY_PIN 0
#define deviceId "67f74c6f2902516e867003b8_66666666"

#define mqttTopic "$oc/devices/" deviceId "/sys/messages/down"
#define publishTopic "$oc/devices/" deviceId "/sys/properties/report"
#define subscribeTopic1 "$oc/devices/" deviceId "/sys/commands/#"
#define subscribeTopic2 "$oc/devices/" deviceId "/sys/commands/response/request_id=123456"
#define responseTopic "$oc/devices/" deviceId "/sys/commands/response/request_id=123456"

WiFiClient espClient;
PubSubClient client(espClient);

bool kValue = true;  // 用于存储 k 的值，初始值设为 true
unsigned long lastPublishTime = 0;  // 上次发布的时间
const unsigned long publishInterval = 5000;  // 发布间隔时间，单位为毫秒
bool receivedKValue = true;  // 新增全局变量用于存储接收到的 k 的属性值
bool lastReceivedKValue = true;  // 用于记录上一次接收到的 k 的值
unsigned long lastCheckTime = 0;  // 上次检查 k 属性值的时间

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    const char* stateKey = "state";
    if (doc.containsKey(stateKey)) {
        bool state = doc[stateKey];
        digitalWrite(RELAY_PIN, state);
        Serial.print("Switch state: ");
        Serial.println(state ? "ON" : "OFF");
    }

    // 检查是否包含 k 的属性
    const char* kKey = "k";
    if (doc.containsKey(kKey)) {
        lastReceivedKValue = receivedKValue;
        receivedKValue = doc[kKey];
        if (receivedKValue != lastReceivedKValue) {
            Serial.print("k value has changed to: ");
            Serial.println(receivedKValue ? "true" : "false");
            // 这里可以添加值变化时要执行的其他操作
        }
    }

    // 处理命令响应
    if (strcmp(topic, subscribeTopic2) == 0) {
        StaticJsonDocument<128> responseDoc;
        responseDoc["result"] = "success";
        responseDoc["message"] = "命令已成功执行";

        char responseBuffer[128];
        serializeJson(responseDoc, responseBuffer);

        if (client.publish(responseTopic, responseBuffer)) {
            Serial.println("Command response published successfully");
        } else {
            Serial.println("Failed to publish command response");
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
            Serial.println("connected");
            client.subscribe(mqttTopic);
            client.subscribe(subscribeTopic1);
            client.subscribe(subscribeTopic2);
        } else {
            Serial.print(client.state());
            delay(5000);
        }
    }
}

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

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void publishData() {
    StaticJsonDocument<256> doc;
    JsonArray services = doc.createNestedArray("services");
    JsonObject service = services.createNestedObject();
    service["service_id"] = "door";
    JsonObject properties = service.createNestedObject("properties");
    properties["k"] = kValue;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    if (client.publish(publishTopic, jsonBuffer)) {
        Serial.println("Data published successfully");
    } else {
        Serial.println("Failed to publish data");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    setup_wifi();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // 检查是否到了发布时间
    if (millis() - lastPublishTime >= publishInterval) {
        publishData();
        lastPublishTime = millis();
        kValue = !kValue;  // 切换 k 的值
    }

    // 检查是否到了检测 k 属性值的时间
    if (millis() - lastCheckTime >= publishInterval) {
        Serial.print("Current k value: ");
        Serial.println(receivedKValue ? "true" : "false");
        lastCheckTime = millis();
    }
}    