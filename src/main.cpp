#include <WiFi.h>
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi และ Broker
const char* ssid        = "Pooh";
const char* password    = "12345678";
const char* mqtt_server = "172.20.10.6"; // IP เครื่องที่รัน Docker (ห้ามใช้ localhost)
const int   mqtt_port   = 1883;

// 2. ข้อมูลการยืนยันตัวตนของ Device 001
const char* mqtt_user   = "device_001";
const char* mqtt_pass   = "1234";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
int tempValue = 25;

void setup() {
  Serial.begin(115200);
  
  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // กำหนด Server ให้กับ MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    // เชื่อมต่อโดยระบุ Client ID, Username และ Password
    if (client.connect("ESP32_Device_001", mqtt_user, mqtt_pass)) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ส่งข้อมูล Telemetry ทุกๆ 5 วินาที
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    tempValue++; // จำลองอุณหภูมิเพิ่มขึ้นทีละ 1

    // สร้างข้อความ JSON
    String payload = "{\"temperature\": " + String(tempValue) + "}";

    // ส่งข้อมูลไปที่ Topic ของตัวเอง
    client.publish("devices/device_001/telemetry", payload.c_str());
    Serial.println("Published: " + payload);
  }
}