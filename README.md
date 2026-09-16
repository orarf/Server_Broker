# Mosquitto MQTT Broker บน Docker + ESP32 IoT Implementation

## 📋 ภาพรวมโครงการ

ชุดเอกสารนี้ประกอบด้วยคู่มือที่สมบูรณ์สำหรับการตั้งค่า **MQTT Broker ที่ปลอดภัย** บน Docker และเขียนโค้ด **Firmware สำหรับ ESP32** เพื่อเชื่อมต่อและส่งข้อมูลเซนเซอร์ไปยัง Broker

---

## 📁 ส่วนที่ 1: Mosquitto MQTT Broker บน Docker

### ขั้นตอนการติดตั้ง

#### 1️⃣ **เตรียมโครงสร้างโฟลเดอร์**
```
mosquitto/
├── mosquitto.conf
├── acl.conf
└── password.txt
```

#### 2️⃣ **สร้างไฟล์ตั้งค่า (mosquitto.conf)**
ไฟล์นี้ทำหน้าที่เป็น "สมองกลาง" ของ Broker ระบุว่า:
- เปิดพอร์ตไหน (default: 1883)
- ใช้ระบบความปลอดภัยแบบไหน
- เปิดใช้งาน ACL (Access Control List)
- ใช้ระบบการยืนยันตัวตน

#### 3️⃣ **สร้างไฟล์ Access Control (acl.conf)**
ไฟล์นี้คือ "สมุดคุมสิทธิ์" ที่ระบุชัดเจน:
- Device แต่ละตัว มีสิทธิ์ **Publish** (ส่ง) ข้อมูลไปยัง Topic ไหน
- Device แต่ละตัว มีสิทธิ์ **Subscribe** (อ่าน) ข้อมูลจาก Topic ไหน

#### 4️⃣ **รันคำสั่ง Docker**
```bash
docker run -d `
  --name mosquitto `
  -p 1883:1883 `
  -v /path/to/mosquitto:/mosquitto/config/mosquitto.conf `
  -v /path/to/acl.conf:/mosquitto/config/acl.conf `
  -v /path/to/password:/mosquitto/config/passwd `
  eclipse-mosquitto
```

#### 5️⃣ **สร้างไฟล์รหัสผ่านสำหรับผู้ใช้**
```bash
# สร้างผู้ใช้
docker exec -it mqtt-broker mosquitto_passwd /mosquitto/config/password.txt device_001

# เพิ่มผู้ใช้เพิ่มเติม
docker exec -it mqtt-broker mosquitto_passwd /mosquitto/config/password.txt admin_server
```

### ✅ ผลลัพธ์หลังการติดตั้ง
- Broker พร้อมรับการเชื่อมต่อ
- มีระบบการยืนยันตัวตน (Username/Password)
- มีการควบคุมการเข้าถึง (ACL) อย่างเข้มงวด

---

## 🔧 ส่วนที่ 2: ESP32 Firmware Implementation

### ข้อมูลไฟล์ **main.cpp**

#### 📌 วัตถุประสงค์
โปรแกรม ESP32 ที่เชื่อมต่อกับ MQTT Broker ผ่าน Wi-Fi และส่งข้อมูลอุณหภูมิแบบเรียลไทม์

#### 🔌 ส่วนประกอบหลัก

**1. ตั้งค่า Wi-Fi และ Broker**
```cpp
const char* ssid        = "Pooh";           // SSID ของ Wi-Fi
const char* password    = "12345678";       // รหัส Wi-Fi
const char* mqtt_server = "172.20.10.6";   // IP Broker (ต้องเป็นที่อยู่ IP จริง ไม่ใช่ localhost)
const int   mqtt_port   = 1883;            // พอร์ต MQTT
```

**2. ข้อมูลการยืนยันตัวตน Device**
```cpp
const char* mqtt_user   = "device_001";    // Username บน Broker
const char* mqtt_pass   = "1234";          // Password บน Broker
```

**3. การเชื่อมต่อและการส่งข้อมูล**

| ขั้นตอน | คำอธิบาย |
|--------|---------|
| `setup()` | เริ่มต้นการสื่อสาร Serial, เชื่อมต่อ Wi-Fi, ตั้งค่า MQTT Server |
| `reconnect()` | จัดการการเชื่อมต่อใหม่หากเชื่อมต่อตัดขาด |
| `loop()` | ส่งข้อมูลอุณหภูมิทุก 5 วินาที |

