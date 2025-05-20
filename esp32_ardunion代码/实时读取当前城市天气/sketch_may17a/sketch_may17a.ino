#include <WiFi.h>
#include <HTTPClient.h>

// WiFi配置
const char* ssid     = "Man";
const char* password = "hjnb0404";
const char* weather_url = "https://wttr.in/guangzhou?format=3"; // 也可换别的城市

void setup() {
  Serial.begin(115200); // 电脑调试
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // STM32接RX=16, TX=17
  delay(100);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(weather_url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      // payload 例: "guangzhou: ⛅️  +25°C"

      // 识别emoji与天气类型
      String weather_type = "Unknown";
      if (payload.indexOf("☀") != -1) weather_type = "Sunny";
      else if (payload.indexOf("☁") != -1) weather_type = "Cloudy";
      else if (payload.indexOf("⛅️") != -1) weather_type = "PartlyCloudy"; // 新增多云
      else if (payload.indexOf("🌦") != -1 || payload.indexOf("🌧") != -1) weather_type = "Rain";
      else if (payload.indexOf("❄") != -1 || payload.indexOf("🌨") != -1) weather_type = "Snow";
      else if (payload.indexOf("🌩") != -1) weather_type = "Thunder";
      else if (payload.indexOf("🌫") != -1 || payload.indexOf("🌁") != -1) weather_type = "Fog";
      else if (payload.indexOf("💨") != -1) weather_type = "Wind";

      // 不管识别与否，都强制提取温度数字
      int temp_pos = payload.indexOf("+");
      if (temp_pos == -1) temp_pos = payload.indexOf("-");
      String temp_str = "N/A";
      if (temp_pos != -1) {
        temp_str = payload.substring(temp_pos);
        temp_str.trim();
      }

      // 只输出: 天气类型,+25°C
      String msg = weather_type + "," + temp_str + "\r\n";
      Serial2.print(msg);
      Serial.print("Send to STM32: ");
      Serial.print(msg);

    } else {
      Serial.print("HTTP failed: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected!");
  }
  delay(600); // 1分钟刷新
}