#### 📊 การทำงานของลูป (Loop Cycle)
```
1. ตรวจสอบการเชื่อมต่อ → ถ้าตัดขาดให้ reconnect
2. ค้างไว้ 5 วินาที
3. เพิ่มค่า tempValue ขึ้น 1
4. สร้าง JSON Payload: {"temperature": 26}
5. ส่งข้อมูลไปยัง Topic: devices/device_001/telemetry
6. กลับไปขั้นตอน 1
```

#### 📤 ข้อมูลที่ส่ง (Payload Format)
```json
{
  "temperature": 25
}
```

#### 🎯 Topic ปลายทาง
```
devices/device_001/telemetry
```

---

## 🧪 การทดสอบระบบ

### เครื่องมือที่ต้องใช้
- **MQTTX** (ดาวน์โหลด: https://mqttx.app/)

### ขั้นตอนการทดสอบ

**1. สร้าง Client 1 (Device 001)**
- Host: `localhost` หรือ `127.0.0.1`
- Port: `1883`
- Username: `device_001`
- Password: `[รหัสผ่านที่ตั้งไว้]`
- Action: **Subscribe** ไปยัง Topic `devices/device_001/telemetry`

**2. สร้าง Client 2 (Admin Server)**
- Host: `localhost`
- Port: `1883`
- Username: `admin_server`
- Password: `[รหัสผ่านที่ตั้งไว้]`
- Action: **Subscribe** ไปยัง Topic `devices/device_001/telemetry`

**3. ผลลัพธ์ที่คาดหวัง**
```
device_001 ส่งข้อมูล → Admin ทั้งสองฝ่ายจะเห็นข้อมูลเดียวกัน
ตัวอย่าง:
{"temperature": 26}
{"temperature": 27}
{"temperature": 28}
```

---

## 🔒 ฟีเจอร์ความปลอดภัย

| ฟีเจอร์ | รายละเอียด |
|-------|----------|
| **Authentication** | Username/Password สำหรับทุก Device |
| **ACL (Access Control List)** | ควบคุมสิทธิ์ Pub/Sub ของแต่ละ Device |
| **Secure Connection** | ใช้พอร์ต 1883 (ปกติ) หรือ 8883 (SSL/TLS) |
| **Device Identification** | Client ID ที่ไม่ซ้ำกันสำหรับแต่ละอุปกรณ์ |

---

## 📌 หมายเหตุสำคัญ

⚠️ **สำคัญมาก!**
- **ห้าม** ใช้ `localhost` เมื่อเชื่อมต่อจาก ESP32 → ต้องใช้ IP Address จริง (เช่น `172.20.10.6`)
- ตรวจสอบว่า Wi-Fi network ที่ ESP32 เชื่อมต่ออยู่ สามารถเข้าถึง Broker ได้
- กำหนดรหัสผ่านที่แข็งแกร่ง ไม่ใช่เพียงแค่ตัวเลข

---

## 📚 ไฟล์ที่อ้างอิง

1. **Mosquitto MQTT Broker Guide.pptx** → คู่มือการตั้งค่า Docker & Security
2. **main.cpp** → Source code สำหรับ ESP32

---

## 🚀 คำสั่งที่ใช้บ่อย

```bash
# ดู log ของ Mosquitto
docker logs mosquitto

# เข้าไป Container
docker exec -it mosquitto sh

# ทดสอบการเชื่อมต่อ
mosquitto_sub -h 172.20.10.6 -p 1883 -u device_001 -P 1234 -t "devices/device_001/telemetry"

# หยุด Container
docker stop mosquitto

# เริ่มใหม่
docker start mosquitto
```

---
## ไฟล์สรุปเพิ่มเติม

[📥 คลิกที่นี่เพื่อดาวน์โหลด Mosquitto MQTT Broker Guide](<https://github.com/orarf/Server_Broker/raw/refs/heads/main/Mosquitto_MQTT_Broker_Guide-Repaired.pptx>)
---

**Created:** 2024  
**Status:** Ready for Production ✅